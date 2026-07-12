#ifndef METAL_GAPLESS_SHARED_H
#define METAL_GAPLESS_SHARED_H

// Definitions shared by the host launcher and kernel

#ifndef __METAL_VERSION__
#include <cstdint>
#endif

// Kernel push-constant block (bound at buffer index 6).
struct MetalGaplessParams {
    uint32_t numSequences;
    uint32_t queryLength;
    // global id of the first subject in this pass
    uint32_t indexOffset;
    uint32_t pad;
};

#endif // METAL_GAPLESS_SHARED_H
