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
#include <vector>
#include <utility>

using namespace std;

using Packed16ShortArray = unsigned short[16];
using Packed4ShortArray = unsigned short[4];

constexpr double PI = M_PI;
constexpr double CDF_MAX_P = 1.02;
constexpr size_t INNER_R_SHIFT = 0;
constexpr double B = 0.00298;
constexpr size_t NUM_UL_IN_REG = 4;
constexpr size_t NUM_SHORT_IN_REG = 16;
constexpr size_t NUM_SHORT_IN_UL = 4;
constexpr size_t REG_SIZE = 256;
constexpr size_t BITS_BYTE = 8;
constexpr size_t BITS_SHORT = 16;
constexpr size_t BITS_UL = 64;
constexpr size_t SAMPLING_SIZE = 1 << 10;

static inline __m256i _mm256_shift_right(__m256i A, unsigned long count) {
    
    unsigned int m = (1u << count) - 1;
    __m256i mask = {0, m, m, m};
    
    __m256i last_bits = _mm256_and_si256(A, mask);
    last_bits = _mm256_slli_epi64(last_bits, (int)(64 - count));
    last_bits = _mm256_permute4x64_epi64 (last_bits,  0b00111001);
    last_bits[3] = 0;
    
    __m256i shift_bits = _mm256_srli_epi64(A, (int)count);
    
    return _mm256_or_si256(last_bits, shift_bits);
}

template  <unsigned int N> __m256i _mm256_shift_right(__m256i A)
{
    return _mm256_alignr_epi8(_mm256_permute2x128_si256(A, A, _MM_SHUFFLE(2, 0, 0, 1)), A, N);
}

template <unsigned int N> __m256i _mm256_shift_left0To16(__m256i A)
{
    return _mm256_alignr_epi8(A, _mm256_permute2x128_si256(A, A, _MM_SHUFFLE(0, 0, 2, 0)), 16 - N);
}

template <unsigned int N> __m256i _mm256_shift_left16(__m256i A)
{
    return _mm256_permute2x128_si256(A, A, _MM_SHUFFLE(0, 0, 2, 0));
}

template <unsigned int N> __m256i _mm256_shift_left16To32(__m256i A)
{
    return _mm256_slli_si256(_mm256_permute2x128_si256(A, A, _MM_SHUFFLE(0, 0, 2, 0)), N - 16);
}


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
    void UnpackCWFrom256Bits(bitset<REG_SIZE> packed_codewords,
                             unsigned short* unpacked_codewords) const;
    __m256i ExtractCodewordFromAVX256Reg(__m256i& packed_codewords,
                                         const __m256i& mask_cw_256) const;
    void UnpackCWFrom256BitsAVX(__m256i packed_codewords,
                                unsigned short* unpacked_codewords) const;
    
public:
    
    Cramer(size_t vector_size,
           size_t num_codewords,
           double probabilty_rejection,
           bool projection_v = true);
    Cramer(const Cramer& rhs);
    ~Cramer();
    
    complex<float>* CramerCompress(const complex<float>* state_vector);
    complex<float>* CramerDecompress(const complex<float>* state_vector);
    
    size_t GetCompressedVectorSize() const;
    double GetMinInnerRadius() const;
    double GetMaxOuterRadius() const;
    size_t GetNumOfCW() const;
    size_t GetNumValsMappedToZero() const;
    double GetFactorOfDistBetweenTurns() const;
    double GetDistBetweenCW() const;
};

#endif /* Cramer_h */
