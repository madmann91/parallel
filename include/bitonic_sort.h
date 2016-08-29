#ifndef BITONIC_SORT_H
#define BITONIC_SORT_H

#include "bitonic_merge.h"

namespace detail {
    template <typename Iterator, typename Cmp, bool Up = true>
    void bitonic_sort(Iterator begin, Iterator end, Cmp cmp = Cmp()) {
        auto d = end - begin;    
        if (d <= 1) return;
        
        auto middle = begin + d / 2;
        constexpr size_t parallel_threshold = 10000;
        #pragma omp taskgroup
        {
            #pragma omp task final(d < parallel_threshold) firstprivate(begin, middle, cmp)
            { detail::bitonic_sort<Iterator, Cmp, !Up>(begin, middle, cmp); }
            #pragma omp task final(d < parallel_threshold) firstprivate(begin, middle, cmp)
            { detail::bitonic_sort<Iterator, Cmp, Up>(middle, end, cmp); }
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
