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
using cmplx = complex<float>;
using Packed4ShortArray = unsigned short[4];
using amp_idx_t = pair<size_t, cmplx>;
using state_vector_t = vector<amp_idx_t>;

constexpr double PI = M_PI;
constexpr float A = 0.01;
constexpr float PT_B = 0.01;
constexpr unsigned short MAX_14BITS = (1 << 15) - 1;

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

cmplx DotProduct(const cmplx* vect_A,
                 const cmplx* vect_B,
                 idx_size size);

double CalculateNorm(const cmplx* amp,
                     idx_size size);

/* This function is used to calculate the integer range of codewords so, it typically takes
 * the error bound, the largest amp in k smallest amp, or the smallest amp in the k largest amps.
 */
int FindCInCThetaForRInLogSpiral(cmplx amp);

int FindCInCThetaForRInUniformSpiral(cmplx amp);

double CalculateFidelity(const cmplx* original,
                         const cmplx* processed,
                         idx_size size);

cmplx FindAmpWithMaxMagnitude(const cmplx* state_vector,
                              idx_size amp_size);

/* Modifies the codewords vector to return points for plotting.
 * Returns the min and max error.
 * // Should have another function which returns the codewords with just the values of c and also encodes the state vector. 
 */
template<typename function>
pair<double, double> FindCodewordsForPlotting(const function& radius_func,
                                              vector<tuple<float, float, double>>& codewords,
                                              int r,
                                              int R,
                                              double error_bound);

template<typename function>
void FindCodewordsForCompression(const function& radius_func,
                                 vector<float>& codewords,
                                 int r,
                                 int R,
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
                                idx_size size,
                                cmplx smallest_amp_in_k_largest,
                                double error_bound,
                                idx_size k_size);

void PlotUniformSpiralAndAmpDensity(const string& filename,
                                    const cmplx* state_vector,
                                    idx_size size,
                                    cmplx smallest_amp_in_k_largest,
                                    double error_bound,
                                    idx_size k_size);

void ApplyUniformTransformToStateVector(cmplx* state_vector,
                                        idx_size size);

void ApplyPTTransformToStateVector(cmplx* state_vector,
                                   idx_size size);

void CompressDecompressStateVector(const string& filename,
                                   cmplx* state_vector,
                                   idx_size size,
                                   int error_exponent,
                                   idx_size k_largest);

void CompressStateVector(const amp_idx_t* k_largest_amps,
                         vector<float>& codewords,
                         cmplx* state_vector,
                         idx_size size,
                         idx_size k,
                         double error_bound,
                         int r);

void DecompressStateVector(const amp_idx_t* k_largest_amps,
                           vector<float>& codewords,
                           cmplx* state_vector,
                           idx_size k,
                           idx_size size);

unsigned short BinarySearchForClosestRadiusToAmp(const vector<float>& codewords,
                                                 double magnitude,
                                                 double error_bound);

unsigned short FindClosestCodewordUsingAngle(const vector<float>& codewords,
                                             cmplx amp,
                                             double magnitude,
                                             unsigned short closest_outer_radius,
                                             double error_bound);

unsigned short MapAmpToCodeword(const amp_idx_t* k_largest_amps,
                                vector<float>& codewords,
                                cmplx amp,
                                idx_size amp_idx,
                                idx_size k,
                                int r,
                                double error_bound,
                                idx_size& num_near_zero_amps);

#endif /* compression_h */

//void FindPitch(const cmplx* amp,
//               size_t size,
//               map<double, pair<double, double>>& mag_phases_pitches)
//{
//    for (size_t i = 0; i < size; ++i) {
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
