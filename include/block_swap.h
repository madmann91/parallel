#ifndef BLOCK_SWAP_H
#define BLOCK_SWAP_H

#include <algorithm>
#include <iterator>
#include <cassert>

/// Gries and Mills block swapping algorithm.
template <typename Iterator>
void block_swap(Iterator a, Iterator b, Iterator c) {
    auto d1 = b - a;
    auto d2 = c - b;

    while (std::min(d1, d2) > 0) {
        if (d1 < d2) {
            auto m = c - d1;
            for (auto it1 = a, it2 = m; it1 != b; ++it1, ++it2) std::iter_swap(it1, it2);
            c = m;
            d2 = c - b;
        } else {
            for (auto it1 = a, it2 = b; it2 != c; ++it1, ++it2) std::iter_swap(it1, it2);
            a = a + d2;
            d1 = b - a;
        }
    }
}

#endif // BLOCK_SWAP_H
