#ifndef COOPERATIVE_GROUPS_REDUCE_H_METAL_SHIM
#define COOPERATIVE_GROUPS_REDUCE_H_METAL_SHIM

#include "../cooperative_groups.h"

namespace cooperative_groups {

template<class Group, class T, class Op>
inline T reduce(const Group&, T value, Op) { return value; }

template<class T> struct plus { T operator()(T a, T b) const { return a + b; } };
template<class T> struct greater { T operator()(T a, T b) const { return a > b ? a : b; } };
template<class T> struct less { T operator()(T a, T b) const { return a < b ? a : b; } };

} // namespace cooperative_groups

#endif // COOPERATIVE_GROUPS_REDUCE_H_METAL_SHIM
