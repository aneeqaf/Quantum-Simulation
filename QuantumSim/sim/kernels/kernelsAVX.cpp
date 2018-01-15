//
//  kernelsAVX.cpp
//  vector_state_sim
//
//  Created by Aneeqa Fatima on 1/14/18.
//

#include <stdio.h>

#include "kernelsAVX.h"


__attribute__((always_inline)) inline void
FirstGroupOf8GatesHelper(bool& negate_Z,
                         idx_size& prev_gc,
                         float* __restrict t_amp,
                         const idx_size __restrict gc[8],
                         const valarray<idx_size>& CZ_bitmasks,
                         const array<idx_size, 2>& T_bitmasks)
{
    const idx_size bit_idx[8] = {static_cast<idx_size>(__builtin_ctzl(gc[0] ^ prev_gc)), 0, 1 , 0,
        static_cast<idx_size>(__builtin_ctzl(gc[4] ^ gc[3])), 0 , 1, 0};

    //Number of T gates to be applied on an amp
    //If no T gate is being applied and amplitude has to be negated, then '-1' is wrapped in.
    int gate_count[8] = {0};
    for (int i = 0; i < 8; ++i) {
        gate_count[i] = __builtin_popcountll(gc[i] & T_bitmasks[0]) + __builtin_popcountll(gc[i] & T_bitmasks[1]);
        if (__builtin_parityl(CZ_bitmasks[bit_idx[i]] & gc[i]) == 1)
            negate_Z = !negate_Z;
        if (negate_Z) {
            if (gate_count[i] == 0)
                gate_count[i] = 4;
            else if (gate_count[i] < 4) {
                gate_count[i] += 4;
                gate_count[i] %= 8;
            }
            else {
                gate_count[i] -= 4;
                gate_count[i] %= 8;
            }
        }
    }
    
    prev_gc = gc[7];
    const __m256 temp_amp0 = _mm256_load_ps (&t_amp[2*gc[0]]);
    const __m256 temp_amp1 = _mm256_load_ps (&t_amp[2*gc[7]]);
    __m256 re_amps = _mm256_shuffle_ps (temp_amp0, temp_amp1, 0b10001000);
    __m256 im_amps = _mm256_shuffle_ps (temp_amp0, temp_amp1, 0b11011101);
    
    const __m256 re_Tgates = {kTGate_re[gate_count[0]], kTGate_re[gate_count[1]], kTGate_re[gate_count[7]],
        kTGate_re[gate_count[6]], kTGate_re[gate_count[3]], kTGate_re[gate_count[2]], kTGate_re[gate_count[4]],
        kTGate_re[gate_count[5]]};
    const __m256 im_Tgates = {kTGate_im[gate_count[0]], kTGate_im[gate_count[1]], kTGate_im[gate_count[7]],
        kTGate_im[gate_count[6]], kTGate_im[gate_count[3]], kTGate_im[gate_count[2]], kTGate_im[gate_count[4]],
        kTGate_im[gate_count[5]]};
    
    const __m256 temp_re = _mm256_mul_ps(im_amps, im_Tgates);
    const __m256 temp_im = _mm256_mul_ps(im_amps, re_Tgates);
    im_amps = _mm256_fmadd_ps(re_amps, im_Tgates, temp_im);
    re_amps = _mm256_fmsub_ps(re_amps, re_Tgates, temp_re);
    
    __m256 first_set = _mm256_shuffle_ps(re_amps, im_amps, 0b01000100);
    __m256 second_set = _mm256_shuffle_ps(re_amps, im_amps, 0b11101110);
    first_set = _mm256_permute_ps(first_set, 0b11011000);
    second_set = _mm256_permute_ps(second_set, 0b11011000);
    
    _mm256_store_ps(&t_amp[2*gc[0]], first_set);
    _mm256_store_ps(&t_amp[2*gc[7]], second_set);
}

__attribute__((always_inline)) inline void
SecondGroupOf8GatesHelper(bool& negate_Z,
                         idx_size& prev_gc,
                         float* __restrict t_amp,
                         const idx_size __restrict gc[8],
                         const valarray<idx_size>& CZ_bitmasks,
                         const array<idx_size, 2>& T_bitmasks)
{
    const idx_size bit_idx[8] = {static_cast<idx_size>(__builtin_ctzl(gc[0] ^ prev_gc)), 0, 1 , 0,
        static_cast<idx_size>(__builtin_ctzl(gc[4] ^ gc[3])), 0 , 1, 0};
    
    //Number of T gates to be applied on an amp
    //If no T gate is being applied and amplitude has to be negated, then '-1' is wrapped in.
    int gate_count[8] = {0};
    for (int i = 0; i < 8; ++i) {
        gate_count[i] = __builtin_popcountll(gc[i] & T_bitmasks[0]) + __builtin_popcountll(gc[i] & T_bitmasks[1]);
        if (__builtin_parityl(CZ_bitmasks[bit_idx[i]] & gc[i]) == 1)
            negate_Z = !negate_Z;
        if (negate_Z) {
            if (gate_count[i] == 0)
                gate_count[i] = 4;
            else if (gate_count[i] < 4) {
                gate_count[i] += 4;
                gate_count[i] %= 8;
            }
            else {
                gate_count[i] -= 4;
                gate_count[i] %= 8;
            }
        }
    }
    
    prev_gc = gc[7];
    const __m256 temp_amp0 = _mm256_load_ps (&t_amp[2*gc[7]]);
    const __m256 temp_amp1 = _mm256_load_ps (&t_amp[2*gc[0]]);
    
    //The order of real and imag amps is 2 floats from 1st group of 4, 2 floats from 2nd group of 4, and so on.
    __m256 re_amps = _mm256_shuffle_ps (temp_amp0, temp_amp1, 0b10001000);
    __m256 im_amps = _mm256_shuffle_ps (temp_amp0, temp_amp1, 0b11011101);
    
    const __m256 re_Tgates = {kTGate_re[gate_count[7]], kTGate_re[gate_count[6]], kTGate_re[gate_count[0]],
        kTGate_re[gate_count[1]], kTGate_re[gate_count[4]], kTGate_re[gate_count[5]], kTGate_re[gate_count[3]],
        kTGate_re[gate_count[2]]};
    const __m256 im_Tgates = {kTGate_im[gate_count[7]], kTGate_im[gate_count[6]], kTGate_im[gate_count[0]],
        kTGate_im[gate_count[1]], kTGate_im[gate_count[4]], kTGate_im[gate_count[5]], kTGate_im[gate_count[3]],
        kTGate_im[gate_count[2]]};
    
    const __m256 temp_re = _mm256_mul_ps(im_amps, im_Tgates);
    const __m256 temp_im = _mm256_mul_ps(im_amps, re_Tgates);
    im_amps = _mm256_fmadd_ps(re_amps, im_Tgates, temp_im);
    re_amps = _mm256_fmsub_ps(re_amps, re_Tgates, temp_re);
    
    __m256 first_set = _mm256_shuffle_ps(re_amps, im_amps, 0b01000100);
    __m256 second_set = _mm256_shuffle_ps(re_amps, im_amps, 0b11101110);
    first_set = _mm256_permute_ps(first_set, 0b11011000);
    second_set = _mm256_permute_ps(second_set, 0b11011000);
    
    _mm256_store_ps(&t_amp[2*gc[7]], first_set);
    _mm256_store_ps(&t_amp[2*gc[0]], second_set);
}


void
ApplyBlockOfCZTGatesAVX(cmplx* __restrict amp,
                        const int total_circuit_qubits,
                        const valarray<idx_size>& CZ_bitmasks,
                        const array<idx_size, 2>& T_bitmasks)
{
    float* __restrict t_amp = (float*)__builtin_assume_aligned(amp, 64);
    const idx_size amp_size = 1ull << total_circuit_qubits;
    idx_size prev_gc = 0;
    
    bool negate_Z = false;
    amp = (cmplx*)__builtin_assume_aligned(amp, 64);
    for (idx_size count = 0; count + 15 < amp_size ; count+=16) {
        
        idx_size gc0 = count ^ (count >> 1);
        idx_size gc4 = (count + 4) ^ ((count + 4) >> 1);
        idx_size gc_first[8] = {gc0, gc0 ^ 1, gc0 ^ 3, gc0 ^ 2, gc4, gc4 ^ 1, gc4 ^ 3, gc4 ^ 2};
        
        FirstGroupOf8GatesHelper(negate_Z, prev_gc, t_amp, gc_first, CZ_bitmasks, T_bitmasks);
        
        gc0 = (count + 8) ^ ((count + 8) >> 1);
        gc4 = (count + 12) ^ ((count + 12) >> 1);
        idx_size gc_second[8] = {gc0, gc0 ^ 1, gc0 ^ 3, gc0 ^ 2, gc4, gc4 ^ 1, gc4 ^ 3, gc4 ^ 2};
        
        SecondGroupOf8GatesHelper(negate_Z, prev_gc, t_amp, gc_second, CZ_bitmasks, T_bitmasks);
    }
}
