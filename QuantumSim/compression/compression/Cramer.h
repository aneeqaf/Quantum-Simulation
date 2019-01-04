//
//  Cramer.h
//  compression
//
//  Created by Aneeqa Fatima on 1/2/19.
//  Copyright © 2019 Aneeqa Fatima. All rights reserved.
//

#ifndef Cramer_h
#define Cramer_h

#include <fstream>
#include <map>
#include <cmath>
#include <complex>
#include <cassert>
#include <algorithm>
#include <complex>
#include <cstring>
#include <iostream>
#include <queue>
#include <stdio.h>
#include <vector>
#include <utility>

#include "zstd.h"

#ifdef GP
#include "gnuplot-iostream/gnuplot-iostream.h"
#endif

using namespace std;

using idx_size = size_t;
using cmplx = complex<float>;
using cmplxd = complex<double>;
using Packed4ShortArray = unsigned short[4];
using Packed8ShortArray = unsigned short[8];

constexpr double PI = M_PI;
constexpr double CDF_MAX_P = 1.009;
constexpr idx_size INNER_R_SHIFT = 20;

static void PlotCWFrequency(const vector<idx_size>& codewords_freq)
{
#ifdef GP
    Gnuplot gp;
    
    vector<pair<unsigned short,idx_size>> codewords_freq_idx;
    for (idx_size i = 1; i < codewords_freq.size(); ++i)
        codewords_freq_idx.push_back(make_pair(i, codewords_freq[i]));
    
    idx_size freq_max = 0;
    for (idx_size i = 1; i < codewords_freq.size(); ++i) {
        if(codewords_freq[i] > freq_max)
            freq_max = codewords_freq[i];
    }
    
    string filename = to_string(codewords_freq.size()) + "_codewords_freq";
    
    gp << "reset\nset nokey\n";
    gp << "set title \"" << codewords_freq.size() << " Codewords Frequency\" font \",14\"\n";
    //    gp << "set ylabel 'Amplitude frequency'\n";
    //    gp << "set xlabel 'Codewords'\n";
    gp << "set output '" << filename << ".png'\n";
    gp << "set tics font 'Times New Roman,12'\n";
    gp << "cw=" << codewords_freq.size() << "\nmax_f=" << freq_max << "\n";
    gp << "title(c) = sprintf(\"\\nAmplitudes to %i Codewords Frequency\"" << ", cw) \n";
    
    gp << "set xr[0:cw]\nset yr[0:max_f]\n";
    gp << "plot " << gp.file1d(codewords_freq_idx, "file.txt") << " using 1:2 with lines lw 2 \n";
    
#endif
}

class Cramer {
    
    vector<cmplx> k_largest_vals;
    
    cmplx avg_near_zero_val;
    idx_size orig_vector_size;
    idx_size compressed_vector_size;
    idx_size r;
    idx_size R;
    idx_size num_codewords;
    idx_size num_zero_amps;
    double factor_dist_bw_turns;
    double codewords_spacing;
    double spiral_length_r;
    
    cmplxd UniformTransformMagnitudeAndAmp(cmplxd amp) const;
    cmplxd PTTransformMagnitudeAndAmp(cmplxd amp) const;
    
    //Polar equation: r = DTheta = DcPi
    double CalculateCInMagnitudeUniformSpiral(double magnitude) const;
    double CalculateThetaGivenMagnitude(double magnitude) const;
    double CalculateMagnitudeGivenC(double c) const;
    double CalculatCWGivenMagnitude(double magnitude) const;
    double CalculatCWGivenTheta(double theta) const;
    double CalculateMagnitudeGivenCW(unsigned short codeword) const;
    double CalculateApproxSpiralLength(double theta) const;
    double CalculateExactSpiralLength(double theta) const;
    double CalculateApproxThetaGivenSpiralLength(double spiral_lenth) const;
    double CalculateThetaGivenCW(unsigned short codeword) const;
    
    unsigned short ShiftCodeWordToCorrectQuadrant(double phase,
                                                  unsigned short codeword) const;
    unsigned short CalculateNearestCodewordToVal(cmplxd val) const;
    unsigned short MapValToCodeword(cmplxd val);
    
public:
    
    Cramer(idx_size vector_size,
           idx_size num_codewords,
           double probabilty_rejection,
           double dist_bw_turns);
    Cramer(const Cramer& rhs);
    
    cmplx* CramerCompress(const cmplx* state_vector);
    cmplx* CramerDecompress(const cmplx* state_vector);
    
    void GetCodewordsForPlotting(vector<pair<float, float>>& codewords) const;
    idx_size GetNumOfLargestVals() const;
    idx_size GetCompressedVectorSize() const;
    double GetMinInnerRadius() const;
    double GetMaxOuterRadius() const;
    idx_size GetNumOfCodewords() const;
    idx_size GetNumValsMappedToZero() const;
    double GetFactorOfDistBetweenTurns() const;
    double GetDistBetweenCodewords() const;
};

#endif /* Cramer_h */
