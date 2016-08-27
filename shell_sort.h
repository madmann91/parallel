#ifndef SHELL_SORT_H
#define SHELL_SORT_H

#include <iterator>
#include <array>

/// Shell sort with Marcin Ciura's sequence.
template <typename Iterator, typename Cmp = std::less<typename std::iterator_traits<Iterator>::value_type> >
void shell_sort(Iterator begin, Iterator end, Cmp cmp = Cmp()) {
    std::array<int, 8> gaps{701, 301, 132, 57, 23, 10, 4, 1};

    for (auto g : gaps) {
        auto first = begin + g;
        for (auto it1 = first; it1 < end; ++it1) {
            auto tmp = *it1;

            auto it2 = it1;
            for (; it2 >= first && cmp(tmp, *(it2 - g)); it2 -= g) {
                *it2 = *(it2 - g);
            }

            *it2 = tmp;
        } 
    }
}

#endif // SHELL_SORT_H
