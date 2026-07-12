// minimal CUDA runtime API, implemented for Metal

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import <IOKit/IOKitLib.h>

#include "cuda_metal_runtime.h"
#include "cuda_metal_launch.h"

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <mutex>
#include <vector>
#include <string>
#include <unordered_map>
#include <unistd.h>

// mmseqs.metallib is embedded into binary
extern "C" {
extern unsigned char mmseqs_metallib[];
extern unsigned int  mmseqs_metallib_len;
}

namespace {

struct Allocation {
    uintptr_t base;
    // usable size requested by the caller
    size_t    size;
    // page-aligned host block
    void*     raw;
    // (__bridge_retained id<MTLBuffer>)
    void*     buffer;
};

std::mutex g_mutex;
std::vector<Allocation> g_allocs;

id<MTLDevice>        g_device       = nil;
id<MTLCommandQueue>  g_queue        = nil;
id<MTLLibrary>       g_library      = nil;
bool                 g_libraryTried = false;

// name -> (__bridge_retained id<MTLComputePipelineState>)
std::unordered_map<std::string, void*> g_pipelines;

// Committed-but-not-yet-waited command buffers
std::vector<void*> g_pending;

cudaError_t          g_lastError    = cudaSuccess;

// Env-gated profiling
double   g_gpuSeconds   = 0.0;
uint64_t g_kernelCount  = 0;
bool     g_profileOn    = false;
bool     g_profileInit  = false;
void ensureProfile() {
    if (g_profileInit) return;
    g_profileInit = true;
    g_profileOn = (getenv("METAL_PROFILE") != nullptr);
    if (g_profileOn) {
        atexit([]{
            fprintf(stderr, "[metal-profile] GPU kernel time: %.3f s across %llu dispatches\n",
                    g_gpuSeconds, (unsigned long long)g_kernelCount);
        });
    }
}

size_t pageRoundUp(size_t n) {
    const size_t pg = (size_t)getpagesize();
    return ((n + pg - 1) / pg) * pg;
}

void ensureDevice() {
    if (g_device == nil) {
        g_device = MTLCreateSystemDefaultDevice();
        if (g_device == nil) {
            fprintf(stderr, "[metal] no Metal device available\n");
            abort();
        }
        g_queue = [g_device newCommandQueue];
    }
}

void ensureLibrary() {
    if (g_libraryTried) return;
    g_libraryTried = true;
    ensureDevice();

    NSError* err = nil;
    dispatch_data_t data = dispatch_data_create(
        mmseqs_metallib, mmseqs_metallib_len,
        dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0),
        DISPATCH_DATA_DESTRUCTOR_DEFAULT);
    g_library = [g_device newLibraryWithData:data error:&err];
    if (g_library != nil) return;

    fprintf(stderr, "[metal] failed to load the embedded kernel library: %s\n",
            err ? err.localizedDescription.UTF8String : "unknown error");
}

// Find the allocation containing `p` (or exactly matching base). Caller holds lock.
Allocation* findAlloc(const void* p) {
    const uintptr_t u = reinterpret_cast<uintptr_t>(p);
    for (auto& a : g_allocs) {
        if (u >= a.base && u < a.base + a.size) return &a;
    }
    return nullptr;
}

} // namespace

// Memory
// Allocate and register page-aligned, uninitialized host memory wrapped in a shared MTLBuffer
static cudaError_t allocRegistered(void** outPtr, size_t size) {
    if (outPtr == nullptr) return (g_lastError = cudaErrorInvalidValue);
    ensureDevice();
    if (size == 0) size = 1;

    const size_t pg = (size_t)getpagesize();
    const size_t bufLen = pageRoundUp(size);
    void* raw = nullptr;
    if (posix_memalign(&raw, pg, bufLen) != 0 || raw == nullptr) {
        return (g_lastError = cudaErrorMemoryAllocation);
    }

    id<MTLBuffer> buffer = [g_device newBufferWithBytesNoCopy:raw
                                                       length:bufLen
                                                      options:MTLResourceStorageModeShared
                                                  deallocator:nil];
    if (buffer == nil) {
        free(raw);
        return (g_lastError = cudaErrorMemoryAllocation);
    }

    std::lock_guard<std::mutex> lk(g_mutex);
    Allocation a;
    a.base   = reinterpret_cast<uintptr_t>(raw);
    a.size   = size;
    a.raw    = raw;
    a.buffer = (__bridge_retained void*)buffer;
    g_allocs.push_back(a);

    *outPtr = raw;
    return cudaSuccess;
}

cudaError_t cudaMalloc(void** devPtr, size_t size) {
    return allocRegistered(devPtr, size);
}

cudaError_t cudaFree(void* devPtr) {
    if (devPtr == nullptr) return cudaSuccess;
    std::lock_guard<std::mutex> lk(g_mutex);
    for (size_t i = 0; i < g_allocs.size(); ++i) {
        if (g_allocs[i].raw == devPtr) {
            id<MTLBuffer> buffer = (__bridge_transfer id<MTLBuffer>)g_allocs[i].buffer; // release
            (void)buffer;
            free(g_allocs[i].raw);
            g_allocs.erase(g_allocs.begin() + i);
            return cudaSuccess;
        }
    }
    return (g_lastError = cudaErrorInvalidValue);
}

cudaError_t cudaMallocHost(void** ptr, size_t size) {
    return allocRegistered(ptr, size);
}

cudaError_t cudaFreeHost(void* ptr) {
    return cudaFree(ptr);
}

cudaError_t cudaMemcpy(void* dst, const void* src, size_t count, cudaMemcpyKind /*kind*/) {
    metalDrainPending();
    if (count) memcpy(dst, src, count);
    return cudaSuccess;
}

cudaError_t cudaMemset(void* devPtr, int value, size_t count) {
    metalDrainPending();
    if (count) memset(devPtr, value, count);
    return cudaSuccess;
}

cudaError_t cudaMemGetInfo(size_t* freeBytes, size_t* totalBytes) {
    ensureDevice();
    size_t total = (size_t)[g_device recommendedMaxWorkingSetSize];
    if (total == 0) total = (size_t)8 * 1024 * 1024 * 1024;
    if (totalBytes) *totalBytes = total;
    if (freeBytes)  *freeBytes  = total;
    return cudaSuccess;
}

// Device
cudaError_t cudaGetDevice(int* device)      { if (device) *device = 0; return cudaSuccess; }
cudaError_t cudaSetDevice(int /*device*/)   { return cudaSuccess; }
cudaError_t cudaGetDeviceCount(int* count)  { ensureDevice(); if (count) *count = (g_device != nil) ? 1 : 0; return cudaSuccess; }
cudaError_t cudaDeviceSynchronize(void)     { metalDrainPending(); return cudaSuccess; }

cudaError_t cudaGetLastError(void) { cudaError_t e = g_lastError; g_lastError = cudaSuccess; return e; }
const char* cudaGetErrorString(cudaError_t error) {
    switch (error) {
        case cudaSuccess: return "cudaSuccess";
        case cudaErrorInvalidValue: return "cudaErrorInvalidValue";
        case cudaErrorMemoryAllocation: return "cudaErrorMemoryAllocation";
        case cudaErrorNotSupported: return "cudaErrorNotSupported";
        case cudaErrorPeerAccessAlreadyEnabled: return "cudaErrorPeerAccessAlreadyEnabled";
        case cudaErrorPeerAccessNotEnabled: return "cudaErrorPeerAccessNotEnabled";
        default: return "cudaErrorUnknown";
    }
}

int metalDeviceMultiProcessorCount(void) {
    static int cached = 0;
    if (cached) return cached;
    int cores = 0;
    io_iterator_t it = 0;
    if (IOServiceGetMatchingServices(kIOMainPortDefault,
                                     IOServiceMatching("AGXAccelerator"), &it) == KERN_SUCCESS) {
        io_object_t svc;
        while ((svc = IOIteratorNext(it)) && cores == 0) {
            CFTypeRef p = IORegistryEntrySearchCFProperty(
                svc, kIOServicePlane, CFSTR("gpu-core-count"), kCFAllocatorDefault,
                kIORegistryIterateRecursively | kIORegistryIterateParents);
            if (p) {
                if (CFGetTypeID(p) == CFNumberGetTypeID())
                    CFNumberGetValue((CFNumberRef)p, kCFNumberIntType, &cores);
                CFRelease(p);
            }
            IOObjectRelease(svc);
        }
        IOObjectRelease(it);
    }
    cached = (cores > 0) ? cores : 16;
    return cached;
}

// Streams / events — synchronous on shared storage
cudaError_t cudaStreamCreate(cudaStream_t* stream) {
    if (stream) *stream = reinterpret_cast<cudaStream_t>(new int(0));
    return cudaSuccess;
}
cudaError_t cudaStreamCreateWithFlags(cudaStream_t* stream, unsigned int /*flags*/) {
    return cudaStreamCreate(stream);
}
cudaError_t cudaStreamDestroy(cudaStream_t stream) { delete reinterpret_cast<int*>(stream); return cudaSuccess; }
cudaError_t cudaStreamSynchronize(cudaStream_t)    { metalDrainPending(); return cudaSuccess; }
cudaError_t cudaStreamQuery(cudaStream_t)          { metalDrainPending(); return cudaSuccess; }
cudaError_t cudaStreamWaitEvent(cudaStream_t, cudaEvent_t, unsigned int) { metalDrainPending(); return cudaSuccess; }

cudaError_t cudaEventCreate(cudaEvent_t* event) {
    if (event) *event = reinterpret_cast<cudaEvent_t>(new int(0));
    return cudaSuccess;
}
cudaError_t cudaEventCreateWithFlags(cudaEvent_t* event, unsigned int /*flags*/) {
    return cudaEventCreate(event);
}
cudaError_t cudaEventDestroy(cudaEvent_t event) { delete reinterpret_cast<int*>(event); return cudaSuccess; }
cudaError_t cudaEventRecord(cudaEvent_t, cudaStream_t) { return cudaSuccess; }
cudaError_t cudaEventSynchronize(cudaEvent_t) { metalDrainPending(); return cudaSuccess; }
cudaError_t cudaEventQuery(cudaEvent_t) { return cudaSuccess; }
cudaError_t cudaEventElapsedTime(float* ms, cudaEvent_t, cudaEvent_t) {
    // not implemented
    if (ms) *ms = 0.0f;
    return cudaSuccess;
}

cudaError_t cudaLaunchHostFunc(cudaStream_t, cudaHostFn_t fn, void* userData) {
    // synchronous, run immediately
    if (fn) fn(userData);
    return cudaSuccess;
}

// Internal
void metalRegisterHostRegion(const void* ptr, size_t bytes) {
    if (ptr == nullptr || bytes == 0) return;
    ensureDevice();
    const size_t pg = (size_t)getpagesize();
    uintptr_t u = reinterpret_cast<uintptr_t>(ptr);
    uintptr_t pageStart = u & ~(uintptr_t)(pg - 1);
    size_t span = (u - pageStart) + bytes;
    size_t len = pageRoundUp(span);

    {
        std::lock_guard<std::mutex> lk(g_mutex);
        // Already fully covered by an existing registration?
        for (auto& a : g_allocs) {
            if (pageStart >= a.base && (pageStart + len) <= (a.base + a.size)) return;
        }
    }

    id<MTLBuffer> buffer = [g_device newBufferWithBytesNoCopy:(void*)pageStart
                                                       length:len
                                                      options:MTLResourceStorageModeShared
                                                  deallocator:nil];
    if (buffer == nil) {
        fprintf(stderr, "[metal] metalRegisterHostRegion failed for %p (%zu bytes)\n", ptr, bytes);
        return;
    }
    std::lock_guard<std::mutex> lk(g_mutex);
    Allocation a;
    a.base   = pageStart;
    a.size   = len;
    // non-owning: never freed by cudaFree
    a.raw    = nullptr;
    a.buffer = (__bridge_retained void*)buffer;
    g_allocs.push_back(a);
}

// Resolve a facade pointer to its backing (MTLBuffer, offset)
static void* metalBufferForPointer(const void* ptr, size_t* offsetOut) {
    std::lock_guard<std::mutex> lk(g_mutex);
    Allocation* a = findAlloc(ptr);
    if (a == nullptr) {
        if (getenv("METAL_DEBUG_REG")) {
            fprintf(stderr, "[metal] MISS ptr=%p ; %zu registered ranges:\n", ptr, g_allocs.size());
            for (auto& r : g_allocs)
                fprintf(stderr, "        [%p .. %p) size=%zu\n",
                        (void*)r.base, (void*)(r.base + r.size), r.size);
        }
        if (offsetOut) *offsetOut = 0; return nullptr;
    }
    if (offsetOut) *offsetOut = reinterpret_cast<uintptr_t>(ptr) - a->base;
    // (__bridge id<MTLBuffer>) retained by the registry
    return a->buffer;
}

// Named compute pipeline from the loaded metallib
static void* metalPipelineForName(const char* name) {
    ensureLibrary();
    if (g_library == nil) return nullptr;
    std::string key(name);
    {
        std::lock_guard<std::mutex> lk(g_mutex);
        auto it = g_pipelines.find(key);
        if (it != g_pipelines.end()) return it->second;
    }
    id<MTLFunction> fn = [g_library newFunctionWithName:[NSString stringWithUTF8String:name]];
    if (fn == nil) return nullptr;
    NSError* err = nil;
    id<MTLComputePipelineState> pso = [g_device newComputePipelineStateWithFunction:fn error:&err];
    if (pso == nil) {
        fprintf(stderr, "[metal] failed to build pipeline '%s': %s\n", name,
                err ? [[err localizedDescription] UTF8String] : "unknown");
        return nullptr;
    }
    void* retained = (__bridge_retained void*)pso;
    std::lock_guard<std::mutex> lk(g_mutex);
    g_pipelines[key] = retained;
    return retained;
}

size_t metalKernelMaxThreads(const char* name) {
    void* p = metalPipelineForName(name);
    if (p == nullptr) return 0;
    id<MTLComputePipelineState> pso = (__bridge id<MTLComputePipelineState>)p;
    return (size_t)[pso maxTotalThreadsPerThreadgroup];
}

cudaError_t metalLaunchKernel(
    const char* kernelName,
    const MetalKernelArg* args,
    int numArgs,
    unsigned int gridThreadgroups,
    unsigned int threadsPerThreadgroup,
    cudaStream_t /*stream*/)
{
    ensureDevice();
    void* psoPtr = metalPipelineForName(kernelName);
    if (psoPtr == nullptr) return (g_lastError = cudaErrorNotSupported);
    id<MTLComputePipelineState> pso = (__bridge id<MTLComputePipelineState>)psoPtr;

    id<MTLCommandBuffer> cb = [g_queue commandBuffer];
    id<MTLComputeCommandEncoder> enc = [cb computeCommandEncoder];
    [enc setComputePipelineState:pso];

    for (int i = 0; i < numArgs; ++i) {
        if (args[i].isBuffer) {
            size_t off = 0;
            void* bufPtr = metalBufferForPointer(args[i].ptr, &off);
            if (bufPtr == nullptr) {
                [enc endEncoding];
                fprintf(stderr, "[metal] launch '%s': unregistered buffer arg %d\n", kernelName, i);
                return (g_lastError = cudaErrorInvalidValue);
            }
            id<MTLBuffer> buf = (__bridge id<MTLBuffer>)bufPtr;
            [enc setBuffer:buf offset:off atIndex:i];
        } else {
            [enc setBytes:args[i].ptr length:args[i].size atIndex:i];
        }
    }

    MTLSize grid = MTLSizeMake(gridThreadgroups, 1, 1);
    MTLSize tg   = MTLSizeMake(threadsPerThreadgroup, 1, 1);
    [enc dispatchThreadgroups:grid threadsPerThreadgroup:tg];
    [enc endEncoding];
    [cb commit];

    // Do NOT wait here; the buffer is drained at the next host consumption point.
    std::lock_guard<std::mutex> lk(g_mutex);
    g_pending.push_back((__bridge_retained void*)cb);
    return cudaSuccess;
}

void metalDrainPending(void) {
    std::vector<void*> pending;
    {
        std::lock_guard<std::mutex> lk(g_mutex);
        pending.swap(g_pending);
    }
    for (void* p : pending) {
        id<MTLCommandBuffer> cb = (__bridge_transfer id<MTLCommandBuffer>)p;
        [cb waitUntilCompleted];
        ensureProfile();
        if (g_profileOn && cb.GPUEndTime > cb.GPUStartTime) {
            g_gpuSeconds += (cb.GPUEndTime - cb.GPUStartTime);
            g_kernelCount++;
        }
        if (cb.status == MTLCommandBufferStatusError) {
            fprintf(stderr, "[metal] command buffer error: %s\n",
                    cb.error ? [[cb.error localizedDescription] UTF8String] : "unknown");
            g_lastError = cudaErrorUnknown;
        }
    }
}
