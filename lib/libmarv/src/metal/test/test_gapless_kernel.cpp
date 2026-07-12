// Compare metal gapless kernels vs CPU gapless DP reference.
// Sweeps GROUP_SIZE and query length across the FP16 and int16
// families, the long-query fallback, and the FP16-overflow -> int16-rescore
// path. The reference is unsaturated int, so scores must match exactly.

#include "../cuda_metal_launch.h"
#include "../cuda_metal_runtime.h"
#include "../metal_gapless_shared.h"

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <vector>
#include <string>
#include <random>
#include <algorithm>

namespace {

constexpr int ALPHABET = 21; // 20 aa + X

// CPU gapless DP (unsaturated), matching the kernel recurrence
//   cell(i,j) = max(0, cell(i-1,j-1) + pssm[letter(j)][i]); track max.
int cpuGaplessScore(const uint8_t* subject, int subjectLen,
                    const int8_t* pssm, int queryLen) {
    std::vector<int> prev(queryLen, 0), cur(queryLen, 0);
    int best = 0;
    for (int j = 0; j < subjectLen; ++j) {
        int letter = subject[j];
        if (letter >= 20) letter = 20; // clamp >=20 -> X
        const int8_t* row = pssm + letter * queryLen;
        for (int i = 0; i < queryLen; ++i) {
            int diag = (i == 0) ? 0 : prev[i - 1];
            int v = diag + row[i];
            cur[i] = v > 0 ? v : 0;
            if (cur[i] > best) best = cur[i];
        }
        std::swap(prev, cur);
    }
    return best;
}

std::string kernelForQueryLen(uint32_t q, unsigned int groupSize, bool fp16) {
    auto maxNRfor = [](unsigned int g){ return g == 8 ? 64u : g == 16 ? 96u : 48u; };
    unsigned int nr = 0;
    for (;;) {
        nr = (((q + groupSize - 1u) / groupSize) + 3u) & ~3u;
        if (nr <= maxNRfor(groupSize)) break;
        if (groupSize >= 32u) break;
        groupSize <<= 1;
    }
    if (q <= 1536u && nr <= maxNRfor(groupSize))
        return std::string(fp16 ? "gapless_tgh_g" : "gapless_tg_g")
               + std::to_string(groupSize) + "_nr" + std::to_string(nr);
    return "gapless_filter_kernel";
}

bool runCase(int queryLen, int numSeqs, int maxSubjLen, uint32_t seed, unsigned int groupSize) {
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> lenDist(1, maxSubjLen);
     // include soft-masked (>=20) and code+32? keep 0..25
    std::uniform_int_distribution<int> aaDist(0, 25);
    std::uniform_int_distribution<int> pssmDist(-8, 8);

    // PSSM int8[ALPHABET][queryLen]
    std::vector<int8_t> pssm(ALPHABET * queryLen);
    for (auto& v : pssm) v = (int8_t)pssmDist(rng);

    // Subjects, contiguous with absolute offsets.
    std::vector<uint8_t> chars;
    std::vector<uint64_t> offsets(numSeqs);
    std::vector<int32_t> lengths(numSeqs);
    for (int s = 0; s < numSeqs; ++s) {
        offsets[s] = chars.size();
        int L = lenDist(rng);
        lengths[s] = L;
        for (int j = 0; j < L; ++j) chars.push_back((uint8_t)aaDist(rng));
    }
    if (chars.empty()) chars.push_back(0);

    // CPU reference
    std::vector<int> ref(numSeqs);
    for (int s = 0; s < numSeqs; ++s)
        ref[s] = cpuGaplessScore(chars.data() + offsets[s], lengths[s], pssm.data(), queryLen);

    // Facade device buffers (real MTLBuffers, host-addressable)
    char*     d_chars   = nullptr;
    uint64_t* d_offsets = nullptr;
    int32_t*  d_lengths = nullptr;
    int8_t*   d_pssm    = nullptr;
    float*    d_scores  = nullptr;
    int32_t*  d_indices = nullptr;
    uint32_t* d_overflow = nullptr;
    cudaMalloc((void**)&d_chars,   chars.size());
    cudaMalloc((void**)&d_offsets, offsets.size() * sizeof(uint64_t));
    cudaMalloc((void**)&d_lengths, lengths.size() * sizeof(int32_t));
    cudaMalloc((void**)&d_pssm,    pssm.size() * sizeof(int8_t));
    cudaMalloc((void**)&d_scores,  numSeqs * sizeof(float));
    cudaMalloc((void**)&d_indices, numSeqs * sizeof(int32_t));
    cudaMalloc((void**)&d_overflow, sizeof(uint32_t));

    memcpy(d_chars,   chars.data(),   chars.size());
    memcpy(d_offsets, offsets.data(), offsets.size() * sizeof(uint64_t));
    memcpy(d_lengths, lengths.data(), lengths.size() * sizeof(int32_t));
    memcpy(d_pssm,    pssm.data(),    pssm.size() * sizeof(int8_t));

    MetalGaplessParams params{(uint32_t)numSeqs, (uint32_t)queryLen, 0u, 0u};

    // Match the host wrapper's selection (may bump groupSize / fall back).
    auto maxNRfor = [](unsigned int gg){ return gg == 8 ? 64u : gg == 16 ? 96u : 48u; };
    unsigned int g = groupSize, nr = 0;
    for (;;) { nr = ((( (uint32_t)queryLen + g - 1u)/g) + 3u) & ~3u; if (nr <= maxNRfor(g)) break; if (g >= 32u) break; g <<= 1; }
    const bool fallback = ((uint32_t)queryLen > 1536u || nr > maxNRfor(g));
    const unsigned int subjectsPerSg = fallback ? 1u : 32u / g;

    MetalKernelArg args[8] = {
        MetalKernelArg::buffer(d_chars),
        MetalKernelArg::buffer(d_offsets),
        MetalKernelArg::buffer(d_lengths),
        MetalKernelArg::buffer(d_pssm),
        MetalKernelArg::buffer(d_scores),
        MetalKernelArg::buffer(d_indices),
        MetalKernelArg::bytes(params),
        MetalKernelArg::buffer(d_overflow),
    };

    // Runs one kernel and returns the mismatch count vs the exact CPU reference.
    auto runAndCheck = [&](const std::string& name, int nargs) -> int {
        unsigned int sgPerTg = 8;
        size_t maxThreads = metalKernelMaxThreads(name.c_str());
        if (maxThreads > 0) { unsigned int m = (unsigned int)(maxThreads/32); if (m==0) m=1; if (m>32) m=32; sgPerTg=m; }
        unsigned int threadsPerTg = sgPerTg * 32u;
        unsigned int subjPerTg = sgPerTg * subjectsPerSg;
        unsigned int grid = (unsigned int)((numSeqs + subjPerTg - 1) / subjPerTg);
        memset(d_scores, 0, numSeqs * sizeof(float));
        *d_overflow = 0;
        cudaError_t err = metalLaunchKernel(name.c_str(), args, nargs, grid, threadsPerTg, nullptr);
        cudaDeviceSynchronize();
        if (err != cudaSuccess) { printf("  launch failed for %s: %s\n", name.c_str(), cudaGetErrorString(err)); return numSeqs; }
        int mm = 0;
        for (int s = 0; s < numSeqs; ++s) {
            if ((int)d_scores[s] != ref[s] || d_indices[s] != s) {
                if (mm < 3) printf("  [%s q=%d] seq %d: metal=%d cpu=%d id=%d\n",
                                   name.c_str(), queryLen, s, (int)d_scores[s], ref[s], d_indices[s]);
                ++mm;
            }
        }
        return mm;
    };

    int mismatches = 0;
    std::string label;
    if (fallback) {
        label = kernelForQueryLen((uint32_t)queryLen, groupSize, false);   // gapless_filter_kernel
        mismatches += runAndCheck(label, 7);
    } else {
        // Exercise both the int16 (exact) and FP16 kernels; small test scores
        // stay under 2048 so FP16 must also match exactly.
        std::string i16 = kernelForQueryLen((uint32_t)queryLen, groupSize, false);
        std::string f16 = kernelForQueryLen((uint32_t)queryLen, groupSize, true);
        mismatches += runAndCheck(i16, 8);
        mismatches += runAndCheck(f16, 8);
        label = f16 + " + int16";
    }

    cudaFree(d_chars); cudaFree(d_offsets); cudaFree(d_lengths);
    cudaFree(d_pssm);  cudaFree(d_scores); cudaFree(d_indices); cudaFree(d_overflow);

    bool ok = (mismatches == 0);
    const char* kernel = label.c_str();
    printf("case q=%-4d n=%-4d maxL=%-4d kernel=%-34s : %s\n",
           queryLen, numSeqs, maxSubjLen, kernel, ok ? "PASS" : "FAIL");
    return ok;
}

// Forces scores >= 2048 to verify the two-tier path: the FP16 kernel must set
// the overflow flag, and the int16 rescore must be exact.
bool overflowCase() {
    const int Q = 700, N = 8, L = 700;           // g16_nr44; self-score = 700*3 = 2100 (> 2048)
    std::vector<int8_t> pssm(ALPHABET * Q, 0);
    for (int c = 0; c < Q; ++c) pssm[0 * Q + c] = 3;  // residue 0 scores +3/position

    std::vector<uint8_t> chars;
    std::vector<uint64_t> offsets(N);
    std::vector<int32_t> lengths(N);
    for (int s = 0; s < N; ++s) {
        offsets[s] = chars.size();
        lengths[s] = L;
        for (int j = 0; j < L; ++j) chars.push_back(0);  // all residue 0
    }
    const int expected = L * 3;                  // = 2100
    std::string i16 = kernelForQueryLen(Q, 8, false);
    std::string f16 = kernelForQueryLen(Q, 8, true);
    unsigned int subjectsPerSg = 2;              // g16

    char*     d_chars=nullptr;  uint64_t* d_offsets=nullptr; int32_t* d_lengths=nullptr;
    int8_t*   d_pssm=nullptr;   float* d_scores=nullptr;     int32_t* d_indices=nullptr;
    uint32_t* d_overflow=nullptr;
    cudaMalloc((void**)&d_chars, chars.size());  cudaMalloc((void**)&d_offsets, N*sizeof(uint64_t));
    cudaMalloc((void**)&d_lengths, N*sizeof(int32_t)); cudaMalloc((void**)&d_pssm, pssm.size());
    cudaMalloc((void**)&d_scores, N*sizeof(float)); cudaMalloc((void**)&d_indices, N*sizeof(int32_t));
    cudaMalloc((void**)&d_overflow, sizeof(uint32_t));
    memcpy(d_chars, chars.data(), chars.size()); memcpy(d_offsets, offsets.data(), N*sizeof(uint64_t));
    memcpy(d_lengths, lengths.data(), N*sizeof(int32_t)); memcpy(d_pssm, pssm.data(), pssm.size());

    MetalGaplessParams params{(uint32_t)N, (uint32_t)Q, 0u, 0u};
    MetalKernelArg args[8] = {
        MetalKernelArg::buffer(d_chars), MetalKernelArg::buffer(d_offsets), MetalKernelArg::buffer(d_lengths),
        MetalKernelArg::buffer(d_pssm), MetalKernelArg::buffer(d_scores), MetalKernelArg::buffer(d_indices),
        MetalKernelArg::bytes(params), MetalKernelArg::buffer(d_overflow),
    };
    auto go = [&](const std::string& name){
        unsigned int sg = 8; size_t mt = metalKernelMaxThreads(name.c_str());
        if (mt>0){ unsigned int m=(unsigned int)(mt/32); if(m==0)m=1; if(m>32)m=32; sg=m; }
        unsigned int subjPerTg = sg*subjectsPerSg, grid=(N+subjPerTg-1)/subjPerTg;
        metalLaunchKernel(name.c_str(), args, 8, grid, sg*32, nullptr); cudaDeviceSynchronize();
    };

    // FP16 first (as the wrapper does): must flag overflow.
    memset(d_scores, 0, N*sizeof(float)); *d_overflow = 0;
    go(f16);
    bool flagged = (*d_overflow == (uint32_t)N);          // all N subjects score >= 2048
    // int16 rescore: must be exact.
    go(i16);
    int mm = 0;
    for (int s = 0; s < N; ++s) if ((int)d_scores[s] != expected) ++mm;

    cudaFree(d_chars); cudaFree(d_offsets); cudaFree(d_lengths); cudaFree(d_pssm);
    cudaFree(d_scores); cudaFree(d_indices); cudaFree(d_overflow);

    bool ok = flagged && (mm == 0);
    printf("overflow  q=%-4d n=%-4d score=%-5d (>=2048) flagged=%d int16_exact=%d : %s\n",
           Q, N, expected, (int)flagged, (int)(mm==0), ok ? "PASS" : "FAIL");
    return ok;
}

} // namespace

int main() {
    int deviceCount = 0;
    cudaGetDeviceCount(&deviceCount);
    if (deviceCount == 0) { printf("no Metal device\n"); return 77; } // skip

    bool all = true;
    // Sweep GROUP_SIZE {8,16,32} across query lengths + the >1536 fallback.
    for (unsigned int gs : {8u, 16u, 32u}) {
        all &= runCase(64,   129, 200,  1, gs);
        all &= runCase(120,  200, 512,  2, gs);
        all &= runCase(200,  257, 800,  3, gs);
        all &= runCase(577,  300, 900,  7, gs);
        all &= runCase(1200, 100, 1500, 4, gs);
        all &= runCase(64,   1,   64,   6, gs);
    }
    all &= runCase(2000, 64,  2200, 5, 32u);   // fallback (>1536)
    all &= overflowCase();                     // FP16 overflow -> int16 rescore

    printf("\n%s\n", all ? "ALL PASS" : "SOME FAILED");
    return all ? 0 : 1;
}
