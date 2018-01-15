//
//  kernelsAVX.cpp
//  vector_state_sim
//
//  Created by Aneeqa Fatima on 1/14/18.
//  Copyright © 2018 Aneeqa Fatima. All rights reserved.
//

#include <stdio.h>

#include "kernelsAVX.h"


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
    for (idx_size count = 0; count + 4 < amp_size ; count+=4) {
        
        const idx_size gc0 = count ^ (count >> 1);
        count += 4;
        const idx_size gc1 = count ^ (count >> 1);
        pair<idx_size, int> gc[8] = {{gc0, 0}, {gc0 ^ 1, 0}, {(gc0 ^ 1) ^ 2, 0}, {((gc0 ^ 1) ^ 2) ^ 1, 0},
                                        {gc1, 0},  {gc1 ^ 1, 0}, {(gc1 ^ 1) ^ 2, 0}, {((gc1 ^ 1) ^ 2) ^ 1, 0}};
        
        const idx_size bit_idx[8] = {static_cast<idx_size>(__builtin_ctzl(gc[0].first ^ prev_gc)), 0, 1 , 0,
            static_cast<idx_size>(__builtin_ctzl(gc[4].first ^ gc[3].first)), 0 , 1, 0};
        
        for (int i = 0; i < 8; ++i) {
            gc[i].second = __builtin_popcountll(gc[i].first & T_bitmasks[0]) + __builtin_popcountll(gc[i].first & T_bitmasks[1]);
            if (__builtin_parityl(CZ_bitmasks[bit_idx[i]] & gc[i].first) == 1)
                negate_Z = !negate_Z;
            if (negate_Z && gc[i].second == 0)
                gc[i].second = 4;
            else if (negate_Z && gc[i].second < 4)
                gc[i].second += 4;
            else if (negate_Z && gc[i].second >= 4)
                gc[i].second -= 4;
            
            gc[i].second %= 8;
        }

        prev_gc = gc[7].first;
        sort(gc, gc + sizeof(gc) / sizeof(*gc));
         const __m256 temp_amp0 = _mm256_load_ps (&t_amp[2*gc[0].first]);
         const __m256 temp_amp1 = _mm256_load_ps (&t_amp[2*gc[4].first]);
        __m256 re_amps = _mm256_shuffle_ps (temp_amp0, temp_amp1, 0b10001000);
        __m256 im_amps = _mm256_shuffle_ps (temp_amp0, temp_amp1, 0b11011101);
        
        const __m256 re_Tgates = {real(kTGate[gc[0].second]), real(kTGate[gc[1].second]), real(kTGate[gc[4].second]),
            real(kTGate[gc[5].second]), real(kTGate[gc[2].second]), real(kTGate[gc[3].second]), real(kTGate[gc[6].second]),
            real(kTGate[gc[7].second])};
        const __m256 im_Tgates = {imag(kTGate[gc[0].second]), imag(kTGate[gc[1].second]), imag(kTGate[gc[4].second]),
            imag(kTGate[gc[5].second]), imag(kTGate[gc[2].second]), imag(kTGate[gc[3].second]), imag(kTGate[gc[6].second]),
            imag(kTGate[gc[7].second])};
        
        const __m256 temp_re = _mm256_mul_ps(im_amps, im_Tgates);
        const __m256 temp_im = _mm256_mul_ps(im_amps, re_Tgates);
        im_amps = _mm256_fmadd_ps(re_amps, im_Tgates, temp_im);
        re_amps = _mm256_fmsub_ps(re_amps, re_Tgates, temp_re);
    
        __m256 first_set = _mm256_shuffle_ps(re_amps, im_amps, 0b01000100);
        __m256 second_set = _mm256_shuffle_ps(re_amps, im_amps, 0b11101110);
        first_set = _mm256_permute_ps(first_set, 0b11011000);
        second_set = _mm256_permute_ps(second_set, 0b11011000);
        
        _mm256_store_ps(&t_amp[2*gc[0].first], first_set);
        _mm256_store_ps(&t_amp[2*gc[4].first], second_set);
    }
}
