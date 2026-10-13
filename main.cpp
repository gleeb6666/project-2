// Variant 6: scalar product (std::inner_product / std::transform_reduce).
// Compiler: MSVC (Visual Studio 2022 Build Tools), /std:c++20 /EHsc /MD, Release.
//   optimised build:   cl /std:c++20 /EHsc /MD /O2 main.cpp
//   unoptimised build: cl /std:c++20 /EHsc /MD /Od main.cpp
// Usage:
//   main.exe gen [dir]            - generate input files a_<N>.txt, b_<N>.txt in dir (default "data")
//   main.exe run <label> [dir]    - run experiments; <label> (e.g. O2, Od) is printed in the header
// Input file format is described in sequence_io.hpp.

#include <cstdio>
#include <filesystem>
#include <string>
#include <thread>
#include <vector>

#include "experiments.hpp"
#include "sequence_io.hpp"

int main(int argc, char** argv) {
    const std::string mode = argc > 1 ? argv[1] : "run";
    if (mode == "gen") {
        const std::filesystem::path dir = argc > 2 ? argv[2] : "data";
        seqio::generateAll(dir);
        std::printf("Generated input files in %s\n", dir.string().c_str());
        return 0;
    }

    const std::string label = argc > 2 ? argv[2] : "unknown";
    const std::filesystem::path dir = argc > 3 ? argv[3] : "data";
    std::printf("Scalar product benchmark, optimisation level: %s, hardware threads: %zu\n", label.c_str(),
                experiments::hardwareThreads());

    std::vector<experiments::SizeResult> results;
    for (std::size_t n : seqio::standardSizes()) results.push_back(experiments::runForSize(dir, n));

    experiments::printLibraryTable(results);
    experiments::printOwnTable(results);
    experiments::printBestK(results);
    return 0;
}
