// SPDX-License-Identifier: MIT

// This file defines the command-line interface for the Hausdorff distance benchmarking tool
// It includes functions for parsing command-line arguments, validating input, and printing usage information.
// SPDX-License-Identifier: MIT

/*
    BenchmarkCli.h

    Command-line interface declarations for the Hausdorff distance benchmark tool.

    This file defines benchmark options, supported algorithm identifiers,
    and command-line parsing entry points.
*/

#pragma once

#include <string>
#include <vector>

namespace HausdorffDistanceCliBenchmark {

    enum class BenchmarkAlgorithm
    {
        BruteForce,
        Shuffle,
        ContourCoherent,
        Boost,
        KDTree
    };

    struct BenchmarkOptions
    {
        std::vector<std::string> folders;
        int batches = 20;
        std::vector<BenchmarkAlgorithm> algorithms;
    };

    void PrintUsage();

    BenchmarkOptions ParseCommandLine(int argc, char* argv[]);

} // namespace HausdorffDistanceCliBenchmark