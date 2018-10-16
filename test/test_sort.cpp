#include <iostream>
#include <vector>
#include <chrono>
#include <functional>

#ifdef BENCH_TBB
#include <tbb/tbb.h>
#endif

#include "shell_sort.h"
#include "inplace_merge_sort.h"
#include "inplace_radix_sort.h"

struct Thingy {
    int value;
    int index;
    Thingy() {}
    Thingy(int i, int j) : value(i), index(j) {}
    Thingy& operator = (int i) { value = i; return *this; }
    bool operator < (const Thingy& other) const { return value < other.value; }
    bool operator == (const Thingy& other) const { return value == other.value; }
    int operator & (int mask) const { return value & mask; }
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

template <typename T> using container_type = std::vector<T>;
template <typename T> using iterator_type = typename container_type<T>::iterator;
template <typename T> using sort_fn = std::function<void (iterator_type<T>, iterator_type<T>)>;

template <typename T>
void check_sort(const std::string& name, sort_fn<T> sort, iterator_type<T> begin, iterator_type<T> end) {
    std::vector<T> tmp_values(begin, end);
    sort(tmp_values.begin(), tmp_values.end());

    bool stable = true;
    for (auto it1 = tmp_values.begin(), it2 = tmp_values.begin() + 1; it2 != tmp_values.end(); ++it1, ++it2) {
        if (it1->value == it2->value && it1->index > it2->index) {
            stable = false;
            break;
        }
    }

    auto diff = find_difference(tmp_values.begin(), tmp_values.end());
    if (diff != tmp_values.end()) {
        std::cerr << name << " is incorrect." << std::endl;
        std::cerr << "index of the first difference: " << diff - tmp_values.begin() << std::endl;
        std::cerr << "broken sequence: ";
        auto it = std::max(diff - 10, tmp_values.begin());
        if (it != tmp_values.begin()) std::cerr << "... ";
        for (int i = 0; i < 20 && it != tmp_values.end(); i++, it++) {
            if (it == diff) std::cerr << "[";
            std::cerr << *it;
            if (it == diff) std::cerr << "]";
            std::cerr << " ";
        }
        if (it != tmp_values.end()) std::cerr << "...";
        std::cerr << std::endl;
    } else {
        std::cout << name << " has been tested successfully, and ";
        if (stable) std::cout << "seems to be stable." << std::endl;
        else          std::cout << "is instable." << std::endl;
    }
}

template <typename T>
void bench_sort(const std::string& name, sort_fn<T> sort, iterator_type<T> begin, iterator_type<T> end) {
    using namespace std::chrono;
    std::vector<T> tmp_values(end - begin);

    microseconds total(0);
    for (int i = 0; i < 100; i++) {
        std::copy(begin, end, tmp_values.begin());
        auto start = high_resolution_clock::now();
        sort(tmp_values.begin(), tmp_values.end());
        auto end = high_resolution_clock::now();
        total += duration_cast<microseconds>(end - start);
    }
    std::cout << name << ": " << duration_cast<milliseconds>(total).count() << " ms" << std::endl;
}

template <typename Iterator, typename Pred>
Iterator pn(Iterator begin, Iterator end, Pred pred) {
    size_t count = 0;
    for (auto it = begin; it != end; ++it) count += pred(*it);

    for (auto it1 = begin, it2 = begin + count; it2 != end;) {
        for (auto it = begin; it != end; it++) std::cout << *it << " ";
        std::cout << std::endl;

        if (pred(*it1)) ++it1;
        else std::iter_swap(it1, it2++);
    }
    return begin + count;
}

int main(int argc, char** argv) {
    container_type<Thingy> values(100000);
    int k = 0;
    for (auto& v : values) { v = rand(); v.index = k++; }

    std::cout << "=========== Testing ============" << std::endl;

    auto shell_sort_fn         = [] (iterator_type<Thingy> begin, iterator_type<Thingy> end) { shell_sort(begin, end); };
    auto inplace_merge_sort_fn = [] (iterator_type<Thingy> begin, iterator_type<Thingy> end) { inplace_merge_sort(begin, end); };
    auto inplace_radix_sort_fn = [] (iterator_type<Thingy> begin, iterator_type<Thingy> end) { inplace_radix_sort<iterator_type<Thingy>, int>(begin, end); };
    auto std_sort_fn           = [] (iterator_type<Thingy> begin, iterator_type<Thingy> end) { std::sort(begin, end); };
#ifdef BENCH_TBB
    auto tbb_parallel_sort_fn  = [] (iterator_type<Thingy> begin, iterator_type<Thingy> end) { tbb::parallel_sort(begin, end); };
#endif

    check_sort<Thingy>("shell_sort", shell_sort_fn, values.begin(), values.end());
    check_sort<Thingy>("inplace_merge_sort", inplace_merge_sort_fn, values.begin(), values.end());
    check_sort<Thingy>("inplace_radix_sort", inplace_radix_sort_fn, values.begin(), values.end());

    std::cout << "========= Benchmarking =========" << std::endl;

    bench_sort<Thingy>("std::sort", std_sort_fn, values.begin(), values.end());
#ifdef BENCH_TBB
    bench_sort<Thingy>("tbb::parallel_sort", tbb_parallel_sort_fn, values.begin(), values.end());
#endif
    bench_sort<Thingy>("inplace_merge_sort", inplace_merge_sort_fn, values.begin(), values.end());
    bench_sort<Thingy>("inplace_radix_sort", inplace_radix_sort_fn, values.begin(), values.end());
}
