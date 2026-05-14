// Copyright(c) 2026 Catherine Timokhina
// SPDX - License - Identifier: MIT

/*
	ContourCoherentHausdorffDistance.hpp

	Fast exact discrete Hausdorff distance for ordered contours.

	This header implements a contour-coherent search strategy for computing
	the exact discrete Hausdorff distance between two ordered contours.

	The method exploits the observation that, for related ordered contours,
	nearest-neighbor indices often change gradually between consecutive contour
	points. For each source point, the directed search starts near the previous nearest-neighbor index, 
	with a small forward bias along the target contour, and expands bidirectionally:

		startIndex, startIndex + 1, startIndex - 1,
		startIndex + 2, startIndex - 2, ...

	The early termination condition is exact for max-based Hausdorff distance:
	once the current minimum distance for a source point is not greater than the
	current directed Hausdorff maximum, this point cannot increase the final
	directed distance.

	Properties:
	  - Computes exact discrete Hausdorff distance, not an approximation.
	  - Optimized for naturally ordered contour data.
	  - Deterministic; does not rely on random shuffling.
	  - Header-only and dependency-free.
	  - Supports custom point types and distance policies.
	  - Uses squared Euclidean distance by default.

	Complexity:
	  - Worst case: O(n * m).
	  - Practical behavior on coherent ordered contours depends on the local
		search width. If the number of inspected candidates per source point is
		bounded by a small constant, the directed computation behaves close to
		O(n), and the symmetric computation close to O(n + m).

	Intended use cases:
	  - Similar or related contours.
	  - Segmentation mask comparison.
	  - Video/tracking contour sequences.
	  - Industrial inspection against a reference contour.
	  - Regression testing of contour extraction algorithms.

	Not intended as a universal accelerator for arbitrary unordered point clouds.
*/

#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <utility>
#include <vector>

namespace ContourHausdorff {

	// A simple 2D point structure that can be used for both integer and floating-point coordinates
	template <typename T>
	struct CPoint
	{
		T x;
		T y;
	};

	using CPointF = CPoint<float>;
	using CPointI = CPoint<int>;

	// Distance functors for different distance metrics. They can be used to easily switch between different distance calculations if needed.	
	struct SquaredL2Distance
	{
		using value_type = float;

		template <typename Point>
		inline value_type operator()(const Point& a, const Point& b) const noexcept
		{
			const value_type dx =
				static_cast<value_type>(a.x) - static_cast<value_type>(b.x);
			const value_type dy =
				static_cast<value_type>(a.y) - static_cast<value_type>(b.y);

			return dx * dx + dy * dy;
		}

		inline value_type finalize(value_type dSq) const noexcept
		{
			return std::sqrt(dSq);
		}
	};

	// L1 distance (Manhattan distance) functor.
	struct L1Distance
	{
		using value_type = float;

		template <typename Point>
		inline value_type operator()(const Point& a, const Point& b) const noexcept
		{
			return std::abs(static_cast<value_type>(a.x) - static_cast<value_type>(b.x)) +
				std::abs(static_cast<value_type>(a.y) - static_cast<value_type>(b.y));
		}

		inline value_type finalize(value_type d) const noexcept
		{
			return d;
		}
	};

	// Chebyshev distance functor.
	struct ChebyshevDistance
	{
		using value_type = float;

		template <typename Point>
		inline value_type operator()(const Point& a, const Point& b) const noexcept
		{
			const value_type dx =
				std::abs(static_cast<value_type>(a.x) - static_cast<value_type>(b.x));
			const value_type dy =
				std::abs(static_cast<value_type>(a.y) - static_cast<value_type>(b.y));

			return std::max(dx, dy);
		}

		inline value_type finalize(value_type d) const noexcept
		{
			return d;
		}
	};

	namespace detail {
		// Precondition: contour1 and contour2 are non-empty
		// This function calculates the directed Hausdorff distance from contour1 to contour2. Uses the specified distance policy; SquaredL2Distance is the default.
		template <typename Point, typename Distance = SquaredL2Distance>
		inline std::pair<std::size_t, typename Distance::value_type> 
			calcDirectedContourCoherentHausdorffDistance(const std::vector<Point>& contour1, const std::vector<Point>& contour2, Distance distance = Distance())
		{
			using DistT = typename Distance::value_type;
			// Contour-coherent search:
			// start from the previous nearest-neighbor index + 1 and expand bidirectionally:
			// startIndex, startIndex +/- 1, startIndex +/- 2, ...
			const int size2 = static_cast<int>(contour2.size());

			enum eSearchState { BothDirections, OneDirection, NoDirections};

			auto reduceDirections = [](eSearchState d) {
				return d == BothDirections ? OneDirection : NoDirections;
				};

			// Count distance evaluations for benchmarking and diagnostics.
			std::size_t checks = 0;

			// Maximum over all points in contour1 of their minimum distance score to contour2.
			DistT hdMaxMin = DistT{ 0 };

			int startIndex = 0;

			for (const auto& a : contour1)
			{
				DistT hdMinCurr = std::numeric_limits<DistT>::max();

				// We will keep track of the best index found in contour2 for the current point a, 
				// which will be used as the starting point for the next iteration.
				int bestIndexContour2 = -1;

				bool goRight = true;
				auto directions = BothDirections;
				int pos = startIndex;
				int offset = 0;

				while (directions != NoDirections)
				{
					if (pos < 0 || pos >= size2) {
						directions = reduceDirections(directions);
						goRight = !goRight;
					}
					else {
						const auto& b = contour2[pos];

						// Calculate the distance between points a and b
						const DistT d = distance(a, b);
						++checks;

						if (d < hdMinCurr) {
							hdMinCurr = d;
							bestIndexContour2 = pos;
						}
						// If the current minimum is already below the current Hausdorff maximum,
						// this point cannot increase the final directed Hausdorff distance.
						if (hdMinCurr <= hdMaxMin) {
							break;
						}
					}

					if (directions == BothDirections) {
						goRight = !goRight;
					}

					pos = goRight
						? startIndex + offset + 1
						: startIndex - offset - 1;

					// Increase offset only after both sides at the current offset were considered,
					// or when only one search direction remains.
					if (goRight || directions == OneDirection) {
						++offset;
					}
				}

				// Forward prediction for ordered contours: if a[i] matched b[j],
				// a[i + 1] often matches near b[j + 1]. No clamp is needed;
				// out-of-range start positions are handled by the search loop.
				startIndex = bestIndexContour2 + 1; 

				hdMaxMin = std::max(hdMaxMin, hdMinCurr);
			}

			return { checks, distance.finalize(hdMaxMin) };
		}
	} // namespace detail

	template <typename Point, typename Distance = SquaredL2Distance>
	inline std::pair<std::size_t, typename Distance::value_type> CalcContourCoherentHausdorffDistance(const std::vector<Point>& contour1, const std::vector<Point>& contour2, Distance distance = Distance())
	{
		using DistT = typename Distance::value_type;

		if (contour1.empty() || contour2.empty())
			return { 0, std::numeric_limits<DistT>::quiet_NaN() };

		auto [check12, h12] = detail::calcDirectedContourCoherentHausdorffDistance(contour1, contour2, distance);
		auto [check21, h21] = detail::calcDirectedContourCoherentHausdorffDistance(contour2, contour1, distance);
		return { check12 + check21, std::max(h12, h21) };
	}
}// namespace ContourHausdorff
