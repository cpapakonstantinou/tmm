#ifndef __BRAGG_H__
#define __BRAGG_H__

/**
 * \file bragg.h
 * \brief Bragg grating using TMM
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
#include <tmm.h>

namespace tmm
{
	/**
	 * \brief Bragg Grating.
	 * 
	 * Solves the transmission and reflection spectrum of a Bragg grating by TMM
	 * 
	 */
	class Bragg : protected TMM
	{
		double _period; ///< The period of the grating
		double _duty_cycle; ///< The dutycycle of the grating
		double _N; ///< The number of periods
	public:

		/**
		 * \brief Construct Bragg grating with specific geometry
		 **/
		Bragg(double period, double duty_cycle, double N);

		/**
		 * \brief Compute transfer matrix for a single grating period
		 * 
		 * \param Tp transfer matrix for single period
		 * \param wavelength Wavelength in meters
		 * \param n1 Effective index in first section
		 * \param n2 Effective index in second section
		 * \param loss Loss in 1/m
		 */
		void transfer_matrix(std::complex<double>** Tp, double wavelength, double n1, double n2, double loss);	
		
		/**
		 * \brief Compute transfer matrix for N grating periods
		 * 
		 * \param T Output 2x2 transfer matrix for N periods
		 * \param wavelength Wavelength in meters
		 * \param n1 Effective index in first section
		 * \param n2 Effective index in second section
		 * \param loss Loss in 1/m
		 */
		void scattering_matrix(std::complex<double>** T, double wavelength, double n1, double n2, double loss);	
		
		/**
		 * \brief Compute reflection and transmission at single wavelength
		 * 
		 * \param wavelength Wavelength in meters
		 * \param n1 Effective index in first section
		 * \param n2 Effective index in second section
		 * \param loss Loss in 1/m
		 * \returns reflection and transmission coefficients
		 */
		std::tuple<double, double> scattering_coefficients(double wavelength, double n1, double n2, double loss);
		
	};
}//namespace tmm
#endif //__BRAGG_H__