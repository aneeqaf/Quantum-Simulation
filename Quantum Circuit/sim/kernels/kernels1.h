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
Apply4X12GateHelper(__m256& res_real_add,
                    __m256& res_imag_add,
                    __m256& res_real_sub,
                    __m256& res_imag_sub,
                    const cmplx* __restrict a)
{
    __m256 t_real = _mm256_setr_ps(real(a[0]), real(a[1]), real(a[4]), real(a[5]),
                                   real(a[8]), real(a[9]), real(a[12]), real(a[13]));
    __m256 t1_real = _mm256_setr_ps(real(a[3]), real(a[2]), real(a[7]), real(a[6]),
                                    real(a[11]), real(a[10]), real(a[15]), real(a[14]));
    __m256 t_imag = _mm256_setr_ps(imag(a[0]), imag(a[1]), imag(a[4]), imag(a[5]),
                                   imag(a[8]), imag(a[9]), imag(a[12]), imag(a[13]));
    __m256 t1_imag = _mm256_setr_ps(imag(a[3]), imag(a[2]), imag(a[7]), imag(a[6]),
                                    imag(a[11]), imag(a[10]), imag(a[15]), imag(a[14]));
    
    __m256 tres_real_add = _mm256_add_ps(t_real, t1_real);
    __m256 tres_imag_sub = _mm256_sub_ps(t_real, t1_real);
    __m256 tres_imag_add = _mm256_add_ps(t_imag, t1_imag);
    __m256 tres_real_sub = _mm256_sub_ps(t1_imag, t_imag);
    tres_real_sub = _mm256_permute_ps(tres_real_sub, 0b10110001);
    tres_imag_sub = _mm256_permute_ps(tres_imag_sub, 0b10110001);
    
    res_real_add = _mm256_add_ps(tres_real_add, tres_real_sub);
    res_real_sub = _mm256_sub_ps(tres_real_add, tres_real_sub);
    res_imag_add = _mm256_add_ps(tres_imag_add, tres_imag_sub);
    res_imag_sub = _mm256_sub_ps(tres_imag_add, tres_imag_sub);
}

__attribute__((always_inline)) inline void
Apply4X12Gate(cmplx* __restrict amp,
              const array<idx_size, 16> indices)
{
//    ApplyXX12Gate( amp, {indices[0], indices[4], indices[8], indices[12]});
//    ApplyXX12Gate( amp, {indices[1], indices[5], indices[9], indices[13]});
//    ApplyXX12Gate( amp, {indices[2], indices[6], indices[10], indices[14]});
//    ApplyXX12Gate( amp, {indices[3], indices[7], indices[11], indices[15]});

//    ApplyXX12Gate( amp, {indices[0], indices[1], indices[2], indices[3]});
//    ApplyXX12Gate( amp, {indices[4], indices[5], indices[6], indices[7]});
//    ApplyXX12Gate( amp, {indices[8], indices[9], indices[10], indices[11]});
//    ApplyXX12Gate( amp, {indices[12], indices[13], indices[14], indices[15]});
    
    __m256 res_real_add, res_imag_add, res_real_sub, res_imag_sub;

    const cmplx a[16] = {amp[indices[0]], amp[indices[4]],  amp[indices[8]], amp[indices[12]],
                        amp[indices[1]], amp[indices[5]],  amp[indices[9]], amp[indices[13]],
                        amp[indices[2]], amp[indices[6]],  amp[indices[10]], amp[indices[14]],
                        amp[indices[3]], amp[indices[7]], amp[indices[11]],  amp[indices[15]]};

    Apply4X12GateHelper(res_real_add, res_imag_add, res_real_sub, res_imag_sub, a);

    float* res_real = (float*)&res_real_add;
    float* res_imag = (float*)&res_imag_add;
    float* res1_real = (float*)&res_real_sub;
    float* res1_imag = (float*)&res_imag_sub;

    amp[indices[0]] = cmplx(res_real[1], res_imag[1]); amp[indices[4]] = cmplx(res_real[0], res_imag[0]);
    amp[indices[8]] = cmplx(res1_real[0], res1_imag[0]); amp[indices[12]] = cmplx(res1_real[1], res1_imag[1]);
    amp[indices[1]] = cmplx(res_real[3], res_imag[3]); amp[indices[5]] = cmplx(res_real[2], res_imag[2]);
    amp[indices[9]] = cmplx(res1_real[2], res1_imag[2]); amp[indices[13]] = cmplx(res1_real[3], res1_imag[3]);
    amp[indices[2]] = cmplx(res_real[5], res_imag[5]); amp[indices[6]] = cmplx(res_real[4], res_imag[4]);
    amp[indices[10]] = cmplx(res1_real[4], res1_imag[4]); amp[indices[14]] = cmplx(res1_real[5], res1_imag[5]);
    amp[indices[3]] = cmplx(res_real[7], res_imag[7]); amp[indices[7]] = cmplx(res_real[6], res_imag[6]);
    amp[indices[11]] = cmplx(res1_real[6], res1_imag[6]); amp[indices[15]] = cmplx(res1_real[7], res1_imag[7]);
    
    const cmplx b[16] = {amp[indices[0]], amp[indices[1]],  amp[indices[2]], amp[indices[3]],
                        amp[indices[4]], amp[indices[5]], amp[indices[6]], amp[indices[7]],
                        amp[indices[8]], amp[indices[9]], amp[indices[10]], amp[indices[11]],
                        amp[indices[12]], amp[indices[13]], amp[indices[14]], amp[indices[15]]};

     Apply4X12GateHelper(res_real_add, res_imag_add, res_real_sub, res_imag_sub, b);

    res_real = (float*)&res_real_add;
    res_imag = (float*)&res_imag_add;
    res1_real = (float*)&res_real_sub;
    res1_imag = (float*)&res_imag_sub;

    amp[indices[0]] = cmplx(res_real[1], res_imag[1]); amp[indices[1]] = cmplx(res_real[0], res_imag[0]);
    amp[indices[2]] = cmplx(res1_real[0], res1_imag[0]); amp[indices[3]] = cmplx(res1_real[1], res1_imag[1]);
    amp[indices[4]] = cmplx(res_real[3], res_imag[3]); amp[indices[5]] = cmplx(res_real[2], res_imag[2]);
    amp[indices[6]] = cmplx(res1_real[2], res1_imag[2]); amp[indices[7]] = cmplx(res1_real[3], res1_imag[3]);
    amp[indices[8]] = cmplx(res_real[5], res_imag[5]); amp[indices[9]] = cmplx(res_real[4], res_imag[4]);
    amp[indices[10]] = cmplx(res1_real[4], res1_imag[4]); amp[indices[11]] = cmplx(res1_real[5], res1_imag[5]);
    amp[indices[12]] = cmplx(res_real[7], res_imag[7]); amp[indices[13]] = cmplx(res_real[6], res_imag[6]);
    amp[indices[14]] = cmplx(res1_real[6], res1_imag[6]); amp[indices[15]] = cmplx(res1_real[7], res1_imag[7]);
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
