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
#include <array>
#include <atomic>
#include <bitset>
#include <cassert>
#include <complex>
#include <cstring>
#include <cmath>
#include <complex>
#include <iostream>
#include <immintrin.h>
#include <memory>
#include <mutex>
#include <unordered_map>
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
constexpr size_t NUM_UI_IN_REG = 8;
constexpr size_t NUM_SHORT_IN_REG = 16;
constexpr size_t NUM_SHORT_IN_UL = 4;
constexpr size_t REG_SIZE = 256;
constexpr size_t BITS_BYTE = 8;
constexpr size_t BITS_SHORT = 16;
constexpr size_t BITS_UL = 64;
constexpr size_t BITS_UI = 32;
constexpr size_t SAMPLING_SIZE = 1 << 10;

constexpr __m256i ZERO_REG = {0, 0, 0, 0};
constexpr __m256i INCREMENT_1_UI = {1 | 1ull << 32, 1 | 1ull << 32, 1 | 1ull << 32, 1 | 1ull << 32};
constexpr __m256i MASK_1ST_SET_UI = {(1ul << 32) - 1, (1ul << 32) - 1,(1ul << 32) - 1, (1ul << 32) - 1};
constexpr __m256i MASK_2ND_SET_UI = {~(1ll << 32), ~(1ll << 32), ~(1ll << 32), ~(1ll << 32)};

constexpr __m256i MASK_8_4xCW_UL = {(1ull << 8) - 1, (1ull << 8) - 1, (1ull << 8) - 1, (1ull << 8) - 1};
constexpr __m256i MASK_9_4xCW_UL = {(1ull << 9) - 1, ((1ull << 9) - 1) << 8, ((1ull << 9) - 1) << 7, ((1ull << 9) - 1) << 6};
constexpr __m256i MASK_10_4xCW_UL = {(1ull << 10) - 1, ((1ull << 10) - 1) << 6, ((1ull << 10) - 1) << 2, ((1ull << 10) - 1) << 8};
constexpr __m256i MASK_11_4xCW_UL = {(1ull << 11) - 1, ((1ull << 11) - 1) << 2, ((1ull << 11) - 1) << 4, ((1ull << 11) - 1) << 6};
constexpr __m256i MASK_12_4xCW_UL = {(1ull << 12) - 1, ((1ull << 12) - 1) << 8, ((1ull << 12) - 1) << 4, (1ull << 12) - 1};
constexpr __m256i MASK_13_4xCW_UL = {(1ull << 13) - 1, ((1ull << 13) - 1) << 1, ((1ull << 12) - 1) << 2, ((1ull << 12) - 1) << 3};

constexpr __m256i MASK_8_4xCW_UI = {((1ull << 8) - 1) | (((1ull << 8) - 1) << 32), ((1ull << 8) - 1) | (((1ull << 8) - 1) << 32),
    ((1ull << 8) - 1) | (((1ull << 8) - 1) << 32), ((1ull << 8) - 1) | (((1ull << 8) - 1) << 32)};
constexpr __m256i MASK_9_4xCW_UI = {((1ull << 9) - 1) | (((1ull << 9) - 1) << 36), (((1ull << 9) - 1) << 8) | (((1ull << 9) - 1) << 35),
    (((1ull << 9) - 1) << 7) | (((1ull << 9) - 1) << 34), (((1ull << 9) - 1) << 6) | (((1ull << 9) - 1) << 33)};
constexpr __m256i MASK_10_4xCW_UI = {((1ull << 10) - 1) | (((1ull << 10) - 1) << 40) , (((1ull << 10) - 1) << 6) | (((1ull << 10) - 1) << 36),
    (((1ull << 10) - 1) << 2) | (((1ull << 10) - 1) << 32), (((1ull << 10) - 1) << 8) | (((1ull << 10) - 1) << 38)};
constexpr __m256i MASK_11_4xCW_UI = {((1ull << 11) - 1) | (((1ull << 11) - 1) << 33), (((1ull << 11) - 1) << 2) | (((1ull << 11) - 1) << 35),
    (((1ull << 11) - 1) << 4) | (((1ull << 11) - 1) << 37), (((1ull << 11) - 1) << 6) | (((1ull << 11) - 1) << 39)};
constexpr __m256i MASK_12_4xCW_UI = {((1ull << 12) - 1) | (((1ull << 12) - 1) << 36), (((1ull << 12) - 1) << 8) | (((1ull << 12) - 1) << 32),
    (((1ull << 12) - 1) << 4) | (((1ull << 12) - 1) << 40), ((1ull << 12) - 1) | (((1ull << 12) - 1) << 36)};
constexpr __m256i MASK_13_4xCW_UI = {((1ull << 13) - 1) | (((1ull << 13) - 1) << 39), (((1ull << 13) - 1) << 1) | (((1ull << 13) - 1) << 40),
    (((1ull << 13) - 1) << 2) | (((1ull << 13) - 1) << 41), (((1ull << 13) - 1) << 3) | (((1ull << 13) - 1) << 42)};

constexpr __m256i MASKS_CW_PER_UL_REG[17] = {ZERO_REG, ZERO_REG, ZERO_REG, ZERO_REG, ZERO_REG, ZERO_REG, ZERO_REG, ZERO_REG,
    MASK_8_4xCW_UL, MASK_9_4xCW_UL, MASK_10_4xCW_UL, MASK_11_4xCW_UL, MASK_12_4xCW_UL, MASK_13_4xCW_UL, ZERO_REG, ZERO_REG, ZERO_REG};

constexpr __m256i MASKS_CW_PER_UI_REG[17] = {ZERO_REG, ZERO_REG, ZERO_REG, ZERO_REG, ZERO_REG, ZERO_REG, ZERO_REG, ZERO_REG,
    MASK_8_4xCW_UI, MASK_9_4xCW_UI, MASK_10_4xCW_UI, MASK_11_4xCW_UI, MASK_12_4xCW_UI, MASK_13_4xCW_UI, ZERO_REG, ZERO_REG, ZERO_REG};

constexpr __m256i IDXS_CW_FOR_MASKS_UL[17] = {{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0, 8, 16, 24},
    {0, 8, 15, 22}, {0, 7, 13, 20}, {0, 6, 12, 18}, {0, 6, 11, 16}, {0, 5, 10, 15}, {0}, {0}, {0}};

constexpr __m256i IDXS_CW_FOR_MASKS_UI[17] = {{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
    {0 | (4ull << 32), 8 | (12ull << 32), 16 | (20ull << 32), 24 | (28ull << 32)},
    {0 | (4ull << 32), 8 | (11ull << 32), 15 | (18ull << 32), 22 | (25ull << 32)},
    {0 | (4ull << 32), 7 | (10ull << 32), 13 | (16ull << 32), 20 | (23ull << 32)},
    {0 | (3ull << 32), 6 | (9ull << 32), 12 | (15ull << 32), 18 | (21ull << 32)},
    {0 | (3ull << 32), 6 | (8ull << 32), 11 | (14ull << 32), 16 | (19ull << 32)},
    {0 | (3ull << 32), 5 | (8ull << 32), 10 | (13ull << 32), 15 | (18ull << 32)},
    {0}, {0}, {0}};

constexpr __m256i  BITS_TO_STARTING_OF_UL[17] = {{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
    {0, 8, 7, 6}, {0, 6, 2, 8}, {0, 2, 4, 6}, {0, 8, 4, 0}, {0, 1, 2, 3}, {0}, {0}, {0}};

constexpr __m256i  BITS_TO_STARTING_OF_UI[17] = {{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
    {0 | (4ull << 32), 8 | (3ull << 32), 7 | (2ull << 32), 6 | (1ull << 32)},
    {0 | (8ull << 32), 6 | (4ull << 32), 2, 8 | (6ull << 32)},
    {0 | (1ull << 32), 2 | (3ull << 32), 4 | (5ull << 32), 6 | (7ull << 32)},
    {0 | (4ull << 32), 8, 4 | (8ull << 32), 0 | (4ull << 32)},
    {0 | (7ull << 32), 1 | (8ull << 32), 2 | (9ull << 32), 3 | (10ull << 32)}, {0}, {0}, {0}};


static inline __m256i _mm256_shift_right(__m256i A, unsigned int count) {
    
    unsigned int m = (1u << count) - 1;
    __m256i mask = {0, m, m, m};
    
    __m256i last_bits = _mm256_and_si256(A, mask);
    last_bits = _mm256_slli_epi64(last_bits, 64 - count);
    last_bits = _mm256_permute4x64_epi64 (last_bits,  0b00111001);
    last_bits[3] = 0;
    
    __m256i shift_bits = _mm256_srli_epi64(A, count);
    
    return _mm256_or_si256(last_bits, shift_bits);
}

static inline __m256i _mm256_shift_left(__m256i A, unsigned int count)
{
    unsigned long long m = ((1ul << count) - 1) << (64 - count);
    __m256i mask = {(long long)m, (long long)m, (long long)m, 0};
    
    __m256i last_bits = _mm256_and_si256(A, mask);
    last_bits = _mm256_srli_epi64(last_bits, 64 - count);
    last_bits = _mm256_permute4x64_epi64 (last_bits,  0b10010000);
    last_bits[0] = 0;
    
    __m256i shift_bits = _mm256_slli_epi64(A, count);
    
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
    size_t num_threads;
    size_t num_zero_amps;
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
    __m256i ExtractCodewordFromAVX256Reg(__m256i& packed_codewords) const;
    void UnpackCWFrom256BitsAVX(__m256i packed_codewords,
                                unsigned short* unpacked_codewords) const;
    
public:
    
    Cramer(size_t vector_size,
           size_t num_codewords,
           size_t num_threads,
           double probabilty_rejection,
           bool projection_v = true);
    Cramer(const Cramer& rhs);
    ~Cramer();
    
    complex<float>* CramerCompress(complex<float>* compressed_v,
                                   const complex<float>* state_vector);
    complex<float>* CramerDecompress(complex<float>* decompressed_v,
                                     const complex<float>* state_vector);
    
    size_t GetCompressedVectorSize() const;
    double GetMinInnerRadius() const;
    double GetMaxOuterRadius() const;
    size_t GetNumOfCW() const;
    size_t GetNumValsMappedToZero() const;
    double GetFactorOfDistBetweenTurns() const;
    double GetDistBetweenCW() const;
    double GetLog2Lambda() const;
};

#endif /* Cramer_h */
