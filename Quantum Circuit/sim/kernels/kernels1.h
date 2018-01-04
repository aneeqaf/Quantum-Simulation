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
              const idx_size* idx /*16*/)
{
    //1st permutation
    __m256 __restrict t_re[2], t_im[2];
    {
        float* __restrict a_re = (float*)__builtin_assume_aligned(&amp[0], 32);
        float* __restrict a_im = (float*)__builtin_assume_aligned(&amp[0], 32) + 1;

        t_re[0] = _mm256_setr_ps(a_re[2*idx[0]], a_re[2*idx[4]], a_re[2*idx[1]], a_re[2*idx[5]],
                              a_re[2*idx[2]], a_re[2*idx[6]], a_re[2*idx[3]], a_re[2*idx[7]]);
        
        t_re[1] = _mm256_setr_ps(a_re[2*idx[12]], a_re[2*idx[8]], a_re[2*idx[13]], a_re[2*idx[9]],
                              a_re[2*idx[14]], a_re[2*idx[10]], a_re[2*idx[15]], a_re[2*idx[11]]);
        
        t_im[0] = _mm256_setr_ps(a_im[2*idx[0]], a_im[2*idx[4]], a_im[2*idx[1]], a_im[2*idx[5]],
                                 a_im[2*idx[2]], a_im[2*idx[6]], a_im[2*idx[3]], a_im[2*idx[7]]);
        
        t_im[1] = _mm256_setr_ps(a_im[2*idx[12]], a_im[2*idx[8]], a_im[2*idx[13]], a_im[2*idx[9]],
                                 a_im[2*idx[14]], a_im[2*idx[10]], a_im[2*idx[15]], a_im[2*idx[11]]);
        
    }
    
    __m256 res_re_add , res_re_sub , res_im_add, res_im_sub ;
    
    {
        __m256 tres0 = _mm256_add_ps(t_re[0], t_re[1]);
        __m256 tres1 = _mm256_sub_ps(t_re[0], t_re[1]);
        __m256 tres2 = _mm256_add_ps(t_im[0], t_im[1]);
        __m256 tres3 = _mm256_sub_ps(t_im[1], t_im[0]);
        tres3 = _mm256_permute_ps(tres3, 0b10110001);
        tres1 = _mm256_permute_ps(tres1, 0b10110001);
        
        res_re_add = _mm256_add_ps(tres0, tres3);
        res_re_sub = _mm256_sub_ps(tres0, tres3);
        res_im_add = _mm256_add_ps(tres2, tres1);
        res_im_sub = _mm256_sub_ps(tres2, tres1);
    }
    
   //--------------------------------------------------------------------------------------------------------
    //2nd permutation
    {
        t_re[0] = _mm256_permute2f128_ps(res_re_add, res_re_sub, 0b00100000);
        t_re[1] = _mm256_permute2f128_ps(res_re_add, res_re_sub, 0b00110001);
        t_im[0] = _mm256_permute2f128_ps(res_im_add, res_im_sub, 0b00100000);
        t_im[1] = _mm256_permute2f128_ps(res_im_add, res_im_sub, 0b00110001);
        t_re[1] = _mm256_permute_ps(t_re[1], 0b01001110);
        t_im[1] = _mm256_permute_ps(t_im[1], 0b01001110);
        
        __m256 tres0 = _mm256_add_ps(t_re[0], t_re[1]);
        __m256 tres1 = _mm256_sub_ps(t_re[0], t_re[1]);
        __m256 tres2 = _mm256_add_ps(t_im[0], t_im[1]);
        __m256 tres3 = _mm256_sub_ps(t_im[1], t_im[0]);
        tres3 = _mm256_permute_ps(tres3, 0b01001110);
        tres1 = _mm256_permute_ps(tres1, 0b01001110);
        
        res_re_add = _mm256_add_ps(tres0, tres3);
        res_re_sub = _mm256_sub_ps(tres0, tres3);
        res_im_add = _mm256_add_ps(tres2, tres1);
        res_im_sub = _mm256_sub_ps(tres2, tres1);
        
    }
 
    //--------------------------------------------------------------------------------------------------------
    //memory writes
    const float* __restrict res_re[2] = {(float*)&res_re_add, (float*)&res_re_sub};
    const float* __restrict res_im[2] = {(float*)&res_im_add, (float*)&res_im_sub};

    amp[idx[0]] = cmplx(res_re[0][3], res_im[0][3]); amp[idx[1]] = cmplx(res_re[0][1], res_im[0][1]);
    amp[idx[2]] = cmplx(res_re[1][1], res_im[1][1]); amp[idx[3]] = cmplx(res_re[1][3], res_im[1][3]);
    amp[idx[4]] = cmplx(res_re[0][2], res_im[0][2]); amp[idx[5]] = cmplx(res_re[0][0], res_im[0][0]);
    amp[idx[6]] = cmplx(res_re[1][0], res_im[1][0]); amp[idx[7]] = cmplx(res_re[1][2], res_im[1][2]);
    amp[idx[8]] = cmplx(res_re[0][6], res_im[0][6]); amp[idx[9]] = cmplx(res_re[0][4], res_im[0][4]);
    amp[idx[10]] = cmplx(res_re[1][4], res_im[1][4]); amp[idx[11]] = cmplx(res_re[1][6], res_im[1][6]);
    amp[idx[12]] = cmplx(res_re[0][7], res_im[0][7]); amp[idx[13]] = cmplx(res_re[0][5], res_im[0][5]);
    amp[idx[14]] = cmplx(res_re[1][5], res_im[1][5]); amp[idx[15]] = cmplx(res_re[1][7], res_im[1][7]);
}

__attribute__((always_inline)) inline void
Apply4Y12Gate(cmplx* __restrict amp,
              const idx_size* indices /*16*/)
{
    array<idx_size, 4> temp_indices = {indices[0], indices[4], indices[8], indices[12]};
    ApplyYY12Gate( amp, temp_indices.data());
    temp_indices = {indices[1], indices[5], indices[9], indices[13]};
    ApplyYY12Gate( amp, temp_indices.data());
    temp_indices = {indices[2], indices[6], indices[10], indices[14]};
    ApplyYY12Gate( amp, temp_indices.data());
    temp_indices = {indices[3], indices[7], indices[11], indices[15]};
    ApplyYY12Gate( amp, temp_indices.data());
    temp_indices = {indices[0], indices[1], indices[2], indices[3]};
    ApplyYY12Gate( amp, temp_indices.data());
    temp_indices = {indices[4], indices[5], indices[6], indices[7]};
    ApplyYY12Gate( amp, temp_indices.data());
    temp_indices = {indices[8], indices[9], indices[10], indices[11]};
    ApplyYY12Gate( amp, temp_indices.data());
    temp_indices = {indices[12], indices[13], indices[14], indices[15]};
    ApplyYY12Gate( amp, temp_indices.data());
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
