//
//  kernals.cpp
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 12/14/17.
//

#include "kernels.h"

void
GroupCZGates(bitset<128>* __restrict qubits_CZ_bitmasks,
             const int num_qubits_amp,
             const vector<int>& gate_qubits)
{
    bitset<128> bits = 0;
    const int new_q = num_qubits_amp - 1;
    
    for (auto q : gate_qubits) {
        bits[new_q - q] = 1;
        qubits_CZ_bitmasks[new_q - q][new_q - q] = 1;
    }
    for (auto q : gate_qubits)
        qubits_CZ_bitmasks[new_q - q] ^= bits;
}

void
GroupTGates(bitset<128>* __restrict T_bitmasks,
            const int num_qubits_amp,
            const vector<int>& gate_qubits)
{
    //Better way to do this? What if more than 2 T_gates incident on a qubit within a cycle.
    bitset<128> t_mask;
    t_mask[(num_qubits_amp - 1) - gate_qubits[0]] = 1;
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
                     idx_size gate_qubits,
                     const int num_qubits_amp,
                     const idx_size starting_idx)
{
    const idx_size gate_qubits_size =  __builtin_popcountll(gate_qubits);
    const idx_size num_q = gate_qubits_size;
    idx_size strides_size = 1, gap = 1ull << (num_q - 1);
    
    strides[0] = starting_idx;
    idx_size prev_gap = gap;
    for (idx_size i = starting_idx ; i < num_q; ++i) {
        idx_size q = __builtin_ctzl(gate_qubits);
        for (idx_size n = 0; strides_size < (1ull << (i+1)); n += prev_gap) {
            strides[n + gap] = strides[n] + (1ull << ((num_qubits_amp - 1) - q));
            ++strides_size;
        }
        gate_qubits ^= 1ull << q;
        prev_gap = gap;
        gap /= 2;
    }
}

void
FormBlockOfCZTGates(idx_size& gate_i,
                    bitset<128>* __restrict CZ_bitmasks,
                    bitset<128>* __restrict T_bitmasks /*2*/,
                    const vector<Gate>& cluster,
                    const int num_qubits_amp)
{
    for(;gate_i < cluster.size(); ++gate_i) {
        const auto gt = cluster[gate_i].ids.back();
        
        if (gt == Gate::Type::Z)
            GroupCZGates(CZ_bitmasks, num_qubits_amp, cluster[gate_i].qubits);
        else if (gt == Gate::Type::T)
            GroupTGates(T_bitmasks, num_qubits_amp, cluster[gate_i].qubits);
        else break;
    }
}

vector<int>
FormBlockOfXYHGates(idx_size& gate_i,
                    const Gate::Type gate_type,
                    const vector<Gate>& all_gates)
{
    vector<int> qubits_in_cluster;
    for(;gate_i < all_gates.size(); ++gate_i) {
        const auto& gt = all_gates[gate_i];
        
        if(gt.ids.back() == gate_type)
            qubits_in_cluster.push_back(gt.qubits.back());
        else break;
    }
    return qubits_in_cluster;
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
                     const int num_qubits_amp,
                     const idx_size* __restrict CZ_bitmasks,
                     const idx_size* __restrict T_bitmasks)
{
    const idx_size amp_size = 1ull << num_qubits_amp;
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
ApplyCZDecomposition(cmplx* __restrict amp,
                      const int num_qubits_amp,
                      const int gate_qubit,
                      const Gate::Type gate_type)
{
    const idx_size amp_size = 1ull << num_qubits_amp;
    const idx_size gate_bitmask = (1ull << (num_qubits_amp - 1 - gate_qubit));
    amp = (cmplx*)__builtin_assume_aligned(amp, 64);
    idx_size count = 0, q = (int)gate_bitmask;
    cmplx multiplier1 = 0, multiplier2 = 1;
    
    if (gate_type == Gate::Type::CZ_D1 || gate_type == Gate::Type::CZ_D6)
        multiplier1 = cmplx(-1, 0);
    else if (gate_type == Gate::Type::CZ_D6)
        multiplier1 = cmplx(1, 0);
    else if (gate_type == Gate::Type::CZ_D7)
        multiplier1 = cmplx(0, 1);

    if (gate_type == Gate::Type::CZ_D5)
        multiplier2 = 2;
    else if (gate_type == Gate::Type::CZ_D6)
        multiplier2 = cmplx(0, -1);

    if (gate_type == Gate::Type::CZ_D2)
        q = 0;
    
    while (count < amp_size) {
        if ((count & gate_bitmask) == q)
            amp[count++] *= multiplier1;
        else if (gate_type == Gate::Type::CZ_D5 || gate_type == Gate::Type::CZ_D6)
            amp[count++] *= multiplier2;
        else
            count += gate_bitmask;
    }
}

void
Apply1QXYGates(cmplx* __restrict amp,
               const int q,
               const int num_qubits,
               const Gate::Type gate_type,
               const int num_threads)
{
    constexpr idx_size num_indices = 2;
    const idx_size amp_size = 1ull << num_qubits, 
    block_size = amp_size/(1ull << q),
    num_iters = block_size/num_indices, gate_bitmask = (1ull << ((num_qubits - 1) - q));
    const array<idx_size, num_indices> indices = {0, 1ull << ((num_qubits - 1) - q)};

    idx_size add = 1;
   
    //AVX functions handles 4 amps at a time.
    void (*gate_func)(cmplx* __restrict, const idx_size*) ;
    if (gate_type == Gate::Type::X_1_2) {
        if (q < num_qubits - 2) {
            gate_func = ApplyX12GateAVX;
            add = 4;
        }
        else
            gate_func = ApplyX12Gate;
    }
    else {
        if (q < num_qubits - 2) {
            gate_func = ApplyY12GateAVX;
            add = 4;
        }
        else
            gate_func = ApplyY12Gate;
    }
    
    amp = (cmplx*)__builtin_assume_aligned(amp, 64);
    #pragma omp parallel for schedule(guided) num_threads(num_threads)
    for (idx_size idx = 0; idx < amp_size; idx += block_size) {
        idx_size iter_count = 0, block_idx = idx;
        array<idx_size, num_indices> temp_indices;
        
        while(iter_count < num_iters) {
            if ((block_idx & gate_bitmask) == 0) {
                iter_count += add;
      
                for (idx_size i = 0; i < num_indices; ++i)
                    temp_indices[i] = indices[i] + block_idx;
                
                gate_func(amp, temp_indices.data());
                
                block_idx += add;
            }
            else
                block_idx += (block_idx & gate_bitmask);
        }
    }
}

template<typename function>
void
Apply2MergedXY12GatesHelper(cmplx* __restrict amp,
                            const idx_size gate_qubits,
                            const int num_qubits_amp,
                            const function& gate_func,
                            const ZeroOptMask& zero_opt_mask,
                            const idx_size add)
{
    constexpr idx_size num_indices = 4;
    const idx_size amp_size = 1ull << num_qubits_amp,
    gate_bitmask = (1ull << ((num_qubits_amp - 1) - __builtin_ctzl(gate_qubits))) |
    (1ull << ((num_qubits_amp - 1) - (63 -  __builtin_clzl(gate_qubits))));
    
    array<idx_size, num_indices> indices;
    ExtractIndicesForAmp(indices.data(), gate_qubits, num_qubits_amp);
    
    amp = (cmplx*)__builtin_assume_aligned(amp, 64);
    idx_size iter_count = 0, idx = 0;
    array<idx_size, num_indices> temp_indices;

    const idx_size  num_iters = amp_size/num_indices;
    while(iter_count < num_iters) {
        if ((idx & gate_bitmask) == 0) {
            iter_count += add;
          
            for (idx_size i = 0; i < num_indices; ++i)
                temp_indices[i] = indices[i] + idx;
            
            gate_func(amp, temp_indices.data());
            
            idx += add;
        }
        else
            idx += (idx & gate_bitmask);
    }
}

void
Apply2MergedXY12Gates(Gate gate1,
                      Gate gate2,
                      cmplx* __restrict amp,
                      const int num_qubits_amp,
                      const ZeroOptMask& zero_opt_mask)
{
    const idx_size qubits = (1ull << gate1.qubits.back()) | (1ull << gate2.qubits.back());

    const Gate::Type g1t = (Gate::Type)gate1.ids.back();
    const Gate::Type g2t = (Gate::Type)gate2.ids.back();

    if(g1t == Gate::Type::X_1_2 && g2t == Gate::Type::X_1_2)
        Apply2MergedXY12GatesHelper(amp, qubits, num_qubits_amp, ApplyXX12Gate, zero_opt_mask);

    else if(g1t == Gate::Type::X_1_2 && g2t == Gate::Type::Y_1_2)
        Apply2MergedXY12GatesHelper(amp, qubits, num_qubits_amp, ApplyXY12Gate, zero_opt_mask);

    else if(g1t == Gate::Type::Y_1_2 && g2t == Gate::Type::Y_1_2)
        Apply2MergedXY12GatesHelper(amp, qubits, num_qubits_amp, ApplyYY12Gate, zero_opt_mask);

    else if(g1t == Gate::Type::Y_1_2 && g2t == Gate::Type::X_1_2)
        Apply2MergedXY12GatesHelper(amp, qubits, num_qubits_amp, ApplyYX12Gate, zero_opt_mask);
}

__attribute__((always_inline)) inline int
GetNextUsedQubitIndex (const idx_size bitmask)
{
    return bitmask ? __builtin_ctzl(bitmask) : kRT;
}

__attribute__((always_inline)) inline int
UpdateXYBitmask(idx_size& X_bitmask,
                idx_size& Y_bitmask,
                idx_size& gates_bitmask,
                idx_size& i_count) {
    
    int Xunused_qubits = GetNextUsedQubitIndex(X_bitmask);
    int Yunused_qubits = GetNextUsedQubitIndex(Y_bitmask);
    int first_q, second_q;
    int gate_type = 0;
    
    if (Xunused_qubits < Yunused_qubits) {
        X_bitmask ^= 1ull << Xunused_qubits;
        first_q = Xunused_qubits;
        second_q = GetNextUsedQubitIndex(X_bitmask);
        if (second_q < Yunused_qubits)
            X_bitmask ^= 1ull << second_q;
        else {
            gate_type = 1;
            second_q = Yunused_qubits;
            Y_bitmask ^= 1ull << Yunused_qubits;
        }
    }
    else {
        Y_bitmask ^= 1ull << Yunused_qubits;
        first_q = Yunused_qubits;
        second_q = GetNextUsedQubitIndex(Y_bitmask);
        if (second_q < Xunused_qubits) {
            gate_type = 2;
            Y_bitmask ^= 1ull << second_q;
            ++i_count;
        }
        else {
            gate_type = 3;
            second_q = Xunused_qubits;
            X_bitmask ^= 1ull << Xunused_qubits;
        }
    }
    gates_bitmask = (1ull << first_q) | (1ull << second_q);
    
    return gate_type;
}


__attribute__((always_inline)) inline idx_size
XYRecursiveTransformHelper(cmplx* __restrict amp,
                           idx_size& X_bitmask,
                           idx_size& Y_bitmask,
                           const int num_qubits,
                           const ZeroOptMask& zero_opt_mask)
{
    idx_size gates_bitmask = 0, i_count = 0;
    int gate_type = UpdateXYBitmask(X_bitmask, Y_bitmask, gates_bitmask, i_count);
    ApplyMergedXYFT(amp, gates_bitmask, gate_type, num_qubits, zero_opt_mask);
    return i_count;
}

idx_size
ApplyHighQXYGates(cmplx* __restrict amp,
                  idx_size& X_bitmask,
                  idx_size& Y_bitmask,
                  const int num_qubits,
                  const ZeroOptMask& zero_opt_mask)
{
    int Xunused_qubits = GetNextUsedQubitIndex(X_bitmask), Yunused_qubits = GetNextUsedQubitIndex(Y_bitmask);
    idx_size k = min(Yunused_qubits, Xunused_qubits), i_count = 0;

    while ((k < kNUM_BRANCHES)) {
         if (X_bitmask || Y_bitmask)
            i_count += XYRecursiveTransformHelper(amp, X_bitmask, Y_bitmask, num_qubits, zero_opt_mask);

        Xunused_qubits = GetNextUsedQubitIndex(X_bitmask);
        Yunused_qubits = GetNextUsedQubitIndex(Y_bitmask);
        k = min(Yunused_qubits, Xunused_qubits);
    }

    return i_count;
}

idx_size
XYFastTransform(cmplx* __restrict amp,
                idx_size X_bitmask,
                idx_size Y_bitmask,
                const int num_qubits,
                const int num_threads,
                const ZeroOptMask& zero_opt_mask,
                const int th)
{
    //base case
    if (num_qubits <= th)
        return XYFastTransformLowQ(amp, X_bitmask, Y_bitmask, num_qubits, num_threads, zero_opt_mask);
    
    return XYFastTransformHighQ(amp, X_bitmask, Y_bitmask, num_qubits, num_threads, zero_opt_mask);
}

idx_size
XYFastTransformIterative(cmplx* __restrict amp,
                         idx_size X_bitmask,
                         idx_size Y_bitmask,
                         const int num_qubits,
                         const int num_threads,
                         const ZeroOptMask& zero_opt_mask,
                         const int th)
{
    idx_size i_count = 0;
    while (X_bitmask || Y_bitmask) {
        
        const idx_size Xunused_qubits = X_bitmask ? __builtin_ctzl(X_bitmask) : kRT;
        const idx_size Yunused_qubits = Y_bitmask ? __builtin_ctzl(Y_bitmask) : kRT;
        const idx_size k = min(Yunused_qubits, Xunused_qubits);
        const idx_size num_iters = 1ull << k, stride = (1ull << num_qubits)/num_iters;
        idx_size gates_bitmask = 0;
        
        X_bitmask >>= k;
        Y_bitmask >>= k;
        
        int gate_type = UpdateXYBitmask(X_bitmask, Y_bitmask, gates_bitmask, i_count);
        
        #pragma omp parallel for schedule(guided) num_threads(num_threads)
        for (idx_size i = 0; i < num_iters ; ++i)
            ApplyMergedXYFT(amp + (i * stride), gates_bitmask, gate_type, (int)(num_qubits - k), zero_opt_mask);
        
        X_bitmask <<= k;
        Y_bitmask <<= k;
    }

    return i_count;
}

idx_size
XYFastTransformHighQ(cmplx* __restrict amp,
                     idx_size X_bitmask,
                     idx_size Y_bitmask,
                     const int num_qubits,
                     const int num_threads,
                     const ZeroOptMask& zero_opt_mask)
{
    idx_size i_count = 0;
   
    if ((X_bitmask & 1) == 1 || (Y_bitmask & 1) == 1)
        i_count += XYRecursiveTransformHelper(amp, X_bitmask, Y_bitmask, num_qubits, zero_opt_mask);
    
    const int Xunused_qubits = GetNextUsedQubitIndex(X_bitmask);
    const int Yunused_qubits = GetNextUsedQubitIndex(Y_bitmask);
    const int k = min(Yunused_qubits, Xunused_qubits);
    
    if (k != kRT) {
        const idx_size num_iters = 1ull << k;
        const idx_size stride = (1ull << num_qubits)/num_iters;
        X_bitmask >>= k;
        Y_bitmask >>= k;
        idx_size temp_i = 0;
        
    #pragma omp parallel for schedule(guided) reduction(+:temp_i) num_threads(num_threads)
        for (idx_size i = 0; i < num_iters ; ++i)
            temp_i += XYFastTransformHighQ(amp + (i * stride), X_bitmask,
                                           Y_bitmask, num_qubits - k, num_threads, zero_opt_mask);
        
        i_count += temp_i / num_iters;
    }
    
    return i_count;
}

idx_size
XYFastTransformLowQ(cmplx* __restrict amp,
                    idx_size X_bitmask,
                    idx_size Y_bitmask,
                    const int num_qubits,
                    const int num_threads,
                    const ZeroOptMask& zero_opt_mask)
{
    idx_size i_count = 0;
    
    idx_size num_gates = (__builtin_popcountll(X_bitmask) + __builtin_popcountll(Y_bitmask)) / 2;
    idx_size gate_bitmasks[num_gates];
    int gate_types [num_gates];
    memset(gate_bitmasks, 0, num_gates * sizeof(idx_size));
    memset(gate_bitmasks, 0, num_gates * sizeof(int));
    for (int i = 0; i < num_qubits; ++i) {
        if (X_bitmask || Y_bitmask)
            gate_types[i] = UpdateXYBitmask(X_bitmask, Y_bitmask, gate_bitmasks[i], i_count);
        else break;
    }
    
    #pragma omp parallel for schedule(guided) num_threads(num_threads)
    for (idx_size i = 0; i < num_gates; ++i)
        ApplyMergedXYFT(amp, gate_bitmasks[i], gate_types[i], num_qubits, zero_opt_mask);

    return i_count;

}


