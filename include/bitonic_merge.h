#ifndef BITONIC_MERGE_H
#define BITONIC_MERGE_H

#ifdef _OPENMP
#include <omp.h>
#endif

#include "bit_scan.h"

namespace detail {
    template <int I, int N, bool Invert>
    struct CompareSwap {
        template <typename Iterator, typename Cmp>
        static void compare_swap(Iterator it1, Iterator it2, Cmp cmp) {
            if (cmp(*it1, *it2) != Invert) std::iter_swap(it1, it2);
            CompareSwap<I + 1, N, Invert>::compare_swap(it1 + 1, it2 + 1, cmp);
        }
    };

    template <int N, bool Invert>
    struct CompareSwap<N, N, Invert> {
        template <typename Iterator, typename Cmp>
        static void compare_swap(Iterator, Iterator, Cmp) {}
    };

    template <int N, bool Up>
    struct BitonicMerge {
        template <typename Iterator, typename Cmp>
        static void unroll(size_t d, Iterator begin, Iterator end, Cmp cmp) {
            if (d >= N) {
                CompareSwap<0, N / 2, Up>::compare_swap(begin, begin + N / 2, cmp);
                BitonicMerge<N / 2, Up>::iteration(begin + N / 2, end, cmp);
            }
            BitonicMerge<N / 2, Up>::unroll(d, begin, begin + N / 2, cmp);
        }

        template <typename Iterator, typename Cmp>
        static void iteration(Iterator begin, Iterator end, Cmp cmp) {
            CompareSwap<0, N / 2, Up>::compare_swap(begin, begin + N / 2, cmp);
            BitonicMerge<N / 2, Up>::iteration(begin, begin + N / 2, cmp);
            BitonicMerge<N / 2, Up>::iteration(begin + N / 2, end, cmp);
        }
    };

    template <bool Up>
    struct BitonicMerge<1, Up> {
        template <typename Iterator, typename Cmp>
        static void unroll(size_t, Iterator begin, Iterator end, Cmp cmp) {}
        template <typename Iterator, typename Cmp>
        static void iteration(Iterator begin, Iterator end, Cmp cmp) {}
    };

    template <typename Iterator, typename Cmp, bool Up>
    void bitonic_merge(Iterator begin, Iterator end, Cmp cmp) {
        auto d = end - begin;
        if (d <= 1) return;

        if (d <= 128 && (d & (d - 1)) == 0) {
            BitonicMerge<128, Up>::unroll(d, begin, end, cmp);
            return;
        }

        // Compute the greatest power of two less than d
        auto n = 1 << (31 - bit_scan_reverse((unsigned int)d));
        n = n >= d ? n >> 1 : n;
        auto k = d - n;
        auto middle = begin + n;
        auto it1 = begin;
        auto it2 = middle;
        for (int i = 0; i < k; i++) {
            if (cmp(*it1, *it2) != Up) std::iter_swap(it1, it2);
            it1++, it2++;
        }

        constexpr size_t parallel_threshold = 10000;
        #pragma omp task final(d < parallel_threshold) firstprivate(begin, middle, cmp)
        { detail::bitonic_merge<Iterator, Cmp, Up>(begin, middle, cmp); }
        #pragma omp task final(d < parallel_threshold) firstprivate(middle, end, cmp)
        { detail::bitonic_merge<Iterator, Cmp, Up>(middle, end, cmp); }
    }
}

/// Transform a bitonic array into a sorted array.
template <typename Iterator, typename Cmp = std::less<typename std::iterator_traits<Iterator>::value_type>, bool Up = true>
void bitonic_merge(Iterator begin, Iterator end, Cmp cmp) {
    #pragma omp parallel if(omp_get_level() == 0)
    {
        #pragma omp single nowait
        { detail::bitonic_merge<Iterator, Cmp, Up>(begin, end, cmp); }
    }
}

#endif // BITONIC_MERGE_H
