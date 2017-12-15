//
//  amp_mutators.hpp
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 12/11/17.
//  Copyright © 2017 Aneeqa Fatima. All rights reserved.
//

#ifndef kernals_h
#define kernals_h

#include <stdio.h>
#include <stdlib.h>
#include <iomanip>
#include <unistd.h>
#include <memory>
#include <iostream>
#include <vector>
#include <complex>
#include <iterator>
#include <utility>
#include <ctime>
#include <cassert>
#include <algorithm>
#include <functional>
#include <valarray>
#include "matrix.h"
#include "gates.h"

using namespace std;

// 0:H, 1:CZ & T, 2:X, 3:Y, 4:Merged X & Y

constexpr float kH = 0.707106781;

using idx_size = size_t;

constexpr cmplx kTGate[8] = {{1,0}, {kH, kH}, {0,1}, {-kH, kH},
    {-1,0}, {-kH, -kH}, {0, -1}, {kH, -kH}};

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
                     const int qubits,
                     const int q_idx)
{
    const idx_size num_q = gate_qubits.size();
    idx_size strides_size = 1, pos = 1ull << (num_q - 1);
    
    strides[0] = q_idx;
    for (idx_size i = q_idx ; i < num_q; ++i) {
        for (idx_size n = 0; n < strides_size &&
             strides_size < (1ull << (i+1)); n += 2) {
            
            strides[n + pos] = strides[n] + (1ull << ((qubits - 1) - gate_qubits[i]));
            ++strides_size;
        }
        pos /= 2;
    }
}

inline void
ApplyXX12Gate(const idx_size* indices,
              valarray<cmplx>& amp)
{
    auto t = amp[indices[0]] + amp[indices[3]];
    auto t1 = amp[indices[1]] + amp[indices[2]];
    auto t2 = cmplx(-imag(amp[indices[0]]), real(amp[indices[0]]))
            - cmplx(-imag(amp[indices[3]]), real(amp[indices[3]]));
    auto t3 = cmplx(-imag(amp[indices[1]]), real(amp[indices[1]]))
            - cmplx(-imag(amp[indices[2]]), real(amp[indices[2]]));
    
    amp[indices[0]] = t1 + t2;
    amp[indices[1]] = t + t3;
    amp[indices[2]] = t - t3;
    amp[indices[3]] = t1 - t2 ;
}

inline void
ApplyXY12Gate(const idx_size* indices,
              valarray<cmplx>& amp)
{
    cmplx temp_amp[4];
    for (idx_size i = 0; i < 4; ++i)
        temp_amp[i] = amp[indices[i]];
    
    amp[indices[0]] = cmplx(-imag(temp_amp[0]), real(temp_amp[0]))
                    - cmplx(-imag(temp_amp[1]), real(temp_amp[1]))
                    + temp_amp[2] - temp_amp[3] ;
    
    amp[indices[1]] = cmplx(-imag(temp_amp[0]), real(temp_amp[0]))
                    + cmplx(-imag(temp_amp[1]), real(temp_amp[1]))
                    + temp_amp[2] + temp_amp[3] ;
    
    amp[indices[2]] = temp_amp[0] - temp_amp[1]
                    + cmplx(-imag(temp_amp[2]), real(temp_amp[2]))
                    - cmplx(-imag(temp_amp[3]), real(temp_amp[3])) ;
    
    amp[indices[3]] = temp_amp[0] + temp_amp[1]
                    + cmplx(-imag(temp_amp[2]), real(temp_amp[2]))
                    + cmplx(-imag(temp_amp[3]), real(temp_amp[3]));
    
}

inline void
ApplyYY12Gate(const idx_size* indices,
              valarray<cmplx>& amp)
{
    auto t = cmplx(-imag(amp[indices[0]]), real(amp[indices[0]]))
            + cmplx(-imag(amp[indices[3]]), real(amp[indices[3]]));
    auto t1 = cmplx(-imag(amp[indices[0]]), real(amp[indices[0]]))
            - cmplx(-imag(amp[indices[3]]), real(amp[indices[3]]));
    auto t2 = cmplx(-imag(amp[indices[1]]), real(amp[indices[1]]))
            + cmplx(-imag(amp[indices[2]]), real(amp[indices[2]]));
    auto t3 = cmplx(-imag(amp[indices[1]]), real(amp[indices[1]]))
            - cmplx(-imag(amp[indices[2]]), real(amp[indices[2]]));
    
    amp[indices[0]] = t - t2;
    amp[indices[1]] = t1 + t3;
    amp[indices[2]] = t1 - t3;
    amp[indices[3]] = t + t2;
}

inline void
ApplyYX12Gate(const idx_size* indices,
              valarray<cmplx>& amp)
{
    auto t = cmplx(-imag(amp[indices[0]]), real(amp[indices[0]]))
    + amp[indices[1]];
    auto t1 = amp[indices[0]]
    + cmplx(-imag(amp[indices[1]]), real(amp[indices[1]]));
    auto t2 = cmplx(-imag(amp[indices[2]]), real(amp[indices[2]]))
    + amp[indices[3]];
    auto t3 = amp[indices[2]]
    + cmplx(-imag(amp[indices[3]]), real(amp[indices[3]]));
    
    amp[indices[0]] = t - t2;
    amp[indices[1]] = t1 - t3;
    amp[indices[2]] = t + t2;
    amp[indices[3]] = t1 + t3;
}

inline void
FindStrides (valarray<idx_size>& strides,
             const vector<int>& gate_qubits,
             const int num_q) {
    
    strides.resize(gate_qubits.size());
    for (idx_size i = 0; i < strides.size(); ++i)
        strides[i] = (1ull << ((num_q - 1) - gate_qubits[i]));
}

inline void
ApplySingleTGate(const Gate& Gate,
                 const int qubits,
                 valarray<cmplx>& amp)
{
    idx_size gap = (1ull << ((qubits - 1) - Gate.qubits[0]));
    idx_size idx = gap, amp_size = amp.size();
    
    for (; idx < amp_size; ++idx) {
        if ((idx & gap) != gap) {
            idx += gap - 1;
            continue;
        }
        amp[idx] *= kTGate[1];
    }
}

inline void
FormBlockOfCZTGates(const vector<Gate>& block_gates,
                 const int qubits,
                 idx_size& gate_i,
                 valarray<idx_size>& CZ_bitmasks,
                 valarray<idx_size>& T_bitmasks)
{
    for(;(gate_i < block_gates.size())
        && (block_gates[gate_i].ids.back() == Gate::type::Z ||
            block_gates[gate_i].ids.back() == Gate::type::T); ++gate_i) {
        
        if (block_gates[gate_i].ids.back() == Gate::type::Z)
            GroupCZGates(CZ_bitmasks, qubits, block_gates[gate_i].qubits);
        else if (block_gates[gate_i].ids.back() == Gate::type::T)
            GroupTGates(T_bitmasks, qubits, block_gates[gate_i].qubits);
    }
}

inline void
ApplyGateOnAmps(const idx_size* indices,
                const idx_size size,
                const Gate::type gate_type,
                const Gate& q_gate,
                valarray<cmplx>& amp)
{
    cmplx temp_amp[size];
    for (idx_size i = 0; i < size; ++i)
        temp_amp[i] = amp[indices[i]];
    
    if (size >= 2 && !(amp[indices[0]] == cmplx(0,0) && amp[indices[1]] == cmplx(0,0))) {
        switch (gate_type) {
            case (Gate::type::X_1_2):{
                amp[indices[0]] = (temp_amp[0]*X12[0][0]) + (temp_amp[1]*X12[0][1]);
                amp[indices[1]] = (temp_amp[0]*X12[1][0]) + (temp_amp[1]*X12[1][1]);
                break;
            }
            case (Gate::type::Y_1_2):{
                amp[indices[0]] = (temp_amp[0]*Y12[0][0]) + (temp_amp[1]*Y12[0][1]);
                amp[indices[1]] = (temp_amp[0]*Y12[1][0]) + (temp_amp[1]*Y12[1][1]);
                break;
            }
            case (Gate::type::Z):{
                amp[indices[1]] *= -1;
                break;
            }
            case (Gate::type::T):{
                amp[indices[1]] *= T[1][1];
                break;
            }
            case (Gate::type::X):{
                swap(amp[indices[0]], amp[indices[1]]);
                break;
            }
            case (Gate::type::Y):{
                amp[indices[0]] = temp_amp[1] * Y[0][1];
                amp[indices[1]] = temp_amp[0] * Y[1][0];
                break;
            }
            default: {
                vector<cmplx> m_temp(temp_amp, temp_amp + sizeof(temp_amp) / sizeof(temp_amp[0]));
                m_temp = matrix_v_mult(q_gate.rows, m_temp);
                for (idx_size i = 0; i < size; ++i)
                    amp[i] = temp_amp[i];
                break;
            }
        }
    }
}

extern void
ApplyBlockOfGates(const int qubits,
                  const valarray<idx_size>& CZ_bitmask,
                  const valarray<idx_size>& T_bitmask,
                  valarray<cmplx>& amp,
                  cmplx rescaling_factor = 0);

extern void
ApplyControlGate(const int num_controls,
              const vector<int>& gate_qubits,
              valarray<cmplx>& amp,
              const int qubits,
              const Gate& g,
              const Gate::type gate_type);

extern void
ApplyNonControl1QGates(const vector<int>& gate_qubits,
                     valarray<cmplx>& amp,
                     const int qubits,
                     const Gate& g,
                     const Gate::type gate_type);


extern void
ApplyMergedXY12Gates(const vector<int>& gate_qubits,
                     const int qubits,
                     valarray<cmplx>& amp,
                     const int gate_order);

extern void
Merge2QXY12Gates(Gate& gate1,
                 Gate& gate2,
                 const int qubits,
                 valarray<cmplx>& amp);

#endif /* kernals_h */
