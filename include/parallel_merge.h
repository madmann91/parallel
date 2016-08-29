#ifndef PARALLEL_MERGE_H
#define PARALLEL_MERGE_H

#include <algorithm>
#include <iterator>

#include "block_swap.h"

namespace detail {

template <typename Iterator, typename Cmp>
void parallel_merge(Iterator a, Iterator b, Iterator c, Cmp cmp) {
    auto d1 = b - a;
    auto d2 = c - b;
    if (d1 <= 0 || d2 <= 0) return;

    Iterator m, p1, p2;
    if (d1 > d2) {
        auto k = a + d1 / 2;
        auto it = std::upper_bound(b, c, *k, cmp);

        m = k + (it - b);
        p1 = k;
        p2 = it;
    } else {
        auto k = b + d2 / 2;
        auto it = std::upper_bound(a, b, *k, cmp);

        m = it + (k - b);
        p1 = it;
        p2 = k + 1;
    }

    block_swap(p1, b, p2);

    constexpr size_t parallel_threshold = 10000;
    #pragma omp task final(d1 + d2 < parallel_threshold) firstprivate(a, p1, m, cmp)
    { parallel_merge(a, p1, m, cmp); }

    parallel_merge(std::next(m), p2, c, cmp);
}

} // namespace detail

/// Parallel in-place merge algorithm.
template <typename Iterator, typename Cmp = std::less<typename std::iterator_traits<Iterator>::value_type> >
void parallel_merge(Iterator a, Iterator b, Iterator c, Cmp cmp = Cmp()) {
    #pragma omp parallel if(omp_get_level() == 0)
    {
        #pragma omp single nowait
        {
            #pragma omp taskgroup
            { detail::parallel_merge(a, b, c, cmp); }
        }
    }
}

#endif // PARALLEL_MERGE_H
