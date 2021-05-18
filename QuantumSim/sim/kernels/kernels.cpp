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
             const vector<idx_size>& gate_qubits)
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
            const vector<idx_size>& gate_qubits)
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
        if (cluster[gate_i].GetType() == Gate::Type::ControlZ)
            GroupCZGates(CZ_bitmasks, num_qubits_amp, cluster[gate_i].GetQubits());
        else if (cluster[gate_i].GetType() == Gate::Type::T)
            GroupTGates(T_bitmasks, num_qubits_amp, cluster[gate_i].GetQubits());
        else break;
    }
}

vector<idx_size>
FormBlockOfXYHGates(idx_size& gate_i,
                    const Gate::Type gate_type,
                    const vector<Gate>& all_gates)
{
    vector<idx_size> qubits_in_cluster;
    for(;gate_i < all_gates.size(); ++gate_i) {
        const auto& gt = all_gates[gate_i];
        
        if(gt.GetType() == gate_type)
            qubits_in_cluster.push_back(gt.GetQubits().back());
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

        if(gt.GetType() == Gate::Type::X_1_2 ||  gt.GetType() == Gate::Type::Y_1_2)
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

__attribute__((always_inline)) inline void
Apply1QGatesToCachedAmps(cmplx* __restrict amp,
                         const idx_size* indices,
                         void (*gate_func)(cmplx*, const idx_size*))
{
    constexpr int num_indices = 2;
    alignas(256) cmplx cached_amps[256] = {};
    float* t_cached_amps = (float*)cached_amps;
    float* __restrict t_amp = (float*)__builtin_assume_aligned(amp, 64);
    
    for (int j = 0; j < 128; j += 4) {
         __m256 a = _mm256_load_ps (&t_amp[2 * (indices[0] + j)]);
         _mm256_store_ps(&t_cached_amps[2 * j], a);
         a = _mm256_load_ps (&t_amp[2 * (indices[1] + j)]);
        _mm256_store_ps(&t_cached_amps[2 * (128 + j)], a);
    }
    
    array<idx_size, num_indices> t_indices, c_indices = {0, 128};
    
    for (int i = 0; i < 128; i += 4)
    {
        for (int j = 0; j < num_indices; ++j)
            t_indices[j] = c_indices[j] + i;
        
        gate_func(cached_amps, t_indices.data());
    }
    
    for (int j = 0; j < 128; j += 4) {
        __m256 a = _mm256_load_ps (&t_cached_amps[2 * j]);
         _mm256_store_ps(&t_amp[2 * (indices[0] + j)], a);
        a = _mm256_load_ps (&t_cached_amps[2 * (128 + j)]);
        _mm256_store_ps(&t_amp[2 * (indices[1] + j)], a);
    }
}


__attribute__((always_inline)) inline void
Apply2QGatesToCachedAmps(cmplx* __restrict amp,
                         const idx_size* indices,
                         void (*gate_func)(cmplx*, const idx_size*))
{
    constexpr int num_indices = 4;
    alignas(256) cmplx cached_amps[256] = {};
    float* t_cached_amps = (float*)cached_amps;
    float* __restrict t_amp = (float*)__builtin_assume_aligned(amp, 64);
    
    for (int j = 0; j < 64; j += 4) {
         __m256 a = _mm256_load_ps (&t_amp[2 * (indices[0] + j)]);
         _mm256_store_ps(&t_cached_amps[2 * j], a);
         a = _mm256_load_ps (&t_amp[2 * (indices[1] + j)]);
        _mm256_store_ps(&t_cached_amps[2 * (64 + j)], a);
         a = _mm256_load_ps (&t_amp[2 * (indices[2] + j)]);
        _mm256_store_ps(&t_cached_amps[2 * (128 + j)], a);
         a = _mm256_load_ps (&t_amp[2 * (indices[3] + j)]);
        _mm256_store_ps(&t_cached_amps[2 * (192 + j)], a);
    }
    
    array<idx_size, num_indices> t_indices, c_indices = {0, 64, 128, 192};
    
    for (int i = 0; i < 64; i += 4)
    {
        for (int j = 0; j < num_indices; ++j)
            t_indices[j] = c_indices[j] + i;
        
        gate_func(cached_amps, t_indices.data());
    }
    
    for (int j = 0; j < 64; j += 4) {
        __m256 a = _mm256_load_ps (&t_cached_amps[2 * j]);
         _mm256_store_ps(&t_amp[2 * (indices[0] + j)], a);
        a = _mm256_load_ps (&t_cached_amps[2 * (64 + j)]);
        _mm256_store_ps(&t_amp[2 * (indices[1] + j)], a);
        a = _mm256_load_ps (&t_cached_amps[2 * (128 + j)]);
        _mm256_store_ps(&t_amp[2 * (indices[2] + j)], a);
        a = _mm256_load_ps (&t_cached_amps[2 * (192 + j)]);
        _mm256_store_ps(&t_amp[2 * (indices[3] + j)], a);
    }
}

__attribute__((always_inline)) inline void
ApplyHighQGatesInBlocksTask(cmplx* __restrict amp,
                        idx_size idx,
                        const idx_size num_iters,
                        const idx_size reverse_t_block,
                        const idx_size gate_bitmask,
                        const idx_size* indices,
                        const idx_size starting_idx,
                        const int num_indices,
                        void (*gate_func)(cmplx*, const idx_size*))
{
    constexpr int max_indices = 4;
//    const idx_size idx_add = num_indices == 2 ? 128 : 64,
//    iter_add = idx_add / 4;
    idx_size iter_count = 0;
    array<idx_size, max_indices> temp_indices;
    bool applied_block = false;
    
//    auto cache_efficient_func = num_indices == 2 ? Apply1QGatesToCachedAmps :  Apply2QGatesToCachedAmps;
    
    while(iter_count < num_iters) {
        if (((idx + reverse_t_block) & gate_bitmask) == 0) {
            ++iter_count;// += iter_add;
          
            for (idx_size i = 0; i < num_indices; ++i)
                temp_indices[i] = indices[i] + idx;
            
            //cache_efficient_func(amp, temp_indices.data(), gate_func);
            gate_func(amp, temp_indices.data());
            
            idx += 4;
            applied_block = true;
        }
        else {
            if (applied_block)
            {
                idx +=  reverse_t_block;
                idx += (idx & gate_bitmask) + starting_idx;
                applied_block = false;
            }
            else
                idx += (idx & gate_bitmask);
        }
    }
}

__attribute__((always_inline)) inline void
ApplyLowQGatesInBlocksTask(cmplx* __restrict amp,
                           const idx_size amp_size,
                           const idx_size block_size,
                           const idx_size num_iters,
                           const idx_size num_threads,
                           const idx_size gate_bitmask,
                           const idx_size* indices,
                           const idx_size add,
                           const int num_indices,
                           void (*gate_func)(cmplx*, const idx_size*))
{
    constexpr int max_indices = 4;
    
    #pragma omp parallel for schedule(guided) num_threads(num_threads)
    for (idx_size idx = 0; idx < amp_size; idx += block_size) {
        idx_size iter_count = 0, block_idx = idx;
        array<idx_size, max_indices> temp_indices;

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


void
Apply1QXYHGates(cmplx* __restrict amp,
                int num_threads,
                const int q,
                const int num_qubits,
                const Gate::Type gate_type)
{
    constexpr idx_size num_indices = 2;
    const idx_size amp_size = 1ull << num_qubits,
        gate_bitmask = (1ull << ((num_qubits - 1) - q));
    const array<idx_size, num_indices> indices = {0, 1ull << ((num_qubits - 1) - q)};
       
    amp = (cmplx*)__builtin_assume_aligned(amp, 64);
    
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
     else if (gate_type == Gate::Type::Y_1_2) {
         if (q < num_qubits - 2) {
             gate_func = ApplyY12GateAVX;
             add = 4;
         }
         else
             gate_func = ApplyY12Gate;
     }
     else {
         if (q < num_qubits - 2) {
             gate_func = ApplyHGateAVX;
             add = 4;
         }
         else
             gate_func = ApplyHGate;
     }
    
    idx_size block_size = indices[1] / num_threads;
    if (block_size > 128) {
        const idx_size num_iters = amp_size/(num_indices * num_threads * add);
        vector<array<idx_size, num_indices>> parallel_starting_idxs(num_threads);
        #pragma omp parallel for num_threads(num_threads)
        for (int t = 0; t < num_threads; ++t) {
            parallel_starting_idxs[t][0] = indices[0] + t * block_size;
            ApplyHighQGatesInBlocksTask(amp, parallel_starting_idxs[t][0], num_iters, indices[1] - parallel_starting_idxs[t][0] - block_size, gate_bitmask, indices.data(), parallel_starting_idxs[t][0], num_indices, gate_func);
        }
    }
    else{
        ApplyLowQGatesInBlocksTask(amp, amp_size, amp_size >> q,
                                   amp_size >> (q + 1),
                                   q < num_threads ? (q == 0 ? 1 : q) : num_threads,
                                   gate_bitmask, indices.data(), add, num_indices,
                                   gate_func);
    }
}


template<typename function>
void
Apply2MergedGatesHelper(cmplx* __restrict amp,
                        const idx_size gate_qubits,
                        const int num_qubits_amp,
                        const function& gate_func,
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
ApplyHighQ2MergedGatesInParallel(cmplx* __restrict amp,
                                int num_threads,
                                const idx_size gate_qubits,
                                const int num_qubits_amp,
                                void (*gate_func)(cmplx*, const idx_size*),
                                const idx_size add)
{
    constexpr idx_size num_indices = 4;
    const idx_size amp_size = 1ull << num_qubits_amp, q1 = __builtin_ctzl(gate_qubits),
        q2 = 63 -  __builtin_clzl(gate_qubits),
        gate_bitmask = (1ull << (num_qubits_amp - 1 - q1)) |
                       (1ull << (num_qubits_amp - 1 - q2));
    
    array<idx_size, num_indices> indices;
    ExtractIndicesForAmp(indices.data(), gate_qubits, num_qubits_amp);
        
    amp = (cmplx*)__builtin_assume_aligned(amp, 64);
    const idx_size block_size = indices[1] / num_threads ;
    const idx_size num_iters = amp_size/(num_indices * num_threads * add);
    
    if (block_size > 64) {
        static vector<array<idx_size, num_indices>> parallel_starting_idxs(num_threads);
        #pragma omp parallel for num_threads(num_threads)
        for (int t = 0; t < num_threads; ++t) {
            parallel_starting_idxs[t][0] = indices[0] + t * block_size;
            ApplyHighQGatesInBlocksTask(amp, parallel_starting_idxs[t][0], num_iters, indices[1] - parallel_starting_idxs[t][0] - block_size, gate_bitmask, indices.data(), parallel_starting_idxs[t][0], num_indices, gate_func);
        }
    }
    else {
        ApplyLowQGatesInBlocksTask(amp, amp_size, amp_size >> q1,
            amp_size >> (q1 + 2), q1 < num_threads ? (q1 == 0 ? 1 : q1) : num_threads,
            gate_bitmask, indices.data(), add, num_indices,
            gate_func);
    }
    //    vector<thread*> execution_threads(num_threads);

//
//        thread* thread_obj = new thread(Apply2QGateInBlocksTask, amp, 0, parallel_starting_idxs[t][0], num_iters, indices[1] - parallel_starting_idxs[t][0] - block_size, gate_bitmask, indices.data(), parallel_starting_idxs[t][0], 4, gate_func);
//        execution_threads[t] = thread_obj;
        

//    for (int t = 0; t < num_threads; ++t)
//        execution_threads[t] -> join();
//
//    for (int t = 0; t < num_threads; ++t)
//        delete execution_threads[t];
}

void
Apply2MergedXY12Gates(Gate gate1,
                      Gate gate2,
                      cmplx* __restrict amp,
                      const int num_qubits_amp)
{
    const idx_size qubits = (1ull << gate1.GetQubits().back()) | (1ull << gate2.GetQubits().back());

    const Gate::Type g1t = (Gate::Type)gate1.GetType();
    const Gate::Type g2t = (Gate::Type)gate2.GetType();

    if(g1t == Gate::Type::X_1_2 && g2t == Gate::Type::X_1_2)
        Apply2MergedGatesHelper(amp, qubits, num_qubits_amp, ApplyXX12Gate);

    else if(g1t == Gate::Type::X_1_2 && g2t == Gate::Type::Y_1_2)
        Apply2MergedGatesHelper(amp, qubits, num_qubits_amp, ApplyXY12Gate);

    else if(g1t == Gate::Type::Y_1_2 && g2t == Gate::Type::Y_1_2)
        Apply2MergedGatesHelper(amp, qubits, num_qubits_amp, ApplyYY12Gate);

    else if(g1t == Gate::Type::Y_1_2 && g2t == Gate::Type::X_1_2)
        Apply2MergedGatesHelper(amp, qubits, num_qubits_amp, ApplyYX12Gate);
    
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


__attribute__((always_inline)) inline pair<idx_size, int>
XYHBitmaskApplicationHelper(cmplx* __restrict amp,
                           idx_size& X_bitmask,
                           idx_size& Y_bitmask,
                           idx_size H_bitmask,
                           const int num_qubits,
                           const int num_threads = 0,
                           bool parallel = false,
                           bool zero_block = false)
{
    idx_size gates_bitmask = 0, i_count = 0;
    int gate_type = UpdateXYBitmask(X_bitmask, Y_bitmask, gates_bitmask, i_count);
    int factor_power = 0;
    
    if (!zero_block)
        factor_power = ApplyMergedXYGates(amp, gate_type, gates_bitmask, num_qubits, num_threads, H_bitmask,  parallel);

    return pair<idx_size, idx_size>(i_count, factor_power);
}

pair<idx_size, int>
XYFastTransform(cmplx* __restrict amp,
                idx_size X_bitmask,
                idx_size Y_bitmask,
                const int num_qubits,
                const int num_threads,
                const int th)
{
    idx_size H_bitmask = 0;
    //base case
    if (num_qubits <= th)
        return XYFastTransformLowQ(amp, X_bitmask, Y_bitmask, H_bitmask, num_qubits, num_threads);
    
    return XYHFastTransformHighQ(amp, X_bitmask, Y_bitmask, H_bitmask, num_qubits, num_threads);
}

//idx_size
//XYFastTransformIterative(cmplx* __restrict amp,
//                         idx_size X_bitmask,
//                         idx_size Y_bitmask,
//                         const int num_qubits,
//                         const int num_threads,
//                         const int th)
//{
//    idx_size i_count = 0;
//    while (X_bitmask || Y_bitmask) {
//
//        const idx_size Xunused_qubits = X_bitmask ? __builtin_ctzl(X_bitmask) : kRT;
//        const idx_size Yunused_qubits = Y_bitmask ? __builtin_ctzl(Y_bitmask) : kRT;
//        const idx_size k = min(Yunused_qubits, Xunused_qubits);
//        const idx_size num_iters = 1ull << k, stride = (1ull << num_qubits)/num_iters;
//        idx_size gates_bitmask = 0;
//
//        X_bitmask >>= k;
//        Y_bitmask >>= k;
//
//        int gate_type = UpdateXYBitmask(X_bitmask, Y_bitmask, gates_bitmask, i_count);
//
//        #pragma omp parallel for schedule(guided) num_threads(num_threads)
//        for (idx_size i = 0; i < num_iters ; ++i)
//            ApplyMergedXYGates(amp + (i * stride), gate_type, gates_bitmask, (int)(num_qubits - k));
//
//        X_bitmask <<= k;
//        Y_bitmask <<= k;
//    }
//
//    return i_count;
//}

pair<idx_size, int>
XYHFastTransformHighQ(cmplx* __restrict amp,
                      idx_size X_bitmask,
                      idx_size Y_bitmask,
                      idx_size H_bitmask,
                      const int num_qubits,
                      const int num_threads)
{
    idx_size i_count = 0;
    int factor_power = 0;
   
    if ((X_bitmask & 1) == 1 || (Y_bitmask & 1) == 1) {
        auto phases = XYHBitmaskApplicationHelper(amp, X_bitmask, Y_bitmask, H_bitmask, num_qubits);
        i_count += phases.first;
        factor_power += phases.second;
    }
    
    const int Xunused_qubits = GetNextUsedQubitIndex(X_bitmask);
    const int Yunused_qubits = GetNextUsedQubitIndex(Y_bitmask);
    const int k = min(Yunused_qubits, Xunused_qubits);
    
    if (k != kRT) {
        const idx_size num_iters = 1ull << k;
        const idx_size stride = (1ull << num_qubits)/num_iters;
        X_bitmask >>= k;
        Y_bitmask >>= k;
        H_bitmask >>= k;
        idx_size temp_i = 0;
        int temp_factor_power = 0;
        
    #pragma omp parallel for schedule(guided) reduction(+:temp_i, temp_factor_power) num_threads(num_threads)
        for (idx_size i = 0; i < num_iters ; ++i) {
//            bool zero_block = false;
//            if (!zero_opt_mask.CheckIfAllNonZeroes() &&
//                !zero_opt_mask.CheckIfBlockIsNotZero(i * stride, stride))
//                zero_block = true;
    
            auto phases = XYHFastTransformHighQ(amp + (i * stride), X_bitmask,
                                               Y_bitmask, H_bitmask, num_qubits - k, num_threads);
            temp_i += phases.first;
            temp_factor_power += phases.second;
            
        }
        i_count += temp_i / num_iters;
        factor_power += temp_factor_power / num_iters;
    }
    
    return pair<idx_size, int>(i_count, factor_power);
}

pair<idx_size, int>
ApplyXYHIterativelyInParallel(cmplx* __restrict amp,
                             idx_size X_bitmask,
                             idx_size Y_bitmask,
                             idx_size H_bitmask,
                             const int num_qubits,
                             const int num_threads)
{
    idx_size i_count = 0;
    int factor_power = 0;
    while (X_bitmask || Y_bitmask) {
        auto phases = XYHBitmaskApplicationHelper(amp, X_bitmask, Y_bitmask, H_bitmask,
                                               num_qubits, num_threads, true);
        i_count += phases.first;
        factor_power += phases.second;
    }

    return pair<idx_size, int>(i_count, factor_power);
}

pair<idx_size, int>
XYFastTransformLowQ(cmplx* __restrict amp,
                    idx_size X_bitmask,
                    idx_size Y_bitmask,
                    idx_size H_bitmask,
                    const int num_qubits,
                    const int num_threads)
{
//    idx_size i_count = 0;
//
//    idx_size num_gates = (__builtin_popcountll(X_bitmask) + __builtin_popcountll(Y_bitmask)) / 2;
//    idx_size gate_bitmasks[num_gates];
//    int gate_types [num_gates];
//    memset(gate_bitmasks, 0, num_gates * sizeof(idx_size));
//    memset(gate_bitmasks, 0, num_gates * sizeof(int));
//    for (int i = 0; i < num_qubits; ++i) {
//        if (X_bitmask || Y_bitmask)
//            gate_types[i] = UpdateXYBitmask(X_bitmask, Y_bitmask, gate_bitmasks[i], i_count);
//        else break;
//    }
//
//    #pragma omp parallel for schedule(guided) num_threads(num_threads)
//    for (idx_size i = 0; i < num_gates; ++i)
//        ApplyMergedXYFT(amp, gate_bitmasks[i], gate_types[i], num_qubits, zero_opt_mask);
//
//    return i_count;
    
    idx_size i_count = 0;
    int factor_power = 0;
    
    for (int i = 0; i < num_qubits; ++i) {
        if (X_bitmask || Y_bitmask) {
            auto phases = XYHBitmaskApplicationHelper(amp, X_bitmask, Y_bitmask,
                                                      H_bitmask, num_qubits);
            i_count += phases.first;
            factor_power += phases.second;
        }
        else break;
    }
    return pair<idx_size, int>(i_count, factor_power);
}

void ApplyHGatesRecursively(cmplx* __restrict amp,
                            int num_qubits,
                            int num_threads,
                            idx_size gate_bm)
{
    if ((gate_bm & 1) == 1) {
        int q1 = __builtin_ctzl(gate_bm);
        int q2 = __builtin_ctzl(gate_bm ^ (1ull << q1));
        idx_size gates_bitmask = (1ull << q1) | (1ull << q2);
        gate_bm ^= gates_bitmask;

        bool AVX = (q1 < num_qubits - 1 && q2 < num_qubits - 2);

        if (AVX)
            Apply2MergedGatesHelper(amp, gates_bitmask, num_qubits,
                                    ApplyHHGateAVX, 4);
        else
            Apply2MergedGatesHelper(amp, gates_bitmask, num_qubits,
                                    ApplyHHGate, 1);
    }

    const int k = GetNextUsedQubitIndex(gate_bm);

    if (k != kRT) {
        const idx_size num_iters = 1ull << k;
        const idx_size stride = (1ull << num_qubits)/num_iters;
        gate_bm >>= k;

    #pragma omp parallel for schedule(guided) num_threads(num_threads)
        for (idx_size i = 0; i < num_iters ; ++i) {
            ApplyHGatesRecursively(amp + (i * stride), num_qubits - k, num_threads,
                                       gate_bm);
        }
    }
}

void ApplyHighHGatesIterativelyInParallel(cmplx* __restrict amp,
                                        int num_qubits,
                                        int num_threads,
                                        idx_size gate_bm)
{
    while (gate_bm) {
        int q1 = __builtin_ctzl(gate_bm);
        int q2 = __builtin_ctzl(gate_bm ^ (1ull << q1));
        idx_size gates_bitmask = (1ull << q1) | (1ull << q2);
        gate_bm ^= gates_bitmask;
        
        ApplyHighQ2MergedGatesInParallel(amp, num_threads, gates_bitmask, num_qubits,
          ApplyHHGateAVX, 4);
    }
}

//void ApplyHGatesRecursively(cmplx* __restrict amp,
//                            int num_qubits,
//                            int num_threads,
//                            int current_q)
//{
//    if (current_q >= num_qubits) return;
//   
//    int q1 = 0;
//    int q2 = 1;
//    idx_size gates_bitmask = (1ull << q1) | (1ull << q2);
//    
//    if (q1 > num_qubits - 1)
//        Apply2MergedGatesHelper(amp, gates_bitmask, num_qubits,
//                                ApplyHHGateAVX, 4);
//    else
//        Apply2MergedGatesHelper(amp, gates_bitmask, num_qubits,
//                                ApplyHHGate, 1);
//
//    const int k = current_q + 2;
//    
//    if (k != kRT) {
//        const idx_size num_iters = 1ull << k;
//        const idx_size stride = (1ull << num_qubits)/num_iters;
//        
//#pragma omp parallel for schedule(guided) num_threads(num_threads)
//        for (idx_size i = 0; i < num_iters ; ++i)
//            ApplyHGatesRecursively(amp + (i * stride), num_qubits - k, num_threads,
//                                   k);
//        
//    }
//}

void ApplyHGatesIteratively(cmplx* __restrict amp,
                             int num_qubits,
                             int num_threads,
                             idx_size gate_bm)
{
    while (gate_bm) {
        int parity = __builtin_popcountll(gate_bm);
        int q1 = __builtin_ctzl(gate_bm);
        int q2 = __builtin_ctzl(gate_bm ^ (1ull << q1));
        idx_size gates_bitmask = (1ull << q1) | (1ull << q2);
        
        if (parity == 1)
        {
            int q = q1 > q2 ? q1 : q2;
            gate_bm ^= 1ull << q;
            Apply1QXYHGates(amp, num_threads, q, num_qubits, Gate::Type::Hadamard);
            return;
        }
        
        gate_bm ^= gates_bitmask;
        bool AVX = (q1 < num_qubits - 1) && (q2 < num_qubits - 2);

        if (AVX)
            Apply2MergedGatesHelper(amp, gates_bitmask, num_qubits,
                                    ApplyHHGateAVX, 4);
        else
            Apply2MergedGatesHelper(amp, gates_bitmask, num_qubits,
                                    ApplyHHGate, 1);
    }
}

__attribute__((always_inline)) inline idx_size
ReverseBits(idx_size num, const idx_size num_bits)
{
    idx_size count = num_bits - 1;
    idx_size reverse_num = num & 1;
      
    for (num >>= 1; num; num >>= 1)
    {
       reverse_num <<= 1;
       reverse_num |= num & 1;
       count--;
    }
    reverse_num <<= count;
    return reverse_num;
}

void CacheOptimalBitReversePermutation(cmplx* __restrict amp,
                                       idx_size num_threads,
                                       const int num_qubits)
{
    const idx_size amp_size = 1ull << num_qubits,
    q = num_qubits >= 16 ? 4 : 2, cache_size = 1ull << q, len_b = num_qubits - (q + q),
    max_b = amp_size / (cache_size * cache_size);
    const idx_size block_size = max_b/num_threads;
    float* __restrict t_amp = (float*)__builtin_assume_aligned(amp, 64);

    static vector<idx_size> reversed_num;
    if (reversed_num.size() == 0)
        for (idx_size i = 0; i < cache_size; ++i)
            reversed_num.push_back(ReverseBits(i, q));

    #pragma omp parallel for schedule(guided) num_threads(num_threads)
    for (idx_size block = 0; block < max_b; block += block_size) {
        alignas(alignof(cmplx) * 8) array<cmplx, 256> cached_amps = {};
        float* __restrict t_cached_amps = (float*)__builtin_assume_aligned(cached_amps.data(), 64);
        const idx_size block_limit = block + block_size;
        for (idx_size b = block; b < block_limit; ++b) {
            idx_size reversed_b = ReverseBits(b, len_b);
            if (b <= reversed_b)
            {
                for (idx_size a = 0; a < cache_size; ++a) {
                    const idx_size reversed_a = reversed_num[a];
                    const idx_size idx = (a << (len_b + q)) + (b << q);
                    for (idx_size c = 0; c < cache_size; c += 4)
                        _mm256_store_ps(t_cached_amps + 2 * ((reversed_a << q) + c),
                                        _mm256_load_ps(t_amp + 2 * (idx + c)));
                }

                for (idx_size c = 0; c < cache_size; ++c) {
                    const idx_size reversed_c = reversed_num[c];
                    const idx_size rev_idx = (reversed_c << (len_b + q)) + (reversed_b << q);
                    for (idx_size a_prime = 0; a_prime < cache_size; ++a_prime)
                        swap(cached_amps[(a_prime << q) + c], amp[rev_idx + a_prime]);
                }

                for (idx_size a = 0; a < cache_size; ++a) {
                    const idx_size reversed_a = reversed_num[a];
                    const idx_size idx = (a << (len_b + q)) + (b << q);
                    for (idx_size c = 0; c < cache_size; c += 4)
                        _mm256_store_ps(t_amp + 2 * (idx + c),
                                        _mm256_load_ps (t_cached_amps + 2 * ((reversed_a << q) + c)));
                }
            }
        }
    }
}

pair<idx_size, int>
ApplyHighXYHGatesByBitReversal(cmplx* __restrict amp,
                               idx_size X_bitmask,
                               idx_size Y_bitmask,
                               idx_size H_bitmask,
                               const int num_qubits,
                               const int num_hi_qubits,
                               const int num_threads)
{
    CacheOptimalBitReversePermutation(amp, num_threads, num_qubits);
    
    H_bitmask = ReverseBits(H_bitmask, num_qubits);
    Y_bitmask = ReverseBits(Y_bitmask, num_qubits);
    X_bitmask = ReverseBits(X_bitmask, num_qubits);
    
    array<int, 3> bm_low = {__builtin_ctzl(X_bitmask), __builtin_ctzl(Y_bitmask), __builtin_ctzl(H_bitmask)};
    const int th = *min_element(bm_low.begin(), bm_low.end(),
            [](int first , int second){
                if (!first) first = 64;
                if (!second) second = 64;
                return first != 64 || second != 64 ? first < second : true;});
    
    H_bitmask >>= th;
    Y_bitmask >>= th;
    X_bitmask >>= th;
    
    const int bits_for_blk = num_qubits - th;
    const idx_size amp_size = (1ull << num_qubits),
    block_size = amp_size > (1ull << bits_for_blk) ? (1ull << bits_for_blk) : amp_size;
    const int block_bits = block_size != amp_size ? bits_for_blk : num_qubits;
        
    idx_size i_count = 0;
    int factor_power = 0;
    
    #pragma omp parallel for schedule(guided) num_threads(num_threads)
    for (idx_size block_begin = 0; block_begin < amp_size; block_begin += block_size) {
        idx_size num_iters = block_begin/block_size;
        idx_size offset_idx = num_iters ^ (num_iters >> 1);

        auto phases = XYFastTransformLowQ(amp + (offset_idx * block_size), X_bitmask,
                                         Y_bitmask, H_bitmask, block_bits, num_threads);
        i_count = phases.first;
        factor_power = phases.second;
        
        idx_size new_H_bitmask =  H_bitmask ^ (H_bitmask & (Y_bitmask | X_bitmask));
        if (new_H_bitmask)
            ApplyHGatesIteratively(amp + (offset_idx * block_size), block_bits, num_threads, new_H_bitmask);
    }
    
    CacheOptimalBitReversePermutation(amp, num_threads, num_qubits);
    
    return pair<idx_size, int>(i_count, factor_power);
}
