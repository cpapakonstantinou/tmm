#ifndef __TMM_CML_H__
#define __TMM_CML_H__

/**
 * \file cml.h
 * \brief compact model library
 * \author cpapakonstantinou
 * \date 2026
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
#include <cmath>

namespace tmm
{

	/**
	 * \brief Taylor expansion model
	 * \tparam O reduction operation type. e.g. std::plus<> or std::minus<>
	 * \param x0 the reference point for the taylor expansion
	 * \param coeffs the coefficients of the model
	 */
	template<typename O>
	struct taylor_expansion
	{
		double x0; ///< expansion point
		std::vector<double> coeffs; ///< variadic number of coefficients
		
		double operator()(const double x) const
		{			
			double dx = x - x0;
			double y = coeffs[0];
			
			for (size_t i = 1; i < coeffs.size(); ++i)
				y = O{}(y, coeffs[i] * std::pow(dx, i));
			
			return y;
		}
	};	

	/**
	 * \brief Taylor expansion model for wavelength dispersion
	 * 
	 *  n(l) = a0 - a1*(l - l0) - a2*(l - l0)^2
	 */
	using wavelength_model_t = taylor_expansion<std::minus<>>;
	
	/**
	 * \brief Taylor expansion model for width dispersion
	 * 
	 *  dn(w) = b1*(w - w0) + b2*(w - w0)^2 + b3*(w - w0)^3
	 */
	using width_model_t = taylor_expansion<std::plus<>>;

	/**
	 * \brief Compact model.
	 * 
	 * Container for different representations of material property. 
	 */
	struct cml
	{
		std::optional<double> constant; ///< defined if material property is constant
		std::optional<std::vector<double>> sampled; ///< defined if material property is sampled
		std::optional<wavelength_model_t> wavelength_model; ///< defined if material property is wavelength dependent
		std::optional<width_model_t> width_model; ///< defined if material property is width dependent

		/**
		 * \brief common accessor for material property.
		 * \param l specify wavelength if wavelength model defined 
		 * \param w specify width if width model defined
		 * \param i specify index if sampled
		 *  
		 */
		double operator()(double l=0.0, double w=0.0, size_t i=0) const
		{
			double prop = 0.0;
			
			if(constant)
				prop = *constant;

			if(sampled)
				prop = (*sampled)[i];
			
			if(wavelength_model)
				prop += (*wavelength_model)(l);
			
			if(width_model)
				prop += (*width_model)(w);
			
			return prop;
		}
	};
};//namespace tmm
#endif //__TMM_CML_H__