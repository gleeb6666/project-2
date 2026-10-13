#pragma once
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <vector>

namespace timing {

inline int repetitionsFor(std::size_t n) {
    return static_cast<int>(std::clamp<std::size_t>(50'000'000 / n, 5, 200));
}

template <class F>
double medianMillis(F&& f, int reps) {
    std::vector<double> times;
    times.reserve(reps);
    for (int i = 0; i < reps; ++i) {
        const auto start = std::chrono::steady_clock::now();
        f();
        const auto stop = std::chrono::steady_clock::now();
        times.push_back(std::chrono::duration<double, std::milli>(stop - start).count());
    }
    std::nth_element(times.begin(), times.begin() + times.size() / 2, times.end());
    return times[times.size() / 2];
}

}  // namespace timing
