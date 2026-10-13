#pragma once
#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <execution>
#include <filesystem>
#include <functional>
#include <numeric>
#include <set>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "parallel_inner_product.hpp"
#include "sequence_io.hpp"
#include "timing.hpp"

namespace experiments {

struct NamedMethod {
    std::string name;
    std::function<long long()> run;
};

struct SizeResult {
    std::size_t n = 0;
    std::vector<double> libraryMs;
    std::vector<double> ownMs;
};

inline std::vector<std::string> libraryMethodNames() {
    return {"inner_product", "TR seq", "TR unseq", "TR par", "TR par_unseq"};
}

inline std::vector<NamedMethod> libraryMethods(const seqio::Sequence& a, const seqio::Sequence& b) {
    auto tr = [&](auto policy) {
        return [&a, &b, policy] { return std::transform_reduce(policy, a.begin(), a.end(), b.begin(), 0LL); };
    };
    return {
        {"inner_product", [&] { return std::inner_product(a.begin(), a.end(), b.begin(), 0LL); }},
        {"TR seq", tr(std::execution::seq)},
        {"TR unseq", tr(std::execution::unseq)},
        {"TR par", tr(std::execution::par)},
        {"TR par_unseq", tr(std::execution::par_unseq)},
    };
}

inline std::size_t hardwareThreads() { return std::max(1u, std::thread::hardware_concurrency()); }

inline std::vector<std::size_t> kValues() {
    std::set<std::size_t> ks{1, 2, 3, 4, 6, 8, 12, 16, 24, 32, 48, 64, 128, 256, 512, 1024};
    for (std::size_t m : {1, 2, 4, 8}) ks.insert(hardwareThreads() * m);
    return {ks.begin(), ks.end()};
}

inline double measure(const NamedMethod& m, long long expected, int reps) {
    if (m.run() != expected) throw std::logic_error("wrong result in " + m.name);
    long long sink = 0;
    const double ms = timing::medianMillis([&] { sink += m.run(); }, reps);
    return sink == 1 ? -ms : ms;
}

inline SizeResult runForSize(const std::filesystem::path& dir, std::size_t n) {
    const auto a = seqio::readSequence(seqio::fileFor(dir, 'a', n));
    const auto b = seqio::readSequence(seqio::fileFor(dir, 'b', n));
    const long long expected = std::inner_product(a.begin(), a.end(), b.begin(), 0LL);
    const int reps = timing::repetitionsFor(n);

    SizeResult r;
    r.n = n;
    for (const auto& m : libraryMethods(a, b)) r.libraryMs.push_back(measure(m, expected, reps));
    for (std::size_t k : kValues()) {
        const NamedMethod own{"own K=" + std::to_string(k), [&a, &b, k] { return parallelInnerProduct(a, b, k); }};
        r.ownMs.push_back(measure(own, expected, reps));
    }
    return r;
}

inline std::size_t bestIndex(const std::vector<double>& times) {
    return static_cast<std::size_t>(std::min_element(times.begin(), times.end()) - times.begin());
}

inline void printLibraryTable(const std::vector<SizeResult>& results) {
    std::puts("\n== 1-2. Library algorithms, time in ms (median) ==");
    std::printf("%12s", "N");
    for (const auto& name : libraryMethodNames()) std::printf("%15s", name.c_str());
    std::puts("");
    for (const auto& r : results) {
        std::printf("%12zu", r.n);
        for (double ms : r.libraryMs) std::printf("%15.4f", ms);
        std::puts("");
    }
}

inline void printOwnTable(const std::vector<SizeResult>& results) {
    std::puts("\n== 3. Own parallel algorithm, time in ms (median) depending on K ==");
    std::printf("%8s", "K");
    for (const auto& r : results) std::printf("%14zu", r.n);
    std::puts("");
    const auto ks = kValues();
    for (std::size_t i = 0; i < ks.size(); ++i) {
        std::printf("%8zu", ks[i]);
        for (const auto& r : results) std::printf("%14.4f", r.ownMs[i]);
        std::puts("");
    }
}

inline void printBestK(const std::vector<SizeResult>& results) {
    const std::size_t hw = hardwareThreads();
    const auto ks = kValues();
    std::printf("\n== Best K (hardware threads = %zu) ==\n", hw);
    std::printf("%12s%10s%14s%12s\n", "N", "best K", "time, ms", "K/threads");
    for (const auto& r : results) {
        const std::size_t i = bestIndex(r.ownMs);
        std::printf("%12zu%10zu%14.4f%12.2f\n", r.n, ks[i], r.ownMs[i], static_cast<double>(ks[i]) / hw);
    }
}

}  // namespace experiments
