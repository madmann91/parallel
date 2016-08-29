#include <iostream>
#include <vector>
#include <chrono>
#include <functional>

#ifdef BENCH_TBB
#include <tbb/tbb.h>
#endif

#include "inplace_merge_sort.h"
#include "bitonic_sort.h"

struct Thingy {
    int value;
    int index;
    Thingy() {}
    Thingy(int i, int j) : value(i), index(j) {}
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

template <typename T> using container_type = std::vector<T>;
template <typename T> using iterator_type = typename container_type<T>::iterator;
template <typename T> using comparator = std::less<T>;
template <typename T> using sort_fn = std::function<void (iterator_type<T>, iterator_type<T>, comparator<T>)>;

template <typename T>
void check_sort(const std::string& name, sort_fn<T> sort, iterator_type<T> begin, iterator_type<T> end, comparator<T> cmp = comparator<T>()) {
    std::vector<T> tmp_values(begin, end);
    sort(tmp_values.begin(), tmp_values.end(), cmp);
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
    }
}

template <typename T>
void bench_sort(const std::string& name, sort_fn<T> sort, iterator_type<T> begin, iterator_type<T> end, comparator<T> cmp = comparator<T>()) {
    using namespace std::chrono;
    std::vector<T> tmp_values(end - begin);

    microseconds total(0);
    for (int i = 0; i < 100; i++) {
        std::copy(begin, end, tmp_values.begin());
        auto start = high_resolution_clock::now();
        sort(tmp_values.begin(), tmp_values.end(), cmp);
        auto end = high_resolution_clock::now();
        total += duration_cast<microseconds>(end - start);
    }
    std::cout << name << ": " << duration_cast<milliseconds>(total).count() << " ms" << std::endl;
}

#ifdef BENCH_TBB
template <typename T>
void tbb_wrapper(iterator_type<T> begin, iterator_type<T> end, comparator<T> cmp) {
    tbb::parallel_sort<iterator_type<T>, comparator<T> >(begin, end, cmp);
}
#endif

int main(int argc, char** argv) {
    container_type<Thingy> values(1000000);
    int k = 0;
    for (auto& v : values) { v = rand(); v.index = k++; }

    std::cout << "=========== Testing ============" << std::endl;

    auto shell_sort_fn = shell_sort<iterator_type<Thingy>, comparator<Thingy> >;
    auto bitonic_sort_fn = bitonic_sort<iterator_type<Thingy>, comparator<Thingy> >;
    auto inplace_merge_sort_fn = inplace_merge_sort<iterator_type<Thingy>, comparator<Thingy> >;
    auto std_sort_fn = std::sort<iterator_type<Thingy>, comparator<Thingy> >;

    check_sort<Thingy>("shell_sort", shell_sort_fn, values.begin(), values.end());
    check_sort<Thingy>("bitonic_sort", bitonic_sort_fn, values.begin(), values.end());
    check_sort<Thingy>("inplace_merge_sort", inplace_merge_sort_fn, values.begin(), values.end());

    std::cout << "========= Benchmarking =========" << std::endl;

    bench_sort<Thingy>("std::sort", std_sort_fn, values.begin(), values.end());
#ifdef BENCH_TBB
    bench_sort<Thingy>("tbb::parallel_sort", tbb_wrapper<Thingy>, values.begin(), values.end());
#endif
    bench_sort<Thingy>("shell_sort", shell_sort_fn, values.begin(), values.end());
    bench_sort<Thingy>("bitonic_sort", bitonic_sort_fn, values.begin(), values.end());
    bench_sort<Thingy>("inplace_merge_sort", inplace_merge_sort_fn, values.begin(), values.end());
}
