#ifndef MMSEQS_UNGAPPEDALIGNER_H
#define MMSEQS_UNGAPPEDALIGNER_H

#include <cstddef>
#include <cstdint>

class BaseMatrix;
class Sequence;

class UngappedAligner {
public:
    class Impl {
    public:
        virtual ~Impl() {}
        virtual void initQuery(const Sequence *q, const int8_t *mat, const BaseMatrix *m) = 0;
        virtual int score(const unsigned char *db_sequence, int32_t db_length) const = 0;
    };

    UngappedAligner(size_t maxSequenceLength, int aaSize, bool aaBiasCorrection, float aaBiasCorrectionScale);

    UngappedAligner(const UngappedAligner&) = delete;
    UngappedAligner& operator=(const UngappedAligner&) = delete;

    void initQuery(const Sequence *q, const int8_t *mat, const BaseMatrix *m) {
        impl->initQuery(q, mat, m);
    }
    int score(const unsigned char *db_sequence, int32_t db_length) const {
        return impl->score(db_sequence, db_length);
    }

private:
    Impl *impl;
};

namespace UngappedDefault {
UngappedAligner::Impl *createImpl(size_t maxSequenceLength,
                                  int aaSize,
                                  bool aaBiasCorrection,
                                  float aaBiasCorrectionScale);
}

#if defined(__x86_64__) || defined(__i386__) || defined(_M_X64) || defined(_M_IX86)
namespace UngappedAVX512 {
UngappedAligner::Impl *createImpl(size_t maxSequenceLength,
                                  int aaSize,
                                  bool aaBiasCorrection,
                                  float aaBiasCorrectionScale);
}
#endif


inline UngappedAligner::UngappedAligner(size_t maxSequenceLength, int aaSize, bool aaBiasCorrection, float aaBiasCorrectionScale)
    : impl(nullptr) {
#if (defined(__x86_64__) || defined(__i386__) || defined(_M_X64) || defined(_M_IX86)) && (defined(__GNUC__) || defined(__clang__))
    bool avx512_supported = false;
#if defined(__has_builtin)
#if __has_builtin(__builtin_cpu_supports)
#if __has_builtin(__builtin_cpu_init)
    __builtin_cpu_init();
#endif
    avx512_supported = __builtin_cpu_supports("avx512f") &&
                       __builtin_cpu_supports("avx512bw") &&
                       __builtin_cpu_supports("avx512dq");
#endif
#endif
    if (avx512_supported) {
        impl = UngappedAVX512::createImpl(maxSequenceLength, aaSize, aaBiasCorrection, aaBiasCorrectionScale);
        return;
    }
#endif
    impl = UngappedDefault::createImpl(maxSequenceLength, aaSize, aaBiasCorrection, aaBiasCorrectionScale);
}

#endif
