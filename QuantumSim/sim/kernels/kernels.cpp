//
//  kernals.cpp
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 12/14/17.
//

#include "kernels.h"

void
GroupCZGates(idx_size* __restrict qubits_CZ_bitmasks,
             const int total_circuit_qubits,
             const vector<int>& gate_qubits)
{
    idx_size bits = 0;
    const int new_q = total_circuit_qubits - 1;
    
    for (auto q : gate_qubits) {
        bits |= ( 1ull << (new_q - q));
        qubits_CZ_bitmasks[new_q - q] |= ( 1ull << (new_q - q));
    }
    for (auto q : gate_qubits)
        qubits_CZ_bitmasks[new_q - q] ^= bits;
}

void
GroupTGates(idx_size* __restrict T_bitmasks,
            const int total_circuit_qubits,
            const vector<int>& gate_qubits)
{
    //Better way to do this? What if more than 2 T_gates incident on a qubit within a cycle.
    const idx_size t_mask = (1ull << ((total_circuit_qubits - 1) - gate_qubits[0]));
    if ((T_bitmasks[0] & t_mask) != t_mask)
        T_bitmasks[0] |= t_mask;
    
    else {
        if ((T_bitmasks[1] & t_mask) == t_mask)
            throw "More than 2 T gates incident on a qubit.";
        
        bool found = false;
        for (int t = 0; t < 2; ++t)
            if ((T_bitmasks[t] & t_mask) != t_mask) {
                T_bitmasks[t] |= t_mask;
                found = true;
            }
        if (!found) {
            T_bitmasks[1] = t_mask;
        }
    }
}

void
ExtractIndicesForAmp(idx_size* strides,
                     const int* gate_qubits,
                     const idx_size gate_qubits_size,
                     const int total_circuit_qubits,
                     const idx_size starting_idx)
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

void
FormBlockOfCZTGates(idx_size& gate_i,
                    idx_size* __restrict CZ_bitmasks,
                    idx_size* __restrict T_bitmasks /*2*/,
                    const vector<Gate>& cluster,
                    const int total_circuit_qubits)
{
    for(;gate_i < cluster.size(); ++gate_i) {
        const auto gt = cluster[gate_i].ids.back();
        
        if (gt == Gate::Type::Z)
            GroupCZGates(CZ_bitmasks, total_circuit_qubits, cluster[gate_i].qubits);
        else if (gt == Gate::Type::T)
            GroupTGates(T_bitmasks, total_circuit_qubits, cluster[gate_i].qubits);
        else break;
    }
}

void
FormBlockOfXYHGates(vector<Gate>& cluster,
                    idx_size& gate_i,
                    const Gate::Type gate_type,
                    const vector<Gate>& all_gates)
{
    for(;gate_i < all_gates.size() && cluster.size() < 2; ++gate_i) {
        const auto& gt = all_gates[gate_i];

        if(gt.ids.back() == Gate::Type::X_1_2 ||  gt.ids.back() == Gate::Type::Y_1_2)
            cluster.push_back(all_gates[gate_i]);
        else break;
    }
}

void
ApplyBlockOfCZTGates(cmplx* __restrict amp,
                     const int total_circuit_qubits,
                     const idx_size* __restrict CZ_bitmasks,
                     const idx_size* __restrict T_bitmasks)
{
    const idx_size amp_size = 1ull << total_circuit_qubits;
    idx_size prev_gc = 0;
    
    bool negate_Z = false;
    amp = (cmplx*)__builtin_assume_aligned(amp, 64);
    for (idx_size count = 0; count < amp_size ; ++count) {
        const idx_size gc = count ^ (count >> 1);
        const idx_size changed_bit = gc ^ prev_gc;
        const idx_size bit_idx = __builtin_ctzl(changed_bit);
        
        cmplx mutated_amp = amp[gc];
      
        if (__builtin_parityl(CZ_bitmasks[bit_idx] & gc) == 1)
            negate_Z = !negate_Z;
        if (negate_Z)
            mutated_amp = -mutated_amp;
        
        const idx_size gate_c = __builtin_popcountll(gc & T_bitmasks[0])
                + __builtin_popcountll(gc & T_bitmasks[1]);

        // &7 is not faster than % 8
        amp[gc] = mutated_amp * kTGate[gate_c % 8];
        
        prev_gc = gc;
    }
}

void
ApplyNonControl1QGates(cmplx* __restrict amp,
                       const int q,
                       const int total_circuit_qubits,
                       const Gate::Type gate_type,
                       const Gate& g)
{
    const idx_size amp_size = 1ull << total_circuit_qubits;
    idx_size iter_count = 0, idx = 0, gate_bitmask = 0;
    
    gate_bitmask |= (1ull << ((total_circuit_qubits - 1) - q));
    constexpr idx_size num_indices = 2;
    const array<idx_size, num_indices> indices = {0, 1ull << ((total_circuit_qubits - 1) - q)};
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

template<typename function>
void
Apply2MergedXY12GatesHelper(cmplx* __restrict amp,
                            const int* gate_qubits,
                            const int total_circuit_qubits,
                            const function& gate_func,
                            const idx_size add = 1)
{
    const idx_size amp_size = 1ull << total_circuit_qubits;
    idx_size gate_bitmask = 0, iter_count = 0;
    constexpr idx_size num_bits = 2;
    for (idx_size i = 0; i < num_bits; ++i)
        gate_bitmask |= (1ull << ((total_circuit_qubits - 1) - gate_qubits[i]));
    
    constexpr idx_size num_indices = 4;
    array<idx_size, num_indices> indices;
    ExtractIndicesForAmp(indices.data(), gate_qubits, num_bits ,total_circuit_qubits);
    array<idx_size, num_indices> temp_indices;
    
    idx_size idx = 0;
    amp = (cmplx*)__builtin_assume_aligned(amp, 64);
    while(iter_count < (amp_size/num_indices)) {
        if ((idx & gate_bitmask) == 0) {
            iter_count+=add;
            
            for (idx_size i = 0; i < num_indices; ++i)
                temp_indices[i] = indices[i] + idx;
            
            gate_func(amp, temp_indices.data());
            
            idx+=add;
        }
        else
            idx += (idx & gate_bitmask);
    }
}

void
Apply2MergedXY12Gates(Gate gate1,
                      Gate gate2,
                      cmplx* __restrict amp,
                      const int total_circuit_qubits)
{
    const int qubits[2] = {gate1.qubits.back(), gate2.qubits.back()};

    const Gate::Type g1t = (Gate::Type)gate1.ids.back();
    const Gate::Type g2t = (Gate::Type)gate2.ids.back();

    if(g1t == Gate::Type::X_1_2 && g2t == Gate::Type::X_1_2)
        Apply2MergedXY12GatesHelper(amp, qubits, total_circuit_qubits, ApplyXX12Gate);

    else if(g1t == Gate::Type::X_1_2 && g2t == Gate::Type::Y_1_2)
        Apply2MergedXY12GatesHelper(amp, qubits, total_circuit_qubits, ApplyXY12Gate);

    else if(g1t == Gate::Type::Y_1_2 && g2t == Gate::Type::Y_1_2)
        Apply2MergedXY12GatesHelper(amp, qubits, total_circuit_qubits, ApplyYY12Gate);

    else if(g1t == Gate::Type::Y_1_2 && g2t == Gate::Type::X_1_2)
        Apply2MergedXY12GatesHelper(amp, qubits, total_circuit_qubits, ApplyYX12Gate);
}

__attribute__((always_inline)) inline int
GetNextUsedQubitIndex (const idx_size bitmask)
{
    return bitmask ? __builtin_ctzl(bitmask) : kRT;
}


__attribute__((always_inline)) inline idx_size
XYRecursiveTransformHelper(cmplx* __restrict amp,
                           idx_size& X_bitmask,
                           idx_size& Y_bitmask,
                           const int num_qubits)
{
    int Xunused_qubits = GetNextUsedQubitIndex(X_bitmask);
    int Yunused_qubits = GetNextUsedQubitIndex(Y_bitmask);
    
    idx_size i_count = 0;
    if (Xunused_qubits < Yunused_qubits) {
        X_bitmask ^= 1ull << Xunused_qubits;
        const int next_qubit = GetNextUsedQubitIndex(X_bitmask);
        if (next_qubit < Yunused_qubits) {
            const int gates_qubits[2] = {Xunused_qubits, next_qubit};
            X_bitmask ^= 1ull << next_qubit;
            if ((Xunused_qubits < num_qubits - 1 && next_qubit < num_qubits - 2))
                Apply2MergedXY12GatesHelper(amp, gates_qubits, num_qubits, ApplyXX12GateAVX, 4);
            else
                Apply2MergedXY12GatesHelper(amp, gates_qubits, num_qubits, ApplyXX12Gate);
        }
        else {
            Y_bitmask ^= 1ull << Yunused_qubits;
            const int gates_qubits[2] = {Xunused_qubits, Yunused_qubits};
            if ((Xunused_qubits < num_qubits - 1 && Yunused_qubits < num_qubits - 2))
                Apply2MergedXY12GatesHelper(amp, gates_qubits, num_qubits, ApplyXY12GateAVX, 4);
            else
                Apply2MergedXY12GatesHelper(amp, gates_qubits, num_qubits, ApplyXY12Gate);
        }
    }
    else {
        Y_bitmask ^= 1ull << Yunused_qubits;
        const int next_qubit = GetNextUsedQubitIndex(Y_bitmask);
        if (next_qubit < Xunused_qubits) {
            const int gates_qubits[2] = {Yunused_qubits, next_qubit};
            Y_bitmask ^= 1ull << next_qubit;
            if ((Yunused_qubits < num_qubits - 1 && next_qubit < num_qubits - 2))
                Apply2MergedXY12GatesHelper(amp, gates_qubits, num_qubits, ApplyYY12GateAVX, 4);
            else
                Apply2MergedXY12GatesHelper(amp, gates_qubits, num_qubits, ApplyYY12Gate);
            ++i_count;
        }
        else {
            X_bitmask ^= 1ull << Xunused_qubits;
            const int gates_qubits[2] = {Yunused_qubits, Xunused_qubits};
            if ((Yunused_qubits < num_qubits - 1 && Xunused_qubits < num_qubits - 2))
                Apply2MergedXY12GatesHelper(amp, gates_qubits, num_qubits, ApplyYX12GateAVX, 4);
            else
                Apply2MergedXY12GatesHelper(amp, gates_qubits, num_qubits, ApplyYX12Gate);
        }
    }
    return i_count;
}

idx_size
XYRecursiveTransform(cmplx* __restrict amp,
                     idx_size X_bitmask,
                     idx_size Y_bitmask,
                     const int num_qubits,
                     const int th)
{
    idx_size i_count = 0;
    //base case
    if (num_qubits <= th) {
        for (int i = 0; i < num_qubits; ++i) {
            if (X_bitmask || Y_bitmask) 
                i_count += XYRecursiveTransformHelper(amp, X_bitmask, Y_bitmask, num_qubits);
            else break;
        }
        return i_count;
    }
    
    if ((X_bitmask & 1) == 1 || (Y_bitmask & 1) == 1)
        i_count += XYRecursiveTransformHelper(amp, X_bitmask, Y_bitmask, num_qubits);
    
    const int Xunused_qubits = GetNextUsedQubitIndex(X_bitmask);
    const int Yunused_qubits = GetNextUsedQubitIndex(Y_bitmask);
    const int k = min(Yunused_qubits, Xunused_qubits);
    
    if (k != kRT) {
        const idx_size num_iters = 1ull << k;
        const idx_size stride = (1ull << num_qubits)/num_iters;
        X_bitmask >>= k;
        Y_bitmask >>= k;
        idx_size temp_i = 0;
        for (idx_size i = 0; i < num_iters ; ++i)
            temp_i += XYRecursiveTransform(amp + (i * stride), X_bitmask, Y_bitmask, num_qubits - k, th);
        i_count += temp_i / num_iters;
    }
    
    return i_count;
}
