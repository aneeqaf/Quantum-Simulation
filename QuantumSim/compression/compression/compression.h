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

static inline double CalculateCWDistanceAsALinearlyIncreasingFunction(double cw_dist,
                                                                      double magnitude)
{
    return cw_dist * (1.0 - magnitude);
}

//static inline double CalculateCodewordInLogSpiral(double magnitude,
//                                                  double error_bound,
//                                                  int r,
//                                                  double cw_dist)
//{
//    return (CalculateCInCThetaGivenMagnitudeLogSpiral(magnitude) - r) / cw_dist;
//}

static inline double CalculateCodewordInUniformSpiral(double magnitude,
                                                      double error_bound,
                                                      int r,
                                                      double cw_dist)
{
    return (CalculateCInCThetaGivenMagnitudeUniformSpiral(magnitude, error_bound) - r) / cw_dist;
}

static inline double CalculateCodewordGivenCInCTheta(double c,
                                                     double error_bound,
                                                     int r,
                                                     double cw_dist)
{
    return (c - r) / cw_dist;
}


static inline double CalculateCforCodewordInCTheta(unsigned short codeword,
                                                   double error_bound,
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
                                                            double cw_dist,
                                                            double error_bound)
{
    double c = CalculateCforCodewordInCTheta(codeword, error_bound, r, cw_dist);
    
    double c_mod2 = fmod(c, 2.0);
    double diff_phase = (phase/PI) - c_mod2;
    c += diff_phase;
    
    return round(CalculateCodewordGivenCInCTheta(c, error_bound, r, cw_dist));
}
static inline unsigned short CalculateNearestCodewordToAmp(cmplxd amp,
                                                           int r,
                                                           idx_size num_codewords,
                                                           double cw_dist,
                                                           double error_bound)
{
    const double magnitude = abs(amp);
    double phase = arg(amp);
    phase = phase < 0 ? phase + (2 * PI) : phase;
    
    const unsigned short floor_cw = floor(CalculateCodewordInUniformSpiral(magnitude, error_bound, r, cw_dist));
    const unsigned short ceil_cw = ceil(CalculateCodewordInUniformSpiral(magnitude, error_bound, r, cw_dist));
    
    const double floor_c = CalculateCforCodewordInCTheta(floor_cw, error_bound, r, cw_dist);
    const double ceil_c = CalculateCforCodewordInCTheta(ceil_cw, error_bound, r, cw_dist);
    
    double floor_magnitude_diff = magnitude - RadiusUniformSpiral(floor_c * PI, error_bound);
    double ceil_magnitude_diff =  RadiusUniformSpiral(ceil_c * PI, error_bound) - magnitude;

    unsigned short cw = floor_magnitude_diff < ceil_magnitude_diff ? floor_cw : ceil_cw;
    
    cw = ShiftCodeWordToCorrectQuadrant(phase, cw, r, cw_dist, error_bound);
//    const double c = CalculateCforCodewordInCTheta(cw, r, cw_dist);
//    double diff = abs(magnitude - RadiusUniformSpiral(c * PI, error_bound));
    
    return cw;
}

cmplxd DotProduct(const cmplxd* vect_A,
                 const cmplxd* vect_B,
                 idx_size state_vector_size);

double CalculateNorm(const cmplxd* amp,
                     idx_size state_vector_size);

/* This function is used to calculate the integer range of codewords so, it typically takes
 * the error bound, the largest amp in k smallest amp, or the smallest amp in the k largest amps.
 */
int FindCInCThetaForRInLogSpiral(double magnitude);

int FindCInCThetaForRInUniformSpiral(double magnitude,
                                     double error_bound);

double CalculateFidelity(const cmplxd* original,
                         const cmplxd* processed,
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
                                int R,
                                int r,
                                double error_bound,
                                idx_size k_size,
                                int num_codewords,
                                idx_size multiplicant = 1,
                                double fidelity = 0,
                                idx_size num_zero_amps = 0);

void PlotUniformSpiralAndAmpDensity(const string& filename,
                                    const cmplx* state_vector,
                                    idx_size state_vector_size,
                                    int R,
                                    int r,
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
                                   int num_codewords,
                                   double probability_acceptance);

idx_size CompressStateVector(vector<cmplx>& k_largest_amps,
                             idx_size& compression_size,
                             int num_codewords,
                             cmplx*& state_vector,
                             idx_size state_vector_size,
                             double min_radius_uniform,
                             double error_bound,
                             double probability_acceptance,
                             cmplx& near_zero_amp,
                             int r,
                             int R);

void DecompressStateVector(const cmplx* k_largest_amps,
                           cmplx*& state_vector,
                            idx_size compression_size,
                           idx_size state_vector_size,
                           idx_size num_codewords,
                           int r,
                           int R,
                           double error_bound,
                           cmplx near_zero_amp);

unsigned short MapAmpToCodeword(vector<cmplx>& k_largest_amps,
                                cmplxd amp,
                                idx_size state_vector_size,
                                int r_uniform,
                                int num_codewords,
                                double error_bound,
                                cmplx& near_zero_amp,
                                idx_size& num_near_zero_amps,
                                double probability_acceptance);

#endif /* compression_h */
