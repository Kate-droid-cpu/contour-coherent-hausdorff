
// SPDX-License-Identifier: MIT

#include "ContourExtractor.h"

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace ContourExtractor {

    //--------------------------------------------private helper functions--------------------------------------------

    namespace {

        std::string ToLower(std::string s)
        {
            std::transform(
                s.begin(),
                s.end(),
                s.begin(),
                [](unsigned char c) { return static_cast<char>(std::tolower(c)); }
            );

            return s;
        }

        bool IsSupportedImageExtension(const std::filesystem::path& path)
        {
            const std::string ext = ToLower(path.extension().string());

            return ext == ".png" ||
                ext == ".jpg" ||
                ext == ".jpeg" ||
                ext == ".bmp" ||
                ext == ".tif" ||
                ext == ".tiff" ||
                ext == ".gif";
        }

    } // namespace

    //---------------------------------------------public API---------------------------------------------

    std::vector<ContourHausdorff::CPointI> LoadSingleContourFromBinaryImage(const std::string& imagePath)
    {
        cv::Mat img = cv::imread(imagePath, cv::IMREAD_GRAYSCALE);

        if (img.empty())
        {
            throw std::runtime_error("Cannot load image: " + imagePath);
        }

        cv::Mat binary;
        const uchar bg = img.at<uchar>(0, 0);

        const int thresholdType = bg < 128
            ? cv::THRESH_BINARY      // black background, white object
            : cv::THRESH_BINARY_INV; // white background, black object

        cv::threshold(img, binary, 127, 255, thresholdType);

        std::vector<std::vector<cv::Point>> contours;

        cv::findContours(binary, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);

        if (contours.empty())
        {
            throw std::runtime_error("No contour found in image: " + imagePath);
        }

        auto largestIt = std::max_element(
            contours.begin(),
            contours.end(),
            [](const std::vector<cv::Point>& a, const std::vector<cv::Point>& b)
            {
                return cv::contourArea(a) < cv::contourArea(b);
            }
        );

        const auto& contour = *largestIt;

        std::vector<ContourHausdorff::CPointI> result;
        result.reserve(contour.size());

        for (const cv::Point& p : contour)
        {
            result.push_back({p.x, p.y});
        }

        return result;
    }

    std::vector<std::vector<ContourHausdorff::CPointI>> LoadContoursFromFolder(const std::string& folderPath)
    {
        namespace fs = std::filesystem;

        std::vector<std::vector<ContourHausdorff::CPointI>> contours;
        if (!fs::exists(folderPath))
        {
            throw std::runtime_error("Folder does not exist: " + folderPath);
        }

        if (!fs::is_directory(folderPath))
        {
            throw std::runtime_error("Path is not a directory: " + folderPath);
        }

        std::vector<fs::path> imagePaths;

        for (const auto& entry : fs::directory_iterator(folderPath))
        {
            if (!entry.is_regular_file())
            {
                continue;
            }

            const fs::path path = entry.path();

            if (IsSupportedImageExtension(path))
            {
                imagePaths.push_back(path);
            }
        }

        std::sort(imagePaths.begin(), imagePaths.end());

        contours.reserve(imagePaths.size());

        for (const fs::path& path : imagePaths)
        {
            try
            {
                contours.push_back(
                    LoadSingleContourFromBinaryImage(path.string())
                );
            }
            catch (const std::exception& e)
            {
                std::cerr << "Warning: failed to load contour from "
                    << path.string() << ": "
                    << e.what() << std::endl;
            }
        }

        return contours;
    }

    void PrintContoursInfo(std::string_view folderPath, const std::vector<std::vector<ContourHausdorff::CPointI>>& contours)
    {
        std::cout << "Folder: " << folderPath << "\n";
        for (size_t i = 0; i < contours.size(); ++i)
        {
            std::cout << "  contour " << i << " points = " << contours[i].size() << "\n";
        }
        std::cout << "\n";
    }
}