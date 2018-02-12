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
        gate_counts[i] = __builtin_popcountll(gray_codes[i] & T_bitmasks[0]) + __builtin_popcountll(gray_codes[i] & T_bitmasks[1]);
        if (__builtin_parityl(CZ_bitmasks[bit_idx[i]] & gray_codes[i]) == 1)
            negate_Z = !negate_Z;
        if (negate_Z)
            gate_counts[i] += 4;
        gate_counts[i] %= 8;
    }
    
}

__attribute__((always_inline)) inline void
FirstGroupOf8GatesHelper(float* __restrict t_amp,
                         const idx_size * gate_counts /*8*/,
                         const idx_size* __restrict gray_codes /*8*/)
{
    const __m256 temp_amp0 = _mm256_load_ps (&t_amp[2 * gray_codes[0]]);
    const __m256 temp_amp1 = _mm256_load_ps (&t_amp[2 * gray_codes[7]]);
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
}

__attribute__((always_inline)) inline void
SecondGroupOf8GatesHelper(float* __restrict t_amp,
                          const idx_size * gate_counts /*8*/,
                          const idx_size* __restrict gray_codes /*8*/)
{
    const __m256 temp_amp0 = _mm256_load_ps (&t_amp[2 * gray_codes[7]]);
    const __m256 temp_amp1 = _mm256_load_ps (&t_amp[2 * gray_codes[0]]);
    
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
}

void
ApplyBlockOfCZTGatesAVX(cmplx* __restrict amp,
                        const int num_qubits_amp,
                        const idx_size* __restrict CZ_bitmasks,
                        const idx_size* __restrict T_bitmasks /*2*/)
{
    float* __restrict t_amp = (float*)__builtin_assume_aligned(amp, 64);
    const idx_size amp_size = (1ull << num_qubits_amp) - 15;
//    idx_size prev_gc = 0;
//    bool negate_Z = false;
    #pragma omp parallel for
    for (idx_size count = 0; count < amp_size ; count+=16) {
        
//        idx_size prev_gc = 0;
//        bool negate_Z = false;
//        for (idx_size c = 0; c + 7 < count ; c+=8) {
//            idx_size gc0 = c ^ (c >> 1);
//            idx_size gc4 = (c + 4) ^ ((c + 4) >> 1);
//            const idx_size gc[8] = {gc0, gc0 ^ 1, gc0 ^ 3, gc0 ^ 2, gc4, gc4 ^ 1, gc4 ^ 3, gc4 ^ 2};
//
//             const idx_size bit_idx[8] = {static_cast<idx_size>(__builtin_ctzl(gc[0] ^ prev_gc)), 0, 1 , 0,
//                 static_cast<idx_size>(__builtin_ctzl(gc[4] ^ gc[3])), 0 , 1, 0};
//
//             for (int i = 0; i < 8; ++i) {
//                 if (__builtin_parityl(CZ_bitmasks[bit_idx[i]] & gc[i]) == 1)
//                     negate_Z = !negate_Z;
//             }
//             prev_gc = gc[7];
//        }
        bool negate_Z = false;
        idx_size prev_gc = !count ? 0 : (count - 1) ^ ((count - 1) >> 1);
        idx_size gc0 = count ^ (count >> 1);
        idx_size gc4 = (count + 4) ^ ((count + 4) >> 1);
        const idx_size gc_first[8] = {gc0, gc0 ^ 1, gc0 ^ 3, gc0 ^ 2, gc4, gc4 ^ 1, gc4 ^ 3, gc4 ^ 2};
        
        idx_size Tgate_count_1[8] = {0};
        GetTGatesCount(Tgate_count_1, negate_Z, prev_gc, gc_first, CZ_bitmasks, T_bitmasks);
        FirstGroupOf8GatesHelper(t_amp, Tgate_count_1, gc_first);
        prev_gc = gc_first[7];
        
        gc0 = (count + 8) ^ ((count + 8) >> 1);
        gc4 = (count + 12) ^ ((count + 12) >> 1);
        const idx_size gc_second[8] = {gc0, gc0 ^ 1, gc0 ^ 3, gc0 ^ 2, gc4, gc4 ^ 1, gc4 ^ 3, gc4 ^ 2};
        
        idx_size Tgate_count_2[8] = {0};
        GetTGatesCount(Tgate_count_2, negate_Z, prev_gc, gc_second, CZ_bitmasks, T_bitmasks);
        FirstGroupOf8GatesHelper(t_amp, Tgate_count_2, gc_second);
//        prev_gc = gc_second[7];
    }
}
