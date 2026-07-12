#include <metal_stdlib>
#include "metal_gapless_shared.h"
using namespace metal;

// Gapless PSSM ungapped prefilter kernel for Metal.
//
// A 32-lane SIMD-group is split into 32/GROUP_SIZE sub-groups of GROUP_SIZE
// lanes, each processing one subject. The query is blocked across a sub-group's
// lanes, each holding NUM_REGS positions permuted into QUARTER=NUM_REGS/4 vec4
// groups (group m = positions m, m+Q, m+2Q, m+3Q); the diagonal shift is one
// cross-lane simd_shuffle_up per subject residue. Metal has no sub-SIMD-group
// shuffle, so every lane iterates to simd_max(subjectLength) and only relaxes
// while j < its own length; subjects sharing a SIMD-group are length-sorted, so
// the padding is small.
//
// Accumulator type (template parameter):
//   half  (FP16):  primary path; integer-exact only to 2048
//   short (int16): integer-exact to 32767; the exact fallback.
// gapless_tgh_g<G>_nr<N> is the FP16 primary, gapless_tg_g<G>_nr<N> the fallback.

constant constexpr int ALPHABET_SIZE = 21;
// FP16 is integer-exact on [-2048, 2048]; beyond that scores round.
constant constexpr float FP16_EXACT_LIMIT = 2048.0f;

// Templated threadgroup-cached kernel.
//   GROUP_SIZE in {8,16,32}; NUM_REGS a multiple of 4; covers queryLength up to
//   GROUP_SIZE*NUM_REGS. ScoreT in {half, short}; FLAG_OVERFLOW only for half.
template<uint GROUP_SIZE, uint NUM_REGS, class ScoreT, bool FLAG_OVERFLOW>
kernel void gapless_tg_kernel(
    device const char*     dbChars        [[buffer(0)]],
    device const uint64_t* dbOffsets      [[buffer(1)]],
    device const int32_t*  dbLengths      [[buffer(2)]],
    device const int8_t*   pssm           [[buffer(3)]],  // [21][queryLength]
    device float*          outScores      [[buffer(4)]],  // TopNMaximaArray d_scores
    device int32_t*        outIndices     [[buffer(5)]],  // TopNMaximaArray d_indices
    constant MetalGaplessParams& params   [[buffer(6)]],
    device atomic_uint*    overflowCount  [[buffer(7)]],  // FP16 overflow tally
    uint simdgroup_id        [[simdgroup_index_in_threadgroup]],
    uint thread_in_simdgroup [[thread_index_in_simdgroup]],
    uint threadgroup_id      [[threadgroup_position_in_grid]],
    uint simdgroups_per_tg   [[simdgroups_per_threadgroup]],
    uint thread_in_tg        [[thread_position_in_threadgroup]],
    uint threads_per_tg      [[threads_per_threadgroup]]
) {
    using Vec = vec<ScoreT, 4>;
    constexpr uint QUARTER      = NUM_REGS / 4;
    constexpr uint PADDED_COLS  = GROUP_SIZE * NUM_REGS;   // zero-padded query cols
    constexpr uint SUBJECTS_PER_SG = 32u / GROUP_SIZE;
    const uint queryLength = params.queryLength;

    threadgroup int8_t tg_pssm[ALPHABET_SIZE * PADDED_COLS];
    for (uint i = thread_in_tg; i < ALPHABET_SIZE * PADDED_COLS; i += threads_per_tg) {
        uint letter = i / PADDED_COLS;
        uint col    = i % PADDED_COLS;
        tg_pssm[i] = (col < queryLength) ? pssm[letter * queryLength + col] : (int8_t)0;
    }
    threadgroup_barrier(mem_flags::mem_threadgroup);

    const uint lane      = thread_in_simdgroup;
    const uint groupLane = lane % GROUP_SIZE;
    const uint groupInSg = lane / GROUP_SIZE;
    const uint globalSg  = threadgroup_id * simdgroups_per_tg + simdgroup_id;
    const uint seqIdx    = globalSg * SUBJECTS_PER_SG + groupInSg;

    const bool active = (seqIdx < params.numSequences);
    const int32_t subjectLength = active ? dbLengths[seqIdx] : 0;
    const uint64_t baseOffset   = active ? dbOffsets[seqIdx] : 0;
    const uint myStart = groupLane * NUM_REGS;

    const int32_t maxLen = simd_max(subjectLength);

    Vec penalty_here[QUARTER];
    #pragma unroll
    for (uint r = 0; r < QUARTER; r++) penalty_here[r] = Vec(0);
    ScoreT prev_border = 0;
    Vec maximum4 = Vec(0);

    for (int32_t j = 0; j < maxLen; j++) {
        if (j < subjectLength) {
            int subjectLetter = min((int)(uchar)dbChars[baseOffset + j], 20);
            threadgroup const int8_t* pssmRow = tg_pssm + subjectLetter * PADDED_COLS;

            Vec old_last = penalty_here[QUARTER - 1];
            Vec old_prev = penalty_here[0];
            uint p0 = myStart;
            Vec scores0 = Vec(pssmRow[p0], pssmRow[p0 + QUARTER],
                              pssmRow[p0 + 2*QUARTER], pssmRow[p0 + 3*QUARTER]);
            Vec diag0 = Vec(prev_border, old_last.x, old_last.y, old_last.z);
            Vec new0 = max(diag0 + scores0, Vec(0));
            penalty_here[0] = new0;
            maximum4 = max(maximum4, new0);

            #pragma unroll
            for (uint m = 1; m < QUARTER; m++) {
                Vec old_cur = penalty_here[m];
                uint pm = myStart + m;
                Vec scores = Vec(pssmRow[pm], pssmRow[pm + QUARTER],
                                 pssmRow[pm + 2*QUARTER], pssmRow[pm + 3*QUARTER]);
                Vec new_val = max(old_prev + scores, Vec(0));
                penalty_here[m] = new_val;
                maximum4 = max(maximum4, new_val);
                old_prev = old_cur;
            }
        }

        ScoreT shuffled = simd_shuffle_up(penalty_here[QUARTER - 1].w, 1);
        prev_border = (groupLane == 0) ? (ScoreT)0 : shuffled;
    }

    ScoreT groupMax = max(max(maximum4.x, maximum4.y), max(maximum4.z, maximum4.w));
    #pragma unroll
    for (uint off = GROUP_SIZE / 2; off >= 1; off >>= 1) {
        groupMax = max(groupMax, simd_shuffle_xor(groupMax, off));
    }

    if (groupLane == 0 && active) {
        outScores[seqIdx]  = (float)groupMax;
        outIndices[seqIdx] = (int32_t)(params.indexOffset + seqIdx);
        if (FLAG_OVERFLOW && (float)groupMax >= FP16_EXACT_LIMIT) {
            atomic_fetch_add_explicit(overflowCount, 1u, memory_order_relaxed);
        }
    }
}

// Named instantiations:
// * gapless_tgh_g<G>_nr<N> (FP16 primary, overflow-flagged)
// * gapless_tg_g<G>_nr<N> (int16 exact fallback)
#define INSTANTIATE_TG(G, N) \
template [[host_name("gapless_tgh_g" #G "_nr" #N)]] kernel void \
    gapless_tg_kernel<G, N, half, true>( \
    device const char*, device const uint64_t*, device const int32_t*, \
    device const int8_t*, device float*, device int32_t*, \
    constant MetalGaplessParams&, device atomic_uint*, uint,uint,uint,uint,uint,uint); \
template [[host_name("gapless_tg_g" #G "_nr" #N)]] kernel void \
    gapless_tg_kernel<G, N, short, false>( \
    device const char*, device const uint64_t*, device const int32_t*, \
    device const int8_t*, device float*, device int32_t*, \
    constant MetalGaplessParams&, device atomic_uint*, uint,uint,uint,uint,uint,uint);

#define TG_4_48(G) \
    INSTANTIATE_TG(G,4)  INSTANTIATE_TG(G,8)  INSTANTIATE_TG(G,12) INSTANTIATE_TG(G,16) \
    INSTANTIATE_TG(G,20) INSTANTIATE_TG(G,24) INSTANTIATE_TG(G,28) INSTANTIATE_TG(G,32) \
    INSTANTIATE_TG(G,36) INSTANTIATE_TG(G,40) INSTANTIATE_TG(G,44) INSTANTIATE_TG(G,48)
#define TG_52_64(G) \
    INSTANTIATE_TG(G,52) INSTANTIATE_TG(G,56) INSTANTIATE_TG(G,60) INSTANTIATE_TG(G,64)
#define TG_68_96(G) \
    INSTANTIATE_TG(G,68) INSTANTIATE_TG(G,72) INSTANTIATE_TG(G,76) INSTANTIATE_TG(G,80) \
    INSTANTIATE_TG(G,84) INSTANTIATE_TG(G,88) INSTANTIATE_TG(G,92) INSTANTIATE_TG(G,96)

TG_4_48(32)
TG_4_48(16) TG_52_64(16) TG_68_96(16)
TG_4_48(8)  TG_52_64(8)
#undef TG_4_48
#undef TG_52_64
#undef TG_68_96
#undef INSTANTIATE_TG

// Fallback for queries too long (>1536aa() for the threadgroup PSSM cache
// One subject per 32-lane SIMD-group, PSSM in device memory, int16 exact.
kernel void gapless_filter_kernel(
    device const char*     dbChars        [[buffer(0)]],
    device const uint64_t* dbOffsets      [[buffer(1)]],
    device const int32_t*  dbLengths      [[buffer(2)]],
    device const int8_t*   pssm           [[buffer(3)]],
    device float*          outScores      [[buffer(4)]],
    device int32_t*        outIndices     [[buffer(5)]],
    constant MetalGaplessParams& params   [[buffer(6)]],
    uint simdgroup_id        [[simdgroup_index_in_threadgroup]],
    uint thread_in_simdgroup [[thread_index_in_simdgroup]],
    uint threadgroup_id      [[threadgroup_position_in_grid]],
    uint simdgroups_per_tg   [[simdgroups_per_threadgroup]]
) {
    const uint seqIdx = threadgroup_id * simdgroups_per_tg + simdgroup_id;
    if (seqIdx >= params.numSequences) return;

    const uint queryLength = params.queryLength;
    const uint lane = thread_in_simdgroup;
    const int32_t subjectLength = dbLengths[seqIdx];
    const uint64_t baseOffset = dbOffsets[seqIdx];

    const uint numRegsPerThread = ((queryLength + 31) / 32 + 3) & ~3u;
    const uint quarter = numRegsPerThread / 4;
    const uint myStart = lane * numRegsPerThread;

    short4 penalty_here[32];
    short  prev_border = 0;
    short4 maximum4 = short4(0);

    for (uint r = 0; r < quarter && r < 32; r++) penalty_here[r] = short4(0);

    for (int32_t j = 0; j < subjectLength; j++) {
        int subjectLetter = min((int)(uchar)dbChars[baseOffset + j], 20);
        device const int8_t* pssmRow = pssm + subjectLetter * queryLength;

        short4 old_last = penalty_here[min(quarter - 1, (uint)31)];
        short4 old_prev = penalty_here[0];
        uint p0 = myStart;
        short4 scores0;
        scores0.x = (p0             < queryLength) ? (short)pssmRow[p0]             : (short)0;
        scores0.y = (p0 + quarter   < queryLength) ? (short)pssmRow[p0 + quarter]   : (short)0;
        scores0.z = (p0 + 2*quarter < queryLength) ? (short)pssmRow[p0 + 2*quarter] : (short)0;
        scores0.w = (p0 + 3*quarter < queryLength) ? (short)pssmRow[p0 + 3*quarter] : (short)0;

        short4 diag0 = short4(prev_border, old_last.x, old_last.y, old_last.z);
        short4 new0 = max(diag0 + scores0, short4(0));
        penalty_here[0] = new0;
        maximum4 = max(maximum4, new0);

        for (uint m = 1; m < quarter && m < 32; m++) {
            short4 old_cur = penalty_here[m];
            uint pm = myStart + m;
            short4 scores;
            scores.x = (pm             < queryLength) ? (short)pssmRow[pm]             : (short)0;
            scores.y = (pm + quarter   < queryLength) ? (short)pssmRow[pm + quarter]   : (short)0;
            scores.z = (pm + 2*quarter < queryLength) ? (short)pssmRow[pm + 2*quarter] : (short)0;
            scores.w = (pm + 3*quarter < queryLength) ? (short)pssmRow[pm + 3*quarter] : (short)0;

            short4 new_val = max(old_prev + scores, short4(0));
            penalty_here[m] = new_val;
            maximum4 = max(maximum4, new_val);
            old_prev = old_cur;
        }

        short shuffled = simd_shuffle_up(penalty_here[min(quarter - 1, (uint)31)].w, 1);
        prev_border = (lane == 0) ? (short)0 : shuffled;
    }

    short groupMax = max(max(maximum4.x, maximum4.y), max(maximum4.z, maximum4.w));
    groupMax = simd_max(groupMax);

    if (lane == 0) {
        outScores[seqIdx]  = (float)groupMax;
        outIndices[seqIdx] = (int32_t)(params.indexOffset + seqIdx);
    }
}
