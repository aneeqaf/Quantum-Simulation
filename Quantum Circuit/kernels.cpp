//
//  kernals.cpp
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 12/14/17.
//

#include <stdio.h>
#include "kernels.h"

void
GroupCZGates(valarray<idx_size>& qubits_CZ_bitmasks,
             const int total_q_cir,
             const vector<int>& gate_qubits)
{
    idx_size bits = 0;
    int new_q = total_q_cir - 1;
    
    for (auto q : gate_qubits) {
        bits |= ( 1ull << (new_q - q));
        qubits_CZ_bitmasks[q] |= ( 1ull << (new_q - q));
    }
    for (auto q : gate_qubits)
        qubits_CZ_bitmasks[q] ^= bits;
}

void
GroupTGates(valarray<idx_size>& T_bitmasks,
            const int total_q_cir,
            const vector<int>& gate_qubits)
{
    //Better way to do this? What if more than 2 T_gates incident on a qubit within a cycle.
    idx_size t_mask = (1ull << ((total_q_cir - 1) - gate_qubits[0]));
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

void
ExtractIndicesForAmp(idx_size* strides,
                     const vector<int>& gate_qubits,
                     const int total_q_cir,
                     const idx_size starting_idx)
{
    const idx_size num_q = gate_qubits.size();
    idx_size strides_size = 1, gap = 1ull << (num_q - 1);
    
    strides[0] = starting_idx;
    idx_size prev_gap = gap;
    for (idx_size i = starting_idx ; i < num_q; ++i) {
        for (idx_size n = 0; strides_size < (1ull << (i+1)); n += prev_gap) {
            
            strides[n + gap] = strides[n] + (1ull << ((total_q_cir - 1) - gate_qubits[i]));
            ++strides_size;
        }
        prev_gap = gap;
        gap /= 2;
    }
}

void
FormBlockOfCZTGates(const vector<Gate>& block_gates,
                    const int total_q_cir,
                    idx_size& gate_i,
                    valarray<idx_size>& CZ_bitmasks,
                    valarray<idx_size>& T_bitmasks)
{
    for(;gate_i < block_gates.size(); ++gate_i) {
        const auto gt = block_gates[gate_i].ids.back();
        
        if (gt == Gate::Type::Z)
            GroupCZGates(CZ_bitmasks, total_q_cir, block_gates[gate_i].qubits);
        else if (gt == Gate::Type::T)
            GroupTGates(T_bitmasks, total_q_cir, block_gates[gate_i].qubits);
        else break;
    }
}

vector<int>
FormBlockOfXYHGates(const vector<Gate>& block_gates,
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
FormBlockOfXYHGates(const vector<Gate>& all_gates,
                    vector<Gate>& block_gates,
                    idx_size& gate_i)
{
    for(;gate_i < all_gates.size() && block_gates.size() < 4; ++gate_i) {
        const auto& gt = all_gates[gate_i];
        
        if(gt.ids.back() == Gate::Type::X_1_2 ||  gt.ids.back() == Gate::Type::Y_1_2)
            block_gates.push_back(all_gates[gate_i]);
        else break;
    }
}

void
ApplyBlockOfCZTGates(const int total_q_cir,
                  const valarray<idx_size>& CZ_bitmasks,
                  const valarray<idx_size>& T_bitmasks,
                  cmplx* __restrict amp,
                  const idx_size amp_size,
                  cmplx rescaling_factor)
{
    const idx_size modified_q = total_q_cir - 1;
    idx_size prev_gc = 0;
    
    const bool rescale_now = rescaling_factor != cmplx(0,0);
    
    bool t_bit_mask_empty = true;
    if (T_bitmasks[0] != 0)
        t_bit_mask_empty = false;
    
    bool negate_Z = false;
    for (idx_size count = 0; count < amp_size ; ++count) {
        
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
                 const int total_q_cir,
                 const Gate& g,
                 const Gate::Type gate_type)
{
    int loop_count = total_q_cir - num_controls;
    idx_size modified_q = total_q_cir -1, idx = 0, gate_bitmask = 0,
    c_bits = 0, iter_count = 0;
    
    for (int j = 0 ; j < num_controls; ++j)
        c_bits |= (1 << (modified_q - (gate_qubits[j])));
    
    for (idx_size i = 0; i < gate_qubits.size(); ++i)
        gate_bitmask |= (1ull << (modified_q - gate_qubits[i]));
    
    idx_size num_indices = 1ull << loop_count;
    idx_size indices [num_indices];
    memset(indices, 0, num_indices * sizeof(idx_size));
    ExtractIndicesForAmp(indices, gate_qubits, total_q_cir, num_controls);
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
                       const idx_size amp_size,
                       const int total_q_cir,
                       const Gate& g,
                       const Gate::Type gate_type)
{
    idx_size iter_count = 0, idx = 0, gate_bitmask = 0;
    
    gate_bitmask |= (1ull << ((total_q_cir - 1) - q));
    idx_size num_indices = 1ull << 1;
    idx_size indices [num_indices];
    memset(indices, 0, num_indices * sizeof(idx_size));
    indices[0] = 0;
    indices[1] = 1ull << ((total_q_cir - 1) - q);
    idx_size temp_indices[num_indices];
    memset(temp_indices, 0, num_indices * sizeof(idx_size));
                         
    while(iter_count < (amp_size/num_indices)) {
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

template <typename function>
void
Apply4MergedXY12Gates(const vector<int>& gate_qubits,
                     const int total_q_cir,
                     cmplx* __restrict amp,
                     const idx_size amp_size,
                     const function* gate_funcs)
{
    idx_size gate_bitmask = 0, num_bits = gate_qubits.size();
    idx_size iter_count = 0;
    for (idx_size i = 0; i < num_bits; ++i)
        gate_bitmask |= (1ull << ((total_q_cir - 1) - gate_qubits[i]));
    
    idx_size num_indices = 1ull << num_bits;
    idx_size indices [num_indices];
    memset(indices, 0, num_indices * sizeof(idx_size));
    ExtractIndicesForAmp(indices, gate_qubits, total_q_cir);
    cmplx amp_slice[num_indices];
    memset(amp_slice, 0, num_indices * sizeof(cmplx));
    
    idx_size idx = 0;
    while(iter_count < (amp_size/num_indices)) {
        if ((idx & gate_bitmask) == 0) {
            ++iter_count;
            
            for (idx_size i = 0; i < num_indices; ++i)
                amp_slice[i] = amp[indices[i] + idx];
            
            Apply4YX12Gate(amp_slice, gate_funcs);
            
            for (idx_size i = 0; i < num_indices; ++i)
                amp[indices[i] + idx] = amp_slice[i];
            
            ++idx;
        }
        else
            idx += (idx & gate_bitmask);
    }
}

template<typename function>
void
Apply2MergedXY12Gates(const vector<int>& gate_qubits,
                     const int total_q_cir,
                     cmplx* __restrict amp,
                     const idx_size size,
                     function& gate_func)
{
    idx_size gate_bitmask = 0, num_bits = gate_qubits.size();
    idx_size iter_count = 0;
    for (idx_size i = 0; i < num_bits; ++i)
        gate_bitmask |= (1ull << ((total_q_cir - 1) - gate_qubits[i]));
    
    idx_size num_indices = 1ull << num_bits;
    idx_size indices [num_indices];
    memset(indices, 0, num_indices * sizeof(idx_size));
    ExtractIndicesForAmp(indices, gate_qubits, total_q_cir);
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
Merge2XY12Gates(Gate& gate1,
                 Gate& gate2,
                 const int total_q_cir,
                 cmplx* __restrict amp,
                 const idx_size amp_size)
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
        Apply2MergedXY12Gates(gate_to_apply.qubits, total_q_cir, amp, amp_size, ApplyXX12Gate);

    else if(g1t == Gate::Type::X_1_2 && g2t == Gate::Type::Y_1_2)
        Apply2MergedXY12Gates(gate_to_apply.qubits, total_q_cir, amp, amp_size, ApplyXY12Gate);

    else if(g1t == Gate::Type::Y_1_2 && g2t == Gate::Type::Y_1_2)
        Apply2MergedXY12Gates(gate_to_apply.qubits, total_q_cir, amp, amp_size, ApplyYY12Gate);

    else if(g1t == Gate::Type::Y_1_2 && g2t == Gate::Type::X_1_2)
        Apply2MergedXY12Gates(gate_to_apply.qubits, total_q_cir, amp, amp_size, ApplyYX12Gate);


}

void
Merge4XY12Gates(vector<Gate>& block_gates,
               const int total_q_cir,
               cmplx* __restrict amp,
               const idx_size amp_size)
{
    vector<int> block_qubits(4, 0);
    function<void (const idx_size* , cmplx*)> gate_app_funcs[2];
    
    int q = 0;
    for (idx_size i = 0; i < 4; i+=2) {
    
        block_qubits[i] = block_gates[i].qubits.back();
        block_qubits[i + 1] = block_gates[i + 1].qubits.back();

        Gate::Type g1t = (Gate::Type)block_gates[i].ids.back();
        Gate::Type g2t = (Gate::Type)block_gates[i + 1].ids.back();
        
        if(g1t == Gate::Type::X_1_2 && g2t == Gate::Type::X_1_2)
            gate_app_funcs[q++] = ApplyXX12Gate;
        
        else if(g1t == Gate::Type::X_1_2 && g2t == Gate::Type::Y_1_2)
            gate_app_funcs[q++] = ApplyXY12Gate;
        
        else if(g1t == Gate::Type::Y_1_2 && g2t == Gate::Type::Y_1_2)
            gate_app_funcs[q++] =  ApplyYY12Gate;
        
        
        else if(g1t == Gate::Type::Y_1_2 && g2t == Gate::Type::X_1_2)
            gate_app_funcs[q++] = ApplyYX12Gate;
        
    }
    
    Apply4MergedXY12Gates(block_qubits, total_q_cir, amp, amp_size, gate_app_funcs);
}

void
ApplyManyXOnSlice(const idx_size num_qbits,
                  cmplx* __restrict amp)
{
    for (idx_size i = 0; i < num_qbits ; ++i){
        const idx_size add = 1 << (i + 1);
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
        const idx_size add = 1 << (i + 1);
        const idx_size i_offset = 1 << i;
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
          const idx_size amp_size,
          const vector<int>& qubits_in_cluster,
          const int total_q_cir,
          const Gate::Type gate_type)
{
    const idx_size num_qubits = qubits_in_cluster.size(),
    slice_size = 1ull << num_qubits;
    
    idx_size gate_bitmask = 0;
    for (idx_size i = 0; i < num_qubits; ++i)
        gate_bitmask |= (1ull << ((total_q_cir - 1) - qubits_in_cluster[i]));
    
    idx_size indices [slice_size];
    memset(indices, 0, slice_size * sizeof(idx_size));
    ExtractIndicesForAmp(indices, qubits_in_cluster, total_q_cir);
    cmplx amp_slice[slice_size];
    memset(amp_slice, 0, slice_size * sizeof(cmplx));
    
    idx_size idx = 0, iter_count = 0;
    while(iter_count < (amp_size/slice_size)) {
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

