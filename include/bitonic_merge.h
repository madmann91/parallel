#ifndef BITONIC_MERGE_H
#define BITONIC_MERGE_H

#ifdef _OPENMP
#include <omp.h>
#endif

#include "bit_scan.h"

namespace detail {
    template <typename Iterator, typename Cmp, bool Up>
    void bitonic_merge(Iterator begin, Iterator end, Cmp cmp) {
        auto d = end - begin;
        if (d <= 1) return;

        // Compute the greatest power of two less than d
        size_t p = 1ull << (63 - bit_scan_reverse((uint64_t)d));
        while (p >= 1) {
            constexpr size_t elems_per_task = 5000;
            size_t n = d - p;
            #pragma omp taskgroup
            {
                for (size_t i = 0; i < n; i += elems_per_task) {
                    auto it1 = begin + i;
                    auto it2 = it1 + p;
                    size_t m = std::min(elems_per_task, n - i);
                    #pragma omp task firstprivate(it1, it2, m) mergeable untied
                    {
                        for (size_t j = 0; j < m; j++) {
                            if (cmp(*it1, *it2) != Up) std::iter_swap(it1, it2);
                            it1++, it2++;
                        }
                    }
                }
            }
            p >>= 1;
        }
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
