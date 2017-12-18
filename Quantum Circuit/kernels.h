//
//  amp_mutators.hpp
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 12/11/17.
//  Copyright © 2017 Aneeqa Fatima. All rights reserved.
//

#ifndef kernals_h
#define kernals_h

#include <algorithm>
#include <cassert>
#include <complex>
#include <cstring>
#include <ctime>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <valarray>
#include <vector>
#include <unistd.h>
#include <utility>

#include "matrix.h"
#include "gates.h"

using namespace std;

// 0:H, 1:CZ & T, 2:X, 3:Y, 4:Merged X & Y

constexpr float kH = 0.707106781;

using idx_size = size_t;

constexpr cmplx kTGate[8] = {1, {kH, kH}, {0,1}, {-kH, kH},
    -1, {-kH, -kH}, {0, -1}, {kH, -kH}};

constexpr cmplx kSqrtCZGate[4] = {1, {0,1}, -1, {0,-1}};

extern void
GroupCZGates(valarray<idx_size>& qubits_CZ_bitmasks,
             const int qubits,
             const vector<int>& gate_qubits);

inline void
GroupTGates(valarray<idx_size>& T_bitmasks,
            const int qubits,
            const vector<int>& gate_qubits)
{
    //Better way to do this? What if more than 2 T_gates incident on a qubit within a cycle.
    idx_size t_mask = (1ull << ((qubits - 1) - gate_qubits[0]));
    if ((T_bitmasks[0] & t_mask) != t_mask)
        T_bitmasks[0] |= t_mask;
    
    else {
        bool found = false;
        for (auto& t : T_bitmasks)
            if ((t & t_mask) != t_mask) {
                t |= t_mask;
                found = true;
            }
        if (!found) {
            T_bitmasks[1] = t_mask;
        }
    }
}

inline void
ExtractIndicesForAmp(idx_size* strides,
                     const vector<int>& gate_qubits,
                     const int qubits)
{
    const idx_size num_q = gate_qubits.size();
    idx_size strides_size = 1, pos = 1ull << (num_q - 1);
    
    strides[0] = 0;
    idx_size prev_gap = pos;
    for (idx_size i = 0 ; i < num_q; ++i) {
        for (idx_size n = 0; strides_size < (1ull << (i+1)); n += prev_gap) {
            
            strides[n + pos] = strides[n] + (1ull << ((qubits - 1) - gate_qubits[i]));
            ++strides_size;
        }
        prev_gap = pos;
        pos /= 2;
    }
}

inline void
ApplyXX12Gate(const idx_size* indices,
              cmplx* __restrict amp)
{
    cmplx a[4] = {amp[indices[0]], amp[indices[1]], amp[indices[2]], amp[indices[3]]};
    auto t = a[0] + a[3];
    auto t1 = a[1] + a[2];
    auto t2 = (cmplx(0,1) * (a[0] - a[3]));
    auto t3 = (cmplx(0,1) * (a[1] - a[2]));
    
    amp[indices[0]] = t1 + t2;
    amp[indices[1]] = t + t3;
    amp[indices[2]] = t - t3;
    amp[indices[3]] = t1 - t2;
}

inline void
ApplyXY12Gate(const idx_size* indices,
              cmplx* __restrict amp)
{
    cmplx a[4] = { amp[indices[0]], amp[indices [1]], amp[indices[2]], amp[indices[3]]};
    
    amp[indices[0]] = (cmplx(0,1) * (a[0] - a[1])) + a[2] - a[3] ;
    amp[indices[1]] = (cmplx(0,1) * (a[0] + a[1])) + a[2] + a[3] ;
    amp[indices[2]] = a[0] - a[1] + (cmplx(0, 1) * (a[2] - a[3]));
    amp[indices[3]] = a[0] + a[1] + (cmplx(0, 1) * (a[2] + a[3]));
}

inline void
ApplyYY12Gate(const idx_size* indices,
              cmplx* __restrict amp)
{
    cmplx a[4] = { amp[indices[0]], amp[indices [1]], amp[indices[2]], amp[indices[3]]};
    auto t = (cmplx(0,1) * (a[0] + a[3]));
    auto t1 = (cmplx(0,1) * (a[0] - a[3]));
    auto t2 = (cmplx(0,1) * (a[1] + a[2]));
    auto t3 = (cmplx(0,1) * (a[1] - a[2]));
    
    amp[indices[0]] = t - t2;
    amp[indices[1]] = t1 + t3;
    amp[indices[2]] = t1 - t3;
    amp[indices[3]] = t + t2;
}

inline void
ApplyYX12Gate(const idx_size* indices,
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

inline valarray<idx_size>
FindStrides (const vector<int>& gate_qubits,
             const int num_q) {
    
    valarray<idx_size> strides(gate_qubits.size());
    for (idx_size i = 0; i < strides.size(); ++i)
        strides[i] = (1ull << ((num_q - 1) - gate_qubits[i]));
    
    return strides;
}

inline void
ApplyGateOnAmps(const idx_size* indices,
                const idx_size size,
                const Gate::Type gate_type,
                const Gate& q_gate,
                cmplx* __restrict amp)
{
    cmplx temp_amp[size];
    for (idx_size i = 0; i < size; ++i)
        temp_amp[i] = amp[indices[i]];
    
    if (size >= 2 && !(amp[indices[0]] == cmplx(0,0) && amp[indices[1]] == cmplx(0,0))) {
        switch (gate_type) {
            case (Gate::Type::X_1_2):{
                amp[indices[0]] = (temp_amp[0]*X12[0][0]) + (temp_amp[1]*X12[0][1]);
                amp[indices[1]] = (temp_amp[0]*X12[1][0]) + (temp_amp[1]*X12[1][1]);
                break;
            }
            case (Gate::Type::Y_1_2):{
                amp[indices[0]] = (temp_amp[0]*Y12[0][0]) + (temp_amp[1]*Y12[0][1]);
                amp[indices[1]] = (temp_amp[0]*Y12[1][0]) + (temp_amp[1]*Y12[1][1]);
                break;
            }
            case (Gate::Type::Z):{
                amp[indices[1]] *= -1;
                break;
            }
            case (Gate::Type::T):{
                amp[indices[1]] *= T[1][1];
                break;
            }
            case (Gate::Type::X):{
                swap(amp[indices[0]], amp[indices[1]]);
                break;
            }
            case (Gate::Type::Y):{
                amp[indices[0]] = temp_amp[1] * Y[0][1];
                amp[indices[1]] = temp_amp[0] * Y[1][0];
                break;
            }
            default: {
                vector<cmplx> m_temp(temp_amp, temp_amp + sizeof(temp_amp) / sizeof(temp_amp[0]));
                m_temp = matrix_v_mult(q_gate.rows, m_temp);
                for (idx_size i = 0; i < size; ++i)
                    amp[indices[i]] = temp_amp[i];
                break;
            }
        }
    }
    else {
        vector<cmplx> m_temp(temp_amp, temp_amp + sizeof(temp_amp) / sizeof(temp_amp[0]));
        m_temp = matrix_v_mult(q_gate.rows, m_temp);
        for (idx_size i = 0; i < size; ++i)
            amp[indices[i]] = temp_amp[i];
    }
}

inline vector<int>
FormBlockOfXYHGates(const vector<Gate>& block_gates,
                    const int qubits,
                    idx_size& gate_i,
                    Gate::Type gate_type)
{
    vector<int> qubits_in_cluster;
    for(;gate_i < block_gates.size() && qubits_in_cluster.size() < 12; ++gate_i) {
        const auto& gt = block_gates[gate_i];
        
        if(gt.ids.back() == gate_type)
            qubits_in_cluster.push_back(gt.qubits.back());
        else break;
    }
    return qubits_in_cluster;
}

void
FormBlockOfCZTGates(const vector<Gate>& block_gates,
                    const int qubits,
                    idx_size& gate_i,
                    valarray<idx_size>& CZ_bitmasks,
                    valarray<idx_size>& T_bitmasks);

void
ApplyBlockOfGates(const int qubits,
                  const valarray<idx_size>& CZ_bitmask,
                  const valarray<idx_size>& T_bitmask,
                  cmplx* __restrict amp,
                  const idx_size size,
                  cmplx rescaling_factor = 0);

void
ApplyControlGate(const int num_controls,
              const vector<int>& gate_qubits,
              cmplx* __restrict amp,
              const int qubits,
              const Gate& g,
              const Gate::Type gate_type);

void
ApplyNonControl1QGates(const int gate_qubit,
                     cmplx* __restrict amp,
                     const idx_size size,
                     const int qubits,
                     const Gate& g,
                     const Gate::Type gate_type);

void
ApplyManyXYHOnSlice(const idx_size num_qbits,
                    cmplx* __restrict amp,
                    Gate::Type gate_type);

void
ApplyMergedXY12Gates(const vector<int>& gate_qubits,
                     const int qubits,
                     cmplx* __restrict amp,
                     const idx_size size,
                     const int gate_order);

void
Merge2QXY12Gates(Gate& gate1,
                 Gate& gate2,
                 const int qubits,
                 cmplx* __restrict amp,
                 const idx_size size);

void
ApplyFWHT(cmplx* __restrict amp,
          const idx_size a_size,
          const vector<int>& qubits_in_cluster,
          const int total_cir_q,
          const Gate::Type gate_type);

#endif /* kernals_h */
