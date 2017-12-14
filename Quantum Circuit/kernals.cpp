//
//  kernals.cpp
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 12/14/17.
//  Copyright © 2017 Aneeqa Fatima. All rights reserved.
//

#include <stdio.h>
#include "kernals.h"


void
ApplyBlockOfGates(const int qubits,
                  const valarray<idx_size>& CZ_bitmask,
                  const valarray<idx_size>& T_bitmask,
                  valarray<cmplx>& amp,
                  cmplx rescaling_factor)
{
    const idx_size size =  amp.size();
    const idx_size modified_q = qubits - 1;
    idx_size prev_gc = 0;
    
    const bool rescale_now = rescaling_factor != cmplx(0,0);
    
    bool t_bit_mask_empty = true;
    if (T_bitmask[0] != 0)
        t_bit_mask_empty = false;
    
    bool negate_Z = false;
    for (idx_size count = 0; count < size ; ++count) {
        
        const idx_size gc = count ^ (count >> 1);
        const idx_size changed_bit = gc ^ prev_gc;
        const idx_size bit_idx = modified_q - __builtin_ctzl(changed_bit);
        
        cmplx mutated_amp = amp[gc];
        mutated_amp = rescale_now ? (mutated_amp * rescaling_factor) : mutated_amp;
        
        if (__builtin_parityl(CZ_bitmask[bit_idx] & gc) == 1)
            negate_Z = !negate_Z;
        if (negate_Z)
            mutated_amp = -mutated_amp;
        
        if (!t_bit_mask_empty) {
            idx_size gate_c = 0;
            
            for (auto t : T_bitmask)
                gate_c += __builtin_popcountll(gc & t);
            
            mutated_amp *= kTGate[gate_c % 8];
        }
        
        amp[gc] = mutated_amp;
        
        prev_gc = gc;
    }
}

void
ApplyGateOnAmps(const idx_size* indices,
                const idx_size size,
                const gate::Gates gate_type,
                const gate& q_gate,
                valarray<cmplx>& amp)
{
    vector<cmplx> temp_amp(size, 0) ;
    for (idx_size i = 0; i < size; ++i)
        temp_amp[i] = amp[indices[i]];
    //Add a size check;
    if (!(amp[indices[0]] == cmplx(0,0) && amp[indices[1]] == cmplx(0,0))) {
        if (gate_type == gate::Gates::X_1_2) {
            amp[indices[0]] = (temp_amp[0]*X12[0][0]) + (temp_amp[1]*X12[0][1]);
            amp[indices[1]] = (temp_amp[0]*X12[1][0]) + (temp_amp[1]*X12[1][1]);
        }
        else if (gate_type == gate::Gates::Y_1_2) {
            amp[indices[0]] = (temp_amp[0]*Y12[0][0]) + (temp_amp[1]*Y12[0][1]);
            amp[indices[1]] = (temp_amp[0]*Y12[1][0]) + (temp_amp[1]*Y12[1][1]);
        }
        else if (gate_type == gate::Gates::Z)
            amp[indices[1]] *= Z[1][1];
        
        else if (gate_type == gate::Gates::T) 
            amp[indices[1]] *= T[1][1];
        
        else if (gate_type == gate::Gates::X)
            swap(amp[indices[0]], amp[indices[1]]);
        
        else if (gate_type == gate::Gates::Y) {
            amp[indices[0]] = temp_amp[1] * Y[0][1];
            amp[indices[1]] = temp_amp[0] * Y[1][0];
        }
        else {
            temp_amp = matrix_v_mult(q_gate.rows, temp_amp);
            for (idx_size i = 0; i < size; ++i)
                amp[i] = temp_amp[i];
        }
    }
}

void
ApplyControlGate(const int num_controls,
                 const vector<int>& gate_qubits,
                 valarray<cmplx>& amp,
                 const int qubits,
                 const gate& g,
                 const gate::Gates gate_type)
{
    valarray<idx_size> strides = FindStrides(gate_qubits, qubits);
    valarray<idx_size> size_gslice(2, gate_qubits.size());
    
    idx_size iter_count = 0, idx = 0, c_bits = 0, gate_bitmask = 0;
    int loop_count = qubits - num_controls;
    
    for (int j = 0 ; j < num_controls; ++j)
        c_bits |= (1 << ((qubits - 1) - (gate_qubits[j])));
    
    for (idx_size i = 0; i < gate_qubits.size(); ++i)
        gate_bitmask |= (1ull << ((qubits - 1) - gate_qubits[i]));
    
    while(iter_count < (1ull << loop_count)) {
        if ((idx & gate_bitmask) == 0) {
            if((idx & c_bits) == c_bits) {
                iter_count += 2;
                
//                valarray<cmplx> amp_slice = amp[gslice(idx, size_gslice, strides)];
//                ApplyGateOnAmps(amp_slice, gate_type, g, amp);
//                amp[gslice(idx, size_gslice, strides)] = amp_slice;
                ++idx;
            }
            else
                idx += c_bits;
        }
        else
            idx += (idx & gate_bitmask);
    }
}

void
ApplyNonControl1QGates(const vector<int>& gate_qubits,
                       valarray<cmplx>& amp,
                       const int qubits,
                       const gate& g,
                       const gate::Gates gate_type)
{
    idx_size num_bits = gate_qubits.size(), iter_count = 0, size = amp.size();
    idx_size idx = 0, gate_bitmask = 0;
    
    gate_bitmask |= (1ull << ((qubits - 1) - gate_qubits[0]));
    idx_size indirect_array [1ull << num_bits];
    indirect_array[0] = 0;
    indirect_array[1] = 1ull << ((qubits - 1) - gate_qubits[0]);
    idx_size num_indices = 1ull << num_bits;
                         
    while(iter_count < (size/(1ull << num_bits))) {
        if ((idx & gate_bitmask) == 0) {
            ++iter_count;
  
            idx_size temp_indices[num_indices];
            for (idx_size i = 0; i < num_indices; ++i)
                temp_indices[i] = indirect_array[i] + idx;
            ApplyGateOnAmps(temp_indices, num_indices, gate_type, g, amp);
            
            ++idx;
        }
        else
            idx += (idx & gate_bitmask);
    }
}

template<typename function>
void
ApplyMergedXY12Gates(const vector<int>& gate_qubits,
                     const int qubits,
                     valarray<cmplx>& amp,
                     function& gate_func)
{
    idx_size gate_bitmask = 0, num_bits = gate_qubits.size();
    idx_size iter_count = 0, size = amp.size();
    for (idx_size i = 0; i < num_bits; ++i)
        gate_bitmask |= (1ull << ((qubits - 1) - gate_qubits[i]));

    valarray<idx_size> indirect_array = ExtractAmpForQGate(gate_qubits, qubits, 0);
    idx_size num_indices = indirect_array.size();
    
    idx_size idx = 0;
    while(iter_count < (size/(1 << num_bits))) {
        if ((idx & gate_bitmask) == 0) {
            ++iter_count;
        
            valarray<idx_size> temp_indices(num_indices);
            temp_indices = indirect_array + idx;
            gate_func(temp_indices, amp);
            
            ++idx;
        }
        else
            idx += (idx & gate_bitmask);
        
    }
}

void
Merge2QXY12Gates(vector<gate>& gates,
                 idx_size gate1,
                 idx_size gate2,
                 const int qubits,
                 valarray<cmplx>& amp)
{
    idx_size gate_to_apply = gate1;
    if (gates[gate1].qubits.back() < gates[gate2].qubits.back()) {
        gates[gate_to_apply].qubits.push_back(gates[gate2].qubits.back());
    }
    else {
        gate_to_apply = gate2;
        gates[gate_to_apply].qubits.push_back(gates[gate1].qubits.back());
        swap(gate1, gate2);
    }
    
    if(gates[gate1].ids.back() == gate::Gates::X_1_2 &&
       gates[gate2].ids.back() == gate::Gates::X_1_2)
        ApplyMergedXY12Gates(gates[gate_to_apply].qubits, qubits, amp, ApplyXX12Gate);
    
    else if(gates[gate1].ids.back() == gate::Gates::X_1_2 &&
            gates[gate2].ids.back() == gate::Gates::Y_1_2)
        ApplyMergedXY12Gates(gates[gate_to_apply].qubits, qubits, amp, ApplyXY12Gate);
    
    else if(gates[gate1].ids.back() == gate::Gates::Y_1_2 &&
            gates[gate2].ids.back() == gate::Gates::X_1_2)
        ApplyMergedXY12Gates(gates[gate_to_apply].qubits, qubits, amp, ApplyYX12Gate);
    
    else if(gates[gate1].ids.back() == gate::Gates::Y_1_2 &&
            gates[gate2].ids.back() == gate::Gates::Y_1_2)
        ApplyMergedXY12Gates(gates[gate_to_apply].qubits, qubits, amp, ApplyYY12Gate);
}
