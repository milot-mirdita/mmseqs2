#ifndef COOPERATIVE_GROUPS_H_METAL_SHIM
#define COOPERATIVE_GROUPS_H_METAL_SHIM

// minimal cooperative_groups stub
namespace cooperative_groups {

struct thread_group {
    __attribute__((always_inline)) void sync() const {}
    unsigned int thread_rank() const { return 0; }
    unsigned int size() const { return 1; }
};

struct thread_block : thread_group {};

template<unsigned int Size>
struct thread_block_tile : thread_group {
    static constexpr unsigned int num_threads = Size;
    template<class T> T shfl(T v, unsigned int) const { return v; }
    template<class T> T shfl_up(T v, unsigned int) const { return v; }
    template<class T> T shfl_down(T v, unsigned int) const { return v; }
};

inline thread_block this_thread_block() { return thread_block{}; }

template<unsigned int Size, class Group>
inline thread_block_tile<Size> tiled_partition(const Group&) { return thread_block_tile<Size>{}; }

} // namespace cooperative_groups

#endif // COOPERATIVE_GROUPS_H_METAL_SHIM
