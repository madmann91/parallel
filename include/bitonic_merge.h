#ifndef BITONIC_MERGE_H
#define BITONIC_MERGE_H

#include "bit_scan.h"

namespace detail {
    template <typename Iterator, typename Cmp, bool Up>
    void bitonic_merge(Iterator begin, Iterator end, Cmp cmp) {
        auto d = end - begin;
        if (d <= 1) return;

        // Compute the greatest power of two less than d
        auto n = 1 << (31 - bit_scan_reverse((unsigned int)d));
        n = n >= d ? n >> 1 : n;
        auto k = d - n;
        auto middle = begin + n;
        auto it1 = begin;
        auto it2 = middle;
        for (int i = 0; i < k; i++) {
            if (Up ^ cmp(*it1, *it2)) std::iter_swap(it1, it2);
            it1++, it2++;
        }

        constexpr size_t parallel_threshold = 10000;
        #pragma omp task final(d < parallel_threshold) firstprivate(begin, middle, cmp)
        { detail::bitonic_merge<Iterator, Cmp, Up>(begin, middle, cmp); }
        #pragma omp task final(d < parallel_threshold) firstprivate(begin, middle, cmp)
        { detail::bitonic_merge<Iterator, Cmp, Up>(middle, middle + k, cmp); }
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
