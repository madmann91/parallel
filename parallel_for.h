#ifndef PARALLEL_FOR_H
#define PARALLEL_FOR_H

/// Parallel for loop over a range defined by iterators.
template <typename Iterator, typename F>
void parallel_for(Iterator begin, Iterator end, F f) {
    #pragma omp parallel if(omp_get_level() == 0)
    {
        #pragma omp for
        for (auto it = begin; it != end; ++it) {
            f(*it);
        }
    }
}

#endif // PARALLEL_FOR_H
