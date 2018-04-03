//
//  kernelsAVX.cpp
//  vector_state_sim
//
//  Created by Aneeqa Fatima on 1/14/18.
//

#include <stdio.h>

#include "kernelsAVX.h"

__attribute__((always_inline)) inline void
GetTGatesCount(idx_size* gate_counts /*8*/,
               bool& negate_Z,
               const idx_size p_gray_code,
               const idx_size* __restrict gray_codes /*8*/,
               const idx_size* __restrict CZ_bitmasks,
               const idx_size* __restrict T_bitmasks /*2*/)
{
    const idx_size bit_idx[8] = {static_cast<idx_size>(__builtin_ctzl(gray_codes[0] ^ p_gray_code)), 0, 1 , 0,
        static_cast<idx_size>(__builtin_ctzl(gray_codes[4] ^ gray_codes[3])), 0 , 1, 0};
    
    //Number of T gates to be applied on an amp
    //If no T gate is being applied and amplitude has to be negated, then '-1' is wrapped in.
    for (int i = 0; i < 8; ++i) {
        gate_counts[i] = __builtin_popcountll(gray_codes[i] & T_bitmasks[0])
        + __builtin_popcountll(gray_codes[i] & T_bitmasks[1]);
       if (__builtin_parityl(CZ_bitmasks[bit_idx[i]] & gray_codes[i]) == 1)
            negate_Z = !negate_Z;
        if (negate_Z)
            gate_counts[i] += 4;
        gate_counts[i] %= 8;
    }
//    cout << "T gate counts : ";
//    for (int i = 0; i < 8; ++i)
//        cout << bit_idx[i] << " " << gray_codes[i] << " " << gate_counts[i] << " , ";
//    cout << endl;
}

__attribute__((always_inline)) inline bool
FirstGroupOf8GatesHelper(float* __restrict t_amp,
                         const idx_size * gate_counts /*8*/,
                         const idx_size* __restrict gray_codes /*8*/)
{
    const __m256 temp_amp0 = _mm256_load_ps (&t_amp[2 * gray_codes[0]]);
    const __m256 temp_amp1 = _mm256_load_ps (&t_amp[2 * gray_codes[7]]);
    
//    if (_mm256_movemask_ps(_mm256_cmp_ps(temp_amp0, kzeros, _CMP_EQ_OQ)) == 255
//        && _mm256_movemask_ps(_mm256_cmp_ps(temp_amp1, kzeros, _CMP_EQ_OQ))  == 255)
//        return true;
    
    __m256 re_amps = _mm256_shuffle_ps (temp_amp0, temp_amp1, 0b10001000);
    __m256 im_amps = _mm256_shuffle_ps (temp_amp0, temp_amp1, 0b11011101);
    
    const __m256 re_Tgates = {kTGate_re[gate_counts[0]], kTGate_re[gate_counts[1]], kTGate_re[gate_counts[7]],
        kTGate_re[gate_counts[6]], kTGate_re[gate_counts[3]], kTGate_re[gate_counts[2]], kTGate_re[gate_counts[4]],
        kTGate_re[gate_counts[5]]};
    const __m256 im_Tgates = {kTGate_im[gate_counts[0]], kTGate_im[gate_counts[1]], kTGate_im[gate_counts[7]],
        kTGate_im[gate_counts[6]], kTGate_im[gate_counts[3]], kTGate_im[gate_counts[2]], kTGate_im[gate_counts[4]],
        kTGate_im[gate_counts[5]]};
    
    const __m256 tim_amps = _mm256_fmadd_ps(re_amps, im_Tgates, _mm256_mul_ps(im_amps, re_Tgates));
    const __m256 tre_amps = _mm256_fmsub_ps(re_amps, re_Tgates, _mm256_mul_ps(im_amps, im_Tgates));
    
    __m256 first_set = _mm256_shuffle_ps(tre_amps, tim_amps, 0b01000100);
    __m256 second_set = _mm256_shuffle_ps(tre_amps, tim_amps, 0b11101110);
    first_set = _mm256_permute_ps(first_set, 0b11011000);
    second_set = _mm256_permute_ps(second_set, 0b11011000);
    
    _mm256_store_ps(&t_amp[2 * gray_codes[0]], first_set);
    _mm256_store_ps(&t_amp[2 * gray_codes[7]], second_set);
    
    return false;
}

__attribute__((always_inline)) inline bool
SecondGroupOf8GatesHelper(float* __restrict t_amp,
                          const idx_size * gate_counts /*8*/,
                          const idx_size* __restrict gray_codes /*8*/)
{
    const __m256 temp_amp0 = _mm256_load_ps (&t_amp[2 * gray_codes[7]]);
    const __m256 temp_amp1 = _mm256_load_ps (&t_amp[2 * gray_codes[0]]);
    
//    if (_mm256_movemask_ps(_mm256_cmp_ps(temp_amp0, kzeros, _CMP_EQ_OQ)) == 255
//        && _mm256_movemask_ps(_mm256_cmp_ps(temp_amp1, kzeros, _CMP_EQ_OQ))  == 255)
//        return true;
    
    //The order of real and imag amps is 2 floats from 1st group of 4, 2 floats from 2nd group of 4, and so on.
    __m256 re_amps = _mm256_shuffle_ps (temp_amp0, temp_amp1, 0b10001000);
    __m256 im_amps = _mm256_shuffle_ps (temp_amp0, temp_amp1, 0b11011101);
    
    const __m256 re_Tgates = {kTGate_re[gate_counts[7]], kTGate_re[gate_counts[6]], kTGate_re[gate_counts[0]],
        kTGate_re[gate_counts[1]], kTGate_re[gate_counts[4]], kTGate_re[gate_counts[5]], kTGate_re[gate_counts[3]],
        kTGate_re[gate_counts[2]]};
    const __m256 im_Tgates = {kTGate_im[gate_counts[7]], kTGate_im[gate_counts[6]], kTGate_im[gate_counts[0]],
        kTGate_im[gate_counts[1]], kTGate_im[gate_counts[4]], kTGate_im[gate_counts[5]], kTGate_im[gate_counts[3]],
        kTGate_im[gate_counts[2]]};
    
    const __m256 tim_amps = _mm256_fmadd_ps(re_amps, im_Tgates, _mm256_mul_ps(im_amps, re_Tgates));
    const __m256 tre_amps = _mm256_fmsub_ps(re_amps, re_Tgates, _mm256_mul_ps(im_amps, im_Tgates));
    
    __m256 first_set = _mm256_shuffle_ps(tre_amps, tim_amps, 0b01000100);
    __m256 second_set = _mm256_shuffle_ps(tre_amps, tim_amps, 0b11101110);
    first_set = _mm256_permute_ps(first_set, 0b11011000);
    second_set = _mm256_permute_ps(second_set, 0b11011000);
    
    _mm256_store_ps(&t_amp[2 * gray_codes[7]], first_set);
    _mm256_store_ps(&t_amp[2 * gray_codes[0]], second_set);
    
    return false;
}

__attribute__((always_inline)) inline bool
ApplyCZTGatesInABlock(float* __restrict t_amp,
                       const int num_qubits_amp,
                       const idx_size* __restrict CZ_bitmasks,
                       const idx_size* __restrict T_bitmasks /*2*/,
                       const int num_threads,
                       const idx_size block_begin,
                       const idx_size block_size,
                       const ZeroOptMask& zero_opt_mask)
{
    const idx_size block_end = block_begin + block_size;
    bool all_zeros = true;
    
    // Before starting a block compute `negate_Z`
    bool negate_Z = false;
    idx_size prev_gc = 0;
    
    if (block_begin) {
        prev_gc = (block_begin - 1) ^ ((block_begin - 1) >> 1);
        idx_size gate_count = 0;
        for (idx_size i = 0; i < (idx_size)num_qubits_amp; ++i) {
            if (((prev_gc & (1ull << i)) == (1ull << i)) && (prev_gc & CZ_bitmasks[i]))
                gate_count += __builtin_popcountll((prev_gc & CZ_bitmasks[i]));
        }
        if (gate_count & 2)
            negate_Z = true;
    }
    
    //Use `negate_Z` to enable a Gray-code optimized loop.
    for (idx_size count = block_begin; count + 15 < block_end ; count+=16) {
        
        idx_size gc0 = count ^ (count >> 1);
        if (zero_opt_mask.CheckIfBlockIsNotZero(gc0, 16)) {
        
            idx_size gc4 = (count + 4) ^ ((count + 4) >> 1);
            const idx_size gc_first[8] = {gc0, gc0 ^ 1, gc0 ^ 3, gc0 ^ 2, gc4, gc4 ^ 1, gc4 ^ 3, gc4 ^ 2};
            
            idx_size Tgate_count_1[8] = {0};
            GetTGatesCount(Tgate_count_1, negate_Z, prev_gc, gc_first, CZ_bitmasks, T_bitmasks);
            bool all_zeros_1 = FirstGroupOf8GatesHelper(t_amp, Tgate_count_1, gc_first);
            prev_gc = gc_first[7];
            
            gc0 = (count + 8) ^ ((count + 8) >> 1);
            gc4 = (count + 12) ^ ((count + 12) >> 1);
            const idx_size gc_second[8] = {gc0, gc0 ^ 1, gc0 ^ 3, gc0 ^ 2, gc4, gc4 ^ 1, gc4 ^ 3, gc4 ^ 2};
            
            idx_size Tgate_count_2[8] = {0};
            GetTGatesCount(Tgate_count_2, negate_Z, prev_gc, gc_second, CZ_bitmasks, T_bitmasks);
            bool all_zeros_2 = FirstGroupOf8GatesHelper(t_amp, Tgate_count_2, gc_second);
            prev_gc = gc_second[7];
            
            if (all_zeros && (!all_zeros_2 || !all_zeros_1)) all_zeros = false;
        }
//        else {
//            for (idx_size i = 2 * (gc0); i < 2 * (gc0 + 16) ; ++i) {
//                assert(t_amp[i] == 0);
//            }
//        }
    }
    return all_zeros;
}

void
ApplyBlockOfCZTGatesAVXParallel(cmplx* __restrict amp,
                                const int num_qubits_amp,
                                const idx_size* __restrict CZ_bitmasks,
                                const idx_size* __restrict T_bitmasks /*2*/,
                                const int num_threads,
                                const ZeroOptMask& zero_opt_mask)
{
    const idx_size amp_size = (1ull << num_qubits_amp), block_size = amp_size > (1u << 12) ? (1u << 12) : amp_size;
    float* __restrict t_amp = (float*)__builtin_assume_aligned(amp, 64);
    
    #pragma omp parallel for schedule(guided) num_threads(num_threads)
    for (idx_size block_begin = 0; block_begin < amp_size; block_begin += block_size)
        ApplyCZTGatesInABlock(t_amp, num_qubits_amp, CZ_bitmasks, T_bitmasks, num_threads,
                              block_begin, block_size, zero_opt_mask);
}

void
ApplyBlockOfCZTGatesAVXSeq(cmplx* __restrict amp,
                           const int num_qubits_amp,
                           const idx_size* __restrict CZ_bitmasks,
                           const idx_size* __restrict T_bitmasks /*2*/)
{
    const idx_size amp_size = (1ull << num_qubits_amp);
    float* __restrict t_amp = (float*)__builtin_assume_aligned(amp, 64);
    bool negate_Z = false;
    idx_size prev_gc = 0;
    bool all_zeros = true;

    for (idx_size count = 0; count + 15 < amp_size ; count+=16) {
        
        idx_size gc0 = count ^ (count >> 1);
        idx_size gc4 = (count + 4) ^ ((count + 4) >> 1);
        const idx_size gc_first[8] = {gc0, gc0 ^ 1, gc0 ^ 3, gc0 ^ 2, gc4, gc4 ^ 1, gc4 ^ 3, gc4 ^ 2};
        
        idx_size Tgate_count_1[8] = {0};
        GetTGatesCount(Tgate_count_1, negate_Z, prev_gc, gc_first, CZ_bitmasks, T_bitmasks);
        bool all_zeros_1 = FirstGroupOf8GatesHelper(t_amp, Tgate_count_1, gc_first);
        prev_gc = gc_first[7];
        
        gc0 = (count + 8) ^ ((count + 8) >> 1);
        gc4 = (count + 12) ^ ((count + 12) >> 1);
        const idx_size gc_second[8] = {gc0, gc0 ^ 1, gc0 ^ 3, gc0 ^ 2, gc4, gc4 ^ 1, gc4 ^ 3, gc4 ^ 2};
        
        idx_size Tgate_count_2[8] = {0};
        GetTGatesCount(Tgate_count_2, negate_Z, prev_gc, gc_second, CZ_bitmasks, T_bitmasks);
        bool all_zeros_2 = FirstGroupOf8GatesHelper(t_amp, Tgate_count_2, gc_second);
        prev_gc = gc_second[7];
        
        if (all_zeros && (!all_zeros_2 || !all_zeros_1)) all_zeros = false;
    }
}

idx_size
ApplyBlockOfCZTAndLowQXYGatesAVX(cmplx* __restrict amp,
                                 const int num_qubits_amp,
                                 const idx_size* __restrict CZ_bitmasks,
                                 const idx_size* __restrict T_bitmasks,
                                 const idx_size Lo_X_bitmask,
                                 const idx_size Lo_Y_bitmask,
                                 const int num_threads,
                                 const int num_high_qubits,
                                 const ZeroOptMask& zero_opt_mask)
{
    const idx_size amp_size = (1ull << num_qubits_amp),
    block_size = amp_size > (1u << (num_qubits_amp - num_high_qubits))
    ? (1u << (num_qubits_amp - num_high_qubits)) : amp_size;
    const int block_bits = block_size != amp_size ? num_qubits_amp - num_high_qubits : num_qubits_amp;
    float* __restrict t_amp = (float*)__builtin_assume_aligned(amp, 64);
    
    idx_size i_count = 0;
    
    #pragma omp parallel for schedule(guided) num_threads(num_threads)
    for (idx_size block_begin = 0; block_begin < amp_size; block_begin += block_size) {
        idx_size num_iters = block_begin/block_size;
        idx_size offset_idx = num_iters ^ (num_iters >> 1);
        
        if (zero_opt_mask.CheckIfBlockIsNotZero(offset_idx * block_size, block_size)) {
            bool all_zeros = ApplyCZTGatesInABlock(t_amp, num_qubits_amp, CZ_bitmasks, T_bitmasks,
                                                   num_threads, block_begin, block_size, zero_opt_mask);
            
            if (!all_zeros)
               i_count = XYFastTransformLowQ(amp + (offset_idx * block_size), Lo_X_bitmask,
                                              Lo_Y_bitmask, block_bits, num_threads, zero_opt_mask);
        }
//        else {
//            for (idx_size i = offset_idx * block_size; i < (offset_idx * block_size) + block_size ; ++i) {
//                assert(amp[i] == cmplx(0, 0));
//            }
//        }
   }
    
    return i_count;
}
