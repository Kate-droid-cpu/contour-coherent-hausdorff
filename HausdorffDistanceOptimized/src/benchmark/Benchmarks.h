// SPDX-License-Identifier: MIT
//
// Benchmark utilities for Hausdorff distance algorithms.
//
// This file runs all-pairs package benchmarks on a set of contours and reports:
//   - total package time
//   - median time per Hausdorff distance
//   - number of distance checks / queries
//   - average Hausdorff distance

#pragma once

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

#include "BenchmarkCli.h"
#include "ContourHausdorff/ContourCoherentHausdorffDistance.hpp"
#include "HausdorffDistanceBaselineLibs.h"
#include "HausdorffDistanceBruteForce.hpp"

// This function benchmarks a given Hausdorff distance calculation function (func) on all pairs of contours in the provided package
template <typename Point, typename Func>
inline void BenchmarkContoursPackage(
    const std::string& name,
    const std::vector<std::vector<Point>>& contours,
    Func func,
    int batches = 20)
{
    if (batches <= 0)
    {
        throw std::invalid_argument("batches must be positive");
    }

    struct Result
    {
        static_assert(sizeof(std::size_t) >= 8,
            "Benchmark check counters require 64-bit std::size_t.");

        std::size_t checks = 0;
        double hdSum = 0.0;
        std::size_t pairCount = 0;
    };

    // This lambda runs the benchmark for all pairs of contours and returns the total number of checks, sum of Hausdorff distances, and pair count.
    auto runPackage = [&]() -> Result
        {
            Result r;

            for (std::size_t i = 0; i < contours.size(); ++i)
            {
                for (std::size_t j = i + 1; j < contours.size(); ++j)
                {
                    const auto [checks, hd] = func(contours[i], contours[j]);
                    r.checks += checks;
                    r.hdSum += hd;
                    ++r.pairCount;
                }
            }

            return r;
        };

    // Warm-up runs to mitigate cold-start effects
    Result sink;
    for (int i = 0; i < 3; ++i)
    {
        sink = runPackage();
    }

    std::vector<double> totalTimesMs;
    totalTimesMs.reserve(batches);

    // Benchmark runs
    for (int batch = 0; batch < batches; ++batch)
    {
        const auto start = std::chrono::steady_clock::now();
        sink = runPackage();
        const auto end = std::chrono::steady_clock::now();

        const std::chrono::duration<double, std::milli> elapsed = end - start;
        totalTimesMs.push_back(elapsed.count());
    }

    std::sort(totalTimesMs.begin(), totalTimesMs.end());

    const double medianTotalMs = totalTimesMs[totalTimesMs.size() / 2];
    const double avgTotalMs =
        std::accumulate(totalTimesMs.begin(), totalTimesMs.end(), 0.0) / totalTimesMs.size();

    const double medianPerHdMs =
        sink.pairCount > 0 ? medianTotalMs / static_cast<double>(sink.pairCount) : 0.0;

    const double checksPerHd =
        sink.pairCount > 0
        ? static_cast<double>(sink.checks) / static_cast<double>(sink.pairCount)
        : 0.0;

    std::cout << std::fixed << std::setprecision(6);
    std::cout << name << "\n";
    std::cout << "  pairs in package        = " << sink.pairCount << "\n";
    std::cout << "  median total package    = " << medianTotalMs << " ms\n";
    std::cout << "  avg total package       = " << avgTotalMs << " ms\n";
    std::cout << "  median per HD           = " << medianPerHdMs << " ms\n";
    std::cout << "  result checks            = " << sink.checks << "\n";
    std::cout << "  checks per HD           = " << checksPerHd << "\n";
    std::cout << "  avg HD                  = "
        << (sink.pairCount > 0 ? sink.hdSum / sink.pairCount : 0.0)
        << "\n";
}

// This function benchmarks the selected Hausdorff distance algorithms on the provided package of contours, running each algorithm for the specified number of batches.
template <typename Point>
inline void BenchmarkSelectedHausdorffAlgorithmsOnPackage(
    const std::vector<std::vector<Point>>& contours,
    const std::vector<HausdorffDistanceCliBenchmark::BenchmarkAlgorithm>& algorithms,
    int batches)
{
    using Algorithm = HausdorffDistanceCliBenchmark::BenchmarkAlgorithm;
    using Contour = std::vector<Point>;

    for (Algorithm algorithm : algorithms)
    {
        switch (algorithm)
        {
        case Algorithm::BruteForce:
            BenchmarkContoursPackage(
                "Brute-force Hausdorff with early stop, all contour pairs",
                contours,
                [](const Contour& a, const Contour& b)
                {
                    return HausdorffDistanceBruteForce::
                        CalcHausdorffDistanceBruteForce<Point>(a, b, false);
                },
                batches
            );
            break;

        case Algorithm::Shuffle:
            BenchmarkContoursPackage(
                "Brute-force Hausdorff with early stop and shuffling, all contour pairs",
                contours,
                [](const Contour& a, const Contour& b)
                {
                    return HausdorffDistanceBruteForce::
                        CalcHausdorffDistanceBruteForce<Point>(a, b, true);
                },
                batches
            );
            break;

        case Algorithm::ContourCoherent:
            BenchmarkContoursPackage(
                "Contour-Coherent Hausdorff Distance, all contour pairs",
                contours,
                [](const Contour& a, const Contour& b)
                {
					return ContourHausdorff::
                        CalcContourCoherentHausdorffDistance<Point>(a, b);
                },
                batches
            );
            break;

        case Algorithm::Boost:
            BenchmarkContoursPackage(
                "Boost discrete Hausdorff, all contour pairs",
                contours,
                [](const Contour& a, const Contour& b)
                {
                    return HausdorffDistanceLibs::
                        CalcHausdorffDistanceBoost(a, b);
                },
                batches
            );
            break;

        case Algorithm::KDTree:
            BenchmarkContoursPackage(
                "KD-tree Hausdorff, all contour pairs",
                contours,
                [](const Contour& a, const Contour& b)
                {
                    return HausdorffDistanceLibs::
                        CalcHausdorffDistanceKDTree(a, b);
                },
                batches
            );
            break;
        }
    }
}

