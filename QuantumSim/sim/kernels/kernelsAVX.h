//
//  kernelsAVX.h
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 1/6/18.
//  Copyright © 2018 Aneeqa Fatima. All rights reserved.
//

#ifndef kernelsAVX_h
#define kernelsAVX_h

#ifndef Xcode
#include <omp.h>
#endif

#include "kernels.h"

constexpr __m256 kneg = {-1, 1, -1, 1, -1, 1, -1, 1};
constexpr __m256 kneg1 = {-0.0f, 0.0f, -0.0f, 0.0f, -0.0f, 0.0f, -0.0f, 0.0f};
constexpr __m256 kneg2 = {-0.0f, -0.0f, -0.0f, -0.0f, -0.0f, -0.0f, -0.0f, -0.0f};

__attribute__((always_inline)) inline void
ApplyX12GateAVX(cmplx* __restrict amp,
                const idx_size* indices /*2*/)
{
    float* __restrict t_amp = (float*)__builtin_assume_aligned(amp, 64);
    __m256 a0 = _mm256_load_ps (&t_amp[2*indices[0]]);
    __m256 a1 = _mm256_load_ps (&t_amp[2*indices[1]]);
    
    __m256 a0_p = _mm256_permute_ps(a0, 0b10110001);
    __m256 a1_p = _mm256_permute_ps(a1, 0b10110001);
    a0_p = _mm256_xor_ps(a0_p, kneg1);
    a1_p = _mm256_xor_ps(a1_p, kneg1);
   
    __m256 t0 = _mm256_add_ps(a0, a0_p);
    __m256 t1 = _mm256_sub_ps(a1, a1_p);
    __m256 t2 = _mm256_sub_ps(a0, a0_p);
    __m256 t3 = _mm256_add_ps(a1, a1_p);
    
    a0 = _mm256_add_ps(t0, t1);
    a1 = _mm256_add_ps(t2, t3);
    
    _mm256_store_ps(&t_amp[2*indices[0]], a0);
    _mm256_store_ps(&t_amp[2*indices[1]], a1);
}

__attribute__((always_inline)) inline void
ApplyY12GateAVX(cmplx* __restrict amp,
                const idx_size* indices /*2*/)
{
    float* __restrict t_amp = (float*)__builtin_assume_aligned(amp, 64);
    __m256 a0 = _mm256_load_ps (&t_amp[2*indices[0]]);
    __m256 a1 = _mm256_load_ps (&t_amp[2*indices[1]]);
    
    __m256 a1_neg = _mm256_xor_ps(a1, kneg2);
    __m256 a0_p = _mm256_permute_ps(a0, 0b10110001);
    __m256 a1_p = _mm256_permute_ps(a1, 0b10110001);
    a0_p = _mm256_xor_ps(a0_p, kneg1);
    a1_p = _mm256_xor_ps(a1_p, kneg1);
    
    __m256 t0 = _mm256_add_ps(a0, a0_p);
    __m256 t1 = _mm256_sub_ps(a1_neg, a1_p);
    __m256 t2 = t0;
    __m256 t3 = _mm256_add_ps(a1, a1_p);
    
    a0 = _mm256_add_ps(t0, t1);
    a1 = _mm256_add_ps(t2, t3);
    
    _mm256_store_ps(&t_amp[2*indices[0]], a0);
    _mm256_store_ps(&t_amp[2*indices[1]], a1);
}

__attribute__((always_inline)) inline void
ApplyXX12GateAVX(cmplx* __restrict amp,
              const idx_size* indices /*4*/)
{
    float* __restrict t_amp = (float*)__builtin_assume_aligned(amp, 64);
    __m256 a0 = _mm256_load_ps (&t_amp[2*indices[0]]);
    __m256 a1 = _mm256_load_ps (&t_amp[2*indices[1]]);
    __m256 a2 = _mm256_load_ps (&t_amp[2*indices[2]]);
    __m256 a3 = _mm256_load_ps (&t_amp[2*indices[3]]);
    
    const __m256 t0 = _mm256_add_ps(a0, a3);
    const __m256 t1 = _mm256_add_ps(a1, a2);
    __m256 t2 = _mm256_sub_ps(a0, a3);
    __m256 t3 = _mm256_sub_ps(a1, a2);
    t2 = _mm256_permute_ps(t2, 0b10110001);
    t2 = _mm256_xor_ps(t2, kneg1);
    t3 = _mm256_permute_ps(t3, 0b10110001);
    t3 = _mm256_xor_ps(t3, kneg1);

    a0 = _mm256_add_ps(t1, t2);
    a1 = _mm256_add_ps(t0, t3);
    a2 = _mm256_sub_ps(t0, t3);
    a3 = _mm256_sub_ps(t1, t2);
    
    _mm256_store_ps(&t_amp[2*indices[0]], a0);
    _mm256_store_ps(&t_amp[2*indices[1]], a1);
    _mm256_store_ps(&t_amp[2*indices[2]], a2);
    _mm256_store_ps(&t_amp[2*indices[3]], a3);
}

__attribute__((always_inline)) inline void
ApplyYY12GateAVX(cmplx* __restrict amp,
                 const idx_size* indices /*4*/)
{
    float* __restrict t_amp = (float*)__builtin_assume_aligned(amp, 64);
    __m256 a0 = _mm256_load_ps (&t_amp[2*indices[0]]);
    __m256 a1 = _mm256_load_ps (&t_amp[2*indices[1]]);
    __m256 a2 = _mm256_load_ps (&t_amp[2*indices[2]]);
    __m256 a3 = _mm256_load_ps (&t_amp[2*indices[3]]);
    
    const __m256 t0 = _mm256_add_ps(a0, a3);
    const __m256 t1 = _mm256_sub_ps(a0, a3);
    const __m256 t2 = _mm256_add_ps(a1, a2);
    const __m256 t3 = _mm256_sub_ps(a1, a2);
    
    a0 = _mm256_sub_ps(t0, t2);
    a1 = _mm256_add_ps(t1, t3);
    a2 = _mm256_sub_ps(t1, t3);
    a3 = _mm256_add_ps(t0, t2);
    
    _mm256_store_ps(&t_amp[2*indices[0]], a0);
    _mm256_store_ps(&t_amp[2*indices[1]], a1);
    _mm256_store_ps(&t_amp[2*indices[2]], a2);
    _mm256_store_ps(&t_amp[2*indices[3]], a3);
}

__attribute__((always_inline)) inline void
ApplyXY12GateAVX(cmplx* __restrict amp,
                 const idx_size* indices /*4*/)
{
    float* __restrict t_amp = (float*)__builtin_assume_aligned(amp, 64);
    __m256 a0 = _mm256_load_ps (&t_amp[2*indices[0]]);
    __m256 a1 = _mm256_load_ps (&t_amp[2*indices[1]]);
    __m256 a2 = _mm256_load_ps (&t_amp[2*indices[2]]);
    __m256 a3 = _mm256_load_ps (&t_amp[2*indices[3]]);
    
    const __m256 t0 = _mm256_sub_ps(a0, a1);
    const __m256 t1 = _mm256_sub_ps(a2, a3);
    const __m256 t2 = _mm256_add_ps(a0, a1);
    const __m256 t3 = _mm256_add_ps(a2, a3);
    
    __m256 it0 = _mm256_permute_ps(t0, 0b10110001);
    it0 = _mm256_xor_ps(it0, kneg1);
    __m256 it1 = _mm256_permute_ps(t1, 0b10110001);
    it1 = _mm256_xor_ps(it1, kneg1);
    __m256 it2 = _mm256_permute_ps(t2, 0b10110001);
    it2 = _mm256_xor_ps(it2, kneg1);
    __m256 it3 = _mm256_permute_ps(t3, 0b10110001);
    it3 = _mm256_xor_ps(it3, kneg1);

    
    a0 = _mm256_add_ps(it0, t1);
    a1 = _mm256_add_ps(it2, t3);
    a2 = _mm256_add_ps(t0, it1);
    a3 = _mm256_add_ps(t2, it3);
    
    _mm256_store_ps(&t_amp[2*indices[0]], a0);
    _mm256_store_ps(&t_amp[2*indices[1]], a1);
    _mm256_store_ps(&t_amp[2*indices[2]], a2);
    _mm256_store_ps(&t_amp[2*indices[3]], a3);
}

__attribute__((always_inline)) inline void
ApplyYX12GateAVX(cmplx* __restrict amp,
              const idx_size* indices /*4*/)
{
    
    float* __restrict t_amp = (float*)__builtin_assume_aligned(amp, 64);
    __m256 a0 = _mm256_load_ps (&t_amp[2*indices[0]]);
    __m256 a1 = _mm256_load_ps (&t_amp[2*indices[1]]);
    __m256 a2 = _mm256_load_ps (&t_amp[2*indices[2]]);
    __m256 a3 = _mm256_load_ps (&t_amp[2*indices[3]]);
    
    __m256 ia0 = _mm256_permute_ps(a0, 0b10110001);
    ia0 = _mm256_xor_ps(ia0, kneg1);
    __m256 ia1 = _mm256_permute_ps(a1, 0b10110001);
    ia1 = _mm256_xor_ps(ia1, kneg1);
    __m256 ia2 = _mm256_permute_ps(a2, 0b10110001);
    ia2 = _mm256_xor_ps(ia2, kneg1);
    __m256 ia3 = _mm256_permute_ps(a3, 0b10110001);
    ia3 = _mm256_xor_ps(ia3, kneg1);
    
    const __m256 t0 = _mm256_add_ps(ia0, a1);
    const __m256 t1 = _mm256_add_ps(a0, ia1);
    const __m256 t2 = _mm256_add_ps(ia2, a3);
    const __m256 t3 = _mm256_add_ps(a2, ia3);
    
    a0 = _mm256_sub_ps(t0, t2);
    a1 = _mm256_sub_ps(t1, t3);
    a2 = _mm256_add_ps(t0, t2);
    a3 = _mm256_add_ps(t1, t3);
    
    _mm256_store_ps(&t_amp[2*indices[0]], a0);
    _mm256_store_ps(&t_amp[2*indices[1]], a1);
    _mm256_store_ps(&t_amp[2*indices[2]], a2);
    _mm256_store_ps(&t_amp[2*indices[3]], a3);
}

void
ApplyBlockOfCZTGatesAVXSeq(cmplx* __restrict amp,
                           const int num_qubits_amp,
                           const idx_size* __restrict CZ_bitmasks,
                           const idx_size* __restrict T_bitmasks);

void
ApplyBlockOfCZTGatesAVXParallel(cmplx* __restrict amp,
                                const int num_qubits_amp,
                                const idx_size* __restrict CZ_bitmasks,
                                const idx_size* __restrict T_bitmasks);

#endif /* kernelsAVX_h */
