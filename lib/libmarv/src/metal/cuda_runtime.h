#ifndef CUDA_RUNTIME_H_METAL_SHIM
#define CUDA_RUNTIME_H_METAL_SHIM

// minimal CUDA runtime API, implemented for Metal

#include "cuda_metal_runtime.h"
#include "cuda_metal_launch.h"   // metalDeviceMultiProcessorCount

#include <cstring>
#include <cstddef>

template<class T>
inline cudaError_t cudaMalloc(T** devPtr, size_t size) {
    return cudaMalloc(reinterpret_cast<void**>(devPtr), size);
}
template<class T>
inline cudaError_t cudaMallocHost(T** ptr, size_t size) {
    return cudaMallocHost(reinterpret_cast<void**>(ptr), size);
}

// Map async memory ops to synchronous
inline cudaError_t cudaMallocAsync(void** devPtr, size_t size, cudaStream_t /*stream*/ = 0) {
    return cudaMalloc(devPtr, size);
}
template<class T>
inline cudaError_t cudaMallocAsync(T** devPtr, size_t size, cudaStream_t /*stream*/ = 0) {
    return cudaMalloc(reinterpret_cast<void**>(devPtr), size);
}
inline cudaError_t cudaFreeAsync(void* devPtr, cudaStream_t /*stream*/ = 0) {
    return cudaFree(devPtr);
}
inline cudaError_t cudaMallocManaged(void** devPtr, size_t size, unsigned int /*flags*/ = 1) {
    return cudaMalloc(devPtr, size);
}
template<class T>
inline cudaError_t cudaMallocManaged(T** devPtr, size_t size, unsigned int /*flags*/ = 1) {
    return cudaMalloc(reinterpret_cast<void**>(devPtr), size);
}

inline cudaError_t cudaMemcpyAsync(void* dst, const void* src, size_t count,
                                   cudaMemcpyKind kind, cudaStream_t /*stream*/ = 0) {
    return cudaMemcpy(dst, src, count, kind);
}
inline cudaError_t cudaMemsetAsync(void* devPtr, int value, size_t count,
                                   cudaStream_t /*stream*/ = 0) {
    return cudaMemset(devPtr, value, count);
}

// device globals are host-addressable
template<class Symbol>
inline cudaError_t cudaMemcpyToSymbol(Symbol& symbol, const void* src, size_t count,
                                      size_t offset = 0,
                                      cudaMemcpyKind /*kind*/ = cudaMemcpyHostToDevice) {
    memcpy(reinterpret_cast<char*>(&symbol) + offset, src, count);
    return cudaSuccess;
}
template<class Symbol>
inline cudaError_t cudaMemcpyToSymbolAsync(Symbol& symbol, const void* src, size_t count,
                                           size_t offset = 0,
                                           cudaMemcpyKind /*kind*/ = cudaMemcpyHostToDevice,
                                           cudaStream_t /*stream*/ = 0) {
    memcpy(reinterpret_cast<char*>(&symbol) + offset, src, count);
    return cudaSuccess;
}

inline cudaError_t cudaMemAdvise(const void* /*devPtr*/, size_t /*count*/,
                                 cudaMemoryAdvise /*advice*/, int /*device*/) {
    return cudaSuccess;
}

// Pinned-host register no-ops with unified memory
inline cudaError_t cudaHostRegister(void* /*ptr*/, size_t /*size*/, unsigned int /*flags*/ = 0) { return cudaSuccess; }
inline cudaError_t cudaHostUnregister(void* /*ptr*/) { return cudaSuccess; }

// Device properties and attributes mapped to Apple GPU
struct cudaDeviceProp {
    char name[256];
    int  major;
    int  minor;
    int  multiProcessorCount;
    int  pageableMemoryAccess;
    int  pageableMemoryAccessUsesHostPageTables;
    int  hostRegisterSupported;
    int  hostRegisterReadOnlySupported;
    int  canMapHostMemory;
    size_t totalGlobalMem;
};

inline cudaError_t cudaGetDeviceProperties(cudaDeviceProp* prop, int /*device*/) {
    if (prop == nullptr) return cudaErrorInvalidValue;
    memset(prop, 0, sizeof(*prop));
    strncpy(prop->name, "Apple GPU (Metal)", sizeof(prop->name) - 1);
    prop->major = 9;
    prop->minor = 0;
    prop->multiProcessorCount = metalDeviceMultiProcessorCount();
    prop->pageableMemoryAccess = 1;
    prop->pageableMemoryAccessUsesHostPageTables = 1;
    prop->canMapHostMemory = 1;
    size_t freeB = 0, totalB = 0;
    cudaMemGetInfo(&freeB, &totalB);
    prop->totalGlobalMem = totalB;
    return cudaSuccess;
}

inline cudaError_t cudaDeviceGetAttribute(int* value, cudaDeviceAttr attr, int /*device*/) {
    if (value == nullptr) return cudaErrorInvalidValue;
    switch (attr) {
        case cudaDevAttrMultiProcessorCount:
            *value = metalDeviceMultiProcessorCount(); break;
        // No direct equivalent. Map to 89 to avoid claiming DPX compatibility
        case cudaDevAttrComputeCapabilityMajor:
            *value = 8; break;
        case cudaDevAttrComputeCapabilityMinor:
            *value = 9; break;
        case cudaDevAttrPageableMemoryAccessUsesHostPageTables:
            *value = 1; break;
        default:
            *value = 0; break;
    }
    return cudaSuccess;
}

inline cudaError_t cudaDeviceSetCacheConfig(cudaFuncCache /*cfg*/) { return cudaSuccess; }

template<class Func>
inline cudaError_t cudaFuncSetAttribute(Func /*func*/, cudaFuncAttribute /*attr*/, int /*value*/) {
    return cudaSuccess;
}

// Mem pools stubs
inline cudaError_t cudaDeviceGetDefaultMemPool(cudaMemPool_t* pool, int /*device*/) {
    if (pool) *pool = reinterpret_cast<cudaMemPool_t>(1);
    return cudaSuccess;
}
inline cudaError_t cudaMemPoolSetAttribute(cudaMemPool_t /*pool*/, cudaMemPoolAttr /*attr*/, void* /*value*/) {
    return cudaSuccess;
}

// Peer access stubs
inline cudaError_t cudaDeviceCanAccessPeer(int* canAccess, int /*device*/, int /*peerDevice*/) {
    if (canAccess) *canAccess = 0;
    return cudaSuccess;
}
inline cudaError_t cudaDeviceEnablePeerAccess(int /*peerDevice*/, unsigned int /*flags*/ = 0) {
    return cudaSuccess;
}
inline cudaError_t cudaDeviceDisablePeerAccess(int /*peerDevice*/) {
    return cudaSuccess;
}

// IPC stubs
inline cudaError_t cudaIpcGetMemHandle(cudaIpcMemHandle_t* handle, void* /*devPtr*/) {
    if (handle) memset(handle, 0, sizeof(*handle));
    return cudaSuccess;
}
inline cudaError_t cudaIpcOpenMemHandle(void** devPtr, cudaIpcMemHandle_t /*handle*/, unsigned int /*flags*/) {
    if (devPtr) *devPtr = nullptr;
    return cudaErrorNotSupported;
}

#endif // CUDA_RUNTIME_H_METAL_SHIM
