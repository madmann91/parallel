#ifndef PARALLEL_MERGE_H
#define PARALLEL_MERGE_H

#include <algorithm>
#include <iterator>

#include "block_swap.h"

namespace detail {

template <typename Iterator, typename Cmp>
void parallel_merge(Iterator a, Iterator b, Iterator c, Cmp cmp, size_t parallel_threshold) {
    auto d1 = std::distance(a, b);
    auto d2 = std::distance(b, c);
    if (d1 <= 0 || d2 <= 0) return;

    if (d1 + d2 < parallel_threshold) {
        std::inplace_merge(a, b, c, cmp);
        return;
    }

    Iterator m, p1, p2, q;
    if (d1 > d2) {
        auto k = std::next(a, d1 / 2);
        auto it = std::upper_bound(b, c, *k, cmp);

        m = std::next(k, std::distance(b, it));
        p1 = k;
        p2 = it;
        q = it;
    } else {
        auto k = std::next(b, d2 / 2);
        auto it = std::upper_bound(a, b, *k, cmp);

        m = std::next(it, std::distance(b, k));
        p1 = it;
        p2 = k;
        q = std::next(k);
    }

    block_swap(p1, b, q);
    #pragma omp task
    { parallel_merge(a, p1, m, cmp, parallel_threshold); }
    #pragma omp task
    { parallel_merge(std::next(m), p2, c, cmp, parallel_threshold); }
}

} // namespace detail

/// Parallel in-place merge algorithm.
template <typename Iterator, typename Cmp = std::less<typename std::iterator_traits<Iterator>::value_type> >
void parallel_merge(Iterator a, Iterator b, Iterator c, Cmp cmp = Cmp()) {
    constexpr size_t parallel_threshold = 1000;

    #pragma omp parallel if(omp_get_level() == 0)
    {
        #pragma omp single
        { detail::parallel_merge(a, b, c, cmp, parallel_threshold); }
    }
}

#endif // PARALLEL_MERGE_H
