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
#include "gates.h"

using namespace std;

using cmplx = complex<float>;
using idx_size = size_t;

constexpr float kH = 0.707106781;

using idx_size = size_t;

constexpr cmplx ki = {0,1};

static void
ApplyXX12Gate(const array<idx_size, 4> indices,
              cmplx* __restrict amp)
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

static void
ApplyXY12Gate(const array<idx_size, 4> indices,
              cmplx* __restrict amp)
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

static void
ApplyYY12Gate(const array<idx_size, 4> indices,
              cmplx* __restrict amp)
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

static void
ApplyYX12Gate(const array<idx_size, 4> indices,
              cmplx* __restrict amp)
{
    cmplx a[4] = { amp[indices[0]], amp[indices [1]], amp[indices[2]], amp[indices[3]]};
    auto t = cmplx(-imag(a[0]), real(a[0])) + a[1];
    auto t1 = a[0] + cmplx(-imag(a[1]), real(a[1]));
    auto t2 = cmplx(-imag(a[2]), real(a[2])) + a[3];
    auto t3 = a[2] + cmplx(-imag(a[3]), real(a[3]));
    
    amp[indices[0]] = t - t2;
    amp[indices[1]] = t1 - t3;
    amp[indices[2]] = t + t2;
    amp[indices[3]] = t1 + t3;
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

template <typename function>
static void
Apply4YX12Gate(cmplx*  __restrict amp_slice,
               const function& gate_func1,
               const function& gate_func2)
{
    
    array<idx_size, 4> t ({0, 4, 8, 12});
    array<idx_size, 4> t1 ({0, 1, 2, 3});
    
    gate_func1(t, amp_slice);
    t[0] = 1;  t[1] = 5; t[2] = 9; t[3] = 13;
    gate_func1(t, amp_slice);
    t[0] = 2;  t[1] = 6; t[2] = 10; t[3] = 14;
    gate_func1(t, amp_slice);
    t[0] = 3;  t[1] = 7; t[2] = 11; t[3] = 15;
    gate_func1(t, amp_slice);
    
    gate_func2(t1, amp_slice);
    t1[0] = 4;  t1[1] = 5; t1[2] = 6; t1[3] = 7;
    gate_func2(t1, amp_slice);
    t1[0] = 8;  t1[1] = 9; t1[2] = 10; t1[3] = 11;
    gate_func2(t1, amp_slice);
    t1[0] = 12;  t1[1] = 13; t1[2] = 14; t1[3] = 15;
    gate_func2(t1, amp_slice);
    
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
            
            gate_func(temp_indices, amp);
            
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
                            const function& gate_func1,
                            const function& gate_func2)
{
    idx_size gate_bitmask = 0, iter_count = 0;
    constexpr idx_size num_bits = 4;
    for (idx_size i = 0; i < num_bits; ++i)
        gate_bitmask |= (1ull << ((total_circuit_qubits - 1) - gate_qubits[i]));
    
    constexpr idx_size num_indices = 16;
    array<idx_size, num_indices> indices;
    ExtractIndicesForAmp(indices.data(), gate_qubits, num_bits ,total_circuit_qubits);
    array<cmplx, num_indices> amp_slice;
    
    idx_size idx = 0;
    while(iter_count < (amp_size/num_indices)) {
        if ((idx & gate_bitmask) == 0) {
            ++iter_count;
            
            for (idx_size i = 0; i < num_indices; ++i)
                amp_slice[i] = amp[indices[i] + idx];
            
            Apply4YX12Gate(amp_slice.data(), gate_func1, gate_func2);
            
            for (idx_size i = 0; i < num_indices; ++i)
                amp[indices[i] + idx] = amp_slice[i];
            
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
    function<void (const array<idx_size, 4>& , cmplx*)> gate_app_funcs[2];
    
    int q = 0;
    for (idx_size i = 0; i < 4; i+=2) {
        
        block_qubits[i] = cluster[i].qubits.back();
        block_qubits[i + 1] = cluster[i + 1].qubits.back();
        
        Gate::Type g1t = (Gate::Type)cluster[i].ids.back();
        Gate::Type g2t = (Gate::Type)cluster[i + 1].ids.back();
        
        if(g1t == Gate::Type::X_1_2 && g2t == Gate::Type::X_1_2)
            gate_app_funcs[q++] = ApplyXX12Gate;
        
        else if(g1t == Gate::Type::X_1_2 && g2t == Gate::Type::Y_1_2)
            gate_app_funcs[q++] = ApplyXY12Gate;
        
        else if(g1t == Gate::Type::Y_1_2 && g2t == Gate::Type::Y_1_2)
            gate_app_funcs[q++] =  ApplyYY12Gate;
        
        
        else if(g1t == Gate::Type::Y_1_2 && g2t == Gate::Type::X_1_2)
            gate_app_funcs[q++] = ApplyYX12Gate;
        
    }
    
    Apply4MergedXY12GatesHelper(amp, amp_size, block_qubits, total_circuit_qubits, gate_app_funcs[0], gate_app_funcs[1]);
}

