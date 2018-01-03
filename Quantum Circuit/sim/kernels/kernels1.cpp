//
//  kernels1.cpp
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 12/28/17.
//  Copyright © 2017 Aneeqa Fatima. All rights reserved.
//

#include "kernels1.h"

vector<int>
FormBlockOfXYHGates(idx_size& gate_i,
                    const Gate::Type gate_type,
                    const vector<Gate>& all_gates)
{
    vector<int> qubits_in_cluster;
    for(;gate_i < all_gates.size() && qubits_in_cluster.size() < 12; ++gate_i) {
        const auto& gt = all_gates[gate_i];
        
        if(gt.ids.back() == gate_type)
            qubits_in_cluster.push_back(gt.qubits.back());
        else break;
    }
    return qubits_in_cluster;
}

//TODO: Test this
void
ApplyControlGate(cmplx* __restrict amp,
                 const int num_controls,
                 const vector<int>& gate_qubits,
                 const int total_circuit_qubits,
                 const Gate& g,
                 const Gate::Type gate_type)
{
    int loop_count = total_circuit_qubits - num_controls;
    const idx_size modified_q = total_circuit_qubits -1, num_indices = 1ull << loop_count;
    idx_size idx = 0, gate_bitmask = 0, c_bits = 0, iter_count = 0;
    
    for (int j = 0 ; j < num_controls; ++j)
        c_bits |= (1 << (modified_q - (gate_qubits[j])));
    
    for (idx_size i = 0; i < gate_qubits.size(); ++i)
        gate_bitmask |= (1ull << (modified_q - gate_qubits[i]));
    
    idx_size indices [num_indices];
    memset(indices, 0, num_indices * sizeof(idx_size));
    ExtractIndicesForAmp(indices, gate_qubits.data(), gate_qubits.size(),total_circuit_qubits, num_controls);
    idx_size temp_indices[num_indices];
    memset(temp_indices, 0, num_indices * sizeof(idx_size));
    
    while(iter_count < (1ull << loop_count)) {
        if ((idx & gate_bitmask) == 0) {
            if((idx & c_bits) == c_bits) {
                iter_count += 2;
                
                for (idx_size i = 0; i < num_indices; ++i)
                    temp_indices[i] = indices[i] + idx;
                ApplyGateOnAmps(amp, temp_indices, num_indices, gate_type, g);
                
                ++idx;
            }
            else
                idx += c_bits;
        }
        else
            idx += (idx & gate_bitmask);
    }
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
            
            gate_func(amp, temp_indices.data());
            
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

void
ApplyManyXOnSlice(cmplx* __restrict amp,
                  const idx_size num_qbits)
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
ApplyManyYOnSlice(cmplx* __restrict amp,
                  const idx_size num_qbits)
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
          const int total_circuit_qubits,
          const Gate::Type gate_type)
{
    const idx_size num_qubits = qubits_in_cluster.size(),
    slice_size = 1ull << num_qubits;
    
    idx_size gate_bitmask = 0;
    for (idx_size i = 0; i < num_qubits; ++i)
        gate_bitmask |= (1ull << ((total_circuit_qubits - 1) - qubits_in_cluster[i]));
    
    idx_size indices [slice_size];
    memset(indices, 0, slice_size * sizeof(idx_size));
    ExtractIndicesForAmp(indices, qubits_in_cluster.data(), qubits_in_cluster.size(), total_circuit_qubits);
    cmplx amp_slice[slice_size];
    memset(amp_slice, 0, slice_size * sizeof(cmplx));
    
    idx_size idx = 0, iter_count = 0;
    while(iter_count < (amp_size/slice_size)) {
        if ((idx & gate_bitmask) == 0) {
            ++iter_count;
            
            for (idx_size i = 0; i < slice_size; ++i)
                amp_slice[i] = amp[indices[i] + idx];
            
            if (gate_type == Gate::Type::X_1_2)
                ApplyManyXOnSlice(amp_slice, num_qubits);
            else
                ApplyManyYOnSlice(amp_slice, num_qubits);
            
            for (idx_size i = 0; i < slice_size; ++i)
                amp[indices[i] + idx] = amp_slice[i];
            
            ++idx;
        }
        else
            idx += (idx & gate_bitmask);
    }
}
