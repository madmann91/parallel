#ifndef PARALLEL_H
#define PARALLEL_H

#include <algorithm>
#include <iterator>

#include <omp.h>

#include "parallel_merge.h"

namespace detail {

template <typename Iterator, typename Cmp>
void parallel_sort(Iterator begin, Iterator end, Cmp cmp, size_t parallel_threshold) {
    auto d = std::distance(begin, end);
    if (d > parallel_threshold) {
        auto middle = std::next(begin, d / 2);

        #pragma omp task
        { parallel_sort(begin, middle, cmp, parallel_threshold); }
        #pragma omp task
        { parallel_sort(middle, end, cmp, parallel_threshold); }

        #pragma omp taskwait

        detail::parallel_merge(begin, middle, end, cmp, parallel_threshold);
    } else {
        std::sort(begin, end, cmp);
    }
}

} // namespace detail

/// Parallel sort using std::sort.
template <typename Iterator, typename Cmp = std::less<typename std::iterator_traits<Iterator>::value_type> >
void parallel_sort(Iterator begin, Iterator end, Cmp cmp = Cmp()) {
    constexpr size_t parallel_threshold = 1000;

    #pragma omp parallel if(omp_get_level() == 0)
    {
        #pragma omp single
        { detail::parallel_sort(begin, end, cmp, parallel_threshold); }
    }
}

#endif // PARALLEL_H
