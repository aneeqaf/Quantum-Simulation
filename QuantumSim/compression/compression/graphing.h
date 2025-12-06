//
//  graphing.h
//  compression
//
//  Created by Aneeqa Fatima on 1/6/19.
//  Copyright © 2019 Aneeqa Fatima. All rights reserved.
//

#ifndef graphing_h
#define graphing_h

#include <algorithm>
#include <atomic>
#include <complex>
#include <cstring>
#include <cmath>
#include <complex>
#include <iostream>
#include <stdio.h>
#include <vector>
#include <utility>
#include "math_helper.h"
// #include "Cramer.h"

#ifdef GP
#include "gnuplot-iostream/gnuplot-iostream.h"
#endif

using namespace std;

static complex<float> FindAmpWithMaxMagnitude(const complex<float> *state_vector,
                                              size_t amp_size)
{
    float amp_real = 0, amp_imag = 0;

    for (size_t i = 0; i < amp_size; ++i)
    {
        if (abs(amp_real) > abs(amp_imag))
        {
            if (abs(amp_real) < abs(state_vector[i].real()) || abs(amp_real) < abs(state_vector[i].imag()))
            {
                amp_real = state_vector[i].real();
                amp_imag = state_vector[i].imag();
            }
        }
        else
        {
            if (abs(amp_imag) < abs(state_vector[i].real()) || abs(amp_imag) < abs(state_vector[i].imag()))
            {
                amp_real = state_vector[i].real();
                amp_imag = state_vector[i].imag();
            }
        }
    }

    if (amp_real < 0)
        amp_real *= -1;
    if (amp_imag < 0)
        amp_imag *= -1;

    return complex<float>(amp_real, amp_imag);
}

static void ApplyUniformTransformToStateVector(complex<float> *state_vector,
                                               size_t state_vector_size)
{
    for (size_t i = 0; i < state_vector_size; ++i)
    {
        double original_phase = atan2((double)state_vector[i].imag(), (double)state_vector[i].real());
        original_phase = original_phase < 0 ? original_phase + (2.0 * M_PI) : original_phase;
        double PT_probability = norm(complex<double>(state_vector[i]));
        double uniform_probability = 1.0 - exp(-PT_probability * (double)state_vector_size);

        double uniform_mag = sqrt(uniform_probability);
        state_vector[i] = complex<double>(uniform_mag * cos(original_phase), uniform_mag * sin(original_phase));
        //        cout << state_vector[i] << "; ";
    }
}

static void ApplyPTTransformToStateVector(complex<float> *state_vector,
                                          size_t state_vector_size)
{
    for (size_t i = 0; i < state_vector_size; ++i)
    {
        double original_phase = atan2((double)state_vector[i].imag(), (double)state_vector[i].real());
        original_phase = original_phase < 0 ? original_phase + (2.0 * M_PI) : original_phase;
        double uniform_probability = norm(complex<double>(state_vector[i]));
        double PT_probability = -log(abs(1.0 - uniform_probability)) / (double)state_vector_size;

        double PT_mag = sqrt(PT_probability);
        state_vector[i] = complex<double>(PT_mag * cos(original_phase), PT_mag * sin(original_phase));
    }
}

static void PlotCWFrequency(const atomic<size_t> *codewords_freq,
                            const size_t num_codewords,
                            size_t num_qubits)
{
#ifdef GP
    Gnuplot gp;

    vector<pair<unsigned short, size_t>> codewords_freq_idx;
    for (size_t i = 0; i < num_codewords; ++i)
        codewords_freq_idx.push_back(make_pair(i, codewords_freq[i].load()));

    size_t freq_max = 0;
    for (size_t i = 0; i < num_codewords; ++i)
    {
        if (codewords_freq[i].load() > freq_max)
            freq_max = codewords_freq[i].load();
    }

    string filename = "output/" + to_string(num_codewords + 1) + "_codewords_freq";

    gp << "reset\nset nokey\n";
    gp << "set title \"" << num_qubits << "q: " << num_codewords << " Codewords Frequency\" font \",14\"\n";
    //    gp << "set ylabel 'Amplitude frequency'\n";
    //    gp << "set xlabel 'Codewords'\n";
    //    gp << "set term png\n";
    gp << "set output '" << filename + to_string(num_qubits) << ".png'\n";
    gp << "set tics font 'Times New Roman,12'\n";
    gp << "cw=" << num_codewords << "\nmax_f=" << freq_max << "\n";
    gp << "title(c) = sprintf(\"\\nAmplitudes to %i Codewords Frequency\"" << ", cw) \n";

    gp << "set xr[0:cw]\nset yr[0:max_f]\n";
    gp << "plot " << gp.file1d(codewords_freq_idx, "file.txt") << " using 1:2 with lines lw 2 \n";

#endif
}

static void PlotPT(const complex<float> *amps,
                   size_t num_qubits)
{
#ifdef GP
    size_t amp_size = 1ull << num_qubits;

    vector<pair<double, double>> amps_g;
    for (size_t i = 0; i < amp_size; ++i)
        amps_g.push_back(make_pair(exp(-norm(amps[i]) * amp_size), norm(amps[i]) * amp_size));

    double temp_lambda = 0;
    double prob_0 = 0;
    for (size_t i = 0; i < amp_size; ++i)
    {
        double p = norm(amps[i]);
        if (p == 0)
        {
            prob_0 += 1.0 / amp_size;
            continue;
        }
        temp_lambda += p;
    }
    temp_lambda /= (double)amp_size;

    string filename = "output/" + to_string(num_qubits) + "_PDF";

    Gnuplot gp;

    gp << "reset\nset nokey\n";
    gp << "set title \"" << num_qubits << "q PT\" font \",14\"\n";
    //    gp << "set ylabel 'Amplitude frequency'\n";
    //    gp << "set xlabel 'Codewords'\n";
    gp << "set term png\n";
    gp << "set output '" << filename << ".png'\n";
    gp << "set tics font 'Times New Roman,12'\n";
    //    gp << "set logscale y\n";
    gp << "plot " << gp.file1d(amps_g, "dist.txt") << " using 2:1 with point lw 2 \n";

#endif
}

static complex<double>
CalculateCDFofErlangDistP(complex<double> amp,
                          int k,
                          double lambda)
{
    double PT_mag = abs(amp);
    double PT_probability = PT_mag * PT_mag;
    double Np = PT_probability * lambda;
    double uniform_probability = 0;

    for (size_t n = 0; n < k; ++n)
        uniform_probability += (1 / Factorial(n)) * pow(Np, n) * exp(-Np);

    double uniform_mag = sqrt(1 - uniform_probability);
    amp = complex<double>(uniform_mag * (amp.real() / PT_mag), uniform_mag * (amp.imag() / PT_mag));

    return amp;
}

static complex<double>
CalculateCDFofGammaDistP(complex<double> amp,
                         double k,
                         double lambda)
{
    double PT_mag = abs(amp);
    PT_mag = PT_mag > 0 ? PT_mag : 1;
    double PT_probability = PT_mag * PT_mag;
    double Np = (PT_probability)*lambda;
    double uniform_probability = gammp(k, Np);
    double uniform_mag = sqrt(uniform_probability);
    amp = complex<double>(uniform_mag * (amp.real() / PT_mag), uniform_mag * (amp.imag() / PT_mag));

    return amp;
}

static complex<double>
CalculateCDFofGammaDistPModified(complex<double> amp,
                                 double k,
                                 double lambda)
{
    double PT_mag = abs(amp);
    PT_mag = PT_mag > 0 ? PT_mag : 1;
    double PT_probability = PT_mag * PT_mag;
    double Np = (PT_probability)*lambda;
    double uniform_probability = gammp(k, Np) - gammp(k, k);
    double uniform_mag = sqrt(uniform_probability);
    amp = complex<double>(uniform_mag * (amp.real() / PT_mag), uniform_mag * (amp.imag() / PT_mag));

    return amp;
}

static complex<double>
CalculateCDFofUniformDistP(complex<double> amp,
                           double num_amps)
{
    double PT_mag = abs(amp);
    PT_mag = PT_mag > 0 ? PT_mag : 1;
    double PT_probability = PT_mag * PT_mag;
    double uniform_probability = PT_probability / num_amps;
    double uniform_mag = sqrt(uniform_probability);
    amp = complex<double>(uniform_mag * (amp.real() / PT_mag), uniform_mag * (amp.imag() / PT_mag));

    return amp;
}

static pair<float, float> CalculateKAndLambda(const complex<float> *amps,
                                              size_t size)
{
    double mean = 0, variance = 0;
    size_t num_amps = 0;

    for (size_t i = 0; i < size; ++i)
    {
        double p = norm(amps[i]);
        if (p > 0)
        { // 1.0/((double)size * (double)size)) {
            ++num_amps;
            mean += p;
        }
    }

    mean /= (double)num_amps;

    for (size_t i = 0; i < size; ++i)
    {
        double p = norm(amps[i]);
        //        if (p > 1.0/((double)size * (double)size))
        variance += (p - mean) * (p - mean);
    }

    variance /= (double)(size - 1);
    //    cout << mean << endl;
    double k = (mean * mean) / variance;
    double lambda = mean / variance;

    return make_pair(k, lambda);
}

static void PlotCDF(complex<float> *amps,
                    size_t num_qubits)
{
#ifdef GP
    size_t amp_size = 1ull << num_qubits;
    size_t size = amp_size;

    vector<double> probs;
    for (size_t i = 0; i < size; ++i)
        probs.push_back(norm(amps[i]));

    sort(probs.begin(), probs.end());

    double cumultive_sum = 0;
    vector<pair<double, double>> amps_g;
    for (size_t i = 0; i < size; ++i)
    {
        cumultive_sum += 1.0 / (double)amp_size;
        amps_g.push_back(make_pair(cumultive_sum, probs[i]));
    }

    double mean = 0;
    size_t num_amps = 0;

    for (size_t i = 0; i < size; ++i)
    {
        double p = norm(amps[i]);
        if (p >= 1.0 / ((double)amp_size * (double)amp_size))
        {
            ++num_amps;
            mean += p;
        }
    }

    mean /= (double)num_amps;

    pair<float, float> k_lambda = CalculateKAndLambda(amps, amp_size);
    double k = k_lambda.first;
    double lambda = k_lambda.second;

    vector<pair<double, double>> e_cdf;
    for (size_t i = 0; i < size; ++i)
    {
        complex<double> temp = CalculateCDFofGammaDistP(amps[i], k, lambda);
        e_cdf.push_back(make_pair(norm(temp), norm(amps[i])));
    }

    //    vector<pair<double,double>> e_cdf1;
    //    for (size_t i = 0; i < size ; ++i)
    //        e_cdf1.push_back(make_pair(1.0 - exp(-norm(amps[i]) * (1/mean)), norm(amps[i])));
    //
    //    vector<pair<double,double>> cdf_pt;
    //    for (size_t i = 0; i < size ; ++i)
    //        cdf_pt.push_back(make_pair(norm(CalculateCDFofGammaDistP(amps[i], 1, amp_size)), norm(amps[i])));

    vector<pair<float, float>> transformed_x_y_g;
    vector<pair<float, float>> transformed_x_y_u;
    for (size_t i = 0; i < size; ++i)
    {
        double p = norm(amps[i]);
        if (p > (k / lambda))
        {
            complex<double> new_amp = CalculateCDFofGammaDistPModified(amps[i], k, lambda);
            transformed_x_y_g.push_back(make_pair(norm(new_amp) + gammp(k, k), norm(amps[i])));
        }
        else
        {
            complex<double> new_amp = CalculateCDFofUniformDistP(amps[i], k / lambda);
            transformed_x_y_u.push_back(make_pair(norm(new_amp), norm(amps[i])));
        }
    }

    string filename = "output/" + to_string(num_qubits) + "_CDF";

    Gnuplot gp;

    gp << setprecision(3);
    gp << "reset\nset nokey\n";
    gp << "set title \"" << num_qubits << "q CDF\" font \",14\"\n";
    //    gp << "set ylabel 'Amplitude frequency'\n";
    //    gp << "set xlabel 'Codewords'\n";
    //    gp << "set term png\n";
    //    gp << "set output '" << filename << ".png'\n";
    gp << "set tics font 'Times New Roman,12'\n";
    gp << "set key right bottom\n";
    //    gp << "set logscale y\n";
    gp << "plot " << gp.file1d(amps_g, "dist.txt") << " using 2:1 with point pt 5 ps 0.5 lc rgb \"black\" t \"Empirical CDF\","
       << gp.file1d(e_cdf, "dist2.txt") << " using 2:1 with point pt 5 ps 0.5 lc rgb \"purple\" t \"Gamma CDF : k = " << k << ", lambda = " << lambda << "\","
       << gp.file1d(transformed_x_y_g, "dist3.txt") << " using 2:1 with point pt 5 ps 0.5 lc rgb \"red\" t \"Modified Gamma CDF x > k/lambda\","
       << gp.file1d(transformed_x_y_u, "dist1.txt") << " using 2:1 with point pt 5 ps 0.5 lc rgb \"green\" t \"Uniform CDF x <= k/lambda\"\n";

#endif
}

// static void PlotUniformSpiralAndAmpDensity(const string& filename,
//                                     const complex<float>* state_vector,
//                                     size_t state_vector_size,
//                                     const Cramer& cramer)
//{
// #ifdef GP
//     Gnuplot gp;
// #endif
//
//     vector<pair<float, float>> codewords;
//
//     vector<pair<float, float>> xy_pts;
//     for(size_t i = 0; i < state_vector_size; ++i)
//         xy_pts.push_back(make_pair(state_vector[i].real(), state_vector[i].imag()));
//
//
//     complex<float> max_amp = FindAmpWithMaxMagnitude(state_vector, state_vector_size);
//     int max_R = abs(max_amp)/(cramer.GetFactorOfDistBetweenTurns() * M_PI);
//     cramer.GetCWForPlotting(codewords);
//
// #ifdef GP
//     //    gp << "if (!exists(\"MP_LEFT\"))   MP_LEFT = .1\n";
//     //    gp <<  "if (!exists(\"MP_RIGHT\"))  MP_RIGHT = .95\n";
//     //    gp << "if (!exists(\"MP_BOTTOM\")) MP_BOTTOM = .1\n";
//     //    gp << "if (!exists(\"MP_TOP\"))    MP_TOP = .8\n";
//     //    gp << "if (!exists(\"MP_GAP\"))    MP_GAP = 0.1\n";
//
//     gp << "reset\nset nokey\n";
//     gp << "set xtics rotate\n";
//     gp << "set bmargin 5\n";
//     gp << "set termoption enhanced\n";
//     gp << "set term png\n";
//     gp << "set output '" << filename << ".png'\n";
//     gp << "set parametric\nset size ratio -1\nset samples 1e5\n";
//     gp << "set tics font 'Times New Roman,18'\n";
//     gp << "set linetype 1 linecolor rgb \"blue\"\n";
//     gp << "set linetype 2 linecolor rgb \"light-blue\"\n";
//     gp << "eb=" << cramer.GetFactorOfDistBetweenTurns() << "\ncw=" << codewords.size() << "\n q=" << log2(state_vector_size) << "\n";
//     gp << "r=" << cramer.GetMinInnerRadius() << "\n R=" << cramer.GetMaxOuterRadius()
//     << "\n maxR=" << max_R <<  "\n";
//     gp << "title(eb) = sprintf(\"\\nUniform distribution and uniform spirals (%iq)\\n\\n r=%.4f  R=%.4f  \\n\\n codewords=%i\""
//     << ", q,  r , R , cw) \n";
//     gp << "set multiplot layout 1,2 columnsfirst title title(eb) font 'Latin Modern Math, 20'\n";
//
//     gp << "x(t) = " << cramer.GetFactorOfDistBetweenTurns() << "*t*cos(t)\n";
//     gp << "y(t) = " << cramer.GetFactorOfDistBetweenTurns() << "*t*sin(t)\n";
//
//     gp << setprecision(2);
//     //    gp << "set title \"Amplitudes after uniform transform \"\n";
//     //    gp << "L = " << cramer.GetFactorOfDistBetweenTurns() * max_R * PI << "\n";
//     //    gp << "set xr[-L:L]\nset yr[-L:L]\n";
//     //    gp << "set xtics -L , L/2 , L\nset ytics -L, L/2, L\nset grid\n";
//     //    gp << "plot " << gp.file1d(xy_pts, "file.dat")
//     //    << " with points pt 7 ps 0.2 lc 'grey'\n";
//     //
//     gp << "set title \"Zoom out \"\n";
//     gp << "L = " << cramer.GetFactorOfDistBetweenTurns() * max_R * PI << "\n";
//     gp << "set xr[-L:L]\nset yr[-L:L]\n";
//     gp << "set xtics -L , L/2 , L\nset ytics -L, L/2, L\nset grid\n";
//     gp << "plot [r*pi:maxR*pi]" << gp.file1d(xy_pts, "file.dat")
//     << " with points pt 7 ps 0.2 lc 'grey', (t <= R * pi ? x(t): 1/0), (t <= R * pi ? y(t): 1/0) ls 1, "
//     << "(t > R * pi ? x(t): 1/0), (t > R * pi ? y(t): 1/0) ls 2, "
//     << gp.file1d(codewords, "file.txt") << " using 1:2 with points pt 7 ps 0.4 lc 'red'\n";
//
//
//     gp << "set title \"Zoom in \"\n";
//     //    gp << "L = " << cramer.GetFactorOfDistBetweenTurns() * (3 * 100)<< "\n";
//     //    gp << "set xr[-L:L]\nset yr[-L:L]\n";
//     //    gp << "set xtics -L , L/2 , L\nset ytics -L, L/2 , L\nset grid\n";
//     //    gp << "plot [r*pi:maxR*pi] " << gp.file1d(xy_pts, "file.dat")  << " with points pt 7 ps 0.2 lc 'grey', "
//     //    << " (t <= R * pi ? x(t): 1/0), (t <= R * pi ? y(t): 1/0) ls 1, "
//     //    << "(t > R * pi ? x(t): 1/0), (t > R * pi ? y(t): 1/0) ls 2\n";
//     //
//     gp << "L = " << cramer.GetFactorOfDistBetweenTurns() * (1.5 * 100)<< "\n";
//     gp << "set xr[-L:L]\nset yr[-L:L]\n";
//     gp << "set xtics -L , L/2 , L\nset ytics -L, L/2 , L\nset grid\n";
//     gp << "plot [r*pi:maxR*pi] " << gp.file1d(xy_pts, "file.dat") << " with points pt 7 ps 0.2 lc 'grey', "
//     << " (t <= R * pi ? x(t): 1/0), (t <= R * pi ? y(t): 1/0) ls 1, "
//     << "(t > R * pi ? x(t): 1/0), (t > R * pi ? y(t): 1/0) ls 2, "
//     << gp.file1d(codewords, "file.txt") << " using 1:2 with points pt 7 ps 0.4 lc 'red'\n";
//
// #endif
// }

static void PlotPhaseError(const vector<float> &error,
                           size_t num_codewords)
{
#ifdef GP
    vector<pair<double, size_t>> error_plot(num_codewords);
    for (int i = 10; i < num_codewords; ++i)
    {
        error_plot[i] = make_pair(error[i], i);
    }

    Gnuplot gp;

    gp << "reset\nset nokey\n";
    gp << "set title \"Phase Error Plot\" font \",14\"\n";
    //    gp << "set ylabel 'Amplitude frequency'\n";
    //    gp << "set xlabel 'Codewords'\n";
    //    gp << "set term png\n";
    //    gp << "set output '" << filename << ".png'\n";
    gp << "set tics font 'Times New Roman,12'\n";
    //    gp << "set logscale y\n";
    gp << "plot " << gp.file1d(error_plot, "dist.txt") << " using 2:1 with linespoints pt 5 ps 0.5 lc rgb \"black\"\n";

#endif
}

static void PlotAmpsAroundCW(const vector<complex<float>> &amps,
                             const complex<float> cw_amp,
                             size_t codeword)
{
#ifdef GP
    vector<pair<double, double>> amps_x_y;
    for (int i = 0; i < amps.size(); ++i)
        amps_x_y.push_back(make_pair(amps[i].real(), amps[i].imag()));

    vector<pair<double, double>> one_pt(1, make_pair(cw_amp.real(), cw_amp.imag()));

    string filename = "output/cw_plots/codeword_plots";

    Gnuplot gp;

    gp << "reset\nset nokey\n";
    gp << "set title \"" << amps.size() << " amps mapping to codeword " << codeword << " plot\" font \",14\"\n";
    //    gp << "set ylabel 'Amplitude frequency'\n";
    //    gp << "set xlabel 'Codewords'\n";
    gp << "set term png\n";
    gp << "set output '" << filename << codeword << ".png'\n";
    gp << "set tics font 'Times New Roman,12'\n";
    //    gp << "set logscale y\n";
    gp << "plot " << gp.file1d(amps_x_y, "dist.txt") << " using 1:2 with point pt 5 ps 0.5 lc rgb \"black\", "
       << gp.file1d(one_pt, "dist1.txt") << " using 1:2 with point pt 5 ps 2 lc rgb \"red\"\n";

#endif
}

static void PlotErrorBins(const vector<vector<float>> &error_bins)
{
#ifdef GP
    vector<pair<float, size_t>> phase_error_bins_x_y;
    vector<pair<float, size_t>> mag_error_bins_x_y;
    for (int i = 0; i < error_bins[0].size(); ++i)
    {
        mag_error_bins_x_y.push_back(make_pair(error_bins[0][i], i));
        phase_error_bins_x_y.push_back(make_pair(error_bins[1][i], i));
    }

    string filename = "output/error_bin_plots";
    Gnuplot gp;

    gp << "reset\nset nokey\n";
    //    gp << "set title \"" << error_bins.size() << " error bins avg error plot\" font \",14\"\n";
    gp << "set style histogram columnstacked title textcolor lt -1\n";
    gp << "set style data boxes\n";
    gp << "set multiplot layout 2,1 title \"" << error_bins[0].size() << " error bins avg error plot\" font \",14\"\n";
    //    gp << "set xlabel \"error bins\"";
    gp << "set term png\n";
    gp << "set style fill solid border lt -1 \n";
    gp << "set output '" << filename << error_bins.size() << ".png'\n";
    gp << "set tics font 'Times New Roman,12'\n";
    gp << "set key\n";
    gp << "plot " << gp.file1d(mag_error_bins_x_y, "dist1.txt") << " using 2:1 lc \"grey\" t \"Magnitude\"\n";
    gp << "plot " << gp.file1d(phase_error_bins_x_y, "dist2.txt") << " using 2:1 lc \"magenta\" t \"Phase\"\n";

#endif
}

static void PlotPointsOnComplexPlane(const complex<float> *original,
                                     const size_t size)
{
#ifdef GP
    pair<float, float> k_lambda = CalculateKAndLambda(original, size);
    double k = k_lambda.first;
    double lambda = k_lambda.second;

    size_t num_zero_amps = 0;
    for (size_t i = 0; i < size; ++i)
    {
        double p = norm(original[i]);
        if (p > 1.0 / ((double)size * (double)size))
            ++num_zero_amps;
    }

    vector<pair<float, float>> orig_x_y;
    vector<pair<float, float>> gamma_x_y;
    vector<pair<float, float>> transformed_x_y_g;
    vector<pair<float, float>> transformed_x_y_u;
    for (size_t i = 0; i < size; ++i)
    {
        double p = norm(original[i]);
        orig_x_y.push_back(make_pair(original[i].real(), original[i].imag()));
        complex<double> new_amp = CalculateCDFofGammaDistP(original[i], k, lambda);
        gamma_x_y.push_back(make_pair(new_amp.real(), new_amp.imag()));
        if (p > (k / lambda))
        {
            new_amp = CalculateCDFofGammaDistPModified(original[i], k, lambda);
            transformed_x_y_g.push_back(make_pair(new_amp.real(), new_amp.imag()));
        }
        else
        {
            new_amp = CalculateCDFofUniformDistP(original[i], k / lambda);
            transformed_x_y_u.push_back(make_pair(new_amp.real(), new_amp.imag()));
        }
    }

    size_t qubits = log2(size);

    Gnuplot gp;

    gp << "reset\nset nokey\n";
    gp << "set multiplot layout 2,2 title \"" << qubits << " qubit amplitudes plots\" font \",14\"\n";
    //    gp << "set ylabel 'Amplitude frequency'\n";
    //    gp << "set xlabel 'Codewords'\n";
    //    gp << "set term png\n";
    gp << "set output '" << qubits << "amplitudes.png'\n";
    gp << "set tics font 'Times New Roman,12'\n";
    //    gp << "set logscale y\n";
    gp << "set key box opaque\n";
    gp << "plot " << gp.file1d(orig_x_y, "dist.txt") << " using 1:2 with point pt 5 ps 0.2 lc rgb \"blue\" title \"Original\"\n";
    gp << "plot " << gp.file1d(gamma_x_y, "dist3.txt") << " using 1:2 with point pt 5 ps 0.2 lc rgb \"purple\" title \" Gamma CDF \"\n";
    gp << "plot " << gp.file1d(transformed_x_y_g, "dist1.txt") << " using 1:2 with point pt 5 ps 0.2 lc rgb \"red\" title \" Gamma CDF : x > k/lambda\"\n";
    gp << "plot " << gp.file1d(transformed_x_y_u, "dist2.txt") << " using 1:2 with point pt 5 ps 0.2 lc rgb \"green\" title \" Uniform CDF : x <= k/lambda\"\n";

#endif
}

#endif /* graphing_h */
