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
#include <iostream>
#include <immintrin.h>
#include <unordered_map>
#include <vector>
#include <utility>

using namespace std;

using Packed4ShortArray = unsigned short[4];
using Packed8ShortArray = unsigned short[8];

constexpr double PI = M_PI;
constexpr double CDF_MAX_P = 1.02;
constexpr size_t INNER_R_SHIFT = 0;
constexpr double B = 0.00298;
constexpr size_t NUM_UL_IN_REG = 4;
constexpr size_t REG_SIZE = 256;
constexpr size_t BITS_SHORT = 16;
constexpr size_t BITS_UL = 64;
constexpr size_t SAMPLING_SIZE = 1 << 10;

class Cramer {
    
    complex<float>* codewords_mappings;
    
    size_t orig_vector_size;
    size_t compressed_vector_UL_size;
    size_t r;
    size_t R;
    size_t num_bits_codewords;
    size_t num_codewords;
    size_t num_codewords_reg;
    atomic<size_t> num_zero_amps;
    double codewords_spacing;
    double spiral_length_r;
    double lambda;
    bool projection_vector;
    
    complex<double> UniformTransformMagnitudeAndAmp(complex<double> amp) const;
    complex<double> PTTransformMagnitudeAndAmp(complex<double> amp) const;
    
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
    size_t CalcCWThatFitIn256BitsReg() const;
    size_t CalcNumULInCompressedVector(size_t num_256_reg) const;
    size_t CalcNum256RegForSizeOfVector() const;
    double CalculateLambdaFromEmpiricalCDF(const complex<float>* state_vector) const;
    
    unsigned short ShiftCWToNearestPhase(double phase,
                                         unsigned short codeword) const;
    unsigned short CalcNearestCWToVal(complex<double> val) const;
    unsigned short MapValToCW(complex<double> val);
    __m256 PackCWIn256BitsAVXReg(const unsigned short* codewords) const;
    void UnpackCWFrom256Bits(const bitset<REG_SIZE>& packed_codewords,
                             unsigned short* unpacked_codewords) const;
    
public:
    
    Cramer(size_t vector_size,
           size_t num_codewords,
           double probabilty_rejection,
           bool projection_v = false);
    Cramer(const Cramer& rhs);
    ~Cramer();
    
    complex<float>* CramerCompress(const complex<float>* state_vector);
    complex<float>* CramerDecompress(const complex<float>* state_vector);
    complex<float>* CramerCompressAVX(const complex<float>* state_vector);
    complex<float>* CramerDecompressAVX(const complex<float>* state_vector);
    
    void GetCWForPlotting(vector<pair<float, float>>& codewords) const;
    size_t GetCompressedVectorSize() const;
    double GetMinInnerRadius() const;
    double GetMaxOuterRadius() const;
    size_t GetNumOfCW() const;
    size_t GetNumValsMappedToZero() const;
    double GetFactorOfDistBetweenTurns() const;
    double GetDistBetweenCW() const;
};

#endif /* Cramer_h */
