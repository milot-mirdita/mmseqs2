#ifndef CUDA_METAL_LAUNCH_H
#define CUDA_METAL_LAUNCH_H

// Metal kernel launchers

#include "cuda_metal_runtime.h"
#include <cstddef>

// A single kernel argument, bound at successive Metal argument indices in order.
struct MetalKernelArg {
    // If isBuffer: `ptr` is a facade pointer (resolved to MTLBuffer+offset).
    // Else:        `ptr` points to `size` bytes bound via setBytes (scalar/struct).
    const void* ptr;
    // only used when isBuffer == false
    size_t      size;
    bool        isBuffer;

    static MetalKernelArg buffer(const void* p) { return MetalKernelArg{p, 0, true}; }
    template<class T>
    static MetalKernelArg bytes(const T& v) { return MetalKernelArg{&v, sizeof(T), false}; }
};

// Dispatch `kernelName` over `gridThreadgroups` threadgroups, each of
// `threadsPerThreadgroup` threads, binding `args[0..numArgs)` at indices
// 0..numArgs-1. Blocks until completion.
// Returns cudaErrorNotSupported if the pipeline is unavailable.
cudaError_t metalLaunchKernel(
    const char* kernelName,
    const MetalKernelArg* args,
    int numArgs,
    unsigned int gridThreadgroups,
    unsigned int threadsPerThreadgroup,
    cudaStream_t stream);

// Occupancy hint: maxTotalThreadsPerThreadgroup for the named pipeline
size_t metalKernelMaxThreads(const char* kernelName);

// Block until all launched-but-unwaited kernels complete
void metalDrainPending(void);

// Register an existing host region so kernels can bind pointers
// Used for registering the databases
void metalRegisterHostRegion(const void* ptr, size_t bytes);

// GPU core count
// used for grid/temp-storage sizing
int metalDeviceMultiProcessorCount(void);

#endif // CUDA_METAL_LAUNCH_H
