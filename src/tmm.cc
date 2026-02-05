/**
 * \brief Transfer Matrix Method
 * \file tmm.cc
 * \author cpapakonstantinou
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

#include <iostream>
#include <memory>
#include <getopt.h>
#include <ctl.h>
#include <bragg.h>

using namespace std;
using namespace tmm;

const char* usage = \
	"usage: tmm [opts]\n"
	"\nGeneral Control:\n"
	"\t-d, --device         <type>             Devices supported: 'bragg' \n"
	"\t-l, --wavelength     <val>[,...]        Wavelength(s) \n"
	"\t--dl     			<val>		       Group delay wavelength interval \n"
	"\nBragg Control:\n"
	"\t-p, --period         <val>[,...]        Grating period(s) \n"
	"\t-c, --dutycycle      <val>[,...]        Dutycycle(s) 0-1\n"
	"\t-N, --n-periods      <val>[,...]        Number of periods\n"
	"\tConstant or Sampled Values:\n"
	"\t--n1                 <val>[,...]        Refractive index for high index region\n"
	"\t--n2                 <val>[,...]        Refractive index for low index region\n"
	"\t-a, --loss           <val>[,...]        Loss\n"
	"\tTaylor Expansion Models:\n"
	"\t--n1-model           <l0,a0,a1,a2,...>  n1(l) = a0 - a1*(l-l0) - a2*(l-l0)^2\n"
	"\t--n2-model           <l0,a0,a1,a2,...>  n2(l) = a0 - a1*(l-l0) - a2*(l-l0)^2\n"
	"\t--loss-model         <a0,a1,a2,...>     loss(l) = a0 - a1*(l-l0) - a2*(l-l0)^2\n"
	"\t--w1                 <val>[,...]        Width(s) for high-index region\n"
	"\t--w2                 <val>[,...]        Width(s) for low-index region\n"
	"\t--n1-width-model     <w0,b0,b1,b2,b3,...>  dn1(w) = b1*(w-w0) + b2*(w-w0)^2 + b3*(w-w0)^3\n"
	"\t--n2-width-model     <w0,b0,b1,b2,b3,...>  dn2(w) = b1*(w-w0) + b2*(w-w0)^2 + b3*(w-w0)^3\n"
	"\t**if using --n#-model and --n#-width-model together specify b0 as 0.0";

int main(int argc, char* argv[])
{
	std::unique_ptr<ctl> ctx = std::make_unique<ctl>();

	try // Parsing CLI
	{
		static struct option long_options[] = {
			{"loss",			required_argument, 0, 'a'},
			{"dutycycle",		required_argument, 0, 'c'},
			{"device",			required_argument, 0, 'd'},
			{"wavelength",		required_argument, 0, 'l'},
			{"period",			required_argument, 0, 'p'},
			{"n-periods",		required_argument, 0, 'N'},
			{"n1",				required_argument, 0, 1},
			{"n2",				required_argument, 0, 2},
			{"n1-model",		required_argument, 0, 3},
			{"n2-model",		required_argument, 0, 4},
			{"loss-model",		required_argument, 0, 5},
			{"w1",				required_argument, 0, 6},
			{"w2",				required_argument, 0, 7},
			{"n1-width-model",	required_argument, 0, 8},
			{"n2-width-model",	required_argument, 0, 9},
			{"dl",				required_argument, 0, 10},
			{"help",			no_argument,       0, 'h'},
			{0, 0, 0, 0}
		};

		int c;
		int option_index = 0;
		
		while ((c = getopt_long(argc, argv, "a:c:d:hl:N:p:", long_options, &option_index)) != -1) 
		{
			switch (c) 
			{
				case 1: // --n1
				{
					std::vector<double> n1;
					parse_numeric<double>(optarg, n1);
		
					if (n1.size() == 1)
						ctx->n1 = std::make_unique<cml>(cml{.constant=n1[0]});
					if (n1.size() > 1)
						ctx->n1 = std::make_unique<cml>(cml{.sampled=std::move(n1)});
					break;
				}
				case 2: // --n2
				{
					std::vector<double> n2;
					parse_numeric<double>(optarg, n2);

					if (n2.size() == 1)
						ctx->n2 = std::make_unique<cml>(cml{.constant=n2[0]});
					if (n2.size() > 1)
						ctx->n2 = std::make_unique<cml>(cml{.sampled=std::move(n2)});
					break;
				}		
				case 3: // --n1-model
				{
					std::vector<double> n1_model;
					if ( parse_numeric<double>(optarg, n1_model) )
					{
						//append not overwrite
						if(ctx->n1 && ctx->n1->width_model)
							ctx->n1->wavelength_model = wavelength_model_t{ .x0 = n1_model[0], 
									.coeffs = std::vector<double>(n1_model.begin() + 1, n1_model.end()) };
						else //overwrite
							ctx->n1 = std::make_unique<cml>(cml{ 
								.wavelength_model = wavelength_model_t{
									.x0 = n1_model[0], 
									.coeffs = std::vector<double>(n1_model.begin() + 1, n1_model.end())
								}
							});
							
					}
					break;
				}
				case 4: // --n2-model
				{
					std::vector<double> n2_model;
					if ( parse_numeric<double>(optarg, n2_model) )
					{
						//append not overwrite
						if(ctx->n2 && ctx->n2->width_model)
							ctx->n2->wavelength_model = wavelength_model_t{ .x0 = n2_model[0], 
									.coeffs = std::vector<double>(n2_model.begin() + 1, n2_model.end()) };
						else //overwrite
							ctx->n2 = std::make_unique<cml>(cml{ 
								.wavelength_model = wavelength_model_t{
									.x0 = n2_model[0], 
									.coeffs = std::vector<double>(n2_model.begin() + 1, n2_model.end())
								}
							});
							
					}
					break;
				}
				case 5: // --loss-model
				{
					std::vector<double> loss_model;
					if ( parse_numeric<double>(optarg, loss_model) )
						ctx->loss = std::make_unique<cml>(cml{ 
							.wavelength_model = wavelength_model_t{
								.x0 = loss_model[0], 
								.coeffs = std::vector<double>(loss_model.begin() + 1, loss_model.end())
							}
						});
					break;
				}
				case 6: // --w1
				{
					parse_numeric<double>(optarg, ctx->width1);
					break;
				}
				case 7: // --w2
				{
					parse_numeric<double>(optarg, ctx->width2);
					break;
				}
				case 8: // --n1-width-model
				{

					std::vector<double> n1_width_model;
					if ( parse_numeric<double>(optarg, n1_width_model) )
					{
						//append not overwrite
						if(ctx->n1 && ctx->n1->wavelength_model)
							ctx->n1->width_model = width_model_t{ .x0 = n1_width_model[0], 
									.coeffs = std::vector<double>(n1_width_model.begin() + 1, n1_width_model.end()) };
						else //overwrite
							ctx->n1 = std::make_unique<cml>(cml{ 
								.width_model = width_model_t{
								.x0 = n1_width_model[0], 
								.coeffs = std::vector<double>(n1_width_model.begin() + 1, n1_width_model.end())
								}
							});
					}
					break;
				}
				case 9: // --n2-width-model
				{
					std::vector<double> n2_width_model;
					if ( parse_numeric<double>(optarg, n2_width_model) )
					{
						//append not overwrite
						if(ctx->n2 && ctx->n2->wavelength_model)
							ctx->n2->width_model = width_model_t{ .x0 = n2_width_model[0], 
									.coeffs = std::vector<double>(n2_width_model.begin() + 1, n2_width_model.end()) };
						else //overwrite
							ctx->n2 = std::make_unique<cml>(cml{ 
								.width_model = width_model_t{
								.x0 = n2_width_model[0], 
								.coeffs = std::vector<double>(n2_width_model.begin() + 1, n2_width_model.end())
								}
							});
					}
					break;
				}
				case 10: // --dl
				{
					char* end = nullptr;
					ctx->dl = std::strtod(optarg, &end);
					break;
				}
				case 'a': // --loss
				{
					std::vector<double> loss;
					parse_numeric<double>(optarg, loss);
					
					if (loss.size() == 1)
						ctx->loss = std::make_unique<cml>(cml{.constant=loss[0]});
					if (loss.size() > 1)
						ctx->loss = std::make_unique<cml>(cml{.sampled=std::move(loss)});
					break;
				}					
				case 'c': // --dutycycle
				{
					parse_numeric<double>(optarg, ctx->duty_cycles, 0.0, 1.0);
					break;
				}
				case 'd': // --device
				{
					string device{optarg};
					if (device == "bragg") 
					{
						ctx->device = BRAGG;
					} 
					break;
				}
				case 'l': // --wavelength
				{
					parse_numeric<double>(optarg, ctx->wavelengths);
					break;
				}
				case 'N': // --n-periods
				{
					parse_numeric<double>(optarg, ctx->Ns);
					break;
				}
				case 'p':
				{
					parse_numeric<double>(optarg, ctx->periods);
					break;
				}		
				case 'h':
				default:
					cerr << usage << endl;
					return -1;
			}
		}
	}
	catch(const exception& ex)
	{
		cerr << "[ERROR] parsing: " << ex.what() << endl;
		return -1;
	}

	try // validation
	{
		// Set default values if not specified
		if (ctx->wavelengths.empty()) 
		{
			cerr << "[ERROR] setup: Must specify at least one wavelength" << endl;
			return -1;
		}

		if(ctx->device != BRAGG) 
		{
			cerr << "[ERROR] setup: supported devices: 'bragg'." << endl;
			return -1;
		}

		if(ctx->dl == 0) 
		{
			cerr << "[WARN] setup: group delay: wavelength interval=0, ignored" << endl;
		}

		if(ctx->dl != 0
			&& (ctx->n1->sampled || ctx->n2->sampled || ctx->loss->sampled) ) 
		{
			cerr << "[WARN] setup: group delay: not supported for sampled data" << endl;
		}
	
		if (ctx->device == BRAGG && ctx->periods.empty())
		{
			cerr << "[ERROR] setup: bragg: Must specify at least one period" << endl;
			return -1;
		}

		if (ctx->device == BRAGG && ctx->duty_cycles.empty()) 
		{
			cerr << "[ERROR] setup: bragg: Must specify dutycycle" << endl;
			return -1;
		}
		
		if (ctx->device == BRAGG && ctx->Ns.empty()) 
		{
			cerr << "[ERROR] setup: bragg: Must specify number of gratings" << endl;
			return -1;
		}

		if (ctx->device == BRAGG && !ctx->n1)
		{
			cerr << "[ERROR] setup: bragg: Must specify n1 with --n1 or --n1-model" << endl;
			return -1;
		}

		if (ctx->device == BRAGG && !ctx->n2)
		{
			cerr << "[ERROR] setup: bragg: Must specify n2 with --n2 or --n2-model" << endl;
			return -1;
		}

		if (ctx->device == BRAGG && !ctx->loss)
		{
			cerr << "[ERROR] setup: bragg: Must specify loss with --loss or --loss-model" << endl;
			return -1;
		}
	}
	catch(const exception& ex)
	{
		cerr << "[ERROR] setup: " << ex.what() << endl;
		return -1;
	}

	try // Running the simulation
	{
		if (ctx->device == BRAGG)
		{
			bool sweep_width1 = !ctx->width1.empty();
			bool sweep_width2 = !ctx->width2.empty();
			bool analyze_group_delay = ctx->dl;
			
			printf("period,duty_cycle,N,wavelength");
			if (sweep_width1) printf(",w1");
			if (sweep_width2) printf(",w2");
			printf(",n1,n2,loss,R,T,phase_r,phase_t");
			if (ctx->dl) printf(",group_delay");
			printf("\n");

			for (const auto& period : ctx->periods)
			{
				for (const auto& duty_cycle : ctx->duty_cycles)
				{
					for (const auto& N : ctx->Ns)
					{
						Bragg grating(period, duty_cycle, N);

						const auto& w1_list = sweep_width1 ? ctx->width1 : std::vector<double>{0.0};
						const auto& w2_list = sweep_width2 ? ctx->width2 : std::vector<double>{0.0};

						for (const auto& w1 : w1_list)
						{
							for (const auto& w2 : w2_list)
							{
								size_t idx = 0;
								for (const auto& wavelength : ctx->wavelengths)
								{
									double n1_val = (*ctx->n1)(wavelength, w1, idx);
									double n2_val = (*ctx->n2)(wavelength, w2, idx);
									double loss_val = (*ctx->loss)(wavelength, 0.0, idx);
									double gdelay_val = 0;

									// Compute reflection and transmission
									auto [R, T, r, t] = grating.scattering_coefficients(
										wavelength,
										n1_val, 
										n2_val, 
										loss_val
									);
									
									//todo: support sampled data
									if( analyze_group_delay 
										&& !ctx->n1->sampled 
											&& !ctx->n2->sampled 
												&& !ctx->loss->sampled)
									{
										auto dwb = wavelength - ctx->dl; //backward difference
										auto dwf = wavelength + ctx->dl; //forward difference

										auto [Rb, Tb, rb, tb] = grating.scattering_coefficients(
											dwb, 
											(*ctx->n1)(dwb, w1, idx), 
											(*ctx->n2)(dwb, w2, idx), 
											loss_val
										);

										auto [Rf, Tf, rf, tf] = grating.scattering_coefficients(
											dwf,
											(*ctx->n1)(dwf, w1, idx), 
											(*ctx->n2)(dwf, w2, idx), 
											loss_val
										);
										//group delay of transmission 
										gdelay_val = group_delay(tb, tf, dwb, dwf);
									}

									// Print results
									printf("%.6g,%.6g,%.6g,%.6g", period, duty_cycle, N, wavelength);
									if (sweep_width1) printf(",%.6g", w1);
									if (sweep_width2) printf(",%.6g", w2);
									printf(",%.6g,%.6g,%.6g,%.6g,%.6g,%.6g,%.6g", n1_val, n2_val, loss_val, R, T, r, t);
									if (analyze_group_delay) printf(",%.6g", gdelay_val);
									printf("\n");

									idx++;
								}
							}
						}
					}
				}
			}
		}
	}
	catch(const exception& ex)
	{
		cerr << "[ERROR] calculation: " << ex.what() << endl;
		return -1;
	}

	return 0;
}