//
//  kernels1.h
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 12/28/17.
//

#ifndef kernels1_h
#define kernels1_h

#include "kernels.h"

using namespace std;


__attribute__((always_inline)) inline void
Apply4X12Gate(cmplx* __restrict amp,
              const array<idx_size, 16> indices)
{
    //1st permutation
    __m128 t_re[4], t_im[4];
    {
        float* __restrict a_re = (float*)(&amp[0]);
        float* __restrict a_im = (float*)(&amp[0]) + 1;
        idx_size* idx = (idx_size*)(&indices[0]);
        
        a_re = (float*)__builtin_assume_aligned(a_re, 32);
        a_im = (float*)__builtin_assume_aligned(a_im, 32);
        for (idx_size i = 0; i < 4; ++i) {
            t_re[i] = _mm_setr_ps(a_re[2*idx[0]], a_re[2*idx[1]], a_re[2*idx[2]], a_re[2*idx[3]]);
            t_im[i] = _mm_setr_ps(a_im[2*idx[0]], a_im[2*idx[1]], a_im[2*idx[2]], a_im[2*idx[3]]);
            idx += 4;
        }
    }
    
    //--------------------------------------------------------------------------------------------------------
    //real arithmetics
    __m128 tres0 = _mm_add_ps(t_re[0], t_re[3]);
    __m128 tres1 = _mm_add_ps(t_re[1], t_re[2]);
    __m128 tres2 = _mm_sub_ps(t_im[3], t_im[0]);
    __m128 tres3 = _mm_sub_ps(t_im[2], t_im[1]);
    
    __m128 res0_re_add = _mm_add_ps(tres1, tres2);
    __m128 res1_re_add = _mm_add_ps(tres0, tres3);
    __m128 res0_re_sub = _mm_sub_ps(tres0, tres3);
    __m128 res1_re_sub = _mm_sub_ps(tres1, tres2);
    
    //--------------------------------------------------------------------------------------------------------
    //imaginery arithmetics
    tres0 = _mm_add_ps(t_im[0], t_im[3]);
    tres1 = _mm_add_ps(t_im[1], t_im[2]);
    tres2 = _mm_sub_ps(t_re[0], t_re[3]);
    tres3 = _mm_sub_ps(t_re[1], t_re[2]);
    
    __m128 res0_im_add = _mm_add_ps(tres1, tres2);
    __m128 res1_im_add = _mm_add_ps(tres0, tres3);
    __m128 res0_im_sub = _mm_sub_ps(tres0, tres3);
    __m128 res1_im_sub = _mm_sub_ps(tres1, tres2);
    
    //--------------------------------------------------------------------------------------------------------
    //2nd permutation
    
    const __m128 slice_re[4] = {_mm_permute_ps(res0_re_add, 0b10011100), _mm_permute_ps(res1_re_add, 0b10011100),
                                _mm_permute_ps(res0_re_sub, 0b10011100), _mm_permute_ps(res1_re_sub, 0b10011100)};
    const __m128 slice_im[4] = {_mm_permute_ps(res0_im_add, 0b01100011), _mm_permute_ps(res1_im_add, 0b01100011),
                                _mm_permute_ps(res0_im_sub, 0b01100011), _mm_permute_ps(res1_im_sub, 0b01100011)};
    
    //--------------------------------------------------------------------------------------------------------
    //real arithmetics
    tres0 = _mm_hadd_ps(slice_re[0], slice_re[1]);
    tres1 = _mm_hadd_ps(slice_re[2], slice_re[3]);
    tres2 = _mm_hsub_ps(slice_im[0], slice_im[1]);
    tres3 = _mm_hsub_ps(slice_im[2], slice_im[3]);
    tres2 = _mm_permute_ps(tres2, 0b10110001);
    tres3 = _mm_permute_ps(tres3, 0b10110001);
    
    res0_re_add = _mm_add_ps(tres0, tres2);
    res0_re_sub = _mm_sub_ps(tres0, tres2);
    res1_re_add = _mm_add_ps(tres1, tres3);
    res1_re_sub = _mm_sub_ps(tres1, tres3);
    
    //--------------------------------------------------------------------------------------------------------
    //imaginary arithmetics
    tres0 = _mm_hadd_ps(slice_im[0], slice_im[1]);
    tres1 = _mm_hadd_ps(slice_im[2], slice_im[3]);
    tres2 = _mm_hsub_ps(slice_re[0], slice_re[1]);
    tres3 = _mm_hsub_ps(slice_re[2], slice_re[3]);
    tres2 = _mm_permute_ps(tres2, 0b10110001);
    tres3 = _mm_permute_ps(tres3, 0b10110001);
    
    res0_im_add = _mm_add_ps(tres0, tres2);
    res0_im_sub = _mm_sub_ps(tres0, tres2);
    res1_im_add = _mm_add_ps(tres1, tres3);
    res1_im_sub = _mm_sub_ps(tres1, tres3);

    //--------------------------------------------------------------------------------------------------------
    //memory writes
    const float* res_re[4] = {(float*)&res0_re_add, (float*)&res0_re_sub, (float*)&res1_re_add, (float*)&res1_re_sub};
    const float* res_im[4] = {(float*)&res0_im_add, (float*)&res0_im_sub, (float*)&res1_im_add, (float*)&res1_im_sub};
    
    amp[indices[0]] = cmplx(res_re[0][1], res_im[0][1]); amp[indices[1]] = cmplx(res_re[0][0], res_im[0][0]);
    amp[indices[2]] = cmplx(res_re[1][0], res_im[1][0]); amp[indices[3]] = cmplx(res_re[1][1], res_im[1][1]);
    amp[indices[4]] = cmplx(res_re[0][3], res_im[0][3]); amp[indices[5]] = cmplx(res_re[0][2], res_im[0][2]);
    amp[indices[6]] = cmplx(res_re[1][2], res_im[1][2]); amp[indices[7]] = cmplx(res_re[1][3], res_im[1][3]);
    amp[indices[8]] = cmplx(res_re[2][1], res_im[2][1]); amp[indices[9]] = cmplx(res_re[2][0], res_im[2][0]);
    amp[indices[10]] = cmplx(res_re[3][0], res_im[3][0]); amp[indices[11]] = cmplx(res_re[3][1], res_im[3][1]);
    amp[indices[12]] = cmplx(res_re[2][3], res_im[2][3]); amp[indices[13]] = cmplx(res_re[2][2], res_im[2][2]);
    amp[indices[14]] = cmplx(res_re[3][2], res_im[3][2]); amp[indices[15]] = cmplx(res_re[3][3], res_im[3][3]);
}

__attribute__((always_inline)) inline void
Apply4Y12Gate(cmplx* __restrict amp,
              const array<idx_size, 16> indices)
{
    ApplyYY12Gate( amp, {indices[0], indices[4], indices[8], indices[12]});
    ApplyYY12Gate( amp, {indices[1], indices[5], indices[9], indices[13]});
    ApplyYY12Gate( amp, {indices[2], indices[6], indices[10], indices[14]});
    ApplyYY12Gate( amp, {indices[3], indices[7], indices[11], indices[15]});
    ApplyYY12Gate( amp, {indices[0], indices[1], indices[2], indices[3]});
    ApplyYY12Gate( amp, {indices[4], indices[5], indices[6], indices[7]});
    ApplyYY12Gate( amp, {indices[8], indices[9], indices[10], indices[11]});
    ApplyYY12Gate( amp, {indices[12], indices[13], indices[14], indices[15]});
}

inline valarray<idx_size>
FindStrides (const vector<int>& gate_qubits,
             const int num_q) {
    
    valarray<idx_size> strides(gate_qubits.size());
    for (idx_size i = 0; i < strides.size(); ++i)
        strides[i] = (1ull << ((num_q - 1) - gate_qubits[i]));
    
    return strides;
}

vector<int>
FormBlockOfXYHGates(idx_size& gate_i,
                    const Gate::Type gate_type,
                    const vector<Gate>& all_gates);

//TODO: Test this
void
ApplyControlGate(cmplx* __restrict amp,
                 const int num_controls,
                 const vector<int>& gate_qubits,
                 const int total_circuit_qubits,
                 const Gate& g,
                 const Gate::Type gate_type);

void
Apply4MergedXY12Gates(vector<Gate>& cluster,
                      cmplx* __restrict amp,
                      const idx_size amp_size,
                      const int total_circuit_qubits);

void
ApplyManyXOnSlice(cmplx* __restrict amp,
                  const idx_size num_qbits);

void
ApplyManyYOnSlice(cmplx* __restrict amp,
                  const idx_size num_qbits);

void
ApplyFWHT(cmplx* __restrict amp,
          const idx_size amp_size,
          const vector<int>& qubits_in_cluster,
          const int total_circuit_qubits,
          const Gate::Type gate_type);

#endif /* kernels1_h */
