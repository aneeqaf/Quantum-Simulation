//
//  kernels1.cpp
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 12/28/17.
//  Copyright © 2017 Aneeqa Fatima. All rights reserved.
//

#include "kernels1.h"

void
ApplyNonControl1QGates(cmplx* __restrict amp,
                       const int q,
                       const int num_qubits_amp,
                       const Gate::Type gate_type,
                       const Gate& g)
{
    const idx_size amp_size = 1ull << num_qubits_amp;
    idx_size iter_count = 0, idx = 0, gate_bitmask = 0;
    
    gate_bitmask |= (1ull << ((num_qubits_amp - 1) - q));
    constexpr idx_size num_indices = 2;
    const array<idx_size, num_indices> indices = {0, 1ull << ((num_qubits_amp - 1) - q)};
    array<idx_size, num_indices> temp_indices;
    
    amp = (cmplx*)__builtin_assume_aligned(amp, 64);
    while(iter_count < (amp_size/num_indices)) {
        if ((idx & gate_bitmask) == 0) {
            ++iter_count;
            
            for (idx_size i = 0; i < num_indices; ++i)
                temp_indices[i] = indices[i] + idx;
            
            ApplyGateOnAmps(amp, temp_indices.data(), num_indices, gate_type, g);
            
            ++idx;
        }
        else
            idx += (idx & gate_bitmask);
    }
}

//TODO: Test this
void
ApplyControlGate(cmplx* __restrict amp,
                 const int num_controls,
                 const vector<int>& gate_qubits,
                 const int num_qubits_amp,
                 const Gate& g,
                 const Gate::Type gate_type)
{
    int loop_count = num_qubits_amp - num_controls;
    const idx_size num_q_1 = num_qubits_amp -1, num_indices = 1ull << loop_count;
    idx_size idx = 0, gate_bitmask = 0, c_bits = 0, iter_count = 0, gate_qubits_bitmask = 0;
    
    for (int j = 0 ; j < num_controls; ++j)
        c_bits |= (1 << (num_q_1 - (gate_qubits[j])));
    
    for (idx_size i = 0; i < gate_qubits.size(); ++i) {
        gate_bitmask |= (1ull << (num_q_1 - gate_qubits[i]));
        gate_qubits_bitmask |= 1ull << gate_qubits[i];
    }
    
    idx_size indices [num_indices];
    memset(indices, 0, num_indices * sizeof(idx_size));
    ExtractIndicesForAmp(indices, gate_qubits_bitmask, num_qubits_amp, num_controls);
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

void
ApplyManyXOnSlice(cmplx* __restrict amp,
                  const idx_size num_qbits)
{
    for (idx_size i = 0; i < num_qbits ; i+=2){
        const idx_size add = 1 << (i + 2);
        const idx_size i_offset0 = 1 << i;
        const idx_size i_offset1 = 1 << (i + 1);
        for (idx_size j = 0; j < (idx_size)(1 << num_qbits); j += add){
            for (idx_size k = 0; k < i_offset0; ++k){
                const auto i0 = j + k;
                const auto i1 = i0 + i_offset0;
                const auto i2 = i0 + i_offset1;
                const auto i3 = i2 + i_offset0;
                const auto t0 = amp[i0] + amp[i3];
                const auto t1 = amp[i1] + amp[i2];
                const auto t2 = ki * (amp[i0] - amp[i3]);
                const auto t3 = ki * (amp[i1] - amp[i2]);
                
                amp[i0] = t1 + t2;
                amp[i1] = t0 + t3;
                amp[i2] = t0 - t3;
                amp[i3] = t1 - t2;
            }
        }
    }
}

void
ApplyManyYOnSlice(cmplx* __restrict amp,
                  const idx_size num_qbits)
{
    for (idx_size i = 0; i < num_qbits ; i+=2){
        const idx_size add = 1 << (i + 2);
        const idx_size i_offset0 = 1 << i;
        const idx_size i_offset1 = 1 << (i + 1);
        for (idx_size j = 0; j < (idx_size)(1 << num_qbits); j += add){
            for (idx_size k = 0; k < i_offset0; ++k){
                const auto i0 = j + k;
                const auto i1 = i0 + i_offset0;
                const auto i2 = i0 + i_offset1;
                const auto i3 = i2 + i_offset0;
                const auto t0 = amp[i0] + amp[i3];
                const auto t1 = amp[i0] - amp[i3];
                const auto t2 = amp[i1] + amp[i2];
                const auto t3 = amp[i1] - amp[i2];
                
                amp[i0] = t0 - t2;
                amp[i1] = t1 + t3;
                amp[i2] = t1 - t3;
                amp[i3] = t0 + t2;
            }
        }
    }
}

void
ApplyFWHT(cmplx* __restrict amp,
          idx_size qubits_in_cluster,
          const int num_qubits_amp,
          const Gate::Type gate_type)
{
    const idx_size num_qubits = __builtin_popcountll(qubits_in_cluster), amp_size = 1ull << num_qubits_amp,
                    slice_size = 1ull << num_qubits;
    
    idx_size gate_bitmask = 0, qubits_bitmask = qubits_in_cluster;
    for (idx_size i = 0; i < num_qubits; ++i) {
        idx_size q = __builtin_ctzl(qubits_in_cluster);
        gate_bitmask |= (1ull << ((num_qubits_amp - 1) - q));
        qubits_in_cluster ^= 1ull << q;
    }
    
    idx_size indices [slice_size];
    memset(indices, 0, slice_size * sizeof(idx_size));
    ExtractIndicesForAmp(indices, qubits_bitmask, num_qubits_amp);
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


