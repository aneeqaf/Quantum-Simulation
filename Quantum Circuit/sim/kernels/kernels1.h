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
     const cmplx a[16] = {amp[indices[0]], amp[indices[4]],  amp[indices[8]], amp[indices[12]],
                        amp[indices[1]], amp[indices[5]],  amp[indices[9]], amp[indices[13]],
                        amp[indices[2]], amp[indices[6]],  amp[indices[10]], amp[indices[14]],
                        amp[indices[3]], amp[indices[7]], amp[indices[11]],  amp[indices[15]]};

    __m128 t_real = _mm_setr_ps(real(a[0]), real(a[4]), real(a[8]), real(a[12]));
    __m128 t1_real = _mm_setr_ps(real(a[1]), real(a[5]), real(a[9]), real(a[13]));
    __m128 t2_real = _mm_setr_ps(real(a[2]), real(a[6]), real(a[10]), real(a[14]));
    __m128 t3_real = _mm_setr_ps(real(a[3]), real(a[7]), real(a[11]), real(a[15]));
    
    __m128 t_imag = _mm_setr_ps(imag(a[0]), imag(a[4]), imag(a[8]), imag(a[12]));
    __m128 t1_imag = _mm_setr_ps(imag(a[1]), imag(a[5]), imag(a[9]), imag(a[13]));
    __m128 t2_imag = _mm_setr_ps(imag(a[2]), imag(a[6]), imag(a[10]), imag(a[14]));
    __m128 t3_imag = _mm_setr_ps(imag(a[3]), imag(a[7]), imag(a[11]), imag(a[15]));
    
    __m128 tres = _mm_add_ps(t_real, t3_real);
    __m128 tres1 = _mm_add_ps(t1_real, t2_real);
    __m128 tres2 = _mm_sub_ps(t3_imag, t_imag);
    __m128 tres3 = _mm_sub_ps(t2_imag, t1_imag);
    
    __m128 res_real_add = _mm_add_ps(tres1, tres2);
    __m128 res1_real_add = _mm_add_ps(tres, tres3);
    __m128 res_real_sub = _mm_sub_ps(tres, tres3);
    __m128 res1_real_sub = _mm_sub_ps(tres1, tres2);
    
    tres = _mm_add_ps(t_imag, t3_imag);
    tres1 = _mm_add_ps(t1_imag, t2_imag);
    tres2 = _mm_sub_ps(t_real, t3_real);
    tres3 = _mm_sub_ps(t1_real, t2_real);
    
    __m128 res_imag_add = _mm_add_ps(tres1, tres2);
    __m128 res1_imag_add = _mm_add_ps(tres, tres3);
    __m128 res_imag_sub = _mm_sub_ps(tres, tres3);
    __m128 res1_imag_sub = _mm_sub_ps(tres1, tres2);
    
    //slice1
    res_real_add = _mm_permute_ps(res_real_add, 0b10011100);
    res_imag_add = _mm_permute_ps(res_imag_add, 0b01100011);
    //slice2
    res1_real_add = _mm_permute_ps(res1_real_add, 0b10011100);
    res1_imag_add = _mm_permute_ps(res1_imag_add, 0b01100011);
    //slice3
    res_real_sub = _mm_permute_ps(res_real_sub, 0b10011100);
    res_imag_sub = _mm_permute_ps(res_imag_sub, 0b01100011);
    //slice4
    res1_real_sub = _mm_permute_ps(res1_real_sub, 0b10011100);
    res1_imag_sub = _mm_permute_ps(res1_imag_sub, 0b01100011);
    
    tres = _mm_hadd_ps(res_real_add, res1_real_add);
    tres1 = _mm_hadd_ps(res_real_sub, res1_real_sub);
    tres2 = _mm_hsub_ps(res_imag_add, res1_imag_add);
    tres3 = _mm_hsub_ps(res_imag_sub, res1_imag_sub);
    tres2 = _mm_permute_ps(tres2, 0b10110001);
    tres3 = _mm_permute_ps(tres3, 0b10110001);
    
    __m128 tres4 = _mm_add_ps(tres, tres2);
    __m128 tres5 = _mm_sub_ps(tres, tres2);
    __m128 tres6 = _mm_add_ps(tres1, tres3);
    __m128 tres7 = _mm_sub_ps(tres1, tres3);
    
    tres = _mm_hadd_ps(res_imag_add, res1_imag_add);
    tres1 = _mm_hadd_ps(res_imag_sub, res1_imag_sub);
    tres2 = _mm_hsub_ps(res_real_add, res1_real_add);
    tres3 = _mm_hsub_ps(res_real_sub, res1_real_sub);
    tres2 = _mm_permute_ps(tres2, 0b10110001);
    tres3 = _mm_permute_ps(tres3, 0b10110001);
    
    res_imag_add = _mm_add_ps(tres, tres2);
    res_imag_sub = _mm_sub_ps(tres, tres2);
    res1_imag_add = _mm_add_ps(tres1, tres3);
    res1_imag_sub = _mm_sub_ps(tres1, tres3);

    float* res_real = (float*)&tres4;
    float* res_imag = (float*)&res_imag_add;
    float* res1_real = (float*)&tres5;
    float* res1_imag = (float*)&res_imag_sub;
    float* res2_real = (float*)&tres6;
    float* res2_imag = (float*)&res1_imag_add;
    float* res3_real = (float*)&tres7;
    float* res3_imag = (float*)&res1_imag_sub;
    
    amp[indices[0]] = cmplx(res_real[1], res_imag[1]); amp[indices[1]] = cmplx(res_real[0], res_imag[0]);
    amp[indices[2]] = cmplx(res1_real[0], res1_imag[0]); amp[indices[3]] = cmplx(res1_real[1], res1_imag[1]);
    amp[indices[4]] = cmplx(res_real[3], res_imag[3]); amp[indices[5]] = cmplx(res_real[2], res_imag[2]);
    amp[indices[6]] = cmplx(res1_real[2], res1_imag[2]); amp[indices[7]] = cmplx(res1_real[3], res1_imag[3]);
    amp[indices[8]] = cmplx(res2_real[1], res2_imag[1]); amp[indices[9]] = cmplx(res2_real[0], res2_imag[0]);
    amp[indices[10]] = cmplx(res3_real[0], res3_imag[0]); amp[indices[11]] = cmplx(res3_real[1], res3_imag[1]);
    amp[indices[12]] = cmplx(res2_real[3], res2_imag[3]); amp[indices[13]] = cmplx(res2_real[2], res2_imag[2]);
    amp[indices[14]] = cmplx(res3_real[2], res3_imag[2]); amp[indices[15]] = cmplx(res3_real[3], res3_imag[3]);
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
