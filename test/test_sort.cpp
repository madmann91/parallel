#include <iostream>
#include <vector>
#include <chrono>

#include <tbb/tbb.h>
#include "parallel_stable_sort.h"

namespace par {
#include "block_swap.h"
#include "parallel_sort.h"
}
using namespace par;

struct Thingy {
    int value;
    Thingy() {}
    Thingy(int i) : value(i) {}
    Thingy& operator = (int i) { value = i; return *this; }
    bool operator < (const Thingy& other) const { return value > other.value; }
    bool operator == (const Thingy& other) const { return value == other.value; }
};

std::ostream& operator << (std::ostream& os, const Thingy& thingy) {
    os << thingy.value;
    return os;
}

template <typename Iterator>
Iterator find_difference(Iterator begin, Iterator end) {
    for (auto it = std::next(begin), prev = begin; it != end; ++prev, ++it) {
        if (*it < *prev) return it;
    }
    return end;
}

template <typename Iterator, typename Cmp, bool Up = true>
void bitonic_merge(Iterator begin, Iterator end, Cmp cmp) {
    auto d = end - begin;
    if (d <= 1) return;

    // Compute the greatest power of two less than d
    auto n = 1 << (31 - __builtin_clz((unsigned int)d));
    n = n >= d ? n >> 1 : n;
    auto k = d - n;
    auto middle = begin + n;
    auto it1 = begin;
    auto it2 = middle;
    for (int i = 0; i < k; i++) {
        if (Up ^ cmp(*it1, *it2)) std::iter_swap(it1, it2);
        it1++, it2++;
    }
    
    bitonic_merge<Iterator, Cmp, Up>(begin, middle, cmp);
    bitonic_merge<Iterator, Cmp, Up>(middle, middle + k, cmp);
}

template <typename Iterator, typename Cmp = std::less<typename std::iterator_traits<Iterator>::value_type>, bool Up = true>
void bitonic_sort(Iterator begin, Iterator end, Cmp cmp = Cmp()) {
    auto d = end - begin;    
    if (d <= 1) return;
    
    auto m = begin + d / 2;
    bitonic_sort<Iterator, Cmp, !Up>(begin, m, cmp);
    bitonic_sort<Iterator, Cmp, Up>(m, end, cmp);
    bitonic_merge<Iterator, Cmp, Up>(begin, end, cmp);
}

int main(int argc, char** argv) {
    std::vector<Thingy> values(1028*1024);
    for (auto& v : values) v = rand();

    std::cout << "Testing..." << std::endl;

    bitonic_sort(values.begin(), values.end());
    if (find_difference(values.begin(), values.end()) != values.end()) {
        std::cout << "NOT SORTED" << std::endl;
        for (int i = 0; i < values.size() && i < 100; i++) {
            std::cout << values[i] << " ";
        }
        std::cout << "..." << std::endl;

        std::cout << find_difference(values.begin(), values.end()) - values.begin() << std::endl;
    }

    std::cout << "Benchmarking..." << std::endl;

    {
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < 100; i++) {
            std::sort(values.begin(), values.end());
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "std::sort: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms" << std::endl;
    }

    {
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < 100; i++) {
            bitonic_sort(values.begin(), values.end());
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "parallel_sort: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms" << std::endl;
    }

    {
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < 100; i++) {
            tbb::parallel_sort(values.begin(), values.end());
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "tbb::parallel_sort: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms" << std::endl;
    }

    {
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < 100; i++) {
            pss::parallel_stable_sort(values.begin(), values.end(), std::less<Thingy>());
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "pss::parallel_stable_sort: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms" << std::endl;
    }

    std::vector<int> p{1, 2, 3, 4, 5, 6, 7, 8};
    block_swap(p.begin(), p.end() - 5, p.end());
    for (auto i : p) std::cout << i << " ";
    std::cout << std::endl;
}
