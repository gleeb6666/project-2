#pragma once
#include <cstddef>
#include <numeric>
#include <thread>
#include <vector>

template <class T>
long long parallelInnerProduct(const std::vector<T>& a, const std::vector<T>& b, std::size_t k) {
    const std::size_t n = a.size();
    std::vector<long long> partial(k, 0);
    std::vector<std::thread> threads;
    threads.reserve(k);
    for (std::size_t i = 0; i < k; ++i) {
        const std::size_t lo = n * i / k;
        const std::size_t hi = n * (i + 1) / k;
        threads.emplace_back([&, i, lo, hi] {
            partial[i] = std::inner_product(a.begin() + lo, a.begin() + hi, b.begin() + lo, 0LL);
        });
    }
    for (auto& t : threads) t.join();
    return std::accumulate(partial.begin(), partial.end(), 0LL);
}
