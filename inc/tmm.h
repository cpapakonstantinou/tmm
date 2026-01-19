#ifndef __TMM_H__
#define __TMM_H__

/**
 * \file tmm.h
 * \brief Transfer Matrix Method library
 * \author cpapakonstantinou
 * \date 2026
 * 
 * Transfer Matrix Method:
 * Solve photonic structures of arbitrarily layered media
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

#include <damm.h>
#include <damm_memory.h>
#include <complex>
#include <functional>
#include <cmath>
#include <tuple>
#include <stdexcept>

namespace tmm
{
	static constexpr double pi = M_PI;	
	static constexpr double eps0 = 8.854188E-12; // Farads per meter (F/m)
	static constexpr double mu0 = 4 * pi * 1E-7; // Henries per meter (H/m)
	static constexpr double c = 1/sqrt(eps0*mu0); // free space speed of light
	static constexpr double eta0 = sqrt(mu0/eps0); // free space impedance

	/**
	 * \brief Transfer Matrix Method base class
	 */
	class TMM
	{
		protected:
		/**
		 * \brief Complex propagation constant
		 * 
		 * Computes beta = k0 * neff - i * alpha/2, where:
		 * - k0 = 2*pi/lambda is the free-space wavenumber
		 * - alpha is the loss coefficient
		 * 
		 * \param neff Effective refractive index
		 * \param wavelength Wavelength in meters
		 * \param loss Loss coefficient in 1/m
		 * \return Complex propagation constant
		 */
		inline std::complex<double> beta(double neff, double wavelength, double loss)
		{
			const double k0 = 2.0 * pi / wavelength;
			const double alpha = loss / 2.0;
			return std::complex<double>(k0 * neff, -alpha);
		}
		
		/**
		 * \brief Transfer matrix for homogeneous layer propagation
		 * 
		 * The propagation matrix within a layer of constant refractive index: neff
		 * 
		 * \param P Propagation matrix
		 * \param wavelength Wavelength in meters
		 * \param length Layer length in meters
		 * \param neff Effective refractive index
		 * \param loss Loss in 1/m
		 */
		inline void 
		homogeneous_layer(std::complex<double>** P, double wavelength, double length, double neff, double loss)
		{
			std::complex<double> b = beta(neff, wavelength, loss);
			std::complex<double> phase = b * length;
			
			P[0][0] = std::exp(std::complex<double>(0, 1) * phase);
			P[0][1] = std::complex<double>(0, 0);
			P[1][0] = std::complex<double>(0, 0);
			P[1][1] = std::exp(std::complex<double>(0, -1) * phase);
		}
		
		/**
		 * \brief Transfer matrix for heterogeneous layer propagation
		 * 
		 * The transfer matrix for a scattering at a refractive index contrast n1, n2
		 * T = [[a, b], [b, a]] where a = (n1+n2)/(2*sqrt(n1*n2)), b = (n1-n2)/(2*sqrt(n1*n2))
		 * 
		 * where a, b are the Fresnel coefficients at normal incidence
		 * 
		 * \param T Transfer matrix
		 * \param n1 Refractive index of first medium
		 * \param n2 Refractive index of second medium
		 */
		inline void index_step(std::complex<double>** T, double n1, double n2)
		{
			std::complex<double> a = (n1 + n2) / (2.0 * std::sqrt(n1 * n2));
			std::complex<double> b = (n1 - n2) / (2.0 * std::sqrt(n1 * n2));
			
			T[0][0] = a;
			T[0][1] = b;
			T[1][0] = b;
			T[1][1] = a;
		}
	};
	
	/**
	 * \brief Matrix power using binary exponentiation
	 * 
	 * Computes T^N for a 2x2 matrix with complexity O(log N)
	 * 
	 * \param T input matrix
	 * \param TN output matrix
	 * \param N power
	 */
	inline void 
	matrix_power(std::complex<double>** T, std::complex<double>** TN, size_t N)
	{
		auto result = damm::aligned_alloc_2D<std::complex<double>, 64>(2, 2);
		damm::identity<std::complex<double>, damm::NONE>(result.get(), 2, 2);
		
		if (N == 0) 
		{
			for (size_t i = 0; i < 2; ++i)
				for (size_t j = 0; j < 2; ++j)
					TN[i][j] = result[i][j];
			return;
		}
		
		auto base = damm::aligned_alloc_2D<std::complex<double>, 64>(2, 2);
		auto temp = damm::aligned_alloc_2D<std::complex<double>, 64>(2, 2);
		
		// Copy T to base
		for (size_t i = 0; i < 2; ++i)
			for (size_t j = 0; j < 2; ++j)
				base[i][j] = T[i][j];
		
		// Binary exponentiation
		while (N > 0)
		{
			if (N & 1)
			{
				// result *= base
				damm::zeros<std::complex<double>, damm::NONE>(temp.get(), 2, 2);
				damm::multiply<std::complex<double>, damm::NONE>(result.get(), base.get(), temp.get(), 2, 2, 2);
				
				for (size_t i = 0; i < 2; ++i)
					for (size_t j = 0; j < 2; ++j)
						result[i][j] = temp[i][j];
			}
			
			N >>= 1;
			
			if (N > 0)
			{
				// base *= base
				damm::zeros<std::complex<double>, damm::NONE>(temp.get(), 2, 2);
				damm::multiply<std::complex<double>, damm::NONE>(base.get(), base.get(), temp.get(), 2, 2, 2);
				
				for (size_t i = 0; i < 2; ++i)
					for (size_t j = 0; j < 2; ++j)
						base[i][j] = temp[i][j];
			}
		}
		
		// Copy result to TN
		for (size_t i = 0; i < 2; ++i)
			for (size_t j = 0; j < 2; ++j)
				TN[i][j] = result[i][j];
	}
	
	/**
	 * \brief Extract reflection and transmission from S-matrix
	 * 
	 * Given the total transfer matrix S, compute:
	 * - R = |S[1,0] / S[0,0]|^2 (reflection coefficient)
	 * - T = |1 / S[0,0]|^2 (transmission coefficient)
	 * 
	 * \param S 2x2 scattering matrix
	 * \param R Output reflection coefficient
	 * \param T Output transmission coefficient
	 */
	inline void scattering_coefficients_db(std::complex<double>** S, double& R, double& T)
	{
		std::complex<double> S00 = S[0][0];
		std::complex<double> S10 = S[1][0];
		
		R = std::pow(std::abs(S10 / S00), 2.0);
		T = std::pow(std::abs(1.0 / S00), 2.0);
	}
	
	/**
	 * \brief Convert linear to decibels
	 */
	inline double to_db(double linear)
	{
		return 10.0 * std::log10(std::max(linear, 1e-15));
	}
	
	/**
	 * \brief Convert dB to linear
	 */
	inline double from_db(double db)
	{
		return std::log(10.0) * db / 10.0;
	}
	
} // namespace tmm

#endif // __TMM_H__