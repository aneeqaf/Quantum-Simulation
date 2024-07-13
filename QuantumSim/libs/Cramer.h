
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
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <utility>
// #include <sleef.h>
#include <immintrin.h>
#include <sleefinline_avx2.h>

#include "math_helper.h"

using namespace std;

using Packed16ShortArray = unsigned short[16];
using Packed4ShortArray = unsigned short[4];

constexpr double PI = M_PI;
constexpr double CDF_MAX_P = 1.02;
constexpr size_t INNER_R_SHIFT = 0;
constexpr size_t NUM_UL_IN_REG = 4;
constexpr size_t NUM_UI_IN_REG = 8;
constexpr size_t NUM_FLOAT_IN_REG = 8;
constexpr size_t NUM_SHORT_IN_REG = 16;
constexpr size_t NUM_SHORT_IN_UL = 4;
constexpr size_t REG_SIZE = 256;
constexpr size_t BITS_BYTE = 8;
constexpr size_t BITS_SHORT = 16;
constexpr size_t BITS_UL = 64;
constexpr size_t BITS_UI = 32;
constexpr size_t SAMPLING_SIZE = 1 << 10;

// Hardcoding for now from experiments. Don't have a good way of calculating for now
constexpr size_t NUM_TURNINGS_CW[16] = {0, 1, 5, 5, 10, 25, 20, 20, 20, 25, 50, 100, 150, 200, 200};

constexpr __m256i ZERO_REG = {0, 0, 0, 0};
constexpr __m256i INCREMENT_1_UI = {1 | 1ull << 32, 1 | 1ull << 32, 1 | 1ull << 32, 1 | 1ull << 32};
constexpr __m256i MASK_1ST_SET_UI = {(1ul << 32) - 1, (1ul << 32) - 1, (1ul << 32) - 1, (1ul << 32) - 1};
constexpr __m256i MASK_2ND_SET_UI = {~(1ll << 32), ~(1ll << 32), ~(1ll << 32), ~(1ll << 32)};
const __m256i ALL_ONES_REG = _mm256_set1_epi64x(-1);

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
constexpr __m256i MASK_10_4xCW_UI = {((1ull << 10) - 1) | (((1ull << 10) - 1) << 40), (((1ull << 10) - 1) << 6) | (((1ull << 10) - 1) << 36),
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

constexpr __m256i IDXS_CW_FOR_MASKS_UL[17] = {{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0, 8, 16, 24}, {0, 8, 15, 22}, {0, 7, 13, 20}, {0, 6, 12, 18}, {0, 6, 11, 16}, {0, 5, 10, 15}, {0}, {0}, {0}};

constexpr __m256i IDXS_CW_FOR_MASKS_UI[17] = {{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0 | (4ull << 32), 8 | (12ull << 32), 16 | (20ull << 32), 24 | (28ull << 32)}, {0 | (4ull << 32), 8 | (11ull << 32), 15 | (18ull << 32), 22 | (25ull << 32)}, {0 | (4ull << 32), 7 | (10ull << 32), 13 | (16ull << 32), 20 | (23ull << 32)}, {0 | (3ull << 32), 6 | (9ull << 32), 12 | (15ull << 32), 18 | (21ull << 32)}, {0 | (3ull << 32), 6 | (8ull << 32), 11 | (14ull << 32), 16 | (19ull << 32)}, {0 | (3ull << 32), 5 | (8ull << 32), 10 | (13ull << 32), 15 | (18ull << 32)}, {0}, {0}, {0}};

constexpr __m256i BITS_TO_STARTING_OF_UL[17] = {{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0, 8, 7, 6}, {0, 6, 2, 8}, {0, 2, 4, 6}, {0, 8, 4, 0}, {0, 1, 2, 3}, {0}, {0}, {0}};

constexpr __m256i BITS_TO_STARTING_OF_UI[17] = {{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0 | (4ull << 32), 8 | (3ull << 32), 7 | (2ull << 32), 6 | (1ull << 32)}, {0 | (8ull << 32), 6 | (4ull << 32), 2, 8 | (6ull << 32)}, {0 | (1ull << 32), 2 | (3ull << 32), 4 | (5ull << 32), 6 | (7ull << 32)}, {0 | (4ull << 32), 8, 4 | (8ull << 32), 0 | (4ull << 32)}, {0 | (7ull << 32), 1 | (8ull << 32), 2 | (9ull << 32), 3 | (10ull << 32)}, {0}, {0}, {0}};

class Cramer
{

private:
    enum Distribution : unsigned int
    {
        exponential,
        erlang,
        gamma
    };

    struct Config
    {

        size_t orig_vector_size;
        size_t num_bits_sector;
        size_t num_bits_codewords;
        size_t num_bits_encoding;
        size_t num_total_codewords;
        size_t num_threads;
        size_t num_zero_amps;
        size_t num_sectors;
        size_t num_turnings;
        size_t num_codewords_reg;
        size_t compressed_vector_UL_size;
        size_t num_codewords_per_sector;
        double magnitude_r;
        double A;
        double spiral_length_r;
        double codewords_spacing;
        bool projection_vector;
        Distribution dist_type;
    };

    struct GlobalContext
    {
        atomic<complex<float>> *codewords_mappings;
        atomic<size_t> *cw_freq;
        int codeword_all_amps;
    };

    Config config;
    GlobalContext global_context;
    GlobalContext new_global_context;
    complex<float> *sector_factors;
    double lambda;
    float k; // k -> shape in Gamma dist

    double CalculateCDFofExponential(complex<double> &amp) const;
    __m256 CalculateCDFofExponentialAVX(__m256 &real,
                                        __m256 &imag) const;
    double CalculateCDFofGammaDist(complex<double> &amp) const;
    __m256 CalculateCDFofGammaDistAVX(__m256 &real,
                                      __m256 &imag) const;
    double CalculateCDFofErlangDist(complex<double> &amp) const;
    __m256 CalculateCDFofErlangDistAVX(__m256 &real,
                                       __m256 &imag) const;
    double UniformTransformMagnitudeAndAmp(complex<double> &amp) const;
    __m256 UniformTransformMagnitudeAndAmpAVX(__m256 &real,
                                              __m256 &imag) const;
    complex<double> PTTransformMagnitudeAndAmp(complex<double> amp) const;

    // Polar equation: r = B * a * theta
    double CalcSizeParameterInUniformSpiral(double radius, double theta) const;
    double CalcThetaForMagnitude(double magnitude) const;
    double CalcMagnitudeForTheta(double theta) const;
    double CalcCWForMagnitude(double magnitude) const;
    __m256 CalcCWForMagnitudeAVX(__m256 magnitudes) const;
    double CalcCWForTheta(double theta) const;
    __m256 CalcCWForThetaAVX(__m256 thetas) const;
    __m256 CalcCWForPhaseAndMagnitudeAVX(__m256 magnitude, __m256 phase);
    double CalcMagnitudeForCW(unsigned short codeword) const;
    double CalcApproxSpiralLen(double theta) const;
    __m256 CalcApproxSpiralLenAVX(__m256 thetas) const;
    __m256 CalcApproxThetaForSpiralLenAVX(__m256 spiral_lengths) const;
    double CalcExactSpiralLen(double theta) const;
    __m256 CalcExactSpiralLenAVX(__m256 theta) const;
    double CalcApproxThetaForSpiralLen(double spiral_lenth) const;
    double CalcThetaForCW(unsigned short codeword) const;
    __m256 CalcThetaForCWAVX(__m256 codewords) const;
    complex<float> CalcValForCW(unsigned short codeword) const;
    size_t CalcCWThatFitIn256BitsReg() const;
    size_t CalcNumULInCompressedVector(size_t num_256_reg) const;
    size_t CalcNum256RegForSizeOfVector() const;
    size_t CalcNum256RegForSizeOfBlock(size_t block_size) const;
    double CalcKFromMeanAndVar(double mean,
                               double variance) const;
    double CalcLambdaFromMeanAndVar(double mean,
                                    double variance) const;

    unsigned short ShiftCWToNearestPhase(double phase,
                                         double codeword) const;
    __m256 ShiftCWToNearestPhaseAVX(__m256 phase,
                                    __m256 codeword) const;
    __m256 GetAdjustedPhaseAVX(__m256 phases,
                               __m256 phase_sectors) const;
    __m256 CalcNearestCWToValWithEncodedSectorAVX(__m256 real,
                                                  __m256 imag);
    unsigned short CalcNearestCWToVal(complex<double> val) const;
    __m256 CalcNearestCWToValAVX(__m256 real,
                                 __m256 imag) const;
    unsigned short MapValToCW(complex<double> val);
    __m256 MapValToCWAVX(__m256 real,
                         __m256 imag);
    __m256i PackCWIn256BitsAVXReg(const unsigned int *codewords) const;
    void UnpackCWFrom256Bits(bitset<REG_SIZE> packed_codewords,
                             unsigned short *unpacked_codewords) const;
    __m256i ExtractCodewordFromAVX256Reg(__m256i &packed_codewords,
                                         const __m256i &mask_cw_256) const;
    __m256i ExtractCodewordFromAVX256Reg(__m256i &packed_codewords) const;
    void UnpackCWFrom256BitsAVX(__m256i packed_codewords,
                                unsigned int *unpacked_codewords) const;
    void PackCWBlocksCrossingBoundaries(__m256i *volatile compressed_vector,
                                        const size_t xtra_leading_cw,
                                        const size_t xtra_trailing_cw,
                                        const size_t compressed_v_offset,
                                        const unsigned int *codewords);

public:
    atomic<bool> kAndLambdaInitialized;

    Cramer(size_t vector_size,
           size_t num_codewords,
           size_t num_threads,
           double probabilty_rejection,
           size_t num_sectors = 1 /* Cannot be 0 and should be powers of 2*/,
           bool projection_v = true);
    Cramer(const Cramer &rhs);
    Cramer &operator=(const Cramer &rhs);
    ~Cramer();

    complex<float> *CramerCompress(complex<float> *compressed_v,
                                   complex<float> *state_vector);
    complex<float> *CramerDecompress(complex<float> *decompressed_v,
                                     const complex<float> *state_vector);

    complex<float> *CramerBlockCompress(complex<float> *compressed_vector,
                                        const complex<float> *state_vector,
                                        const size_t block_idx,
                                        const size_t block_size);
    complex<float> *CramerBlockDecompress(complex<float> *decompressed_v,
                                          const complex<float> *state_vector,
                                          const size_t block_idx,
                                          const size_t block_size);
    void CommitGlobalContext();
    void CramerBlockSectorSwitch(complex<float> *state_vector,
                                 const unsigned short *volatile sectors,
                                 const size_t block_idx,
                                 const size_t block_size);
    complex<float> *SetAllAmpsToZero(complex<float> *state_vector);
    complex<float> *SetAllAmpsToOne(complex<float> *state_vector);
    void Rescale(const __m256 rescaling);
    void CalcKandLambdaFromEmpiricalCDF(complex<float> *state_vector,
                                        const size_t block_size);

    size_t GetCompressedVectorSize() const;
    double GetMinInnerRadius() const;
    double GetMaxOuterRadius() const;
    size_t GetNumOfCW() const;
    size_t GetNumValsMappedToZero() const;
    size_t GetGlobalCodeword() const;
    double GetFactorOfDistBetweenTurns() const;
    double GetDistBetweenCW() const;
};

#endif /* Cramer_h */
