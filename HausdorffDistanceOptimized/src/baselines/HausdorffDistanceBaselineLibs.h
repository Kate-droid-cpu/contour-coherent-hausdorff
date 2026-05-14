// SPDX-License-Identifier: MIT

/*
	HausdorffDistanceBaselineLibs.h

	Baseline Hausdorff distance implementations based on external libraries.

	These functions are used for benchmarking and comparison:
	  - Boost.Geometry discrete Hausdorff distance
	  - OpenCV FLANN KD-tree nearest-neighbor baseline
	  - OpenCV HausdorffDistanceExtractor for binary images

	These baselines are not part of the dependency-free core algorithm.
*/

#pragma once

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "ContourHausdorff/ContourCoherentHausdorffDistance.hpp"

namespace HausdorffDistanceLibs {

	// Computes discrete Hausdorff distance using Boost.Geometry.
	std::pair<std::size_t, float> CalcHausdorffDistanceBoost(const std::vector<ContourHausdorff::CPointI>& contour1, 
		const std::vector<ContourHausdorff::CPointI>& contour2);

	// Computes Hausdorff distance using an OpenCV FLANN KD-tree nearest-neighbor baseline.
	// The returned count is the number of nearest-neighbor queries, not the internal
	// number of distance evaluations performed by the KD-tree.
	std::pair<std::size_t, float> CalcHausdorffDistanceKDTree(const std::vector<ContourHausdorff::CPointI>& contour1, 
		const std::vector<ContourHausdorff::CPointI>& contour2);

	// Computes Hausdorff distance between two binary images using OpenCV's
	// HausdorffDistanceExtractor. This is an image-level baseline, not a contour-vector API.
	float CalcHausdorffDistanceOpenCV(const std::string& imagePath1, const std::string& imagePath2);
}