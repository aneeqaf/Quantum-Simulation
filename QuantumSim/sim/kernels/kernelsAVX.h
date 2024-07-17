//
//  kernelsAVX.h
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 1/6/18.
//  Copyright © 2018 Aneeqa Fatima. All rights reserved.
//

#ifndef kernelsAVX_h
#define kernelsAVX_h

#include <algorithm>
#include <array>
#include <bitset>
#include <cassert>
#include <cstdlib>
#include <complex>
#include <cstring>
#include <ctime>
#include <cmath>
#include <functional>
#include <immintrin.h>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <memory>
#include <stdalign.h>
#include <stdio.h>
#include <stdlib.h>
#include <valarray>
#include <vector>
#include <utility>
#include <unistd.h>
#include <thread>
// #include <sleef.h>
// #include <sleefinline_avx2.h>
#include "omp.h"

#include "Cramer.h"
#include "gates.h"
#include "profile.h"
#include "zero_opt_mask.h"
#include "math_helper.h"
#include "qubit_partition.h"

using namespace std;

constexpr float kH = 0.707106781;

using idx_size = unsigned long long;

constexpr cmplx kTGate[8] = {1, {kH, kH}, {0, 1}, {-kH, kH}, -1, {-kH, -kH}, {0, -1}, {kH, -kH}};
constexpr float kTGate_re[8] = {1, kH, 0, -kH, -1, -kH, 0, kH};
constexpr float kTGate_im[8] = {0, kH, 1, kH, 0, -kH, -1, -kH};
constexpr cmplx kSqrtCZGate[4] = {1, {0, 1}, -1, {0, -1}};
constexpr cmplx kCZDecomposition[4] = {{1, -1}, {0, 1}, {1, 1}, {1, 0}};
constexpr cmplx ki = {0, 1};
constexpr int kRT = 8 * sizeof(idx_size) + 1;
const unsigned kNUM_BRANCHES = thread::hardware_concurrency(); // ceil(log(thread::hardware_concurrency()));

constexpr __m256 kneg = {-1, 1, -1, 1, -1, 1, -1, 1};
constexpr __m256 kzeros = {0, 0, 0, 0, 0, 0, 0, 0};
constexpr __m256 kneg1 = {-0.0f, 0.0f, -0.0f, 0.0f, -0.0f, 0.0f, -0.0f, 0.0f};
constexpr __m256 kneg2 = {-0.0f, -0.0f, -0.0f, -0.0f, -0.0f, -0.0f, -0.0f, -0.0f};
constexpr __m256 kneg3 = {0.0f, -0.0f, 0.0f, -0.0f, 0.0f, -0.0f, 0.0f, -0.0f};
constexpr __m128 kneg128 = {-0.0f, -0.0f, -0.0f, -0.0f};

const __m256 kfAll1s = (__m256)_mm256_set1_epi64x(-1);
const float kAllOnes = static_cast<float>(((__m128)_mm_set1_epi64x(-1))[0]);

// NaN has all bits set to one in a float.
const __m256 kzero01[4] = {{kfAll1s}, {0, 0, kAllOnes, kAllOnes, 0, 0, kAllOnes, kAllOnes}, {0, 0, 0, 0, kAllOnes, kAllOnes, kAllOnes, kAllOnes}, {0, 0, 0, 0, 0, 0, kAllOnes, kAllOnes}};
const __m256 kzero10[4] = {{kfAll1s}, {kAllOnes, kAllOnes, 0, 0, kAllOnes, kAllOnes, 0, 0}, {kAllOnes, kAllOnes, kAllOnes, kAllOnes, 0, 0, 0, 0}, {kAllOnes, kAllOnes, 0, 0, 0, 0, 0, 0}};
const __m256 knegZ[4] = {{0}, {0, 0, -0.0f, -0.0f, 0, 0, -0.0f, -0.0f}, {0, 0, 0, 0, -0.0f, -0.0f, -0.0f, -0.0f}, {0, 0, -0.0f, -0.0f, -0.0f, -0.0f, 0, 0}};

constexpr idx_size kCmplxInL1Cache = (1ull << 13);

constexpr idx_size kBlockApplicationLimit = 7;
//__attribute__((always_inline)) inline bool
// CheckIfBlockIsZero(const ZeroOptMask& zero_opt_mask,
//                   const int idx,
//                   const idx_size block_size)
//{
//    const idx_size num_qubits = zero_opt_mask.size();
//
//    for (idx_size i = 0; i < num_qubits; ++i) {
//        const idx_size zero_block = (1ull << (num_qubits - 1 - i));
//        if (zero_opt_mask[i] != '*' && zero_block >= block_size) {
//            if (zero_opt_mask[i] == '0' && (~(idx + block_size - 1) & zero_block))
//                return true;
//            else if (zero_opt_mask[i] == '1' && ((idx + block_size - 1) & zero_block))
//                return true;
//        }
//    }
//    return false;
//}

__attribute__((always_inline)) inline void
ApplyHGateAVX(cmplx *__restrict amp,
              const idx_size *indices /*2*/)
{
    float *__restrict t_amp = (float *)__builtin_assume_aligned(amp, 64);
    __m256 a0 = _mm256_load_ps(&t_amp[2 * indices[0]]);
    __m256 a1 = _mm256_load_ps(&t_amp[2 * indices[1]]);

    __m256 t0 = _mm256_add_ps(a0, a1);
    __m256 t1 = _mm256_sub_ps(a0, a1);

    _mm256_store_ps(&t_amp[2 * indices[0]], t0);
    _mm256_store_ps(&t_amp[2 * indices[1]], t1);
}

__attribute__((always_inline)) inline void
ApplyX12GateAVX(cmplx *__restrict amp,
                const idx_size *indices /*2*/)
{
    float *__restrict t_amp = (float *)__builtin_assume_aligned(amp, 64);
    __m256 a0 = _mm256_load_ps(&t_amp[2 * indices[0]]);
    __m256 a1 = _mm256_load_ps(&t_amp[2 * indices[1]]);

    //    if (_mm256_movemask_ps(_mm256_cmp_ps(a0, kzeros, _CMP_EQ_OQ)) == 255
    //        && _mm256_movemask_ps(_mm256_cmp_ps(a1, kzeros, _CMP_EQ_OQ))  == 255)
    //        return;

    __m256 a0_p = _mm256_permute_ps(a0, 0b10110001);
    __m256 a1_p = _mm256_permute_ps(a1, 0b10110001);
    a0_p = _mm256_xor_ps(a0_p, kneg1);
    a1_p = _mm256_xor_ps(a1_p, kneg1);

    __m256 t0 = _mm256_add_ps(a0, a0_p);
    __m256 t1 = _mm256_sub_ps(a1, a1_p);
    __m256 t2 = _mm256_sub_ps(a0, a0_p);
    __m256 t3 = _mm256_add_ps(a1, a1_p);

    a0 = _mm256_add_ps(t0, t1);
    a1 = _mm256_add_ps(t2, t3);

    _mm256_store_ps(&t_amp[2 * indices[0]], a0);
    _mm256_store_ps(&t_amp[2 * indices[1]], a1);
}

__attribute__((always_inline)) inline void
ApplyY12GateAVX(cmplx *__restrict amp,
                const idx_size *indices /*2*/)
{
    float *__restrict t_amp = (float *)__builtin_assume_aligned(amp, 64);
    __m256 a0 = _mm256_load_ps(&t_amp[2 * indices[0]]);
    __m256 a1 = _mm256_load_ps(&t_amp[2 * indices[1]]);

    //    if (_mm256_movemask_ps(_mm256_cmp_ps(a0, kzeros, _CMP_EQ_OQ)) == 255
    //        && _mm256_movemask_ps(_mm256_cmp_ps(a1, kzeros, _CMP_EQ_OQ))  == 255)
    //        return;
    //
    __m256 a1_neg = _mm256_xor_ps(a1, kneg2);
    __m256 a0_p = _mm256_permute_ps(a0, 0b10110001);
    __m256 a1_p = _mm256_permute_ps(a1, 0b10110001);
    a0_p = _mm256_xor_ps(a0_p, kneg1);
    a1_p = _mm256_xor_ps(a1_p, kneg1);

    __m256 t0 = _mm256_add_ps(a0, a0_p);
    __m256 t1 = _mm256_sub_ps(a1_neg, a1_p);
    __m256 t2 = t0;
    __m256 t3 = _mm256_add_ps(a1, a1_p);

    a0 = _mm256_add_ps(t0, t1);
    a1 = _mm256_add_ps(t2, t3);

    _mm256_store_ps(&t_amp[2 * indices[0]], a0);
    _mm256_store_ps(&t_amp[2 * indices[1]], a1);
}

__attribute__((always_inline)) inline void
ApplyHHGateAVX(cmplx *__restrict amp,
               const idx_size *indices /*4*/)
{
    float *__restrict t_amp = (float *)__builtin_assume_aligned(amp, 64);
    __m256 a0 = _mm256_load_ps(&t_amp[2 * indices[0]]);
    __m256 a1 = _mm256_load_ps(&t_amp[2 * indices[1]]);
    __m256 a2 = _mm256_load_ps(&t_amp[2 * indices[2]]);
    __m256 a3 = _mm256_load_ps(&t_amp[2 * indices[3]]);

    const __m256 t0 = _mm256_add_ps(a0, a3);
    const __m256 t1 = _mm256_sub_ps(a0, a3);
    const __m256 t2 = _mm256_add_ps(a1, a2);
    const __m256 t3 = _mm256_sub_ps(a1, a2);

    a0 = _mm256_sub_ps(t0, t2);
    a1 = _mm256_add_ps(t1, t3);
    a2 = _mm256_sub_ps(t1, t3);
    a3 = _mm256_add_ps(t0, t2);

    _mm256_store_ps(&t_amp[2 * indices[0]], a3);
    _mm256_store_ps(&t_amp[2 * indices[1]], a2);
    _mm256_store_ps(&t_amp[2 * indices[2]], a1);
    _mm256_store_ps(&t_amp[2 * indices[3]], a0);
}

__attribute__((always_inline)) inline void
ApplyXX12GateAVX(cmplx *__restrict amp,
                 const idx_size *indices /*4*/)
{
    float *__restrict t_amp = (float *)__builtin_assume_aligned(amp, 64);
    __m256 a0 = _mm256_load_ps(&t_amp[2 * indices[0]]);
    __m256 a1 = _mm256_load_ps(&t_amp[2 * indices[1]]);
    __m256 a2 = _mm256_load_ps(&t_amp[2 * indices[2]]);
    __m256 a3 = _mm256_load_ps(&t_amp[2 * indices[3]]);

    //    if (_mm256_movemask_ps(_mm256_cmp_ps(a0, kzeros, _CMP_EQ_OQ)) == 255
    //        && _mm256_movemask_ps(_mm256_cmp_ps(a1, kzeros, _CMP_EQ_OQ))  == 255
    //        && _mm256_movemask_ps(_mm256_cmp_ps(a2, kzeros, _CMP_EQ_OQ))  == 255
    //        && _mm256_movemask_ps(_mm256_cmp_ps(a3, kzeros, _CMP_EQ_OQ))  == 255)
    //        return;

    const __m256 t0 = _mm256_add_ps(a0, a3);
    const __m256 t1 = _mm256_add_ps(a1, a2);
    __m256 t2 = _mm256_sub_ps(a0, a3);
    __m256 t3 = _mm256_sub_ps(a1, a2);
    t2 = _mm256_permute_ps(t2, 0b10110001);
    t2 = _mm256_xor_ps(t2, kneg1);
    t3 = _mm256_permute_ps(t3, 0b10110001);
    t3 = _mm256_xor_ps(t3, kneg1);

    a0 = _mm256_add_ps(t1, t2);
    a1 = _mm256_add_ps(t0, t3);
    a2 = _mm256_sub_ps(t0, t3);
    a3 = _mm256_sub_ps(t1, t2);

    _mm256_store_ps(&t_amp[2 * indices[0]], a0);
    _mm256_store_ps(&t_amp[2 * indices[1]], a1);
    _mm256_store_ps(&t_amp[2 * indices[2]], a2);
    _mm256_store_ps(&t_amp[2 * indices[3]], a3);
}

__attribute__((always_inline)) inline void
ApplyYY12GateAVX(cmplx *__restrict amp,
                 const idx_size *indices /*4*/)
{
    float *__restrict t_amp = (float *)__builtin_assume_aligned(amp, 64);
    __m256 a0 = _mm256_load_ps(&t_amp[2 * indices[0]]);
    __m256 a1 = _mm256_load_ps(&t_amp[2 * indices[1]]);
    __m256 a2 = _mm256_load_ps(&t_amp[2 * indices[2]]);
    __m256 a3 = _mm256_load_ps(&t_amp[2 * indices[3]]);

    //    if (_mm256_movemask_ps(_mm256_cmp_ps(a0, kzeros, _CMP_EQ_OQ)) == 255
    //        && _mm256_movemask_ps(_mm256_cmp_ps(a1, kzeros, _CMP_EQ_OQ))  == 255
    //        && _mm256_movemask_ps(_mm256_cmp_ps(a2, kzeros, _CMP_EQ_OQ))  == 255
    //        && _mm256_movemask_ps(_mm256_cmp_ps(a3, kzeros, _CMP_EQ_OQ))  == 255)
    //        return;

    const __m256 t0 = _mm256_add_ps(a0, a3);
    const __m256 t1 = _mm256_sub_ps(a0, a3);
    const __m256 t2 = _mm256_add_ps(a1, a2);
    const __m256 t3 = _mm256_sub_ps(a1, a2);

    a0 = _mm256_sub_ps(t0, t2);
    a1 = _mm256_add_ps(t1, t3);
    a2 = _mm256_sub_ps(t1, t3);
    a3 = _mm256_add_ps(t0, t2);

    _mm256_store_ps(&t_amp[2 * indices[0]], a0);
    _mm256_store_ps(&t_amp[2 * indices[1]], a1);
    _mm256_store_ps(&t_amp[2 * indices[2]], a2);
    _mm256_store_ps(&t_amp[2 * indices[3]], a3);
}

__attribute__((always_inline)) inline void
ApplyXY12GateAVX(cmplx *__restrict amp,
                 const idx_size *indices /*4*/)
{
    float *__restrict t_amp = (float *)__builtin_assume_aligned(amp, 64);
    __m256 a0 = _mm256_load_ps(&t_amp[2 * indices[0]]);
    __m256 a1 = _mm256_load_ps(&t_amp[2 * indices[1]]);
    __m256 a2 = _mm256_load_ps(&t_amp[2 * indices[2]]);
    __m256 a3 = _mm256_load_ps(&t_amp[2 * indices[3]]);

    //    if (_mm256_movemask_ps(_mm256_cmp_ps(a0, kzeros, _CMP_EQ_OQ)) == 255
    //        && _mm256_movemask_ps(_mm256_cmp_ps(a1, kzeros, _CMP_EQ_OQ))  == 255
    //        && _mm256_movemask_ps(_mm256_cmp_ps(a2, kzeros, _CMP_EQ_OQ))  == 255
    //        && _mm256_movemask_ps(_mm256_cmp_ps(a3, kzeros, _CMP_EQ_OQ))  == 255)
    //        return;

    const __m256 t0 = _mm256_sub_ps(a0, a1);
    const __m256 t1 = _mm256_sub_ps(a2, a3);
    const __m256 t2 = _mm256_add_ps(a0, a1);
    const __m256 t3 = _mm256_add_ps(a2, a3);

    __m256 it0 = _mm256_permute_ps(t0, 0b10110001);
    it0 = _mm256_xor_ps(it0, kneg1);
    __m256 it1 = _mm256_permute_ps(t1, 0b10110001);
    it1 = _mm256_xor_ps(it1, kneg1);
    __m256 it2 = _mm256_permute_ps(t2, 0b10110001);
    it2 = _mm256_xor_ps(it2, kneg1);
    __m256 it3 = _mm256_permute_ps(t3, 0b10110001);
    it3 = _mm256_xor_ps(it3, kneg1);

    a0 = _mm256_add_ps(it0, t1);
    a1 = _mm256_add_ps(it2, t3);
    a2 = _mm256_add_ps(t0, it1);
    a3 = _mm256_add_ps(t2, it3);

    _mm256_store_ps(&t_amp[2 * indices[0]], a0);
    _mm256_store_ps(&t_amp[2 * indices[1]], a1);
    _mm256_store_ps(&t_amp[2 * indices[2]], a2);
    _mm256_store_ps(&t_amp[2 * indices[3]], a3);
}

__attribute__((always_inline)) inline void
ApplyYX12GateAVX(cmplx *__restrict amp,
                 const idx_size *indices /*4*/)
{

    float *__restrict t_amp = (float *)__builtin_assume_aligned(amp, 64);
    __m256 a0 = _mm256_load_ps(&t_amp[2 * indices[0]]);
    __m256 a1 = _mm256_load_ps(&t_amp[2 * indices[1]]);
    __m256 a2 = _mm256_load_ps(&t_amp[2 * indices[2]]);
    __m256 a3 = _mm256_load_ps(&t_amp[2 * indices[3]]);

    //    if (_mm256_movemask_ps(_mm256_cmp_ps(a0, kzeros, _CMP_EQ_OQ)) == 255
    //        && _mm256_movemask_ps(_mm256_cmp_ps(a1, kzeros, _CMP_EQ_OQ))  == 255
    //        && _mm256_movemask_ps(_mm256_cmp_ps(a2, kzeros, _CMP_EQ_OQ))  == 255
    //        && _mm256_movemask_ps(_mm256_cmp_ps(a3, kzeros, _CMP_EQ_OQ))  == 255)
    //        return;

    __m256 ia0 = _mm256_permute_ps(a0, 0b10110001);
    ia0 = _mm256_xor_ps(ia0, kneg1);
    __m256 ia1 = _mm256_permute_ps(a1, 0b10110001);
    ia1 = _mm256_xor_ps(ia1, kneg1);
    __m256 ia2 = _mm256_permute_ps(a2, 0b10110001);
    ia2 = _mm256_xor_ps(ia2, kneg1);
    __m256 ia3 = _mm256_permute_ps(a3, 0b10110001);
    ia3 = _mm256_xor_ps(ia3, kneg1);

    const __m256 t0 = _mm256_add_ps(ia0, a1);
    const __m256 t1 = _mm256_add_ps(a0, ia1);
    const __m256 t2 = _mm256_add_ps(ia2, a3);
    const __m256 t3 = _mm256_add_ps(a2, ia3);

    a0 = _mm256_sub_ps(t0, t2);
    a1 = _mm256_sub_ps(t1, t3);
    a2 = _mm256_add_ps(t0, t2);
    a3 = _mm256_add_ps(t1, t3);

    _mm256_store_ps(&t_amp[2 * indices[0]], a0);
    _mm256_store_ps(&t_amp[2 * indices[1]], a1);
    _mm256_store_ps(&t_amp[2 * indices[2]], a2);
    _mm256_store_ps(&t_amp[2 * indices[3]], a3);
}

__attribute__((always_inline)) inline void
ApplyYY12HHGateAVX(cmplx *__restrict amp,
                   const idx_size *indices /*4*/)
{
    float *__restrict t_amp = (float *)__builtin_assume_aligned(amp, 64);
    __m256 a1 = _mm256_load_ps(&t_amp[2 * indices[1]]);
    __m256 a2 = _mm256_load_ps(&t_amp[2 * indices[2]]);

    a1 = _mm256_xor_ps(a1, kneg2);
    a2 = _mm256_xor_ps(a2, kneg2);

    _mm256_store_ps(&t_amp[2 * indices[1]], a1);
    _mm256_store_ps(&t_amp[2 * indices[2]], a2);
}

__attribute__((always_inline)) inline void
ApplyXX12HHGateAVX(cmplx *__restrict amp,
                   const idx_size *indices /*4*/)
{
    float *__restrict t_amp = (float *)__builtin_assume_aligned(amp, 64);
    __m256 a0 = _mm256_load_ps(&t_amp[2 * indices[0]]);
    __m256 a1 = _mm256_load_ps(&t_amp[2 * indices[1]]);
    __m256 a2 = _mm256_load_ps(&t_amp[2 * indices[2]]);
    __m256 a3 = _mm256_load_ps(&t_amp[2 * indices[3]]);

    __m256 t0 = _mm256_add_ps(a0, a3);
    __m256 t1 = _mm256_add_ps(a1, a2);
    __m256 t2 = _mm256_sub_ps(a0, a3);
    __m256 t3 = _mm256_sub_ps(a1, a2);
    t2 = _mm256_permute_ps(t2, 0b10110001);
    t2 = _mm256_xor_ps(t2, kneg1);
    t3 = _mm256_permute_ps(t3, 0b10110001);
    t3 = _mm256_xor_ps(t3, kneg1);

    a0 = _mm256_add_ps(t1, t0);
    a1 = _mm256_sub_ps(t2, t3);
    a2 = _mm256_add_ps(t2, t3);
    a3 = _mm256_sub_ps(t1, t0);

    _mm256_store_ps(&t_amp[2 * indices[0]], a0);
    _mm256_store_ps(&t_amp[2 * indices[1]], a1);
    _mm256_store_ps(&t_amp[2 * indices[2]], a2);
    _mm256_store_ps(&t_amp[2 * indices[3]], a3);
}

__attribute__((always_inline)) inline void
ApplyXY12HHGateAVX(cmplx *__restrict amp,
                   const idx_size *indices /*4*/)
{
    float *__restrict t_amp = (float *)__builtin_assume_aligned(amp, 64);
    __m256 a0 = _mm256_load_ps(&t_amp[2 * indices[0]]);
    __m256 a1 = _mm256_load_ps(&t_amp[2 * indices[1]]);
    __m256 a2 = _mm256_load_ps(&t_amp[2 * indices[2]]);
    __m256 a3 = _mm256_load_ps(&t_amp[2 * indices[3]]);

    __m256 ia0 = _mm256_permute_ps(a0, 0b10110001);
    ia0 = _mm256_xor_ps(ia0, kneg1);
    __m256 _ia1 = _mm256_permute_ps(a1, 0b10110001);
    _ia1 = _mm256_xor_ps(_ia1, kneg3);
    __m256 ia2 = _mm256_permute_ps(a2, 0b10110001);
    ia2 = _mm256_xor_ps(ia2, kneg1);
    __m256 ia3 = _mm256_permute_ps(a3, 0b10110001);
    ia3 = _mm256_xor_ps(ia3, kneg1);

    __m256 t0 = _mm256_add_ps(ia0, a2);
    __m256 t1 = _mm256_add_ps(a0, ia2);
    __m256 t2 = _mm256_sub_ps(_ia1, a3);
    __m256 t3 = _mm256_add_ps(a1, ia3);

    a0 = _mm256_add_ps(t0, t1);
    a1 = _mm256_sub_ps(t2, t3);
    a2 = _mm256_sub_ps(t0, t1);
    a3 = _mm256_add_ps(t3, t2);

    _mm256_store_ps(&t_amp[2 * indices[0]], a0);
    _mm256_store_ps(&t_amp[2 * indices[1]], a1);
    _mm256_store_ps(&t_amp[2 * indices[2]], a2);
    _mm256_store_ps(&t_amp[2 * indices[3]], a3);
}

__attribute__((always_inline)) inline void
ApplyYX12HHGateAVX(cmplx *__restrict amp,
                   const idx_size *indices /*4*/)
{
    float *__restrict t_amp = (float *)__builtin_assume_aligned(amp, 64);
    __m256 a0 = _mm256_load_ps(&t_amp[2 * indices[0]]);
    __m256 a1 = _mm256_load_ps(&t_amp[2 * indices[1]]);
    __m256 a2 = _mm256_load_ps(&t_amp[2 * indices[2]]);
    __m256 a3 = _mm256_load_ps(&t_amp[2 * indices[3]]);

    __m256 ia0 = _mm256_permute_ps(a0, 0b10110001);
    ia0 = _mm256_xor_ps(ia0, kneg1);
    __m256 ia1 = _mm256_permute_ps(a1, 0b10110001);
    ia1 = _mm256_xor_ps(ia1, kneg1);
    __m256 _ia2 = _mm256_permute_ps(a2, 0b10110001);
    _ia2 = _mm256_xor_ps(_ia2, kneg3);
    __m256 ia3 = _mm256_permute_ps(a3, 0b10110001);
    ia3 = _mm256_xor_ps(ia3, kneg1);

    __m256 t0 = _mm256_add_ps(ia0, a1);
    __m256 t1 = _mm256_add_ps(a0, ia1);
    __m256 t2 = _mm256_sub_ps(_ia2, a3);
    __m256 t3 = _mm256_add_ps(a2, ia3);

    a0 = _mm256_add_ps(t0, t1);
    a1 = _mm256_sub_ps(t0, t1);
    a2 = _mm256_sub_ps(t2, t3);
    a3 = _mm256_add_ps(t3, t2);

    _mm256_store_ps(&t_amp[2 * indices[0]], a0);
    _mm256_store_ps(&t_amp[2 * indices[1]], a1);
    _mm256_store_ps(&t_amp[2 * indices[2]], a2);
    _mm256_store_ps(&t_amp[2 * indices[3]], a3);
}

__attribute__((always_inline)) inline bool
ApplyxCZGateAVX(cmplx *__restrict amp,
                int num_threads,
                const int num_qubits_amp,
                const idx_size *volatile __restrict gate_bitmask,
                const ZeroOptMask &zero_opt_mask)
{
    /*0 : Z; 1 : 01; 2 : 10 */
    float *__restrict t_amp = (float *)__builtin_assume_aligned(amp, 64);

    const __m256 mm256_neg = knegZ[gate_bitmask[0] & 3];
    const __m256 mm256_01 = kzero01[gate_bitmask[1] & 3];
    const __m256 mm256_10 = kzero10[gate_bitmask[2] & 3];
    const idx_size amp_size = 2 * (1ull << num_qubits_amp);
    //
    //    cout << "Z last two bits : " << (gate_bitmask[0] & 3)  << endl;
    //    cout << "01 last two bits : " << (gate_bitmask[1] & 3) << endl;
    //    cout << "10 last two bits : " << (gate_bitmask[2] & 3) << endl;
    //
    //    cout << "Z bm : " << (gate_bitmask[0])  << endl;
    //    cout << "01 bm : " << (gate_bitmask[1]) << endl;
    //    cout << "10 bm : " << (gate_bitmask[2]) << endl;

    bool all_zeros = true;

#pragma omp parallel for schedule(guided) num_threads(num_threads)
    for (idx_size i = 0; i < amp_size; i += 8)
    {
        idx_size amp_idx = i / 2;
        if (((~amp_idx - 3) & gate_bitmask[1]) || (amp_idx & gate_bitmask[2]))
        {
            _mm256_store_ps(&t_amp[i], kzeros);
            continue;
        }

        __m256 temp_amp = _mm256_load_ps(&t_amp[i]);
        if (_mm256_movemask_ps(_mm256_cmp_ps(temp_amp, kzeros, _CMP_EQ_OQ)) == 255)
            continue;

        temp_amp = _mm256_and_ps(temp_amp, mm256_01);
        temp_amp = _mm256_and_ps(temp_amp, mm256_10);
        temp_amp = _mm256_xor_ps(temp_amp, mm256_neg);

        if (__builtin_parityll(amp_idx & gate_bitmask[0]))
            temp_amp = _mm256_xor_ps(temp_amp, kneg2);

        if (all_zeros && _mm256_movemask_ps(_mm256_cmp_ps(temp_amp, kzeros, _CMP_EQ_OQ)) != 255)
        {
#pragma omp atomic write
            all_zeros = false;
        }

        _mm256_store_ps(&t_amp[i], temp_amp);
    }

    return all_zeros;
}

__attribute__((always_inline)) inline void
ApplyRzPhasesAVX(float *__restrict t_amp,
                 const __m256 &phases_idx,
                 const idx_size idx /* starting idx of 8 contiguous idxs */)
{
    // Separate out the multiplies since each amp occupies two slots in m256.
    __m256 first_4_multiples = _mm256_permute2f128_ps(phases_idx, phases_idx, 0b00000000);
    __m256 second_4_multiples = _mm256_permute2f128_ps(phases_idx, phases_idx, 0b00010001);
    const __m256i idxs = {0, 0x0000000100000001, 0x0000000200000002, 0x0000000300000003};
    first_4_multiples = _mm256_permutevar_ps(first_4_multiples, idxs);
    second_4_multiples = _mm256_permutevar_ps(second_4_multiples, idxs);

    const __m256 cos_first_4 = Sleef_cosf8_u10avx2(first_4_multiples);
    const __m256 cos_second_4 = Sleef_cosf8_u10avx2(second_4_multiples);
    const __m256 sin_first_4 = Sleef_sinf8_u10avx2(first_4_multiples);
    const __m256 sin_second_4 = Sleef_sinf8_u10avx2(second_4_multiples);

    __m256 a0 = _mm256_load_ps(&t_amp[2 * idx]);
    __m256 a1 = _mm256_load_ps(&t_amp[2 * (idx + 4)]);

    // Switch imag and real
    __m256 ia0 = _mm256_permute_ps(a0, 0b10110001);
    ia0 = _mm256_xor_ps(ia0, kneg1);
    __m256 ia1 = _mm256_permute_ps(a1, 0b10110001);
    ia1 = _mm256_xor_ps(ia1, kneg1);

    // Multiply with real part of exp
    a0 = _mm256_mul_ps(cos_first_4, a0);
    a1 = _mm256_mul_ps(cos_second_4, a1);

    // Multiply with imag part of exp
    ia0 = _mm256_mul_ps(sin_first_4, ia0);
    ia1 = _mm256_mul_ps(sin_second_4, ia1);

    // Complete (a + bi)(cos(theta) + sin(theta)i)
    a0 = _mm256_add_ps(a0, ia0);
    a1 = _mm256_add_ps(a1, ia1);

    _mm256_store_ps(&t_amp[2 * idx], a0);
    _mm256_store_ps(&t_amp[2 * (idx + 4)], a1);
}

double
CalculateRzPhase(const idx_size idx,
                 const idx_size num_qubits,
                 const double *phases /*num qubits*/);

double
CalculateQftCRkPhase(const idx_size idx,
                     const idx_size num_qubits,
                     const idx_size *CRk_bitmasks);

void PrepareQFTCRkBitmask(idx_size *CRk_bitmasks,
                          const idx_size cycle_num,
                          const idx_size num_qubits);

double
CalculateCRzPhaseAndPopulatePhasesMatrix(double *control_phase_sums /* num_qubits */,
                                         double *target_phase_sums /* num_qubits */,
                                         const idx_size idx,
                                         const idx_size num_qubits,
                                         const vector<vector<double>> &phases);

pair<__m256, bool>
CalculatePhaseForRzInGroupsOf8(volatile double &acc_phase_first,
                               volatile idx_size &gray_code_idx,
                               idx_size reg_idx,
                               idx_size block_begin_idx,
                               const idx_size num_qubits,
                               const double *phases /*num qubits*/);

pair<__m256, bool>
CalculatePhaseForCRzInGroupsOf8(double *volatile control_phase_sums,
                                double *volatile target_phase_sums,
                                volatile double &base_phase,
                                idx_size gray_code_idx,
                                idx_size reg_idx,
                                idx_size block_begin_idx,
                                const idx_size num_qubits,
                                const vector<vector<double>> &phases);

void CalculatePhaseForQftCRkInGroupsOf8(idx_size gray_code_idx,
                                        const idx_size num_qubits,
                                        const idx_size *CRk_bitmasks);

void ApplyRzGatesAVX(cmplx *__restrict amp,
                     const idx_size num_qubits,
                     /* difference between starting and end idx should be multiple of 8 */
                     const idx_size idx_begin,
                     const idx_size idx_end,
                     const double *phases /* num qubits */);

void ApplyCRzGatesAVX(cmplx *__restrict amp,
                      const idx_size num_qubits,
                      /* difference between starting and end idx should be multiple of 8 */
                      const idx_size idx_begin,
                      const idx_size idx_end,
                      const vector<vector<double>> &phases /* num qubits x num qubits */);

void ApplyQftCRkGatesAVX(cmplx *__restrict amp,
                         const idx_size target,
                         const idx_size num_qubits,
                         /* difference between starting and end idx should be multiple of 8 */
                         const idx_size idx_begin,
                         const idx_size idx_end,
                         const idx_size *CRk_bitmasks /* num qubits */);

pair<idx_size, idx_size>
XYFastTransformLowQ(cmplx *__restrict amp,
                    idx_size X_bitmask,
                    idx_size Y_bitmask,
                    idx_size H_bitmask,
                    const int num_qubits,
                    const int num_threads);

pair<idx_size, int>
XYHFastTransformHighQ(cmplx *__restrict amp,
                      idx_size X_bitmask,
                      idx_size Y_bitmask,
                      idx_size H_bitmask,
                      const int num_qubits,
                      const int num_threads);

void ApplyHGatesIteratively(cmplx *__restrict amp,
                            idx_size num_qubits,
                            idx_size num_threads,
                            idx_size gate_bm);

void ApplyBlockOfCZTGatesAVXSeq(cmplx *__restrict amp,
                                const int num_qubits_amp,
                                const idx_size *volatile __restrict CZ_bitmasks,
                                const idx_size *volatile __restrict T_bitmasks);

pair<idx_size, idx_size>
ApplyBlockOfCZTAndLowQXYHGatesAVX(cmplx *&amp,
                                  Cramer *cramer,
                                  const int num_qubits_amp,
                                  const idx_size *volatile __restrict CZ_bitmasks,
                                  const idx_size *volatile __restrict T_bitmasks,
                                  const idx_size Lo_X_bitmask,
                                  const idx_size Lo_Y_bitmask,
                                  const idx_size Lo_H_bitmask,
                                  const int num_threads,
                                  const int num_high_qubits,
                                  const ZeroOptMask &zero_opt_mask);

void RescaleAndApplyGlobalICounter(cmplx *&__restrict amp,
                                   idx_size &global_factor_power,
                                   idx_size &global_i_counter,
                                   const size_t amp_size,
                                   const size_t num_threads = 8);

#endif /* kernelsAVX_h */
