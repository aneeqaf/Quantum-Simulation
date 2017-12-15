//
//  kernals.cpp
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 12/14/17.
//  Copyright © 2017 Aneeqa Fatima. All rights reserved.
//

#include <stdio.h>
#include "kernels.h"

void
GroupCZGates(valarray<idx_size>& qubits_CZ_bitmasks,
             const int qubits,
             const vector<int>& gate_qubits)
{
    idx_size bits = 0;
    int new_q = qubits - 1;
    
    for (auto q : gate_qubits) {
        bits |= ( 1ull << (new_q - q));
        qubits_CZ_bitmasks[q] |= ( 1ull << (new_q - q));
    }
    for (auto q : gate_qubits)
        qubits_CZ_bitmasks[q] ^= bits;
}

void
ApplyBlockOfGates(const int qubits,
                  const valarray<idx_size>& CZ_bitmasks,
                  const valarray<idx_size>& T_bitmasks,
                  valarray<cmplx>& amp,
                  cmplx rescaling_factor)
{
    const idx_size size =  amp.size();
    const idx_size modified_q = qubits - 1;
    idx_size prev_gc = 0;
    
    const bool rescale_now = rescaling_factor != cmplx(0,0);
    
    bool t_bit_mask_empty = true;
    if (T_bitmasks[0] != 0)
        t_bit_mask_empty = false;
    
    bool negate_Z = false;
    for (idx_size count = 0; count < size ; ++count) {
        
        const idx_size gc = count ^ (count >> 1);
        const idx_size changed_bit = gc ^ prev_gc;
        const idx_size bit_idx = modified_q - __builtin_ctzl(changed_bit);
        
        cmplx mutated_amp = amp[gc];
        mutated_amp = rescale_now ? (mutated_amp * rescaling_factor) : mutated_amp;
        
        if (__builtin_parityl(CZ_bitmasks[bit_idx] & gc) == 1)
            negate_Z = !negate_Z;
        if (negate_Z)
            mutated_amp = -mutated_amp;
        
        if (!t_bit_mask_empty) {
            idx_size gate_c = 0;
            
            for (auto t : T_bitmasks)
                gate_c += __builtin_popcountll(gc & t);
            
            mutated_amp *= kTGate[gate_c % 8];
        }
        
        amp[gc] = mutated_amp;
        
        prev_gc = gc;
    }
}

//Test this
void
ApplyControlGate(const int num_controls,
                 const vector<int>& gate_qubits,
                 valarray<cmplx>& amp,
                 const int qubits,
                 const Gate& g,
                 const Gate::type gate_type)
{
    int loop_count = qubits - num_controls;
    idx_size idx = 0, gate_bitmask = 0, c_bits = 0, iter_count = 0;
    
    for (int j = 0 ; j < num_controls; ++j)
        c_bits |= (1 << ((qubits - 1) - (gate_qubits[j])));
    
    for (idx_size i = 0; i < gate_qubits.size(); ++i)
        gate_bitmask |= (1ull << ((qubits - 1) - gate_qubits[i]));
    
    idx_size indices [1ull << loop_count];
    ExtractIndicesForAmp(indices, gate_qubits, qubits, num_controls);
    idx_size num_indices = 1ull << loop_count;
    
    while(iter_count < (1ull << loop_count)) {
        if ((idx & gate_bitmask) == 0) {
            if((idx & c_bits) == c_bits) {
                iter_count += 2;
                
                idx_size temp_indices[num_indices];
                for (idx_size i = 0; i < num_indices; ++i)
                    temp_indices[i] = indices[i] + idx;
                ApplyGateOnAmps(temp_indices, num_indices, gate_type, g, amp);
                
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
                       const Gate& g,
                       const Gate::type gate_type)
{
    idx_size num_bits = gate_qubits.size(), iter_count = 0, size = amp.size();
    idx_size idx = 0, gate_bitmask = 0;
    
    gate_bitmask |= (1ull << ((qubits - 1) - gate_qubits[0]));
    idx_size indices [1ull << num_bits];
    indices[0] = 0;
    indices[1] = 1ull << ((qubits - 1) - gate_qubits[0]);
    idx_size num_indices = 1ull << num_bits;
                         
    while(iter_count < (size/(1ull << num_bits))) {
        if ((idx & gate_bitmask) == 0) {
            ++iter_count;
  
            idx_size temp_indices[num_indices];
            for (idx_size i = 0; i < num_indices; ++i)
                temp_indices[i] = indices[i] + idx;
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

    idx_size indices [1ull << num_bits];
    ExtractIndicesForAmp(indices, gate_qubits, qubits, 0);
    idx_size num_indices = 1ull << num_bits;
    
    idx_size idx = 0;
    while(iter_count < (size/(1ull << num_bits))) {
        if ((idx & gate_bitmask) == 0) {
            ++iter_count;
        
            idx_size temp_indices[num_indices];
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
Merge2QXY12Gates(Gate& gate1,
                 Gate& gate2,
                 const int qubits,
                 valarray<cmplx>& amp)
{
    Gate gate_to_apply = gate1;
    if (gate1.qubits.back() < gate2.qubits.back())
        gate_to_apply.qubits.push_back(gate2.qubits.back());
    else {
        gate_to_apply = gate2;
        gate_to_apply.qubits.push_back(gate1.qubits.back());
        swap(gate1, gate2);
    }
    
    if(gate1.ids.back() == Gate::type::X_1_2 &&
       gate2.ids.back() == Gate::type::X_1_2)
        ApplyMergedXY12Gates(gate_to_apply.qubits, qubits, amp, ApplyXX12Gate);
    
    else if(gate1.ids.back() == Gate::type::X_1_2 &&
            gate2.ids.back() == Gate::type::Y_1_2)
        ApplyMergedXY12Gates(gate_to_apply.qubits, qubits, amp, ApplyXY12Gate);
    
    else if(gate1.ids.back() == Gate::type::Y_1_2 &&
            gate2.ids.back() == Gate::type::X_1_2)
        ApplyMergedXY12Gates(gate_to_apply.qubits, qubits, amp, ApplyYX12Gate);
    
    else if(gate1.ids.back() == Gate::type::Y_1_2 &&
            gate2.ids.back() == Gate::type::Y_1_2)
        ApplyMergedXY12Gates(gate_to_apply.qubits, qubits, amp, ApplyYY12Gate);
}
