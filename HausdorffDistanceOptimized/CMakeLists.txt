# CMakeList.txt : CMake project for HausdorffDistanceOptimized, include source and define
# project specific logic here.
#

cmake_minimum_required(VERSION 3.15)

project(HausdorffDistanceOptimized
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenCV CONFIG REQUIRED)
find_package(Boost REQUIRED)

message(STATUS "OpenCV version: ${OpenCV_VERSION}")

add_executable(HausdorffDistanceOptimized
    main.cpp
    include/ContourHausdorff/ContourCoherentHausdorffDistance.hpp

    src/baselines/HausdorffDistanceBaselineLibs.h
    src/baselines/HausdorffDistanceBruteForce.hpp
    src/baselines/HausdorffDistanceBaselineLibs.cpp

    src/benchmark/BenchmarkCli.cpp
    src/benchmark/Benchmarks.h
    src/benchmark/BenchmarkCli.h

    src/image/ContourExtractor.h
    src/image/ContourExtractor.cpp
)

target_sources(HausdorffDistanceOptimized PRIVATE
    scripts/run_benchmarks.ps1
    scripts/run_benchmarks.sh
)

target_include_directories(HausdorffDistanceOptimized
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/
        ${CMAKE_CURRENT_SOURCE_DIR}/src/baselines
        ${CMAKE_CURRENT_SOURCE_DIR}/src/benchmark
        ${CMAKE_CURRENT_SOURCE_DIR}/src/image
        ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(HausdorffDistanceOptimized
    PRIVATE
        ${OpenCV_LIBS}
        Boost::headers
)

target_compile_definitions(HausdorffDistanceOptimized PRIVATE HD_VERBOSE_OPENCV=0)