//
//  utility.h
//  QuantumSimProj
//
//  Created by Aneeqa Fatima on 6/28/18.
//  Copyright © 2018 Aneeqa Fatima. All rights reserved.
//

#ifndef utility_h
#define utility_h

#include <bitset>
#include <stdio.h>
#include <string>
#include <inttypes.h>

using namespace std;

using cmplx = complex<float>;
using Packed8CharArray = unsigned char[8];

constexpr double PI = M_PI;
constexpr double RANGE_PHASE = 63;
constexpr double RANGE_MAG = 255;
constexpr int N = 128;
constexpr cmplx NEG_1 = cmplx(-1, 0);

static inline void print_u128_u(__int128 u128,
                                string& bit128_str)
{
    if (u128 == 0)
        return;
    
    __int128 leading  = u128/10;
    print_u128_u(leading, bit128_str);
    uint64_t trailing = u128 % 10;
    bit128_str += to_string(trailing);
}

cmplx inline ReconstructComplexFromPolar(float magnitude,
                                         float phase,
                                         unsigned char signs)
{
    float amp_real = 0, amp_imag = 0;
    
    //    double step_size = 1/(RANGE_PHASE + 1);
    
    //    if (phase + 0.0005 < step_size)
    //        phase += step_size/5;
    //    else if (((PI/2) - phase) + 0.0005 < step_size)
    //        phase -= step_size/5;
    
    
    amp_real = signs & (1 << 7) ? -magnitude * cos(phase) : magnitude * cos(phase);
    amp_imag = signs & (1 << 6) ? -magnitude * sin(phase) : magnitude * sin(phase);
    
    return cmplx(amp_real, amp_imag);
}

unsigned char inline DiscretizeMagnitudeFromUD(float val)
{
    return (unsigned char)floor(val * RANGE_MAG) ;
}

unsigned char inline DiscretizePhaseFromUD(cmplx amp)
{
    double amp_real = amp.real(), amp_imag = amp.imag();
    bool neg_real = false, neg_imag = false;
    if (amp_real < 0) {
        amp_real *= -1;
        neg_real = true;
    }
    if (amp_imag < 0) {
        amp_imag *= -1;
        neg_imag = true;
    }
    
    double phase = atan(amp_imag/amp_real);
    double phase_to_discretize = sin(phase);
    
    //    double step_size = (PI/2)/(RANGE_PHASE + 1);
    //
    //    double debug2 = (PI / 2.0);
    //    bool debug1 = (PI / 2.0) == phase;
    //    unsigned int debug = (unsigned char)round((phase / (PI / 2.0)) * RANGE_PHASE);
    //    unsigned char discretized_phase = (unsigned char)floor((phase / (PI / 2.0)) * RANGE_PHASE);
    
    unsigned char discretized_phase = (unsigned char)round(phase_to_discretize * RANGE_PHASE);
    
    if (neg_real)
        discretized_phase |= 1 << 7;
    if (neg_imag)
        discretized_phase |= 1 << 6;
    
    return discretized_phase;
}

float inline ReverseDiscretizePhase(unsigned char val)
{
    val &= ((unsigned char)RANGE_PHASE);
    
    return asin(val / RANGE_PHASE);
}

float inline ReverseDiscretizeMagnitude(unsigned char val)
{
    return val / RANGE_MAG;
}

float inline SampleFromPTGivenUD(float amp)
{
    float x = std::log(abs(1.0 - amp)) / N;
    return x == 0 ? 0 : -x;
}

float inline SampleFromUDGivenPT(float amp)
{
    return amp >= 0 ? 1.0 - exp(-amp * N) : 0;
}

#endif /* utility_h */
