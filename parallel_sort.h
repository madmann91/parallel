#ifndef PARALLEL_SORT_H
#define PARALLEL_SORT_H

#include <algorithm>
#include <iterator>

#include <omp.h>

#include "parallel_merge.h"
#include "shell_sort.h"

namespace detail {

template <typename Iterator, typename Cmp>
void parallel_sort(Iterator begin, Iterator end, Cmp cmp) {
    auto d = end - begin;
    if (d <= 1) return;

    size_t simple_sort_threshold = 5000;
    if (d < simple_sort_threshold) {
        shell_sort(begin, end, cmp);
        return;
    }

    auto middle = begin + d / 2;

    constexpr size_t parallel_threshold = 10000;
    #pragma omp taskgroup
    {
        #pragma omp task final(d < parallel_threshold) firstprivate(begin, middle, cmp)
        { parallel_sort(begin, middle, cmp); }
        { parallel_sort(middle, end, cmp); }
    }

    parallel_merge(begin, middle, end, cmp);
}

} // namespace detail

/// Parallel in-place merge sort.
template <typename Iterator, typename Cmp = std::less<typename std::iterator_traits<Iterator>::value_type> >
void parallel_sort(Iterator begin, Iterator end, Cmp cmp = Cmp()) {
    #pragma omp parallel if(omp_get_level() == 0)
    {
        #pragma omp single nowait
        { detail::parallel_sort(begin, end, cmp); }
    }
}

#endif // PARALLEL_SORT_H
