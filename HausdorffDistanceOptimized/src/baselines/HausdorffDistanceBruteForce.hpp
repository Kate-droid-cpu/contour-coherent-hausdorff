// SPDX-License-Identifier: MIT

/*
	HausdorffDistanceBruteForce.hpp

	Exact brute-force discrete Hausdorff distance baselines.

	Includes:
	  - deterministic brute-force search with early termination
	  - brute-force search with deterministic random shuffling

	These implementations are intended primarily for benchmarking and
	comparison against contour-coherent acceleration methods.
*/

#pragma once
#include <vector>
#include <random>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <utility>

#include "ContourHausdorff/ContourCoherentHausdorffDistance.hpp"

namespace HausdorffDistanceBruteForce {

	namespace detail {
		// This function calculates the directed Hausdorff distance from contour1 to contour2. Uses the specified distance policy; SquaredL2Distance is the default.
		template <typename Point, typename Distance = ContourHausdorff::SquaredL2Distance>
		inline std::pair<std::size_t, typename Distance::value_type>
			calcDirectedHausdorffBruteForceShuffling(const std::vector<Point>& contour1, const std::vector<Point>& contour2, Distance distance = Distance{})
		{
			using DistT = typename Distance::value_type;

			// Fixed seed for reproducible benchmark results.
			std::mt19937 rng(4);
			// Create shuffled copies for deterministic randomized traversal.
			auto order1 = contour1;
			auto order2 = contour2;

			std::shuffle(order1.begin(), order1.end(), rng);
			std::shuffle(order2.begin(), order2.end(), rng);

			std::size_t checks = 0;

			DistT hdMaxMin = DistT{ 0 };
			for (const auto& a : order1)
			{
				DistT hdMinCurr = std::numeric_limits<DistT>::max();
				for (const auto& b : order2)
				{
					const DistT d = distance(a, b);
					++checks;
					if (d < hdMinCurr) {
						hdMinCurr = d;
					}
					if (hdMinCurr <= hdMaxMin) {
						break;
					}
				}
				if (hdMinCurr > hdMaxMin) {
					hdMaxMin = hdMinCurr;
				}
			}
			return { checks, distance.finalize(hdMaxMin) };
		}

		template <typename Point, typename Distance = ContourHausdorff::SquaredL2Distance>
		inline std::pair<std::size_t, typename Distance::value_type>
			calcDirectedHausdorffBruteForce(const std::vector<Point>& contour1, const std::vector<Point>& contour2, Distance distance = Distance{})
		{
			using DistT = typename Distance::value_type;
			std::size_t checks = 0;
			DistT hdMaxMin = DistT{ 0 };
			for (const auto& a : contour1)
			{
				DistT hdMinCurr = std::numeric_limits<DistT>::max();
				for (const auto& b : contour2)
				{
					const DistT d = distance(a, b);
					++checks;
					if (d < hdMinCurr) {
						hdMinCurr = d;
					}
					if (hdMinCurr <= hdMaxMin) {
						break;
					}
				}
				if (hdMinCurr > hdMaxMin) {
					hdMaxMin = hdMinCurr;
				}
			}
			return { checks, distance.finalize(hdMaxMin) };
		}
	} // namespace detail

	template <typename Point, typename Distance = ContourHausdorff::SquaredL2Distance>
	inline std::pair<std::size_t, typename Distance::value_type>
		CalcHausdorffDistanceBruteForce(const std::vector<Point>& contour1, const std::vector<Point>& contour2, bool shuffling = false, Distance distance = Distance{})
	{
		using DistT = typename Distance::value_type;

		if (contour1.empty() || contour2.empty())
			return { 0, std::numeric_limits<DistT>::quiet_NaN() };

		if (shuffling) {
			auto [check1, h12] = detail::calcDirectedHausdorffBruteForceShuffling(contour1, contour2, distance);
			auto [check2, h21] = detail::calcDirectedHausdorffBruteForceShuffling(contour2, contour1, distance);

			return { check1 + check2, std::max(h12, h21) };
		}

		auto [check1, h12] = detail::calcDirectedHausdorffBruteForce(contour1, contour2, distance);
		auto [check2, h21] = detail::calcDirectedHausdorffBruteForce(contour2, contour1, distance);

		return { check1 + check2, std::max(h12, h21) };
	}

} // namespace HausdorffDistanceBruteForce