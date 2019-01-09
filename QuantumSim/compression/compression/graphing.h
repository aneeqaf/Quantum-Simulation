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
#include <complex>
#include <cstring>
#include <cmath>
#include <complex>
#include <iostream>
#include <stdio.h>
#include <vector>
#include <utility>
#include "Cramer.h"

#ifdef GP
#include "gnuplot-iostream/gnuplot-iostream.h"
#endif

using namespace std;

static complex<float> FindAmpWithMaxMagnitude(const complex<float>* state_vector,
                                       size_t amp_size)
{
    float amp_real = 0, amp_imag = 0;
    
    for (size_t i = 0; i < amp_size; ++i) {
        if (abs(amp_real) > abs(amp_imag)) {
            if (abs(amp_real) < abs(state_vector[i].real()) || abs(amp_real) < abs(state_vector[i].imag())) {
                amp_real = state_vector[i].real();
                amp_imag = state_vector[i].imag();
            }
        }
        else {
            if (abs(amp_imag) < abs(state_vector[i].real()) || abs(amp_imag) < abs(state_vector[i].imag())) {
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

static void ApplyUniformTransformToStateVector(complex<float>* state_vector,
                                               size_t state_vector_size)
{
    for (size_t i = 0; i < state_vector_size; ++i) {
        double original_phase = atan2((double)state_vector[i].imag(), (double)state_vector[i].real());
        original_phase = original_phase < 0 ? original_phase + ( 2.0 * M_PI) : original_phase;
        double PT_probability = norm(complex<double>(state_vector[i]));
        double uniform_probability = 1.0 - exp(-PT_probability * (double)state_vector_size);
        
        double uniform_mag = sqrt(uniform_probability);
        state_vector[i] = complex<double>(uniform_mag * cos(original_phase), uniform_mag * sin(original_phase));
        //        cout << state_vector[i] << "; ";
    }
}

static void ApplyPTTransformToStateVector(complex<float>* state_vector,
                                          size_t state_vector_size)
{
    for (size_t i = 0; i < state_vector_size; ++i) {
        double original_phase = atan2((double)state_vector[i].imag(), (double)state_vector[i].real());
        original_phase = original_phase < 0 ? original_phase + ( 2.0 * M_PI) : original_phase;
        double uniform_probability = norm(complex<double>(state_vector[i]));
        double PT_probability = -log(abs(1.0 - uniform_probability)) / (double)state_vector_size;
        
        double PT_mag = sqrt(PT_probability);
        state_vector[i] = complex<double>(PT_mag * cos(original_phase), PT_mag * sin(original_phase));
    }
}

static void PlotCWFrequency(const vector<size_t>& codewords_freq,
                            size_t num_qubits)
{
#ifdef GP
    Gnuplot gp;
    
    vector<pair<unsigned short,size_t>> codewords_freq_idx;
    for (size_t i = 1; i < codewords_freq.size(); ++i)
        codewords_freq_idx.push_back(make_pair(i, codewords_freq[i]));
    
    size_t freq_max = 0;
    for (size_t i = 1; i < codewords_freq.size(); ++i) {
        if(codewords_freq[i] > freq_max)
            freq_max = codewords_freq[i];
    }
    
    string filename = to_string(codewords_freq.size()) + "_codewords_freq";
    
    gp << "reset\nset nokey\n";
    gp << "set title \"" << num_qubits << "q: " << codewords_freq.size() << " Codewords Frequency\" font \",14\"\n";
    //    gp << "set ylabel 'Amplitude frequency'\n";
    //    gp << "set xlabel 'Codewords'\n";
    gp << "set term png\n";
    gp << "set output '" << filename + to_string(num_qubits) << ".png'\n";
    gp << "set tics font 'Times New Roman,12'\n";
    gp << "cw=" << codewords_freq.size() << "\nmax_f=" << freq_max << "\n";
    gp << "title(c) = sprintf(\"\\nAmplitudes to %i Codewords Frequency\"" << ", cw) \n";
    
    gp << "set xr[0:cw]\nset yr[0:max_f]\n";
    gp << "plot " << gp.file1d(codewords_freq_idx, "file.txt") << " using 1:2 with lines lw 2 \n";
    
#endif
}

static void PlotPT(const complex<float>* amps,
                   size_t num_qubits)
{
    size_t amp_size = 1ull << num_qubits;
    
    vector<pair<double,double>> amps_g;
    for (size_t i = 0; i < amp_size ; ++i)
        amps_g.push_back(make_pair(exp(-norm(amps[i]) * amp_size), norm(amps[i]) * amp_size));
    
    double temp_lambda = 0;
    double prob_0 = 0;
    for (size_t i = 0; i < amp_size; ++i) {
        double p = norm(amps[i]);
        if (p == 0) {
            prob_0 += 1.0/amp_size;
            continue;
        }
        temp_lambda += p;
    }
    temp_lambda /= (double)amp_size;
    
    string filename = to_string(num_qubits) + "_PDF";
    
#ifdef GP
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

static void PlotCDF(complex<float>* amps,
                    size_t num_qubits)
{
    size_t amp_size = 1ull << num_qubits;
    
    vector<double> probs;
    for (size_t i = 0; i < amp_size; ++i)
        probs.push_back(norm(amps[i]));
    
    sort(probs.begin(), probs.end());
    
    double cumultive_sum = 0;
    vector<pair<double,double>> amps_g;
    for (size_t i = 0; i < amp_size ; ++i) {
        cumultive_sum += 1.0/(double)amp_size;
        amps_g.push_back(make_pair(cumultive_sum, probs[i]));
    }
    
    double lambda = 0;
    size_t non0_p = 0;
    for (size_t i = 0; i < amp_size; ++i) {
        if (probs[i] > 1/(amp_size * amp_size)) {
            ++non0_p;
            lambda += probs[i];
        }
    }
    lambda /= (double)non0_p;
    lambda = 1.0/lambda;
    
    vector<pair<double,double>> e_cdf;
    for (size_t i = 0; i < amp_size ; ++i) {
        e_cdf.push_back(make_pair(1.0 - exp(-norm(amps[i]) * lambda), norm(amps[i])));
    }
    
    
    vector<pair<double,double>> cdf_pt;
    for (size_t i = 0; i < amp_size ; ++i)
        cdf_pt.push_back(make_pair(1.0 - exp(-norm(amps[i]) * (double)amp_size), norm(amps[i])));
    
    string filename = to_string(num_qubits) + "_CDF";
    
#ifdef GP
    Gnuplot gp;
    
    gp << "reset\nset nokey\n";
    gp << "set title \"" << num_qubits << "q CDF\" font \",14\"\n";
    //    gp << "set ylabel 'Amplitude frequency'\n";
    //    gp << "set xlabel 'Codewords'\n";
    gp << "set term png\n";
    gp << "set output '" << filename << ".png'\n";
    gp << "set tics font 'Times New Roman,12'\n";
    gp << "set key right bottom\n";
    //    gp << "set logscale y\n";
    gp << "plot " << gp.file1d(amps_g, "dist.txt") << " using 2:1 with point pt 5 ps 0.5 lc rgb \"black\" t \"empirical\","
    << gp.file1d(e_cdf, "dist2.txt") << " using 2:1 with point pt 5 ps 0.5 lc rgb \"red\" t \"calculated empirically\","
    << gp.file1d(cdf_pt, "dist1.txt") << " using 2:1 with point pt 5 ps 0.5 lc rgb \"purple\" t \"analytical\"\n";
    
#endif
}

static void PlotUniformSpiralAndAmpDensity(const string& filename,
                                    const complex<float>* state_vector,
                                    size_t state_vector_size,
                                    const Cramer& cramer)
{
#ifdef GP
    Gnuplot gp;
#endif
    
    vector<pair<float, float>> codewords;
    
    vector<pair<float, float>> xy_pts;
    for(size_t i = 0; i < state_vector_size; ++i)
        xy_pts.push_back(make_pair(state_vector[i].real(), state_vector[i].imag()));
    
    
    complex<float> max_amp = FindAmpWithMaxMagnitude(state_vector, state_vector_size);
    int max_R = abs(max_amp)/(cramer.GetFactorOfDistBetweenTurns() * M_PI);
    cramer.GetCWForPlotting(codewords);
    
#ifdef GP
    //    gp << "if (!exists(\"MP_LEFT\"))   MP_LEFT = .1\n";
    //    gp <<  "if (!exists(\"MP_RIGHT\"))  MP_RIGHT = .95\n";
    //    gp << "if (!exists(\"MP_BOTTOM\")) MP_BOTTOM = .1\n";
    //    gp << "if (!exists(\"MP_TOP\"))    MP_TOP = .8\n";
    //    gp << "if (!exists(\"MP_GAP\"))    MP_GAP = 0.1\n";
    
    gp << "reset\nset nokey\n";
    gp << "set xtics rotate\n";
    gp << "set bmargin 5\n";
    gp << "set termoption enhanced\n";
    gp << "set term png\n";
    gp << "set output '" << filename << ".png'\n";
    gp << "set parametric\nset size ratio -1\nset samples 1e5\n";
    gp << "set tics font 'Times New Roman,18'\n";
    gp << "set linetype 1 linecolor rgb \"blue\"\n";
    gp << "set linetype 2 linecolor rgb \"light-blue\"\n";
    gp << "eb=" << cramer.GetFactorOfDistBetweenTurns() << "\ncw=" << codewords.size() << "\n q=" << log2(state_vector_size) << "\n";
    gp << "r=" << cramer.GetMinInnerRadius() << "\n R=" << cramer.GetMaxOuterRadius()
    << "\n maxR=" << max_R <<  "\n";
    gp << "title(eb) = sprintf(\"\\nUniform distribution and uniform spirals (%iq)\\n\\n r=%.4f  R=%.4f  \\n\\n codewords=%i\""
    << ", q,  r , R , cw) \n";
    gp << "set multiplot layout 1,2 columnsfirst title title(eb) font 'Latin Modern Math, 20'\n";
    
    gp << "x(t) = " << cramer.GetFactorOfDistBetweenTurns() << "*t*cos(t)\n";
    gp << "y(t) = " << cramer.GetFactorOfDistBetweenTurns() << "*t*sin(t)\n";
    
    gp << setprecision(2);
    //    gp << "set title \"Amplitudes after uniform transform \"\n";
    //    gp << "L = " << cramer.GetFactorOfDistBetweenTurns() * max_R * PI << "\n";
    //    gp << "set xr[-L:L]\nset yr[-L:L]\n";
    //    gp << "set xtics -L , L/2 , L\nset ytics -L, L/2, L\nset grid\n";
    //    gp << "plot " << gp.file1d(xy_pts, "file.dat")
    //    << " with points pt 7 ps 0.2 lc 'grey'\n";
    //
    gp << "set title \"Zoom out \"\n";
    gp << "L = " << cramer.GetFactorOfDistBetweenTurns() * max_R * PI << "\n";
    gp << "set xr[-L:L]\nset yr[-L:L]\n";
    gp << "set xtics -L , L/2 , L\nset ytics -L, L/2, L\nset grid\n";
    gp << "plot [r*pi:maxR*pi]" << gp.file1d(xy_pts, "file.dat")
    << " with points pt 7 ps 0.2 lc 'grey', (t <= R * pi ? x(t): 1/0), (t <= R * pi ? y(t): 1/0) ls 1, "
    << "(t > R * pi ? x(t): 1/0), (t > R * pi ? y(t): 1/0) ls 2, "
    << gp.file1d(codewords, "file.txt") << " using 1:2 with points pt 7 ps 0.4 lc 'red'\n";
    
    
    gp << "set title \"Zoom in \"\n";
    //    gp << "L = " << cramer.GetFactorOfDistBetweenTurns() * (3 * 100)<< "\n";
    //    gp << "set xr[-L:L]\nset yr[-L:L]\n";
    //    gp << "set xtics -L , L/2 , L\nset ytics -L, L/2 , L\nset grid\n";
    //    gp << "plot [r*pi:maxR*pi] " << gp.file1d(xy_pts, "file.dat")  << " with points pt 7 ps 0.2 lc 'grey', "
    //    << " (t <= R * pi ? x(t): 1/0), (t <= R * pi ? y(t): 1/0) ls 1, "
    //    << "(t > R * pi ? x(t): 1/0), (t > R * pi ? y(t): 1/0) ls 2\n";
    //
    gp << "L = " << cramer.GetFactorOfDistBetweenTurns() * (1.5 * 100)<< "\n";
    gp << "set xr[-L:L]\nset yr[-L:L]\n";
    gp << "set xtics -L , L/2 , L\nset ytics -L, L/2 , L\nset grid\n";
    gp << "plot [r*pi:maxR*pi] " << gp.file1d(xy_pts, "file.dat") << " with points pt 7 ps 0.2 lc 'grey', "
    << " (t <= R * pi ? x(t): 1/0), (t <= R * pi ? y(t): 1/0) ls 1, "
    << "(t > R * pi ? x(t): 1/0), (t > R * pi ? y(t): 1/0) ls 2, "
    << gp.file1d(codewords, "file.txt") << " using 1:2 with points pt 7 ps 0.4 lc 'red'\n";
    
#endif
}

#endif /* graphing_h */
