#ifndef CUDA_METAL_RUNTIME_H
#define CUDA_METAL_RUNTIME_H

// minimal CUDA runtime API, implemented for Metal

#include <cstddef>
#include <cstdint>

// Opaque runtime types
typedef struct CUstream_metal*   cudaStream_t;
typedef struct CUevent_metal*    cudaEvent_t;
typedef struct CUmemPool_metal*  cudaMemPool_t;
typedef void (*cudaHostFn_t)(void* userData);

typedef enum cudaError {
    cudaSuccess = 0,
    cudaErrorInvalidValue = 1,
    cudaErrorMemoryAllocation = 2,
    cudaErrorNotSupported = 801,
    cudaErrorUnknown = 999,
    cudaErrorPeerAccessAlreadyEnabled = 704,
    cudaErrorPeerAccessNotEnabled = 705
} cudaError_t;

typedef enum cudaMemcpyKind {
    cudaMemcpyHostToHost = 0,
    cudaMemcpyHostToDevice = 1,
    cudaMemcpyDeviceToHost = 2,
    cudaMemcpyDeviceToDevice = 3,
    cudaMemcpyDefault = 4
} cudaMemcpyKind;

typedef enum cudaDeviceAttr {
    cudaDevAttrComputeCapabilityMajor = 75,
    cudaDevAttrComputeCapabilityMinor = 76,
    cudaDevAttrMultiProcessorCount = 16,
    cudaDevAttrPageableMemoryAccessUsesHostPageTables = 100
} cudaDeviceAttr;

typedef enum cudaFuncCache {
    cudaFuncCachePreferNone = 0,
    cudaFuncCachePreferShared = 1,
    cudaFuncCachePreferL1 = 2,
    cudaFuncCachePreferEqual = 3
} cudaFuncCache;

typedef enum cudaMemPoolAttr {
    cudaMemPoolAttrReleaseThreshold = 4
} cudaMemPoolAttr;

typedef enum cudaMemoryAdvise {
    cudaMemAdviseSetAccessedBy = 5
} cudaMemoryAdvise;

typedef enum cudaFuncAttribute {
    cudaFuncAttributeMaxDynamicSharedMemorySize = 8
} cudaFuncAttribute;

enum {
    cudaEventDefault = 0,
    cudaEventDisableTiming = 2,
    cudaStreamDefault = 0,
    cudaStreamNonBlocking = 1,
    cudaHostRegisterDefault = 0,
    cudaIpcMemLazyEnablePeerAccess = 1
};

// cudaStreamLegacy is the default stream
#define cudaStreamLegacy ((cudaStream_t)0)

typedef struct cudaIpcMemHandle_st { char reserved[64]; } cudaIpcMemHandle_t;

// Core runtime entry points
cudaError_t cudaMalloc(void** devPtr, size_t size);
cudaError_t cudaFree(void* devPtr);
cudaError_t cudaMallocHost(void** ptr, size_t size);
cudaError_t cudaFreeHost(void* ptr);

cudaError_t cudaMemcpy(void* dst, const void* src, size_t count, cudaMemcpyKind kind);
cudaError_t cudaMemset(void* devPtr, int value, size_t count);

cudaError_t cudaGetDevice(int* device);
cudaError_t cudaSetDevice(int device);
cudaError_t cudaGetDeviceCount(int* count);
cudaError_t cudaDeviceSynchronize(void);

cudaError_t cudaGetLastError(void);
const char* cudaGetErrorString(cudaError_t error);

// Streams (backed by an ordered command queue; synchronous on shared storage)
cudaError_t cudaStreamCreate(cudaStream_t* stream);
cudaError_t cudaStreamCreateWithFlags(cudaStream_t* stream, unsigned int flags);
cudaError_t cudaStreamDestroy(cudaStream_t stream);
cudaError_t cudaStreamSynchronize(cudaStream_t stream);
cudaError_t cudaStreamQuery(cudaStream_t stream);
cudaError_t cudaStreamWaitEvent(cudaStream_t stream, cudaEvent_t event, unsigned int flags = 0);

// Events
cudaError_t cudaEventCreate(cudaEvent_t* event);
cudaError_t cudaEventCreateWithFlags(cudaEvent_t* event, unsigned int flags);
cudaError_t cudaEventDestroy(cudaEvent_t event);
cudaError_t cudaEventRecord(cudaEvent_t event, cudaStream_t stream);
cudaError_t cudaEventSynchronize(cudaEvent_t event);
cudaError_t cudaEventQuery(cudaEvent_t event);
cudaError_t cudaEventElapsedTime(float* ms, cudaEvent_t start, cudaEvent_t end);

cudaError_t cudaLaunchHostFunc(cudaStream_t stream, cudaHostFn_t fn, void* userData);

cudaError_t cudaMemGetInfo(size_t* freeBytes, size_t* totalBytes);

#endif // CUDA_METAL_RUNTIME_H
