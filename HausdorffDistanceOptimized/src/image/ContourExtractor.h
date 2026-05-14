// SPDX-License-Identifier: MIT

/*
	ContourExtractor.h

	Utilities for loading contours from binary images using OpenCV.

	These functions are intended for benchmark dataset preparation and
	contour extraction from image folders.
*/

#pragma once

#include "ContourHausdorff/ContourCoherentHausdorffDistance.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace ContourExtractor {

	// Loads the largest contour from a binary image.
	std::vector<ContourHausdorff::CPointI> LoadSingleContourFromBinaryImage(const std::string& imagePath);

	// Loads contours from all supported images in a folder.
	std::vector<std::vector<ContourHausdorff::CPointI>> LoadContoursFromFolder(const std::string& folderPath);

	// Prints basic information about loaded contours.
    void PrintContoursInfo(std::string_view folderPath, const std::vector<std::vector<ContourHausdorff::CPointI>>& contours);

} // namespace ContourExtractor