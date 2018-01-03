//
//  temp.cpp
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 12/28/17.
//  Copyright © 2017 Aneeqa Fatima. All rights reserved.
//

#include <stdio.h>
#include <vector>
#include <complex>
#include <array>
#include <functional>
#include <immintrin.h>
#include "libs/gates.h"

using namespace std;

using cmplx = complex<float>;
using idx_size = size_t;

constexpr float kH = 0.707106781;

using idx_size = size_t;

constexpr cmplx ki = {0,1};

void
ExtractIndicesForAmp(idx_size* strides,
                     const int* gate_qubits,
                     const idx_size gate_qubits_size,
                     const int total_circuit_qubits,
                     const idx_size starting_idx = 0)
{
    const idx_size num_q = gate_qubits_size;
    idx_size strides_size = 1, gap = 1ull << (num_q - 1);
    
    strides[0] = starting_idx;
    idx_size prev_gap = gap;
    for (idx_size i = starting_idx ; i < num_q; ++i) {
        for (idx_size n = 0; strides_size < (1ull << (i+1)); n += prev_gap) {
            
            strides[n + gap] = strides[n] + (1ull << ((total_circuit_qubits - 1) - gate_qubits[i]));
            ++strides_size;
        }
        prev_gap = gap;
        gap /= 2;
    }
}

void
Apply4X12Gate(cmplx* __restrict amp,
              const array<idx_size, 16> indices)
{
    //1st permutation
    __m128 __restrict t_re[4], t_im[4];
    {
        float* __restrict a_re = (float*)__builtin_assume_aligned(&amp[0], 32);
        float* __restrict a_im = (float*)__builtin_assume_aligned(&amp[0], 32) + 1;
        idx_size* idx = (idx_size*)(&indices[0]);
        
        for (idx_size i = 0; i < 4; ++i) {
            t_re[i] = _mm_setr_ps(a_re[2*idx[0]], a_re[2*idx[1]], a_re[2*idx[2]], a_re[2*idx[3]]);
            t_im[i] = _mm_setr_ps(a_im[2*idx[0]], a_im[2*idx[1]], a_im[2*idx[2]], a_im[2*idx[3]]);
            idx += 4;
        }
    }
    
    __m128 res0_re_add , res1_re_add, res0_re_sub , res1_re_sub, res0_im_add,
    res1_im_add , res0_im_sub , res1_im_sub ;
    
    {
        // ~~~~~~ real arithmetics ~~~~~~
        const __m128 tres0 = _mm_add_ps(t_re[3], t_re[0]);
        const __m128 tres1 = _mm_add_ps(t_re[2], t_re[1]);
        const __m128 tres2 = _mm_sub_ps(t_im[3], t_im[0]);
        const __m128 tres3 = _mm_sub_ps(t_im[2], t_im[1]);
        
        res0_re_add = _mm_add_ps(tres1, tres2);
        res1_re_add = _mm_add_ps(tres0, tres3);
        res0_re_sub = _mm_sub_ps(tres0, tres3);
        res1_re_sub = _mm_sub_ps(tres1, tres2);
        
        // ~~~~~~ imaginary arithmetics ~~~~~~
        const __m128 tres4 = _mm_add_ps(t_im[0], t_im[3]);
        const __m128 tres5 = _mm_add_ps(t_im[1], t_im[2]);
        const __m128 tres6 = _mm_sub_ps(t_re[0], t_re[3]);
        const __m128 tres7 = _mm_sub_ps(t_re[1], t_re[2]);
        
        res0_im_add = _mm_add_ps(tres5, tres6);
        res1_im_add = _mm_add_ps(tres4, tres7);
        res0_im_sub = _mm_sub_ps(tres4, tres7);
        res1_im_sub = _mm_sub_ps(tres5, tres6);
    }
    //--------------------------------------------------------------------------------------------------------
    //2nd permutation
    
    const __m128 slice_re[4] = {_mm_permute_ps(res0_re_add, 0b10011100), _mm_permute_ps(res1_re_add, 0b10011100),
                                _mm_permute_ps(res0_re_sub, 0b10011100), _mm_permute_ps(res1_re_sub, 0b10011100)};
    const __m128 slice_im[4] = {_mm_permute_ps(res0_im_add, 0b01100011), _mm_permute_ps(res1_im_add, 0b01100011),
                                _mm_permute_ps(res0_im_sub, 0b01100011), _mm_permute_ps(res1_im_sub, 0b01100011)};
    
    {
        // ~~~~~~ real arithmetics ~~~~~~
        const __m128 tres8 = _mm_hadd_ps(slice_re[0], slice_re[1]);
        const __m128 tres9 = _mm_hadd_ps(slice_re[2], slice_re[3]);
        __m128 tres10 = _mm_hsub_ps(slice_im[0], slice_im[1]);
        __m128 tres11 = _mm_hsub_ps(slice_im[2], slice_im[3]);
        tres10 = _mm_permute_ps(tres10, 0b10110001);
        tres11 = _mm_permute_ps(tres11, 0b10110001);
        
        res0_re_add = _mm_add_ps(tres8, tres10);
        res0_re_sub = _mm_sub_ps(tres8, tres10);
        res1_re_add = _mm_add_ps(tres9, tres11);
        res1_re_sub = _mm_sub_ps(tres9, tres11);
        
        // ~~~~~~ imaginary arithmetics ~~~~~~
        const __m128 tres12 = _mm_hadd_ps(slice_im[0], slice_im[1]);
        const __m128 tres13 = _mm_hadd_ps(slice_im[2], slice_im[3]);
        __m128 tres14 = _mm_hsub_ps(slice_re[0], slice_re[1]);
        __m128 tres15 = _mm_hsub_ps(slice_re[2], slice_re[3]);
        tres14 = _mm_permute_ps(tres14, 0b10110001);
        tres15 = _mm_permute_ps(tres15, 0b10110001);
        
        res0_im_add = _mm_add_ps(tres12, tres14);
        res0_im_sub = _mm_sub_ps(tres12, tres14);
        res1_im_add = _mm_add_ps(tres13, tres15);
        res1_im_sub = _mm_sub_ps(tres13, tres15);
    }

    //--------------------------------------------------------------------------------------------------------
    //memory writes
    const float* res_re[4] = {(float*)&res0_re_add, (float*)&res0_re_sub, (float*)&res1_re_add, (float*)&res1_re_sub};
    const float* res_im[4] = {(float*)&res0_im_add, (float*)&res0_im_sub, (float*)&res1_im_add, (float*)&res1_im_sub};
    
    amp[indices[0]] = cmplx(res_re[0][1], res_im[0][1]); amp[indices[1]] = cmplx(res_re[0][0], res_im[0][0]);
    amp[indices[2]] = cmplx(res_re[1][0], res_im[1][0]); amp[indices[3]] = cmplx(res_re[1][1], res_im[1][1]);
    amp[indices[4]] = cmplx(res_re[0][3], res_im[0][3]); amp[indices[5]] = cmplx(res_re[0][2], res_im[0][2]);
    amp[indices[6]] = cmplx(res_re[1][2], res_im[1][2]); amp[indices[7]] = cmplx(res_re[1][3], res_im[1][3]);
    amp[indices[8]] = cmplx(res_re[2][1], res_im[2][1]); amp[indices[9]] = cmplx(res_re[2][0], res_im[2][0]);
    amp[indices[10]] = cmplx(res_re[3][0], res_im[3][0]); amp[indices[11]] = cmplx(res_re[3][1], res_im[3][1]);
    amp[indices[12]] = cmplx(res_re[2][3], res_im[2][3]); amp[indices[13]] = cmplx(res_re[2][2], res_im[2][2]);
    amp[indices[14]] = cmplx(res_re[3][2], res_im[3][2]); amp[indices[15]] = cmplx(res_re[3][3], res_im[3][3]);
}

__attribute__((always_inline)) inline void
ApplyYY12Gate(cmplx* __restrict amp,
              const array<idx_size, 4> indices)
{
    const cmplx a[4] = {amp[indices[0]], amp[indices [1]], amp[indices[2]], amp[indices[3]]};
    const auto t = ki * (a[0] + a[3]);
    const auto t1 = ki * (a[0] - a[3]);
    const auto t2 = ki * (a[1] + a[2]);
    const auto t3 = ki * (a[1] - a[2]);

    amp[indices[0]] = t - t2;
    amp[indices[1]] = t1 + t3;
    amp[indices[2]] = t1 - t3;
    amp[indices[3]] = t + t2;
}

__attribute__((always_inline)) inline void
Apply4Y12Gate(cmplx* __restrict amp,
              const array<idx_size, 16> indices)
{
    ApplyYY12Gate( amp, {indices[0], indices[4], indices[8], indices[12]});
    ApplyYY12Gate( amp, {indices[1], indices[5], indices[9], indices[13]});
    ApplyYY12Gate( amp, {indices[2], indices[6], indices[10], indices[14]});
    ApplyYY12Gate( amp, {indices[3], indices[7], indices[11], indices[15]});
    ApplyYY12Gate( amp, {indices[0], indices[1], indices[2], indices[3]});
    ApplyYY12Gate( amp, {indices[4], indices[5], indices[6], indices[7]});
    ApplyYY12Gate( amp, {indices[8], indices[9], indices[10], indices[11]});
    ApplyYY12Gate( amp, {indices[12], indices[13], indices[14], indices[15]});
}


template <typename function>
void
Apply4MergedXY12GatesHelper(cmplx* __restrict amp,
                      const idx_size amp_size,
                      const int* gate_qubits,
                      const int total_circuit_qubits,
                      const function& gate_func)
{
    idx_size gate_bitmask = 0, iter_count = 0;
    constexpr idx_size num_bits = 4;
    for (idx_size i = 0; i < num_bits; ++i)
        gate_bitmask |= (1ull << ((total_circuit_qubits - 1) - gate_qubits[i]));
    
    constexpr idx_size num_indices = 16;
    array<idx_size, num_indices> indices;
    ExtractIndicesForAmp(indices.data(), gate_qubits, num_bits ,total_circuit_qubits);
    array<idx_size, num_indices> temp_indices;
    
    idx_size idx = 0;
    while(iter_count < (amp_size/num_indices)) {
        if ((idx & gate_bitmask) == 0) {
            ++iter_count;
            
            for (idx_size i = 0; i < num_indices; ++i)
                temp_indices[i] = indices[i] + idx;
            
            gate_func(amp, temp_indices);
            
            ++idx;
        }
        else
            idx += (idx & gate_bitmask);
    }
}

void
Apply4MergedXY12Gates(vector<Gate>& cluster,
                cmplx* __restrict amp,
                const idx_size amp_size,
                const int total_circuit_qubits)
{
    int block_qubits[4];
    
    for (idx_size i = 0; i < 4; ++i)
        block_qubits[i] = cluster[i].qubits.back();
    
    Gate::Type g1t = (Gate::Type)cluster[0].ids.back();
    
    if(g1t == Gate::Type::X_1_2)
        Apply4MergedXY12GatesHelper(amp, amp_size, block_qubits, total_circuit_qubits, Apply4X12Gate);
    
    else if(g1t == Gate::Type::Y_1_2)
        Apply4MergedXY12GatesHelper(amp, amp_size, block_qubits, total_circuit_qubits, Apply4Y12Gate);
}

