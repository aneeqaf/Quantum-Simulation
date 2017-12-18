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
FormBlockOfCZTGates(const vector<Gate>& block_gates,
                    const int qubits,
                    idx_size& gate_i,
                    valarray<idx_size>& CZ_bitmasks,
                    valarray<idx_size>& T_bitmasks)
{
    for(;gate_i < block_gates.size(); ++gate_i) {
        const auto gt = block_gates[gate_i].ids.back();
        
        if (gt == Gate::Type::Z)
            GroupCZGates(CZ_bitmasks, qubits, block_gates[gate_i].qubits);
        else if (gt == Gate::Type::T)
            GroupTGates(T_bitmasks, qubits, block_gates[gate_i].qubits);
        else break;
    }
}

void
ApplyBlockOfGates(const int qubits,
                  const valarray<idx_size>& CZ_bitmasks,
                  const valarray<idx_size>& T_bitmasks,
                  cmplx* __restrict amp,
                  const idx_size size,
                  cmplx rescaling_factor)
{
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

//TODO: Test this
void
ApplyControlGate(const int num_controls,
                 const vector<int>& gate_qubits,
                 cmplx* __restrict amp,
                 const int qubits,
                 const Gate& g,
                 const Gate::Type gate_type)
{
    int loop_count = qubits - num_controls;
    idx_size idx = 0, gate_bitmask = 0, c_bits = 0, iter_count = 0;
    
    for (int j = 0 ; j < num_controls; ++j)
        c_bits |= (1 << ((qubits - 1) - (gate_qubits[j])));
    
    for (idx_size i = 0; i < gate_qubits.size(); ++i)
        gate_bitmask |= (1ull << ((qubits - 1) - gate_qubits[i]));
    
    idx_size num_indices = 1ull << loop_count;
    idx_size indices [num_indices];
    memset(indices, 0, num_indices * sizeof(idx_size));
    ExtractIndicesForAmp(indices, gate_qubits, qubits, num_controls);
    idx_size temp_indices[num_indices];
    memset(temp_indices, 0, num_indices * sizeof(idx_size));
    
    while(iter_count < (1ull << loop_count)) {
        if ((idx & gate_bitmask) == 0) {
            if((idx & c_bits) == c_bits) {
                iter_count += 2;
                
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
ApplyNonControl1QGates(const int q,
                       cmplx* __restrict amp,
                       const idx_size size,
                       const int qubits,
                       const Gate& g,
                       const Gate::Type gate_type)
{
    idx_size iter_count = 0, idx = 0, gate_bitmask = 0;
    
    gate_bitmask |= (1ull << ((qubits - 1) - q));
    idx_size num_indices = 1ull << 1;
    idx_size indices [num_indices];
    memset(indices, 0, num_indices * sizeof(idx_size));
    indices[0] = 0;
    indices[1] = 1ull << ((qubits - 1) - q);
    idx_size temp_indices[num_indices];
    memset(temp_indices, 0, num_indices * sizeof(idx_size));
                         
    while(iter_count < (size/num_indices)) {
        if ((idx & gate_bitmask) == 0) {
            ++iter_count;
  
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
                     const int total_q,
                     cmplx* __restrict amp,
                     const idx_size size,
                     function& gate_func)
{
    idx_size gate_bitmask = 0, num_bits = gate_qubits.size();
    idx_size iter_count = 0;
    for (idx_size i = 0; i < num_bits; ++i)
        gate_bitmask |= (1ull << ((total_q - 1) - gate_qubits[i]));

    idx_size num_indices = 1ull << num_bits;
    idx_size indices [num_indices];
    memset(indices, 0, num_indices * sizeof(idx_size));
    ExtractIndicesForAmp(indices, gate_qubits, total_q);
    idx_size temp_indices[num_indices];
    memset(temp_indices, 0, num_indices * sizeof(idx_size));
    
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
Merge2QXY12Gates(Gate& gate1,
                 Gate& gate2,
                 const int qubits,
                 cmplx* __restrict amp,
                 const idx_size size)
{
    Gate gate_to_apply = gate1;
    if (gate1.qubits.back() < gate2.qubits.back())
        gate_to_apply.qubits.push_back(gate2.qubits.back());
    else {
        gate_to_apply = gate2;
        gate_to_apply.qubits.push_back(gate1.qubits.back());
        swap(gate1, gate2);
    }
    
    Gate::Type g1t = (Gate::Type)gate1.ids.back();
    Gate::Type g2t = (Gate::Type)gate2.ids.back();
    
    if(g1t == Gate::Type::X_1_2 && g2t == Gate::Type::X_1_2)
        ApplyMergedXY12Gates(gate_to_apply.qubits, qubits, amp, size, ApplyXX12Gate);
    
    else if(g1t == Gate::Type::X_1_2 && g2t == Gate::Type::Y_1_2)
        ApplyMergedXY12Gates(gate_to_apply.qubits, qubits, amp, size, ApplyXY12Gate);
    
    else if(g1t == Gate::Type::Y_1_2 && g2t == Gate::Type::X_1_2)
        ApplyMergedXY12Gates(gate_to_apply.qubits, qubits, amp, size, ApplyYX12Gate);
    
    else if(g1t == Gate::Type::Y_1_2 && g2t == Gate::Type::Y_1_2)
        ApplyMergedXY12Gates(gate_to_apply.qubits, qubits, amp, size, ApplyYY12Gate);
}

void
ApplyManyXOnSlice(const idx_size num_qbits,
                  cmplx* __restrict amp)
{
    for (idx_size i = 0; i < num_qbits ; ++i){
        const idx_size add = 1 << (i+1);
        const idx_size i_offset = 1 << i;
        for (idx_size j = 0; j < (idx_size)(1 << num_qbits); j += add){
            for (idx_size k = 0; k < (idx_size)(1<<i); ++k){
                const cmplx temp[2] = {amp[j + k], amp[j + k + i_offset]};
                amp[j + k] = (temp[0]*X12[0][0]) + (temp[1]*X12[0][1]);
                amp[j + k + i_offset] = (temp[0]*X12[1][0]) + (temp[1]*X12[1][1]);
            }
        }
    }
}

void
ApplyManyYOnSlice(const idx_size num_qbits,
                  cmplx* __restrict amp)
{
    for (idx_size i = 0; i < num_qbits ; ++i){
        const idx_size add = 1 << (i+1);
        const idx_size i_offset = 1<<i;
        for (idx_size j = 0; j < (idx_size)(1 << num_qbits); j += add){
            for (idx_size k = 0; k < (idx_size)(1<<i); ++k){
                const cmplx temp[2] = {amp[j + k], amp[j + k + i_offset]};
                const cmplx t = temp[0]*Y12[0][0];
                amp[j + k] = t + (temp[1]*Y12[0][1]);
                amp[j + k + i_offset] = t + (temp[1]*Y12[1][1]);
            }
        }
    }
}

void
ApplyFWHT(cmplx* __restrict amp,
          const idx_size a_size,
          const vector<int>& qubits_in_cluster,
          const int total_cir_q,
          const Gate::Type gate_type)
{
    const idx_size num_qubits = qubits_in_cluster.size(),
    slice_size = 1ull << num_qubits;
    
    idx_size gate_bitmask = 0;
    for (idx_size i = 0; i < num_qubits; ++i)
        gate_bitmask |= (1ull << ((total_cir_q - 1) - qubits_in_cluster[i]));
    
    idx_size indices [slice_size];
    memset(indices, 0, slice_size * sizeof(idx_size));
    ExtractIndicesForAmp(indices, qubits_in_cluster, total_cir_q);
    cmplx amp_slice[slice_size];
    memset(amp_slice, 0, slice_size * sizeof(cmplx));
    
    idx_size idx = 0, iter_count = 0;
    while(iter_count < (a_size/slice_size)) {
        if ((idx & gate_bitmask) == 0) {
            ++iter_count;
            
            for (idx_size i = 0; i < slice_size; ++i)
                amp_slice[i] = amp[indices[i] + idx];
            
            if (gate_type == Gate::Type::X_1_2)
                ApplyManyXOnSlice(num_qubits, amp_slice);
            else
               ApplyManyYOnSlice(num_qubits, amp_slice);
                   
            for (idx_size i = 0; i < slice_size; ++i)
                amp[indices[i] + idx] = amp_slice[i];
            
            ++idx;
        }
        else
            idx += (idx & gate_bitmask);
    }
}

