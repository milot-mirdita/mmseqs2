#ifndef CUDA_BACKEND_H
#define CUDA_BACKEND_H

// CUDA backend abstraction to native, HIP and Metal
#if defined(__HIPCC__)

#ifndef __grid_constant__
#define __grid_constant__
#endif

#define cudaStream_t hipStream_t
#define cudaEvent_t hipEvent_t
#define cudaError_t hipError_t
#define cudaMemcpyKind hipMemcpyKind
#define cudaMemPool_t hipMemPool_t
#define cudaIpcMemHandle_t hipIpcMemHandle_t
#define cudaDeviceProp hipDeviceProp_t

#define cudaSuccess hipSuccess
#define cudaMemcpyHostToDevice hipMemcpyHostToDevice
#define cudaMemcpyDeviceToHost hipMemcpyDeviceToHost
#define cudaMemcpyHostToHost hipMemcpyHostToHost
#define cudaMemcpyDeviceToDevice hipMemcpyDeviceToDevice
#define cudaMemcpyDefault hipMemcpyDefault
#define cudaEventDisableTiming hipEventDisableTiming
#define cudaStreamLegacy hipStreamLegacy
#define cudaErrorPeerAccessAlreadyEnabled hipErrorPeerAccessAlreadyEnabled
#define cudaErrorPeerAccessNotEnabled hipErrorPeerAccessNotEnabled
#define cudaIpcMemLazyEnablePeerAccess hipIpcMemLazyEnablePeerAccess
#define cudaDevAttrComputeCapabilityMajor hipDeviceAttributeComputeCapabilityMajor
#define cudaDevAttrComputeCapabilityMinor hipDeviceAttributeComputeCapabilityMinor
#define cudaDevAttrMultiProcessorCount hipDeviceAttributeMultiprocessorCount
#define cudaDevAttrPageableMemoryAccessUsesHostPageTables hipDeviceAttributePageableMemoryAccessUsesHostPageTables
#define cudaDeviceAttributeComputeCapabilityMajor hipDeviceAttributeComputeCapabilityMajor
#define cudaDeviceAttributeComputeCapabilityMinor hipDeviceAttributeComputeCapabilityMinor
#define cudaDeviceAttributeMultiprocessorCount hipDeviceAttributeMultiprocessorCount
#define cudaFuncAttributeMaxDynamicSharedMemorySize hipFuncAttributeMaxDynamicSharedMemorySize
#define cudaFuncCachePreferShared hipFuncCachePreferShared
#define cudaMemPoolAttrReleaseThreshold hipMemPoolAttrReleaseThreshold

#define cudaGetLastError hipGetLastError
#define cudaGetErrorString hipGetErrorString
#define cudaGetDevice hipGetDevice
#define cudaSetDevice hipSetDevice
#define cudaGetDeviceCount hipGetDeviceCount
#define cudaDeviceGetAttribute hipDeviceGetAttribute
#define cudaDeviceCanAccessPeer hipDeviceCanAccessPeer
#define cudaDeviceEnablePeerAccess hipDeviceEnablePeerAccess
#define cudaDeviceDisablePeerAccess hipDeviceDisablePeerAccess
#define cudaDeviceSynchronize hipDeviceSynchronize
#define cudaDeviceSetCacheConfig hipDeviceSetCacheConfig
#define cudaDeviceGetDefaultMemPool hipDeviceGetDefaultMemPool
#define cudaMemPoolSetAttribute hipMemPoolSetAttribute
#define cudaMemGetInfo hipMemGetInfo
#define cudaMalloc hipMalloc
#define cudaMallocAsync hipMallocAsync
#define cudaFree hipFree
#define cudaFreeAsync hipFreeAsync
#define cudaMallocHost hipHostMalloc
#define cudaFreeHost hipHostFree
#define cudaMemcpy hipMemcpy
#define cudaMemcpyAsync hipMemcpyAsync
#define cudaMemsetAsync hipMemsetAsync
#define cudaLaunchHostFunc hipLaunchHostFunc
#define cudaFuncSetAttribute hipFuncSetAttribute
#define cudaEventCreate hipEventCreate
#define cudaEventCreateWithFlags hipEventCreateWithFlags
#define cudaEventDestroy hipEventDestroy
#define cudaEventQuery hipEventQuery
#define cudaEventRecord hipEventRecord
#define cudaEventSynchronize hipEventSynchronize
#define cudaEventElapsedTime hipEventElapsedTime
#define cudaStreamCreate hipStreamCreate
#define cudaStreamCreateWithFlags hipStreamCreateWithFlags
#define cudaStreamDestroy hipStreamDestroy
#define cudaStreamQuery hipStreamQuery
#define cudaStreamSynchronize hipStreamSynchronize
#define cudaStreamWaitEvent hipStreamWaitEvent
#define cudaIpcGetMemHandle hipIpcGetMemHandle
#define cudaIpcOpenMemHandle hipIpcOpenMemHandle

#include <thrust/system/hip/execution_policy.h>
namespace thrust {
namespace cuda = hip;
}

#elif defined(__METAL_BACKEND__)

#define __global__
#define __device__
#define __host__
#define __forceinline__ inline
#define __restrict__
#define __launch_bounds__(...)
#define __grid_constant__
#define __constant__

#ifndef CUERR
#define CUERR
#endif

#ifndef NO_NVTOOLSEXT
#define NO_NVTOOLSEXT 1
#endif

// macOS mmap has no MAP_POPULATE
#ifndef MAP_POPULATE
#define MAP_POPULATE 0
#endif

// Dummy device-coordinate globals
struct __MetalDim3 { unsigned int x, y, z; };
inline __MetalDim3 threadIdx{0, 0, 0};
inline __MetalDim3 blockIdx {0, 0, 0};
inline __MetalDim3 blockDim {1, 1, 1};
inline __MetalDim3 gridDim  {1, 1, 1};

// CUDA vector types
struct char4  { signed char   x, y, z, w; };
struct uchar4 { unsigned char x, y, z, w; };
struct short2 { short x, y; };
struct short4 { short x, y, z, w; };
struct int2   { int x, y; };
struct int4   { int x, y, z, w; };
struct uint2  { unsigned int x, y; };
struct alignas(8)  float2 { float x, y; };
struct alignas(16) float4 { float x, y, z, w; };

inline char4  make_char4(signed char x, signed char y, signed char z, signed char w) { return char4{x, y, z, w}; }
inline uchar4 make_uchar4(unsigned char x, unsigned char y, unsigned char z, unsigned char w) { return uchar4{x, y, z, w}; }
inline short2 make_short2(short x, short y) { return short2{x, y}; }
inline short4 make_short4(short x, short y, short z, short w) { return short4{x, y, z, w}; }
inline int2   make_int2(int x, int y) { return int2{x, y}; }
inline int4   make_int4(int x, int y, int z, int w) { return int4{x, y, z, w}; }
inline uint2  make_uint2(unsigned int x, unsigned int y) { return uint2{x, y}; }
inline float2 make_float2(float x, float y) { return float2{x, y}; }
inline float4 make_float4(float x, float y, float z, float w) { return float4{x, y, z, w}; }

// CUDA shims
#include "metal/cuda_fp16.h"
#include "metal/cuda_runtime.h"
#include "metal/cooperative_groups.h"
#include "metal/thrust_impl.h"

#endif

#endif // CUDA_BACKEND_H
