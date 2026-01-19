/**
 * \file bragg.cc
 * \brief implementations for brag.h
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

#include <bragg.h>

namespace tmm
{
	Bragg::Bragg(double period, double duty_cycle, double N) :
	_period(period),
	_duty_cycle(duty_cycle),
	_N(N)
	{ }

	void 
	Bragg::transfer_matrix(std::complex<double>** Tp, double wavelength, double n1, double n2, double loss)
	{
		double l1 = _period * _duty_cycle;
		double l2 = _period * (1.0 - _duty_cycle);
		
		auto T_11 = damm::aligned_alloc_2D<std::complex<double>, 64>(2, 2);
		auto T_12 = damm::aligned_alloc_2D<std::complex<double>, 64>(2, 2);
		auto T_22 = damm::aligned_alloc_2D<std::complex<double>, 64>(2, 2);
		auto T_21 = damm::aligned_alloc_2D<std::complex<double>, 64>(2, 2);
		auto work = damm::aligned_alloc_2D<std::complex<double>, 64>(2, 2);
		
		// Build period: Tp = T_11 * T_12 * T_22 * T_21
		homogeneous_layer(T_11.get(), wavelength, l1, n1, loss);
		index_step(T_12.get(), n1, n2);
		homogeneous_layer(T_22.get(), wavelength, l2, n2, loss);
		index_step(T_21.get(), n2, n1);
		
		// work = T_11 * T_12
		damm::zeros<std::complex<double>, damm::NONE>(work.get(), 2, 2);
		damm::multiply<std::complex<double>, damm::NONE>(T_11.get(), T_12.get(), work.get(), 2, 2, 2);
		
		// T_11 = work * T_22 (reuse T_11)
		damm::zeros<std::complex<double>, damm::NONE>(T_11.get(), 2, 2);
		damm::multiply<std::complex<double>, damm::NONE>(work.get(), T_22.get(), T_11.get(), 2, 2, 2);
		
		// Tp = T_11 * T_21
		damm::zeros<std::complex<double>, damm::NONE>(Tp, 2, 2);
		damm::multiply<std::complex<double>, damm::NONE>(T_11.get(), T_21.get(), Tp, 2, 2, 2);
	}

	void 
	Bragg::scattering_matrix(std::complex<double>** T, double wavelength, double n1, double n2, double loss)
	{
		auto Tp = damm::aligned_alloc_2D<std::complex<double>, 64>(2, 2);
		
		transfer_matrix(Tp.get(), wavelength, n1, n2, loss);
		matrix_power(Tp.get(), T, _N);
	}

	std::tuple<double, double>
	Bragg::scattering_coefficients(double wavelength, double n1, double n2, double loss)
	{
		auto sparams = damm::aligned_alloc_2D<std::complex<double>, 64>(2, 2);
		
		scattering_matrix(sparams.get(), wavelength, n1, n2, loss);

		double R, T;

		scattering_coefficients_db(sparams.get(), R, T);

		return std::make_tuple(R, T);
	}


}//namespace tmm