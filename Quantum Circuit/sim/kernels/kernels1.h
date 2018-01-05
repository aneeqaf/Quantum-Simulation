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
        __m256 tres_re_add = _mm256_add_ps(t_re[0], t_re[1]);
        __m256 tres_im_sub = _mm256_sub_ps(t_re[0], t_re[1]);
        __m256 tres_im_add = _mm256_add_ps(t_im[0], t_im[1]);
        __m256 tres_re_sub = _mm256_sub_ps(t_im[1], t_im[0]);
        tres_re_sub = _mm256_permute_ps(tres_re_sub, 0b10110001);
        tres_im_sub = _mm256_permute_ps(tres_im_sub, 0b10110001);
        
        res_re_add = _mm256_add_ps(tres_re_add, tres_re_sub);
        res_re_sub = _mm256_sub_ps(tres_re_add, tres_re_sub);
        res_im_add = _mm256_add_ps(tres_im_add, tres_im_sub);
        res_im_sub = _mm256_sub_ps(tres_im_add, tres_im_sub);
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
        
        __m256 tres_re_add = _mm256_add_ps(t_re[0], t_re[1]);
        __m256 tres_im_sub = _mm256_sub_ps(t_re[0], t_re[1]);
        __m256 tres_im_add = _mm256_add_ps(t_im[0], t_im[1]);
        __m256 tres_re_sub = _mm256_sub_ps(t_im[1], t_im[0]);
        tres_re_sub = _mm256_permute_ps(tres_re_sub, 0b10110001);
        tres_im_sub = _mm256_permute_ps(tres_im_sub, 0b10110001);
        
        res_re_add = _mm256_add_ps(tres_re_add, tres_re_sub);
        res_re_sub = _mm256_sub_ps(tres_re_add, tres_re_sub);
        res_im_add = _mm256_add_ps(tres_im_add, tres_im_sub);
        res_im_sub = _mm256_sub_ps(tres_im_add, tres_im_sub);
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

//TODO : Debug
__attribute__((always_inline)) inline void
Apply4Y12Gate(cmplx* __restrict amp,
              const idx_size* idx /*16*/)
{
    //    array<idx_size, 4> temp_indices = {idx[0], idx[4], idx[8], idx[12]};
    //    ApplyYY12Gate( amp, temp_indices.data());
    //    temp_indices = {idx[1], idx[5], idx[9], idx[13]};
    //    ApplyYY12Gate( amp, temp_indices.data());
    //    temp_indices = {idx[2], idx[6], idx[10], idx[14]};
    //    ApplyYY12Gate( amp, temp_indices.data());
    //    temp_indices = {idx[3], idx[7], idx[11], idx[15]};
    //    ApplyYY12Gate( amp, temp_indices.data());
    //    temp_indices = {idx[0], idx[1], idx[2], idx[3]};
    //    ApplyYY12Gate( amp, temp_indices.data());
    //    temp_indices = {idx[4], idx[5], idx[6], idx[7]};
    //    ApplyYY12Gate( amp, temp_indices.data());
    //    temp_indices = {idx[8], idx[9], idx[10], idx[11]};
    //    ApplyYY12Gate( amp, temp_indices.data());
    //    temp_indices = {idx[12], idx[13], idx[14], idx[15]};
    //    ApplyYY12Gate( amp, temp_indices.data());
    
    //1st permutation
    __m128 __restrict t_re[4], t_im[4];
    {
        float* __restrict a_re = (float*)__builtin_assume_aligned(&amp[0], 32);
        float* __restrict a_im = (float*)__builtin_assume_aligned(&amp[0], 32) + 1;
        idx_size* tidx = (idx_size*)(&idx[0]);
        
        for (idx_size i = 0; i < 4; ++i) {
            t_re[i] = _mm_setr_ps(a_re[2*tidx[0]], a_re[2*tidx[1]], a_re[2*tidx[2]], a_re[2*tidx[3]]);
            t_im[i] = _mm_setr_ps(-a_im[2*tidx[0]], -a_im[2*tidx[1]], -a_im[2*tidx[2]], -a_im[2*tidx[3]]);
            tidx += 4;
        }
    }
    
    __m128 res0_re_add , res1_re_add, res0_re_sub , res1_re_sub, res0_im_add,
    res1_im_add , res0_im_sub , res1_im_sub ;
    
    {
        // ~~~~~~ real arithmetics ~~~~~~
        const __m128 tres0 = _mm_add_ps(t_im[0], t_im[3]);
        const __m128 tres1 = _mm_sub_ps(t_im[0], t_im[3]);
        const __m128 tres2 = _mm_add_ps(t_im[1], t_im[2]);
        const __m128 tres3 = _mm_sub_ps(t_im[1], t_im[2]);
        
        res0_re_sub = _mm_sub_ps(tres0, tres2);
        res0_re_add = _mm_add_ps(tres1, tres3);
        res1_re_sub = _mm_sub_ps(tres1, tres3);
        res1_re_add = _mm_add_ps(tres0, tres2);
        
        // ~~~~~~ imaginary arithmetics ~~~~~~
        const __m128 tres4 = _mm_add_ps(t_re[0], t_re[3]);
        const __m128 tres5 = _mm_sub_ps(t_re[0], t_re[3]);
        const __m128 tres6 = _mm_sub_ps(t_re[1], t_re[2]);
        const __m128 tres7 = _mm_add_ps(t_re[1], t_re[2]);
        
        res0_im_sub = _mm_sub_ps(tres4, tres6);
        res0_im_add = _mm_add_ps(tres5, tres7);
        res1_im_sub = _mm_sub_ps(tres5, tres7);
        res1_im_add = _mm_add_ps(tres4, tres6);
    }
    
    //--------------------------------------------------------------------------------------------------------
    //2nd permutation
    {
        const __m128 neg = _mm_setr_ps(-1.0, -1.0, -1.0, -1.0);
        const __m128 __restrict slice_re[4] = {_mm_permute_ps(res0_re_sub, 0b10011100), _mm_permute_ps(res0_re_add, 0b10011100),
            _mm_permute_ps(res1_re_sub, 0b10011100), _mm_permute_ps(res1_re_add, 0b10011100)};
        const __m128 __restrict slice_im[4] = {_mm_permute_ps(res0_im_sub, 0b01100011), _mm_permute_ps(res0_im_add, 0b01100011),
            _mm_permute_ps(res1_im_sub, 0b01100011), _mm_permute_ps(res1_im_add, 0b01100011)};
        
        
        // ~~~~~~ real arithmetics ~~~~~~
        __m128 tres8 = _mm_hadd_ps(slice_im[0], slice_im[1]);
        __m128 tres9 = _mm_hadd_ps(slice_im[2], slice_im[3]);
        const __m128 tres10 = _mm_hsub_ps(slice_im[0], slice_im[1]);
        const __m128 tres11 = _mm_hsub_ps(slice_im[2], slice_im[3]);
        tres8 = _mm_mul_ps(tres8, neg);
        tres9 = _mm_mul_ps(tres9, neg);
        
        res0_re_sub = _mm_hsub_ps(tres8, tres10);
        res0_re_add = _mm_hadd_ps(tres8, tres10);
        res1_re_sub = _mm_hsub_ps(tres9, tres11);
        res1_re_add = _mm_hadd_ps(tres9, tres11);
        
        // ~~~~~~ imaginary arithmetics ~~~~~~
        __m128 tres12 = _mm_hadd_ps(slice_re[0], slice_re[1]);
        __m128 tres13 = _mm_hadd_ps(slice_re[2], slice_re[3]);
        const __m128 tres14 = _mm_hsub_ps(slice_re[0], slice_re[1]);
        const __m128 tres15 = _mm_hsub_ps(slice_re[2], slice_re[3]);
        tres12 = _mm_mul_ps(tres12, neg);
        tres13 = _mm_mul_ps(tres13, neg);
        
        res0_re_sub = _mm_hsub_ps(tres12, tres14);
        res0_re_add = _mm_hadd_ps(tres12, tres14);
        res1_re_sub = _mm_hsub_ps(tres13, tres15);
        res1_re_add = _mm_hadd_ps(tres13, tres15);
    }
    
    //--------------------------------------------------------------------------------------------------------
    //memory writes
    const float* __restrict res_re[4] = {(float*)&res0_re_sub, (float*)&res0_re_add, (float*)&res1_re_sub, (float*)&res1_re_add};
    const float* __restrict res_im[4] = {(float*)&res0_im_sub, (float*)&res0_im_add, (float*)&res1_im_sub, (float*)&res1_im_add};
    
    //    const cmplx a[4] = {amp[idx[0]], amp[idx [1]], amp[idx[2]], amp[idx[3]]};
    //    const auto t = ki * (a[0] + a[3]);
    //    const auto t1 = ki * (a[0] - a[3]);
    //    const auto t2 = ki * (a[1] + a[2]);
    //    const auto t3 = ki * (a[1] - a[2]);
    //
    //    amp[idx[0]] = t - t2;
    //    amp[idx[1]] = t1 + t3;
    //    amp[idx[2]] = t1 - t3;
    //    amp[idx[3]] = t + t2;
    
    amp[idx[0]] = cmplx(res_re[0][0], res_im[0][0]); amp[idx[1]] = cmplx(res_re[1][2], res_im[1][2]);
    amp[idx[2]] = cmplx(res_re[0][2], res_im[0][2]); amp[idx[3]] = cmplx(res_re[1][0], res_im[1][0]);
    amp[idx[4]] = cmplx(res_re[0][1], res_im[0][1]); amp[idx[5]] = cmplx(res_re[1][1], res_im[1][1]);
    amp[idx[6]] = cmplx(res_re[0][3], res_im[0][3]); amp[idx[7]] = cmplx(res_re[1][3], res_im[1][3]);
    amp[idx[8]] = cmplx(res_re[2][0], res_im[2][0]); amp[idx[9]] = cmplx(res_re[3][0], res_im[3][0]);
    amp[idx[10]] = cmplx(res_re[2][2], res_im[2][2]); amp[idx[11]] = cmplx(res_re[3][2], res_im[3][2]);
    amp[idx[12]] = cmplx(res_re[2][1], res_im[2][1]); amp[idx[13]] = cmplx(res_re[3][1], res_im[3][1]);
    amp[idx[14]] = cmplx(res_re[2][3], res_im[2][3]); amp[idx[15]] = cmplx(res_re[3][3], res_im[3][3]);
    
    for(int i = 0 ; i < 16; ++i)
        cout << amp[i] << "\n";
    cout << "\n";
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
