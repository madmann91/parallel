#ifndef BITONIC_SORT_H
#define BITONIC_SORT_H

#ifdef _OPENMP
#include <omp.h>
#endif

#include "bitonic_merge.h"

namespace detail {
    template <typename Iterator, typename Cmp, bool Up = true>
    void bitonic_sort(Iterator begin, Iterator end, Cmp cmp = Cmp()) {
        auto d = end - begin;
        if (d <= 1) return;

        typedef typename std::iterator_traits<Iterator>::value_type T;
        constexpr size_t simple_sort_threshold = 50000;
        if (d < simple_sort_threshold) {
            std::sort(begin, end, [=] (const T& a, const T& b) { return cmp(a, b) == Up; });
            return;
        }
        
        auto middle = begin + d / 2;
        constexpr size_t parallel_threshold = 50000;
        #pragma omp taskgroup
        {
            #pragma omp task final(d < parallel_threshold) firstprivate(begin, middle, cmp) mergeable
            { detail::bitonic_sort<Iterator, Cmp, !Up>(begin, middle, cmp); }
            { detail::bitonic_sort<Iterator, Cmp,  Up>(middle, end, cmp); }
        }

        detail::bitonic_merge<Iterator, Cmp, Up>(begin, end, cmp);
    }
}

/// Sorts an array in place using bitonic sort.
template <typename Iterator, typename Cmp = std::less<typename std::iterator_traits<Iterator>::value_type>>
void bitonic_sort(Iterator begin, Iterator end, Cmp cmp = Cmp()) {
    #pragma omp parallel if(omp_get_level() == 0)
    {
        #pragma omp single nowait
        { detail::bitonic_sort(begin, end, cmp); }
    }
}

#endif // BITONIC_SORT_H
