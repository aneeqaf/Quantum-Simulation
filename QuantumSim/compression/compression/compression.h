//
//  compression.h
//  compression
//
//  Created by Aneeqa Fatima on 12/6/18.
//  Copyright © 2018 Aneeqa Fatima. All rights reserved.
//

#ifndef compression_h
#define compression_h

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
#include <tuple>
#include <utility>

#ifdef GP
#include "gnuplot-iostream/gnuplot-iostream.h"
#endif

using namespace std;

using idx_size = size_t;
using cmplx = complex<double>;
using Packed4ShortArray = unsigned short[4];
using Packed8ShortArray = unsigned short[8];
using amp_idx_t = pair<size_t, cmplx>;
using state_vector_t = vector<amp_idx_t>;

constexpr double PI = M_PI;
constexpr float A = 0.01;
constexpr float PT_B = 0.01;
constexpr unsigned short MAX_14BITS = (1 << 15) - 1;

static inline double RadiusLogSpiral(double angle,
                              double error_bound)
{
    return A * exp(PT_B * angle);
}

static inline double RadiusUniformSpiral(double angle,
                                  double error_bound)
{
    return error_bound * angle;
}

static inline double CalculateCInCThetaGivenMagnitudeLogSpiral(double magnitude)
{
    return (1.0/(PT_B * PI)) * log(magnitude/A);
}

static inline double CalculateCInCThetaGivenMagnitudeUniformSpiral(double magnitude,
                                                                   double error_bound)
{
    return magnitude/(error_bound * PI);
}

static inline double CalculateCodewordInLogSpiral(double magnitude,
                                                  double error_bound,
                                                  int r,
                                                  double cw_dist)
{
    return (CalculateCInCThetaGivenMagnitudeLogSpiral(magnitude) - r)/cw_dist;
}

static inline double CalculateCodewordInUniformSpiral(double magnitude,
                                                      double error_bound,
                                                      int r,
                                                      double cw_dist)
{
    return (CalculateCInCThetaGivenMagnitudeUniformSpiral(magnitude, error_bound) - r)/cw_dist;
}

static inline double CalculateCodewordGivenCInCTheta(double c,
                                                     int r,
                                                     double cw_dist)
{
    return (c - r)/cw_dist;
}


static inline double CalculateCforCodewordInCTheta(unsigned short codeword,
                                                   int r,
                                                   double cw_dist)
{
    return ((double)r + ((double)codeword * cw_dist));
}



static inline double sciToDub(const string& str) {
    
    stringstream ss(str);
    double d = 0;
    ss >> d;
    
    if (ss.fail()) {
        string s = "Unable to format ";
        s += str;
        s += " as a number!";
        throw (s);
    }
    
    return (d);
}

static inline unsigned short ShiftCodeWordToCorrectQuadrant(double phase,
                                                            unsigned short codeword,
                                                            int r,
                                                            double cw_dist)
{
    double c = CalculateCforCodewordInCTheta(codeword, r, cw_dist);
    
    double c_mod2 = fmod(c, 2.0);
    double diff_phase = (phase/PI) - c_mod2;
    c += diff_phase;
    
    return round(CalculateCodewordGivenCInCTheta(c, r, cw_dist));
}
static inline unsigned short CalculateNearestCodewordToAmp(cmplx amp,
                                                           int r,
                                                           double cw_dist,
                                                           double error_bound)
{
    const double magnitude = abs(amp);
    double phase = arg(amp);
    phase = phase < 0 ? phase + (2 * PI) : phase;
    
    const unsigned short floor_cw = floor(CalculateCodewordInUniformSpiral(magnitude, error_bound, r, cw_dist));
    const unsigned short ceil_cw = ceil(CalculateCodewordInUniformSpiral(magnitude, error_bound, r, cw_dist));
    
    const double floor_c = CalculateCforCodewordInCTheta(floor_cw, r, cw_dist);
    const double ceil_c = CalculateCforCodewordInCTheta(ceil_cw, r, cw_dist);
    
    double floor_magnitude_diff = magnitude - RadiusUniformSpiral(floor_c * PI, error_bound);
    double ceil_magnitude_diff =  RadiusUniformSpiral(ceil_c * PI, error_bound) - magnitude;

    unsigned short cw = floor_magnitude_diff < ceil_magnitude_diff ? floor_cw : ceil_cw;
    
    cw = ShiftCodeWordToCorrectQuadrant(phase, cw, r, cw_dist);
//    const double c = CalculateCforCodewordInCTheta(cw, r, cw_dist);
//    double diff = abs(magnitude - RadiusUniformSpiral(c * PI, error_bound));
    
    return cw;
}

cmplx DotProduct(const cmplx* vect_A,
                 const cmplx* vect_B,
                 idx_size state_vector_size);

double CalculateNorm(const cmplx* amp,
                     idx_size state_vector_size);

/* This function is used to calculate the integer range of codewords so, it typically takes
 * the error bound, the largest amp in k smallest amp, or the smallest amp in the k largest amps.
 */
int FindCInCThetaForRInLogSpiral(cmplx amp);

int FindCInCThetaForRInUniformSpiral(cmplx amp);

double CalculateFidelity(const cmplx* original,
                         const cmplx* processed,
                         idx_size state_vector_size);

cmplx FindAmpWithMaxMagnitude(const cmplx* state_vector,
                              idx_size amp_size);

/* Modifies the codewords vector to return points for plotting.
 * Returns the min and max error.
 * // Should have another function which returns the codewords with just the values of c and also encodes the state vector. 
 */
template<typename function>
void FindCodewordsForPlotting(const function& radius_func,
                              vector<tuple<float, float>>& codewords,
                              int r,
                              int R,
                              int num_codewords,
                              double error_bound);

/*Returns:
 *   1) k largest amps from the state vector, where k = 1/16 x amp_size.
 *   2) Smallest amp from the k largest amp (this is to help in the calculation of the R).
 */
pair<amp_idx_t*, cmplx> ExtractFractionsOfAmpsFromState(const cmplx* state_vector,
                                                    idx_size amp_size,
                                                    idx_size k_largest);

void PlotLogSpiralAndAmpDensity(const string& filename,
                                const cmplx* state_vector,
                                idx_size state_vector_size,
                                cmplx smallest_amp_in_k_largest,
                                double error_bound,
                                idx_size k_size,
                                int num_codewords,
                                idx_size multiplicant = 1,
                                double fidelity = 0,
                                idx_size num_zero_amps = 0);

void PlotUniformSpiralAndAmpDensity(const string& filename,
                                    const cmplx* state_vector,
                                    idx_size state_vector_size,
                                    cmplx smallest_amp_in_k_largest,
                                    double error_bound,
                                    idx_size k_size,
                                    int num_codewords,
                                    idx_size multiplicant = 1);

void ApplyUniformTransformToStateVector(cmplx* state_vector,
                                        idx_size state_vector_size);

void ApplyPTTransformToStateVector(cmplx* state_vector,
                                   idx_size state_vector_size);

void CompressDecompressStateVector(const string& filename,
                                   cmplx* state_vector,
                                   idx_size state_vector_size,
                                   int error_exponent,
                                   idx_size k_largest,
                                   int num_codewords);

idx_size CompressStateVector(const amp_idx_t* k_largest_amps,
                         int num_codewords,
                         cmplx*& state_vector,
                         idx_size state_vector_size,
                         idx_size k,
                         double error_bound,
                         double& near_zero_cw,
                         int r,
                         int R);

void DecompressStateVector(const amp_idx_t* k_largest_amps,
                           cmplx*& state_vector,
                           idx_size k_largest_size,
                           idx_size state_vector_size,
                           idx_size num_codewords,
                           int r,
                           int R,
                           double error_bound,
                           double near_zero_c);

unsigned short MapAmpToCodeword(const amp_idx_t* k_largest_amps,
                                cmplx amp,
                                idx_size amp_idx,
                                idx_size k,
                                int r,
                                int R,
                                int num_codewords,
                                double error_bound,
                                double& near_zero_cw,
                                idx_size& num_near_zero_amps);

#endif /* compression_h */

//void FindPitch(const cmplx* amp,
//               size_t state_vector_size,
//               map<double, pair<double, double>>& mag_phases_pitches)
//{
//    for (size_t i = 0; i < state_vector_size; ++i) {
//        double amp_real = amp[i].real(), amp_imag = amp[i].imag();
//        bool neg_real = false, neg_imag = false;
//        if (amp_real < 0) {
//            amp_real *= -1;
//            neg_real = true;
//        }
//        if (amp_imag < 0) {
//            amp_imag *= -1;
//            neg_imag = true;
//        }
//        
//        double phase = atan(amp_imag/amp_real);
//        double magnitude = abs(amp[i]);
//        
//        double b = (1/phase) * log(magnitude);
//        double pitch = (PI/2) - atan(1/b);
//        mag_phases_pitches[magnitude] = pair<double, double>(phase, pitch);
//        //        double new_b = 1/tan((PI/2) - pitch);
//        //        double new_magnitude = exp(new_b * phase);
//        //
//        //        cout << b << " : " << pitch << " | " << new_b << " : " << magnitude << " -> " << new_magnitude << endl;
//        
//    }
//    
//    //    for (auto mpp : mag_phases_pitches) {
//    //        cout << mpp.first << " , " << mpp.second.first << " , " << mpp.second.second << endl;
//    //    }
//}
