// SPDX-License-Identifier: MIT

/*
    Main entry point for the Contour-Coherent Hausdorff Distance benchmark tool.

    The program loads ordered contours from one or more image folders,
    runs the selected Hausdorff distance algorithms, and prints benchmark
    results to the console.
*/

#include <exception>
#include <iostream>
#include <string>

#include "Benchmarks.h"
#include "ContourExtractor.h"
#include "BenchmarkCli.h"

int main(int argc, char* argv[])
{
    std::cout << "Contour-Coherent Hausdorff Distance benchmark\n";

    try
    {
        const HausdorffDistanceCliBenchmark::BenchmarkOptions options = HausdorffDistanceCliBenchmark::ParseCommandLine(argc, argv);

        for (const auto& folderPath : options.folders)
        {
            auto contours = ContourExtractor::LoadContoursFromFolder(folderPath);

            ContourExtractor::PrintContoursInfo(folderPath, contours);

            BenchmarkSelectedHausdorffAlgorithmsOnPackage<ContourHausdorff::CPointI>(
                contours,
                options.algorithms,
                options.batches
            );
        }

    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        HausdorffDistanceCliBenchmark::PrintUsage();
        return 1;
    }

    return 0;
}
