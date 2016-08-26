#ifndef BLOCK_SWAP_H
#define BLOCK_SWAP_H

#include <algorithm>
#include <iterator>
#include <cassert>

/// Gries and Mills block swapping algorithm.
template <typename Iterator>
void block_swap(Iterator a, Iterator b, Iterator c) {
    auto d1 = std::distance(a, b);
    auto d2 = std::distance(b, c);

    assert(d1 <= std::distance(a, c) &&
           d2 <= std::distance(a, c) &&
           std::distance(a, c) >= 0);

    while (std::min(d1, d2) > 0) {
        if (d1 < d2) {
            auto m = std::prev(c, d1);
            for (auto it1 = a, it2 = m; it1 != b; ++it1, ++it2) std::iter_swap(it1, it2);
            c = m;
            d2 = std::distance(b, c);
        } else {
            for (auto it1 = a, it2 = b; it2 != c; ++it1, ++it2) std::iter_swap(it1, it2);
            a = std::next(a, d2);
            d1 = std::distance(a, b);
        }
    }
}

#endif // BLOCK_SWAP_H
