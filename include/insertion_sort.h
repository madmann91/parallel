#ifndef INSERTION_SORT_H
#define INSERTION_SORT_H

#include <iterator>

/// Insertion sort.
template <typename Iterator, typename Cmp = std::less<typename std::iterator_traits<Iterator>::value_type> >
void insertion_sort(Iterator begin, Iterator end, Cmp cmp = Cmp()) {
    for (auto it1 = begin + 1; it1 < end; ++it1) {
        auto tmp = std::move(*it1);

        auto it2 = it1;
        for (; it2 > begin && cmp(tmp, *(it2 - 1)); it2--) {
            *it2 = std::move(*(it2 - 1));
        }

        *it2 = std::move(tmp);
    }
}

#endif // INSERTION_SORT
