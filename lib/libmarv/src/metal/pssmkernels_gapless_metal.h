#ifndef PSSMKERNELS_GAPLESS_METAL_H
#define PSSMKERNELS_GAPLESS_METAL_H

// Metal gapless dispatch
// The kernel wants the PSSM row-major as int8[21][queryLen]


#include "cuda_metal_launch.h"
#include "metal_gapless_shared.h"
#include "pssm.cuh"
#include "util.cuh"
#include "config.hpp"

#include <cstdint>
#include <cstdio>

namespace cudasw4 {
namespace metalgapless {

// OutputScores is cudasw4's TopNMaximaArray; the kernel writes float scores and
// ids straight into its backing, so there is no dense readback. Completion is
// ensured when cudasw4 next touches `stream` through the thrust policy, which
// drains the GPU.
template<class OutputScores>
void call_GaplessFilter_metal(
    OutputScores& d_scores,
    GpuPermutedPSSMforGapless& permutedPSSM,
    const char* d_inputChars,
    const SequenceLengthT* d_inputLengths,
    const size_t* d_inputOffsets,
    PositionsIterator d_selectedPositions,
    size_t numSequences,
    SequenceLengthT queryLength,
    cudaStream_t stream)
{
    if (numSequences == 0) return;

    // First selected subject index in this pass; the selection is contiguous.
    const size_t base = static_cast<size_t>(d_selectedPositions[0]);

    // Pointer-offset binding: whole char buffer at offset 0 (aligned), the
    // offsets/lengths arrays sliced to +base.
    const char*             charBase    = d_inputChars;              // offset 0
    const size_t*           offsetsBase = d_inputOffsets + base;     // byte off = base*8
    const SequenceLengthT*  lengthsBase = d_inputLengths + base;     // byte off = base*4

    const int8_t* pssm = permutedPSSM.metalPssmData();

    // Kernel writes straight into the TopNMaximaArray backing arrays.
    float*   outScores  = d_scores.d_scores;
    int32_t* outIndices = d_scores.d_indices;

    // NUM_REGS = per-lane register count = ceil(queryLength/GROUP_SIZE) rounded up
    // to a multiple of 4. Prefer the smallest GROUP_SIZE (most subjects per
    // SIMD-group), bumping up when NUM_REGS exceeds the instantiated set
    // (g8 covers queryLength <= 512, g16 <= 1536); longer queries fall back.
    auto maxNRfor = [](unsigned int g) -> unsigned int {
        return g == 8 ? 64u : g == 16 ? 96u : 48u; // must match instantiations
    };
    unsigned int groupSize = 8;
    unsigned int numRegs = 0;
    for (;;) {
        numRegs = (((static_cast<unsigned int>(queryLength) + groupSize - 1u) / groupSize) + 3u) & ~3u;
        if (numRegs <= maxNRfor(groupSize)) break;
        // fall through to fallback
        if (groupSize >= 32u) break;
        groupSize <<= 1;
    }
    const bool useTiled = (queryLength <= 1536u && numRegs <= maxNRfor(groupSize));
    const unsigned int subjectsPerSg = useTiled ? (32u / groupSize) : 1u;

    MetalGaplessParams params;
    params.numSequences = static_cast<uint32_t>(numSequences);
    params.queryLength  = static_cast<uint32_t>(queryLength);
    params.indexOffset  = static_cast<uint32_t>(d_scores.indexOffset);
    params.pad          = 0;

    uint32_t* overflow = permutedPSSM.metalOverflowBuffer();

    MetalKernelArg args[8];
    args[0] = MetalKernelArg::buffer(charBase);
    args[1] = MetalKernelArg::buffer(offsetsBase);
    args[2] = MetalKernelArg::buffer(lengthsBase);
    args[3] = MetalKernelArg::buffer(pssm);
    args[4] = MetalKernelArg::buffer(outScores);
    args[5] = MetalKernelArg::buffer(outIndices);
    args[6] = MetalKernelArg::bytes(params);
    args[7] = MetalKernelArg::buffer(overflow);   // used only by the FP16 (tgh) kernels

    // Pack as many simdgroups per threadgroup as the pipeline allow
    auto launch = [&](const char* name, int nargs) {
        unsigned int sgPerTg = 8;
        const size_t maxThreads = metalKernelMaxThreads(name);
        if (maxThreads > 0) {
            unsigned int maxSg = static_cast<unsigned int>(maxThreads / 32);
            if (maxSg == 0) maxSg = 1;
            if (maxSg > 32) maxSg = 32;
            sgPerTg = maxSg;
        }
        const unsigned int tpg = sgPerTg * 32u;
        const unsigned int subjPerTg = sgPerTg * subjectsPerSg;
        const unsigned int grid = static_cast<unsigned int>((numSequences + subjPerTg - 1) / subjPerTg);
        metalLaunchKernel(name, args, nargs, grid, tpg, stream);
    };

    if (!useTiled) {
        // long query: exact int16, one subject per SIMD-group, single pass.
        launch("gapless_filter_kernel", 7);
        return;
    }

    // FP16 primary (fast), exact int16 rescore only for passes that overflow (score >= 2048).
    char primary[48], fallback[48];
    std::snprintf(primary,  sizeof(primary),  "gapless_tgh_g%u_nr%u", groupSize, numRegs);
    std::snprintf(fallback, sizeof(fallback), "gapless_tg_g%u_nr%u",  groupSize, numRegs);

    launch(primary, 8);
    cudaDeviceSynchronize();
    const uint32_t now = *overflow;
    if (now != permutedPSSM.metalOverflowSeen) {
        permutedPSSM.metalOverflowSeen = now;
        // int16 exact, overwrites scores
        launch(fallback, 8);
        cudaDeviceSynchronize();
    }
}

} // namespace metalgapless
} // namespace cudasw4

#endif // PSSMKERNELS_GAPLESS_METAL_H
