#pragma once
// Input file format (plain text, UTF-8/ASCII):
//   line 1      : N - the number of elements (positive integer)
//   next N lines: one integer per line, value in [-1000; 1000]
// For every length N two files are produced: a_<N>.txt and b_<N>.txt
// (the two vectors whose scalar product is computed).

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <random>
#include <string>
#include <vector>

namespace seqio {

using Sequence = std::vector<int>;

inline const std::vector<std::size_t>& standardSizes() {
    static const std::vector<std::size_t> sizes{10'000, 100'000, 1'000'000, 10'000'000};
    return sizes;
}

inline std::filesystem::path fileFor(const std::filesystem::path& dir, char name, std::size_t n) {
    return dir / (std::string(1, name) + "_" + std::to_string(n) + ".txt");
}

inline void writeSequence(const std::filesystem::path& path, const Sequence& s) {
    std::ofstream out(path);
    out << s.size() << '\n';
    for (int v : s) out << v << '\n';
}

inline Sequence readSequence(const std::filesystem::path& path) {
    std::ifstream in(path);
    std::size_t n = 0;
    in >> n;
    Sequence s(n);
    for (int& v : s) in >> v;
    return s;
}

inline void generateAll(const std::filesystem::path& dir) {
    std::filesystem::create_directories(dir);
    std::mt19937 gen(20240613);
    std::uniform_int_distribution<int> dist(-1000, 1000);
    for (std::size_t n : standardSizes()) {
        for (char name : {'a', 'b'}) {
            Sequence s(n);
            for (int& v : s) v = dist(gen);
            writeSequence(fileFor(dir, name, n), s);
        }
    }
}

}  // namespace seqio
