// Thrust compatibility layer for  Metal backend

#ifndef METAL_THRUST_IMPL_H
#define METAL_THRUST_IMPL_H

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <numeric>
#include <functional>
#include <iterator>
#include <tuple>
#include <type_traits>
#include <vector>

#include "cuda_metal_launch.h"

namespace thrust {

struct execution_policy_tag {};
struct seq_policy : execution_policy_tag {};
static const seq_policy seq = {};

namespace cuda {
    struct par_policy : thrust::execution_policy_tag {
        // The algorithm runs synchronously on the host, so drain pending kernels
        // whose output it is about to read.
        const par_policy& on(cudaStream_t) const { metalDrainPending(); return *this; }
        template <class Alloc> const par_policy& operator()(const Alloc&) const { return *this; }
    };
    static const par_policy par        = {};
    static const par_policy par_nosync = {};
}

template <class T>
struct is_exec : std::is_base_of<execution_policy_tag, typename std::decay<T>::type> {};

template <class T = void> using greater = std::greater<T>;
using std::get;
using std::tuple;
using std::make_tuple;

template <class It> typename std::iterator_traits<It>::difference_type
distance(It a, It b) { return std::distance(a, b); }

// device pointers are raw pointers into unified memory
template <class T> T* raw_pointer_cast(T* p)    { return p; }
template <class T> T* device_pointer_cast(T* p) { return p; }

template <class T>
struct device_malloc_allocator {
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    template <class U> struct rebind { using other = device_malloc_allocator<U>; };
    pointer allocate(size_type n) { T* p = nullptr; cudaMalloc((void**)&p, n * sizeof(T)); return p; }
    void deallocate(pointer p, size_type) { cudaFree(p); }
};

template <class It, class F>
class transform_iterator {
    It it_{}; mutable F f_{};
public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = typename std::decay<decltype(std::declval<F>()(*std::declval<It>()))>::type;
    using difference_type = std::ptrdiff_t;
    using pointer = void; using reference = value_type;
    transform_iterator() = default;
    transform_iterator(It it, F f) : it_(it), f_(f) {}
    reference operator*() const { return f_(*it_); }
    reference operator[](difference_type n) const { return f_(it_[n]); }
    transform_iterator& operator++() { ++it_; return *this; }
    transform_iterator& operator+=(difference_type n) { it_ += n; return *this; }
    transform_iterator operator+(difference_type n) const { return transform_iterator(it_ + n, f_); }
    difference_type operator-(const transform_iterator& o) const { return it_ - o.it_; }
    bool operator==(const transform_iterator& o) const { return it_ == o.it_; }
    bool operator!=(const transform_iterator& o) const { return it_ != o.it_; }
    bool operator<(const transform_iterator& o) const { return it_ < o.it_; }
};
template <class It, class F>
transform_iterator<It, F> make_transform_iterator(It it, F f) { return transform_iterator<It, F>(it, f); }

template <class... Its>
class zip_iterator {
    std::tuple<Its...> its_;
    using seq_t = std::index_sequence_for<Its...>;
    template <size_t... I> auto deref(std::index_sequence<I...>) const
        { return std::tuple<typename std::iterator_traits<Its>::reference...>(*std::get<I>(its_)...); }
    template <size_t... I> void adv(std::ptrdiff_t n, std::index_sequence<I...>)
        { int _[] = { (std::advance(std::get<I>(its_), n), 0)... }; (void)_; }
public:
    using iterator_category = std::random_access_iterator_tag;
    using reference = std::tuple<typename std::iterator_traits<Its>::reference...>;
    using value_type = std::tuple<typename std::iterator_traits<Its>::value_type...>;
    using difference_type = std::ptrdiff_t;
    using pointer = void;
    zip_iterator() = default;
    explicit zip_iterator(std::tuple<Its...> t) : its_(t) {}
    reference operator*() const { return deref(seq_t{}); }
    reference operator[](difference_type n) const { auto t = *this; t += n; return *t; }
    zip_iterator& operator++() { adv(1, seq_t{}); return *this; }
    zip_iterator& operator--() { adv(-1, seq_t{}); return *this; }
    zip_iterator operator++(int) { auto t = *this; ++*this; return t; }
    zip_iterator& operator+=(difference_type n) { adv(n, seq_t{}); return *this; }
    zip_iterator& operator-=(difference_type n) { adv(-n, seq_t{}); return *this; }
    zip_iterator operator+(difference_type n) const { auto t = *this; t += n; return t; }
    zip_iterator operator-(difference_type n) const { auto t = *this; t -= n; return t; }
    difference_type operator-(const zip_iterator& o) const { return std::get<0>(its_) - std::get<0>(o.its_); }
    bool operator==(const zip_iterator& o) const { return std::get<0>(its_) == std::get<0>(o.its_); }
    bool operator!=(const zip_iterator& o) const { return !(*this == o); }
    bool operator<(const zip_iterator& o) const { return std::get<0>(its_) < std::get<0>(o.its_); }
};
template <class... Its>
zip_iterator<Its...> make_zip_iterator(std::tuple<Its...> t) { return zip_iterator<Its...>(t); }
template <class... Its>
zip_iterator<Its...> make_zip_iterator(Its... its) { return zip_iterator<Its...>(std::make_tuple(its...)); }

template <class It, class T> void fill(It f, It l, const T& v) { std::fill(f, l, v); }
template <class Pol, class It, class T, class = typename std::enable_if<is_exec<Pol>::value>::type>
void fill(Pol, It f, It l, const T& v) { std::fill(f, l, v); }

template <class It, class T> void sequence(It f, It l, T init, T step) { for (T v = init; f != l; ++f, v += step) *f = v; }
template <class It, class T> void sequence(It f, It l, T init) { sequence(f, l, init, T(1)); }
template <class It> void sequence(It f, It l) { typedef typename std::iterator_traits<It>::value_type V; sequence(f, l, V(0), V(1)); }
template <class Pol, class It, class = typename std::enable_if<is_exec<Pol>::value>::type>
void sequence(Pol, It f, It l) { sequence(f, l); }
template <class Pol, class It, class T, class = typename std::enable_if<is_exec<Pol>::value>::type>
void sequence(Pol, It f, It l, T init) { sequence(f, l, init); }
template <class Pol, class It, class T, class = typename std::enable_if<is_exec<Pol>::value>::type>
void sequence(Pol, It f, It l, T init, T step) { sequence(f, l, init, step); }

template <class It, class F> F for_each(It f, It l, F fn) { return std::for_each(f, l, fn); }
template <class Pol, class It, class F, class = typename std::enable_if<is_exec<Pol>::value>::type>
F for_each(Pol, It f, It l, F fn) { return std::for_each(f, l, fn); }

template <class In, class Out, class F> Out transform(In f, In l, Out o, F fn) { return std::transform(f, l, o, fn); }
template <class Pol, class In, class Out, class F, class = typename std::enable_if<is_exec<Pol>::value>::type>
Out transform(Pol, In f, In l, Out o, F fn) { return std::transform(f, l, o, fn); }

template <class It, class T> It lower_bound(It f, It l, const T& v) { return std::lower_bound(f, l, v); }
template <class It, class T, class C> It lower_bound(It f, It l, const T& v, C c) { return std::lower_bound(f, l, v, c); }
template <class Pol, class It, class T, class = typename std::enable_if<is_exec<Pol>::value>::type>
It lower_bound(Pol, It f, It l, const T& v) { return std::lower_bound(f, l, v); }
template <class Pol, class It, class T, class C, class = typename std::enable_if<is_exec<Pol>::value>::type>
It lower_bound(Pol, It f, It l, const T& v, C c) { return std::lower_bound(f, l, v, c); }

template <class In, class Out, class Op> Out inclusive_scan(In f, In l, Out o, Op op) {
    typedef typename std::iterator_traits<Out>::value_type V;
    bool first = true; V acc = V();
    for (; f != l; ++f, ++o) { acc = first ? (V)(*f) : op(acc, (V)(*f)); first = false; *o = acc; }
    return o;
}
template <class In, class Out> Out inclusive_scan(In f, In l, Out o) {
    typedef typename std::iterator_traits<Out>::value_type V; return thrust::inclusive_scan(f, l, o, std::plus<V>());
}
template <class Pol, class In, class Out, class = typename std::enable_if<is_exec<Pol>::value>::type>
Out inclusive_scan(Pol, In f, In l, Out o) { return inclusive_scan(f, l, o); }
template <class Pol, class In, class Out, class Op, class = typename std::enable_if<is_exec<Pol>::value>::type>
Out inclusive_scan(Pol, In f, In l, Out o, Op op) { return inclusive_scan(f, l, o, op); }

template <class It> void sort(It f, It l) { std::sort(f, l); }
template <class It, class C> void sort(It f, It l, C c) { std::sort(f, l, c); }
template <class Pol, class It, class = typename std::enable_if<is_exec<Pol>::value>::type>
void sort(Pol, It f, It l) { std::sort(f, l); }
template <class Pol, class It, class C, class = typename std::enable_if<is_exec<Pol>::value>::type>
void sort(Pol, It f, It l, C c) { std::sort(f, l, c); }

template <class... Its, class Comp>
void sort(zip_iterator<Its...> f, zip_iterator<Its...> l, Comp comp) {
    const size_t n = (size_t)(l - f);
    std::vector<size_t> idx(n); std::iota(idx.begin(), idx.end(), (size_t)0);
    std::sort(idx.begin(), idx.end(), [&](size_t a, size_t b){ return comp(f[a], f[b]); });
    typedef typename zip_iterator<Its...>::value_type V;
    std::vector<V> tmp; tmp.reserve(n);
    for (size_t i = 0; i < n; ++i) tmp.push_back(V(f[idx[i]]));
    for (size_t i = 0; i < n; ++i) f[i] = tmp[i];
}
template <class... Its>
void sort(zip_iterator<Its...> f, zip_iterator<Its...> l) {
    typedef typename zip_iterator<Its...>::value_type V;
    sort(f, l, [](const V& a, const V& b){ return a < b; });
}
template <class Pol, class... Its, class Comp, class = typename std::enable_if<is_exec<Pol>::value>::type>
void sort(Pol, zip_iterator<Its...> f, zip_iterator<Its...> l, Comp comp) { sort(f, l, comp); }
template <class Pol, class... Its, class = typename std::enable_if<is_exec<Pol>::value>::type>
void sort(Pol, zip_iterator<Its...> f, zip_iterator<Its...> l) { sort(f, l); }

namespace detail {

// Stable LSD radix sort of raw (key,value) arrays; Desc reverses the key order.
template <class KV, class VV, bool Desc>
void radix_sort_by_key(KV* keys, size_t n, VV* vals) {
    if (n < 2) return;
    constexpr int Bits = 8, Passes = 4, Buckets = 1 << Bits;
    constexpr std::uint32_t Mask = Buckets - 1;

    // Order-preserving map key -> uint32 (ascending; complemented for Desc).
    auto enc = [](KV k) -> std::uint32_t {
        std::uint32_t u; std::memcpy(&u, &k, 4);
        if constexpr (std::is_floating_point<KV>::value) u ^= (u >> 31) ? 0xFFFFFFFFu : 0x80000000u;
        else if constexpr (std::is_signed<KV>::value)    u ^= 0x80000000u;
        if constexpr (Desc) u = ~u;
        return u;
    };

    static thread_local std::vector<KV> kbuf;
    static thread_local std::vector<VV> vbuf;
    kbuf.resize(n); vbuf.resize(n);
    KV* ks = keys; KV* kd = kbuf.data();
    VV* vs = vals; VV* vd = vbuf.data();

    // One pass over the keys builds every digit histogram.
    size_t hist[Passes][Buckets];
    std::memset(hist, 0, sizeof(hist));
    for (size_t i = 0; i < n; ++i) {
        std::uint32_t u = enc(ks[i]);
        #pragma unroll
        for (int p = 0; p < Passes; ++p) hist[p][(u >> (Bits * p)) & Mask]++;
    }
    // Prefix-sum each histogram; flag passes whose digit is uniform.
    bool skip[Passes];
    for (int p = 0; p < Passes; ++p) {
        size_t sum = 0; bool sk = false;
        for (int b = 0; b < Buckets; ++b) { size_t c = hist[p][b]; if (c == n) sk = true; hist[p][b] = sum; sum += c; }
        skip[p] = sk;
    }
    // Scatter (skipping uniform passes), recomputing the digit from the key.
    for (int p = 0; p < Passes; ++p) {
        if (skip[p]) continue;
        const int shift = Bits * p;
        for (size_t i = 0; i < n; ++i) {
            size_t pos = hist[p][(enc(ks[i]) >> shift) & Mask]++;
            kd[pos] = ks[i]; vd[pos] = vs[i];
        }
        std::swap(ks, kd); std::swap(vs, vd);
    }
    if (ks != keys) { std::copy(ks, ks + n, keys); std::copy(vs, vs + n, vals); }
}

// {,stable_}sort_by_key. For the common case (raw arrays, arithmetic 4-byte key,
// thrust::greater/less) radix-sort
template <class KeyIt, class ValIt, class Comp, bool Stable>
void sort_by_key_impl(KeyIt kf, KeyIt kl, ValIt vf, Comp comp) {
    typedef typename std::iterator_traits<KeyIt>::value_type KV;
    typedef typename std::iterator_traits<ValIt>::value_type VV;
    const size_t n = (size_t)std::distance(kf, kl);
    constexpr bool isGreater = std::is_same<Comp, std::greater<KV>>::value || std::is_same<Comp, std::greater<void>>::value;
    constexpr bool isLess    = std::is_same<Comp, std::less<KV>>::value    || std::is_same<Comp, std::less<void>>::value;
    constexpr bool radixable = std::is_pointer<KeyIt>::value && std::is_pointer<ValIt>::value
                             && std::is_arithmetic<KV>::value && sizeof(KV) == 4 && (isGreater || isLess);
    if constexpr (radixable) {
        radix_sort_by_key<KV, VV, isGreater>(&*kf, n, &*vf);
        return;
    }
    struct Rec { KV key; VV val; };
    static thread_local std::vector<Rec> buf;
    buf.resize(n);
    for (size_t i = 0; i < n; ++i) buf[i] = { kf[i], vf[i] };
    auto cmp = [&](const Rec& a, const Rec& b) { return comp(a.key, b.key); };
    if (Stable) std::stable_sort(buf.begin(), buf.end(), cmp);
    else        std::sort(buf.begin(), buf.end(), cmp);
    for (size_t i = 0; i < n; ++i) { kf[i] = buf[i].key; vf[i] = buf[i].val; }
}
} // namespace detail

template <class KeyIt, class ValIt>
void sort_by_key(KeyIt kf, KeyIt kl, ValIt vf) {
    using KV = typename std::iterator_traits<KeyIt>::value_type;
    detail::sort_by_key_impl<KeyIt, ValIt, std::less<KV>, false>(kf, kl, vf, std::less<KV>());
}
template <class KeyIt, class ValIt, class Comp>
void sort_by_key(KeyIt kf, KeyIt kl, ValIt vf, Comp comp) {
    detail::sort_by_key_impl<KeyIt, ValIt, Comp, false>(kf, kl, vf, comp);
}
template <class Pol, class KeyIt, class ValIt, class = typename std::enable_if<is_exec<Pol>::value>::type>
void sort_by_key(Pol, KeyIt kf, KeyIt kl, ValIt vf) { sort_by_key(kf, kl, vf); }
template <class Pol, class KeyIt, class ValIt, class Comp, class = typename std::enable_if<is_exec<Pol>::value>::type>
void sort_by_key(Pol, KeyIt kf, KeyIt kl, ValIt vf, Comp comp) { sort_by_key(kf, kl, vf, comp); }

template <class KeyIt, class ValIt>
void stable_sort_by_key(KeyIt kf, KeyIt kl, ValIt vf) {
    using KV = typename std::iterator_traits<KeyIt>::value_type;
    detail::sort_by_key_impl<KeyIt, ValIt, std::less<KV>, true>(kf, kl, vf, std::less<KV>());
}
template <class KeyIt, class ValIt, class Comp>
void stable_sort_by_key(KeyIt kf, KeyIt kl, ValIt vf, Comp comp) {
    detail::sort_by_key_impl<KeyIt, ValIt, Comp, true>(kf, kl, vf, comp);
}
template <class Pol, class KeyIt, class ValIt, class = typename std::enable_if<is_exec<Pol>::value>::type>
void stable_sort_by_key(Pol, KeyIt kf, KeyIt kl, ValIt vf) { stable_sort_by_key(kf, kl, vf); }
template <class Pol, class KeyIt, class ValIt, class Comp, class = typename std::enable_if<is_exec<Pol>::value>::type>
void stable_sort_by_key(Pol, KeyIt kf, KeyIt kl, ValIt vf, Comp comp) { stable_sort_by_key(kf, kl, vf, comp); }

template <class In1, class In2, class Out, class Comp>
Out merge(In1 af, In1 al, In2 bf, In2 bl, Out o, Comp c) { return std::merge(af, al, bf, bl, o, c); }
template <class In1, class In2, class Out>
Out merge(In1 af, In1 al, In2 bf, In2 bl, Out o) { return std::merge(af, al, bf, bl, o); }
template <class Pol, class In1, class In2, class Out, class = typename std::enable_if<is_exec<Pol>::value>::type>
Out merge(Pol, In1 af, In1 al, In2 bf, In2 bl, Out o) { return std::merge(af, al, bf, bl, o); }
template <class Pol, class In1, class In2, class Out, class Comp, class = typename std::enable_if<is_exec<Pol>::value>::type>
Out merge(Pol, In1 af, In1 al, In2 bf, In2 bl, Out o, Comp c) { return std::merge(af, al, bf, bl, o, c); }

} // namespace thrust

#endif // METAL_THRUST_IMPL_H
