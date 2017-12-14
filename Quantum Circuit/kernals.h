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

inline void
GroupCZGates(valarray<idx_size>& qubits_CZ_bitmask,
             const int qubits,
             const vector<int>& gate_qubits)
{
    idx_size bits = 0;
    int new_q = qubits - 1;
    
    for (auto q : gate_qubits) {
        bits |= ( 1ull << (new_q - q));
        qubits_CZ_bitmask[q] |= ( 1ull << (new_q - q));
    }
    for (auto q : gate_qubits)
        qubits_CZ_bitmask[q] ^= bits;
}

inline void
GroupTGates(valarray<idx_size>& T_bitmask,
            const int qubits,
            const vector<int>& gate_qubits)
{
    //Better way to do this? What if more than 2 T_gates incident on a qubit within a cycle.
    idx_size t_mask = (1ull << ((qubits - 1) - gate_qubits[0]));
    if ((T_bitmask[0] & t_mask) != t_mask)
        T_bitmask[0] |= t_mask;
    
    else {
        bool found = false;
        for (auto& t : T_bitmask)
            if ((t & t_mask) != t_mask) {
                t |= t_mask;
                found = true;
            }
        if (!found) {
            T_bitmask[1] = t_mask;
        }
    }
}

inline valarray<idx_size>
ExtractAmpForQGate(const vector<int>& gate_qubits,
                   const int qubits,
                   const int q_idx)
{
    const idx_size num_q = gate_qubits.size();
    idx_size temp_index = 0;
    idx_size strides_count = 1;
    
    valarray<idx_size> strides(1ull << num_q);
    idx_size pos = 1ull << (num_q - 1);
    
    strides[0] = q_idx;
    for (idx_size i = q_idx ; i < num_q; ++i) {
        for (idx_size n = 0; n < strides_count &&
             strides_count < (1ull << (i+1)); n += 2) {
            
            temp_index = strides[n] + (1ull << ((qubits - 1) - gate_qubits[i]));
            
            strides[n + pos] = temp_index;
            ++strides_count;
        }
        pos /= 2;
    }
    return strides;
}

inline void
ApplyXX12Gate(const valarray<idx_size>& indices,
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
ApplyXY12Gate(const valarray<idx_size>& indices,
              valarray<cmplx>& amp)
{
    valarray<cmplx> temp_amp(indices.size());
    for (idx_size i = 0; i < indices.size(); ++i)
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
ApplyYY12Gate(const valarray<idx_size>& indices,
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
ApplyYX12Gate(const valarray<idx_size>& indices,
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

inline valarray<idx_size>
FindStrides (const vector<int>& gate_qubits,
             const int num_q) {
    
    valarray<idx_size> strides(gate_qubits.size());
    for (idx_size i = 0; i < strides.size(); ++i)
        strides[i] = (1ull << ((num_q - 1) - gate_qubits[i]));
    
    return strides;
}

inline void
ApplySingleTGate(const gate& gate,
                 const int qubits,
                 valarray<cmplx>& amp)
{
    idx_size gap = (1ull << ((qubits - 1) - gate.qubits[0]));
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
FormBlockOfCZTGates(const vector<gate>& block_gates,
                 const int qubits,
                 idx_size& gate_i,
                 valarray<idx_size>& CZ_bitmasks,
                 valarray<idx_size>& T_bitmasks)
{

    for(;(gate_i < block_gates.size())
        && (block_gates[gate_i].ids.back() == gate::Gates::Z ||
            block_gates[gate_i].ids.back() == gate::Gates::T); ++gate_i) {
        
        if (block_gates[gate_i].ids.back() == gate::Gates::Z)
            GroupCZGates(CZ_bitmasks, qubits, block_gates[gate_i].qubits);
        else if (block_gates[gate_i].ids.back() == gate::Gates::T)
            GroupTGates(T_bitmasks, qubits, block_gates[gate_i].qubits);
    }
}

extern void
ApplyBlockOfGates(const int qubits,
                  const valarray<idx_size>& CZ_bitmask,
                  const valarray<idx_size>& T_bitmask,
                  valarray<cmplx>& amp,
                  cmplx rescaling_factor = 0);

extern void
ApplyGateOnAmps(const idx_size* indices,
                const idx_size size,
                const gate::Gates gate_type,
                const gate& q_gate,
                valarray<cmplx>& amp);

extern void
ApplyControlGate(const int num_controls,
              const vector<int>& gate_qubits,
              valarray<cmplx>& amp,
              const int qubits,
              const gate& g,
              const gate::Gates gate_type);

extern void
ApplyNonControl1QGates(const vector<int>& gate_qubits,
                     valarray<cmplx>& amp,
                     const int qubits,
                     const gate& g,
                     const gate::Gates gate_type);


template<typename function>
extern void
ApplyMergedXY12Gates(const vector<int>& gate_qubits,
                     const int qubits,
                     valarray<cmplx>& amp,
                     function& gate_func);

extern void
Merge2QXY12Gates(vector<gate>& gates,
                 idx_size gate1,
                 idx_size gate2,
                 const int qubits,
                 valarray<cmplx>& amp);

#endif /* kernals_h */
