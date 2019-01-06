//
//  Cramer.h
//  compression
//
//  Created by Aneeqa Fatima on 1/2/19.
//  Copyright © 2019 Aneeqa Fatima. All rights reserved.
//

#ifndef Cramer_h
#define Cramer_h

#include <algorithm>
#include <atomic>
#include <bitset>
#include <complex>
#include <cstring>
#include <cmath>
#include <complex>
#include <cassert>
#include <fstream>
#include <map>
#include <iostream>
#include <immintrin.h>
#include <queue>
#include <stdio.h>
#include <vector>
#include <utility>

using namespace std;

using idx_size = size_t;
using cmplx = complex<float>;
using cmplxd = complex<double>;
using Packed4ShortArray = unsigned short[4];
using Packed8ShortArray = unsigned short[8];

constexpr double PI = M_PI;
constexpr double CDF_MAX_P = 1.02;
constexpr idx_size INNER_R_SHIFT = 0;
constexpr double B = 0.00298;
constexpr idx_size NUM_UL_IN_REG = 4;
constexpr idx_size REG_SIZE = 256;
constexpr idx_size BITS_SHORT = 16;
constexpr idx_size BITS_UL = 64;

class Cramer {
        
    idx_size orig_vector_size;
    idx_size compressed_vector_UL_size;
    idx_size r;
    idx_size R;
    idx_size num_bits_codewords;
    idx_size num_codewords;
    idx_size num_codewords_reg;
    atomic<idx_size> num_zero_amps;
    double codewords_spacing;
    double spiral_length_r;
    
    cmplxd UniformTransformMagnitudeAndAmp(cmplxd amp) const;
    cmplxd PTTransformMagnitudeAndAmp(cmplxd amp) const;
    
    //Polar equation: r = BTheta = BcPi
    double CalcCInMagnitudeUniformSpiral(double magnitude) const;
    double CalcThetaForMagnitude(double magnitude) const;
    double CalcMagnitudeForC(double c) const;
    double CalcCWForMagnitude(double magnitude) const;
    double CalcCWForTheta(double theta) const;
    double CalcMagnitudeForCW(unsigned short codeword) const;
    double CalcApproxSpiralLen(double theta) const;
    double CalcExactSpiralLen(double theta) const;
    double CalcApproxThetaForSpiralLen(double spiral_lenth) const;
    double CalcThetaForCW(unsigned short codeword) const;
    idx_size CalcCWThatFitIn256BitsReg() const;
    idx_size CalcNumULInCompressedVector(idx_size num_256_reg) const;
    idx_size CalcNum256RegForSizeOfVector() const;
    
    unsigned short ShiftCWToNearestPhase(double phase,
                                         unsigned short codeword) const;
    unsigned short CalcNearestCWToVal(cmplxd val) const;
    unsigned short MapValToCW(cmplxd val);
    __m256 PackCWIn256BitsAVXReg(const unsigned short* codewords) const;
    void UnpackCWFrom256Bits(const bitset<REG_SIZE>& packed_codewords,
                             unsigned short* unpacked_codewords) const;
    
public:
    
    Cramer(idx_size vector_size,
           idx_size num_codewords,
           double probabilty_rejection);
    
    cmplx* CramerCompress(const cmplx* state_vector);
    cmplx* CramerDecompress(const cmplx* state_vector);
    cmplx* CramerCompressAVX(const cmplx* state_vector);
    cmplx* CramerDecompressAVX(const cmplx* state_vector);
    
    void GetCWForPlotting(vector<pair<float, float>>& codewords) const;
    idx_size GetCompressedVectorSize() const;
    double GetMinInnerRadius() const;
    double GetMaxOuterRadius() const;
    idx_size GetNumOfCW() const;
    idx_size GetNumValsMappedToZero() const;
    double GetFactorOfDistBetweenTurns() const;
    double GetDistBetweenCW() const;
};

#endif /* Cramer_h */
