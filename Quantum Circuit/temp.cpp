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
#include "gates.h"

using namespace std;

using cmplx = complex<float>;
using idx_size = size_t;

constexpr float kH = 0.707106781;

using idx_size = size_t;

constexpr cmplx ki = {0,1};
static const __m256 kM256CmplxNeg1 = _mm256_setr_ps(1.0, 1.0, 1.0, 1.0, 1.0, -1.0, 1.0, -1.0);
static const __m256 kM256CmplxNeg2 = _mm256_setr_ps(1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0);
static const __m256 kM256Neg = _mm256_setr_ps(1.0, 1.0, 1.0, 1.0, -1.0, -1.0, -1.0, -1.0);
static const __m128 kM128CmplxNeg = _mm_setr_ps(1.0, -1.0, 1.0, -1.0);


__attribute__((always_inline)) inline void
ApplyXX12Gate(cmplx* __restrict amp,
              const array<idx_size, 4> indices)
{
    const cmplx a[4] = {amp[indices[0]], amp[indices[1]], amp[indices[2]], amp[indices[3]]};
    const auto t = a[0] + a[3];
    const auto t1 = a[1] + a[2];
    const auto t2 = ki * (a[0] - a[3]);
    const auto t3 = ki * (a[1] - a[2]);

    amp[indices[0]] = t1 + t2;
    amp[indices[1]] = t + t3;
    amp[indices[2]] = t - t3;
    amp[indices[3]] = t1 - t2;
}

__attribute__((always_inline)) inline void
ApplyXY12Gate(cmplx* __restrict amp,
              const array<idx_size, 4> indices)
{
    const cmplx a[4] = { amp[indices[0]], amp[indices [1]], amp[indices[2]], amp[indices[3]]};
    const auto t = a[0] - a[1];
    const auto t1 = a[2] - a[3];
    const auto t2 = a[0] + a[1];
    const auto t3 = a[2] + a[3];

    amp[indices[0]] = (ki * t) + t1;
    amp[indices[1]] = (ki * t2) + t3;
    amp[indices[2]] = t + (ki * t1);
    amp[indices[3]] = t2 + (ki * t3);
}

__attribute__((always_inline)) inline void
ApplyYY12Gate(cmplx* __restrict amp,
              const array<idx_size, 4> indices)
{
    cmplx a[4] = { amp[indices[0]], amp[indices [1]], amp[indices[2]], amp[indices[3]]};
    auto t = ki * (a[0] + a[3]);
    auto t1 = ki * (a[0] - a[3]);
    auto t2 = ki * (a[1] + a[2]);
    auto t3 = ki * (a[1] - a[2]);

    amp[indices[0]] = t - t2;
    amp[indices[1]] = t1 + t3;
    amp[indices[2]] = t1 - t3;
    amp[indices[3]] = t + t2;
}

__attribute__((always_inline)) inline void
ApplyYX12Gate(cmplx* __restrict amp,
              const array<idx_size, 4> indices)
{
    cmplx a[4] = { amp[indices[0]], amp[indices [1]], amp[indices[2]], amp[indices[3]]};
    auto t = (ki * a[0]) + a[1];
    auto t1 = a[0] + (ki * a[1]);
    auto t2 = (ki * a[2]) + a[3];
    auto t3 = a[2] + (ki * a[3]);
    
    amp[indices[0]] = t - t2;
    amp[indices[1]] = t1 - t3;
    amp[indices[2]] = t + t2;
    amp[indices[3]] = t1 + t3;
}

inline void
ApplyXX12GateAVX(cmplx* __restrict amp,
                 const array<idx_size, 4> indices)
{
//    cmplx a[4] = {amp[indices[0]], amp[indices[1]], amp[indices[2]], amp[indices[3]]};
//    __m128 a0 = _mm_setr_ps(real(a[0]), imag(a[0]), real(a[1]), imag(a[1]));
//    __m128 a1 = _mm_setr_ps(real(a[3]), imag(a[3]), real(a[2]), imag(a[2]));
//    __m128 t = _mm_add_ps(a0, a1);
//    __m128 t1 = _mm_sub_ps(a0, a1);
//    t1 = _mm_permute_ps(t1, 0x5);
//    t1 = _mm_mul_ps(t1, kM128CmplxNeg);
//    t = _mm_permute_ps(t, 0b10110001);
//
//    __m128 val1 = _mm_add_ps(t, t1);
//    __m128 val2 = _mm_sub_ps(t, t1);
//    float* t_amp1 = (float*)&val1;
//    float* t_amp2 = (float*)&val2;
//
//    amp[indices[0]] = cmplx(t_amp1[0], t_amp1[1]);
//    amp[indices[1]] = cmplx(t_amp1[2], t_amp1[3]);
//    amp[indices[2]] = cmplx(t_amp2[0], t_amp1[1]);
//    amp[indices[3]] = cmplx(t_amp2[2], t_amp1[3]);
    
        cmplx a[4] = {amp[indices[0]], amp[indices[1]], amp[indices[2]], amp[indices[3]]};
        __m256 a0 = _mm256_setr_ps(real(a[0]), imag(a[0]), real(a[1]), imag(a[1]),
                                    imag(a[0]), real(a[0]), imag(a[1]), real(a[1]));
        __m256 a1 = _mm256_setr_ps(real(a[3]), imag(a[3]), real(a[2]), imag(a[2]),
                                   -imag(a[3]), -real(a[3]), -imag(a[2]), -real(a[2]));
        __m256 t = _mm256_add_ps(a0, a1);
        t = _mm256_mul_ps(t, kM256CmplxNeg1);
        __m256 t1 = _mm256_permute2f128_ps(t, t, 1);
        // t = _mm256_fmadd_ps(kM256Neg, t, t1);

        float* t_amp = (float*)&t;

        amp[indices[0]] = cmplx(t_amp[0], t_amp[1]);
        amp[indices[1]] = cmplx(t_amp[2], t_amp[3]);
        amp[indices[2]] = cmplx(t_amp[4], t_amp[5]);
        amp[indices[3]] = cmplx(t_amp[6], t_amp[7]);
}



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

__attribute__((always_inline)) inline void
Apply4X12Gate(cmplx* __restrict amp,
              const array<idx_size, 16> indices)
{
    ApplyXX12GateAVX( amp, {indices[0], indices[4], indices[8], indices[12]});
    ApplyXX12GateAVX( amp, {indices[1], indices[5], indices[9], indices[13]});
    ApplyXX12GateAVX( amp, {indices[2], indices[6], indices[10], indices[14]});
    ApplyXX12GateAVX( amp, {indices[3], indices[7], indices[11], indices[15]});
    ApplyXX12GateAVX( amp, {indices[0], indices[1], indices[2], indices[3]});
    ApplyXX12GateAVX( amp, {indices[4], indices[5], indices[6], indices[7]});
    ApplyXX12GateAVX( amp, {indices[8], indices[9], indices[10], indices[11]});
    ApplyXX12GateAVX( amp, {indices[12], indices[13], indices[14], indices[15]});
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


template<typename function>
void
Apply2MergedXY12GatesHelper(cmplx* __restrict amp,
                            const int* gate_qubits,
                            const int total_circuit_qubits,
                            const idx_size size,
                            const function& gate_func)
{
    idx_size gate_bitmask = 0, iter_count = 0;
    constexpr idx_size num_bits = 2;
    for (idx_size i = 0; i < num_bits; ++i)
        gate_bitmask |= (1ull << ((total_circuit_qubits - 1) - gate_qubits[i]));
    
    constexpr idx_size num_indices = 4;
    array<idx_size, num_indices> indices;
    ExtractIndicesForAmp(indices.data(), gate_qubits, num_bits ,total_circuit_qubits);
    array<idx_size, num_indices> temp_indices;
    
    idx_size idx = 0;
    while(iter_count < (size/num_indices)) {
        if ((idx & gate_bitmask) == 0) {
            ++iter_count;
            
            for (idx_size i = 0; i < num_indices; ++i)
                temp_indices[i] = indices[i] + idx;
            
            gate_func(amp , temp_indices);
            
            ++idx;
        }
        else
            idx += (idx & gate_bitmask);
    }
}

void
Apply2MergedXY12Gates(Gate& gate1,
                      Gate& gate2,
                      cmplx* __restrict amp,
                      const idx_size amp_size,
                      const int total_circuit_qubits)
{
    const int qubits[2] = {gate1.qubits.back(), gate2.qubits.back()};
    
    const Gate::Type g1t = (Gate::Type)gate1.ids.back();
    const Gate::Type g2t = (Gate::Type)gate2.ids.back();
    
    if(g1t == Gate::Type::X_1_2 && g2t == Gate::Type::X_1_2)
        Apply2MergedXY12GatesHelper(amp, qubits, total_circuit_qubits, amp_size, ApplyXX12Gate);
    
    else if(g1t == Gate::Type::X_1_2 && g2t == Gate::Type::Y_1_2)
        Apply2MergedXY12GatesHelper(amp, qubits, total_circuit_qubits, amp_size, ApplyXY12Gate);
    
    else if(g1t == Gate::Type::Y_1_2 && g2t == Gate::Type::Y_1_2)
        Apply2MergedXY12GatesHelper(amp, qubits, total_circuit_qubits, amp_size, ApplyYY12Gate);
    
    else if(g1t == Gate::Type::Y_1_2 && g2t == Gate::Type::X_1_2)
        Apply2MergedXY12GatesHelper(amp, qubits, total_circuit_qubits, amp_size, ApplyYX12Gate);
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

