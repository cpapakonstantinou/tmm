#ifndef __TMM_CTL_H__
#define __TMM_CTL_H__

/**
 * \file tmm_ctl.h
 * \brief application control file
 * \author cpapakonstantinou
 * \date 2026
 * 
 */

// Copyright (c) 2026  Constantine Papakonstantinou
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include <vector>
#include <optional>
#include <cml.h>

namespace tmm
{
	/**
	 * \brief Device type for TMM
	 */
	enum device_t: uint8_t
	{
		BRAGG,
	};

	/**
	 * \brief Control structure for TMM
	 */
	struct ctl
	{
		// Material properties
		std::unique_ptr<cml> n1; ///< High index region property
		std::unique_ptr<cml> n2; ///< Low index region property
		std::unique_ptr<cml> loss; ///< Loss property (dB/cm)
		
		// Sweep-able parameters
		std::vector<double> width1;	///< Widths to test in high index region (um)
		std::vector<double> width2;	///< Widths to test in low index region (um)
		std::vector<double> periods; ///< Periods to test (um)
		std::vector<double> duty_cycles; ///< Duty cycles to test
		std::vector<double> Ns;	///< Number of periods to test
		std::vector<double> wavelengths; ///< Wavelengths to test (um)
		
		//Device type
		device_t device = BRAGG; ///< Device type
	};

	/**
	 * \brief Parse comma-separated list of numeric types
	 * 
	 * \tparam T The numeric type
	 * \param str The list in string format
	 * \param result The list in numeric format
	 * \param min Optional qualifier for minimum allowable value
	 * \param max Optional qualifier for maximum allowable value
	 * 
	 */
	template<typename T>
	inline size_t 
	parse_numeric(const char* str, std::vector<T>& result, 
				  std::optional<T> min = std::nullopt, 
				  std::optional<T> max = std::nullopt)
	requires 
	(
		std::same_as<T, double>   ||
		std::same_as<T, float>    ||
		std::same_as<T, unsigned> ||
		std::same_as<T, int>
	)
	{
		result.clear();
		char* end = nullptr;
		const char* current = str;
		
		while (*current)
		{
			T val;
			if constexpr (std::is_same_v<T, double>)
				val = std::strtod(current, &end);
			else if constexpr (std::is_same_v<T, float>)
				val = std::strtof(current, &end);
			else if constexpr (std::is_same_v<T, unsigned>)
				val = std::strtoul(current, &end, 10);
			else // int
				val = std::strtol(current, &end, 10);
			
			if (current == end) break;
			
			// Check bounds if specified
			if (min.has_value() && val < min.value())
				throw std::runtime_error(std::string(str) + " out of bounds (below minimum)");
			if (max.has_value() && val > max.value())
				throw std::runtime_error(std::string(str) + " out of bounds (above maximum)");
			
			result.push_back(val);
			current = (*end) ? end + 1 : end;
		}
		
		return result.size();
	}
};
#endif