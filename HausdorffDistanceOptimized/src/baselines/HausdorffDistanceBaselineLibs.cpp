// SPDX-License-Identifier: MIT

//project headers
#include "HausdorffDistanceBaselineLibs.h"

// stl
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

#if HD_VERBOSE_OPENCV
#include <iostream>
#endif

//boost
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/algorithms/discrete_hausdorff_distance.hpp>

//opencv
#include <opencv2/core.hpp>
#include <opencv2/flann.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/shape.hpp>

namespace bg = boost::geometry;
using BoostPoint = bg::model::d2::point_xy<int>;
using BoostContour = bg::model::linestring<BoostPoint>;

namespace HausdorffDistanceLibs {

    //--------------------------------------------private helper functions--------------------------------------------

    namespace {

        // Helper function to convert a contour represented as a vector of CPointI to a Boost linestring (contour)
        BoostContour convertToBoostContour(const std::vector<ContourHausdorff::CPointI>& contour)
        {
            BoostContour result;
            result.reserve(contour.size());

            for (const auto& p : contour)
            {
                result.push_back(BoostPoint(p.x, p.y));
            }

            return result;
        }

        // Helper function to convert a contour represented as a vector of CPointI to a cv::Mat suitable for FLANN
        cv::Mat contourToFlannMatIntPoints(const std::vector<ContourHausdorff::CPointI>& contour)
        {
            cv::Mat mat(static_cast<int>(contour.size()), 2, CV_32F);

            for (int i = 0; i < static_cast<int>(contour.size()); ++i)
            {
                mat.at<float>(i, 0) = static_cast<float>(contour[i].x);
                mat.at<float>(i, 1) = static_cast<float>(contour[i].y);
            }

            return mat;
        }

        // KD-tree nearest-neighbor baseline using OpenCV FLANN. Note: depending on FLANN settings, this may be approximate.
        std::pair<std::size_t, float> calcDirectedHausdorffDistanceKDTree(const std::vector<ContourHausdorff::CPointI>& contour1, const std::vector<ContourHausdorff::CPointI>& contour2)
        {
            cv::Mat data = contourToFlannMatIntPoints(contour2);

            cv::flann::Index kdTree(
                data,
                cv::flann::KDTreeIndexParams(1)
            );
            //cv::flann::Index kdTree(data, cv::flann::LinearIndexParams());

            cv::Mat query(1, 2, CV_32F);
            cv::Mat indices(1, 1, CV_32S);
            cv::Mat dists(1, 1, CV_32F);

            float hdMaxMinSq = 0.0f;
            std::size_t queries = 0;

            for (const auto& a : contour1)
            {
                query.at<float>(0, 0) = static_cast<float>(a.x);
                query.at<float>(0, 1) = static_cast<float>(a.y);

                kdTree.knnSearch(
                    query,
                    indices,
                    dists,
                    1,
                    cv::flann::SearchParams(-1)
                );
                ++queries;

                const int nearestIndex = indices.at<int>(0, 0);
                const auto& b = contour2[nearestIndex];

                const auto dx = static_cast<float>(a.x) - static_cast<float>(b.x);
                const auto dy = static_cast<float>(a.y) - static_cast<float>(b.y);
                const float dSq = dx * dx + dy * dy;

                if (dSq <= hdMaxMinSq)
                {
                    continue;
                }

                hdMaxMinSq = dSq;
            }

            return { queries, std::sqrt(hdMaxMinSq) };
        }

	} // namespace

    //---------------------------------------------public API functions--------------------------------------------

    // Calculates the Hausdorff distance between two contours using a KD-tree for efficient nearest neighbor search. 
    // Returns a pair containing the total number of nearest neighbor queries performed and the calculated Hausdorff distance
    std::pair<std::size_t, float> CalcHausdorffDistanceKDTree( const std::vector<ContourHausdorff::CPointI>& contour1, const std::vector<ContourHausdorff::CPointI>& contour2)
    {
        if (contour1.empty() || contour2.empty())
        {
            return { 0, std::numeric_limits<float>::quiet_NaN() };
        }

        const auto [queries12, h12] = calcDirectedHausdorffDistanceKDTree(contour1, contour2);

        const auto [queries21, h21] = calcDirectedHausdorffDistanceKDTree(contour2, contour1);

        return { queries12 + queries21, std::max(h12, h21) };
    }

    // Calculates the Hausdorff distance between two contours using Boost.Geometry's discrete_hausdorff_distance algorithm.
    std::pair<std::size_t, float> CalcHausdorffDistanceBoost(const std::vector<ContourHausdorff::CPointI>& contour1, const std::vector<ContourHausdorff::CPointI>& contour2)
    {
        BoostContour boostContour1 = convertToBoostContour(contour1);
        BoostContour boostContour2 = convertToBoostContour(contour2);

        auto h12 = static_cast<float>(boost::geometry::discrete_hausdorff_distance(boostContour1, boostContour2));
        auto h21 = static_cast<float>(boost::geometry::discrete_hausdorff_distance(boostContour2, boostContour1));
        return { 0, std::max(h12, h21) };
    }

    // Calculates Hausdorff distance between two binary images using OpenCV's
    // HausdorffDistanceExtractor after extracting contours.
    float CalcHausdorffDistanceOpenCV(const std::string& imagePath1, const std::string& imagePath2)
    {
        cv::Mat a = cv::imread(imagePath1, 0);
        cv::Mat b = cv::imread(imagePath2, 0);

        if (a.empty() || b.empty())
        {
            throw std::runtime_error("Cannot load image: " + imagePath1 + " or " + imagePath2);
        }

        std::vector<std::vector<cv::Point>> ca, cb;

        cv::findContours(a, ca, cv::RETR_CCOMP, cv::CHAIN_APPROX_TC89_KCOS);
        cv::findContours(b, cb, cv::RETR_CCOMP, cv::CHAIN_APPROX_TC89_KCOS);

        if (ca.empty() || cb.empty())
        {
            throw std::runtime_error("No contour found in one of the images");
        }

        const int distanceFlag = cv::NORM_L2;
        const float rankProportion = 0.0f;

        cv::Ptr<cv::HausdorffDistanceExtractor> extractor =
            cv::createHausdorffDistanceExtractor(distanceFlag, rankProportion);

        #if HD_VERBOSE_OPENCV
                std::cout << "OpenCV rankProportion = "
                    << extractor->getRankProportion()
                    << '\n';
        #endif

        return extractor->computeDistance(ca[0], cb[0]);
    }
} // namespace HausdorffDistanceLibs