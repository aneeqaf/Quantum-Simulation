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
constexpr double MAX_UCHAR = 255.0;

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

unsigned char inline DiscretizeUDSample(float val,
                                        double range)
{
    if (val < 0 && range == 2 * PI)
        val += (2 * PI);
    
    return (unsigned char)(floor(((val + 0.5) / range) * MAX_UCHAR));
}

float inline ReverseDiscretizationUDSample(unsigned char val,
                                           double range)
{
    return (val * range) / MAX_UCHAR;
}

float inline SampleFromPTGivenUD(float amp, size_t N)
{
    float x = std::log(abs(1.0 - amp)) / N;
    return x == 0 ? 0 : -x;
}

float inline SampleFromUDGivenPT(float amp, size_t N)
{
    return amp >= 0 ? 1.0 - exp(-amp * N) : 0;
}

#endif /* utility_h */
