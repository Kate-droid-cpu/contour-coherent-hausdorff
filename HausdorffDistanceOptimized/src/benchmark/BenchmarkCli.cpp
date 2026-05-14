// SPDX-License-Identifier: MIT
// This file implements the command-line interface for the Hausdorff distance benchmarking tool.
// It includes functions for parsing command-line arguments, validating input, and printing usage information.

#include "BenchmarkCli.h"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace HausdorffDistanceCliBenchmark {

    //--------------------------------------------private helper functions--------------------------------------------
    namespace {
        // Helper function to convert a string to lowercase
        std::string toLowerStr(std::string s)
        {
            std::transform(
                s.begin(),
                s.end(),
                s.begin(),
                [](unsigned char c) { return static_cast<char>(std::tolower(c)); }
            );

            return s;
        }

        std::string trim(std::string s)
        {
            const auto first = s.find_first_not_of(" \t\n\r");
            if (first == std::string::npos)
            {
                return {};
            }

            const auto last = s.find_last_not_of(" \t\n\r");
            return s.substr(first, last - first + 1);
        }

        // Helper function to split a comma-separated string into a vector of lowercase strings
        std::vector<std::string> splitByCommaStr(const std::string& text)
        {
            std::vector<std::string> result;
            std::stringstream ss(text);
            std::string item;

            while (std::getline(ss, item, ','))
            {
                item = trim(item);
                if (!item.empty())
                {
                    result.push_back(toLowerStr(item));
                }
            }

            return result;
        }

        // Parses a comma-separated list of algorithm names and returns a vector of BenchmarkAlgorithm enums
        std::vector<BenchmarkAlgorithm> ParseAlgorithms(const std::string& text)
        {
            const auto names = splitByCommaStr(text);

            std::vector<BenchmarkAlgorithm> result;

            for (const std::string& name : names)
            {
                if (name == "all")
                {
                    return {
                        BenchmarkAlgorithm::BruteForce,
                        BenchmarkAlgorithm::Shuffle,
                        BenchmarkAlgorithm::ContourCoherent,
                        BenchmarkAlgorithm::Boost,
                        BenchmarkAlgorithm::KDTree
                    };
                }
                else if (name == "bruteforce" || name == "bf")
                {
                    result.push_back(BenchmarkAlgorithm::BruteForce);
                }
                else if (name == "shuffle" || name == "shuffling")
                {
                    result.push_back(BenchmarkAlgorithm::Shuffle);
                }
                else if (name == "contourcoherent" || name == "cc" || name == "contour")
                {
                    result.push_back(BenchmarkAlgorithm::ContourCoherent);
                }
                else if (name == "boost")
                {
                    result.push_back(BenchmarkAlgorithm::Boost);
                }
                else if (name == "kdtree" || name == "kd")
                {
                    result.push_back(BenchmarkAlgorithm::KDTree);
                }
                else
                {
                    throw std::runtime_error("Unknown algorithm: " + name);
                }
            }

            return result;
        }
	} // namespace

    //---------------------------------------------public API---------------------------------------------
    void PrintUsage()
    {
        std::cout
            << "Usage:\n"
            << "  HausdorffDistanceOptimized.exe "
            << "--folder <path> [--folder <path> ...] "
            << "[--batches N] "
            << "[--algorithms contour,bruteforce,shuffle,boost,kdtree|all]\n\n"
            << "Examples:\n"
            << "  HausdorffDistanceOptimized.exe --folder pathToImages\\snowflake --batches 3 --algorithms contour,shuffle\n"
            << "  HausdorffDistanceOptimized.exe --folder pathToImages\\MPEG7\\children --algorithms all\n\n"
            << "Algorithms:\n"
            << "  contour / cc / contourcoherent\n"
            << "  bruteforce / bf\n"
            << "  shuffle\n"
            << "  boost\n"
            << "  kdtree\n"
            << "  all\n";
    }

    // Parses command-line arguments and returns a BenchmarkOptions struct with the specified options
    BenchmarkOptions ParseCommandLine(int argc, char* argv[])
    {
        BenchmarkOptions options;

        // Default algorithms
        options.algorithms = {
            BenchmarkAlgorithm::BruteForce,
            BenchmarkAlgorithm::Shuffle,
            BenchmarkAlgorithm::ContourCoherent
        };

        for (int i = 1; i < argc; ++i)
        {
            const std::string arg = argv[i];

            if (arg == "--help" || arg == "-h")
            {
                PrintUsage();
                std::exit(0);
            }
            else if (arg == "--folder" || arg == "-f")
            {
                if (i + 1 >= argc)
                {
                    throw std::runtime_error("--folder requires a path");
                }

                options.folders.push_back(argv[++i]);
            }
            else if (arg == "--batches" || arg == "-b")
            {
                if (i + 1 >= argc)
                {
                    throw std::runtime_error("--batches requires a number");
                }

                options.batches = std::stoi(argv[++i]);

                if (options.batches <= 0)
                {
                    throw std::runtime_error("--batches must be positive");
                }
            }
            else if (arg == "--algorithms" || arg == "-a")
            {
                if (i + 1 >= argc)
                {
                    throw std::runtime_error("--algorithms requires a comma-separated list");
                }

                options.algorithms = ParseAlgorithms(argv[++i]);

                if (options.algorithms.empty())
                {
                    throw std::runtime_error("--algorithms cannot be empty");
                }
            }
            else
            {
                throw std::runtime_error("Unknown argument: " + arg);
            }
        }

        if (options.folders.empty())
        {
            throw std::runtime_error("At least one --folder must be specified");
        }

        return options;
    }

} // namespace HausdorffDistanceCliBenchmark

