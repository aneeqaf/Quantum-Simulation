//
//  kernels1.h
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 12/28/17.
//

#ifndef kernels1_h
#define kernels1_h

#include "kernels.h"
#include "kernelsAVX.h"

using namespace std;

__attribute__((always_inline)) inline void
Apply4X12Gate(cmplx* __restrict amp,
              const array<idx_size, 16> indices)
{
//        ApplyXX12Gate( amp, {indices[0], indices[4], indices[8], indices[12]});
//        ApplyXX12Gate( amp, {indices[1], indices[5], indices[9], indices[13]});
//        ApplyXX12Gate( amp, {indices[2], indices[6], indices[10], indices[14]});
//        ApplyXX12Gate( amp, {indices[3], indices[7], indices[11], indices[15]});
//        ApplyXX12Gate( amp, {indices[0], indices[1], indices[2], indices[3]});
//        ApplyXX12Gate( amp, {indices[4], indices[5], indices[6], indices[7]});
//        ApplyXX12Gate( amp, {indices[8], indices[9], indices[10], indices[11]});
//        ApplyXX12Gate( amp, {indices[12], indices[13], indices[14], indices[15]});
}

__attribute__((always_inline)) inline void
Apply4Y12Gate(cmplx* __restrict amp,
              const array<idx_size, 16> indices)
{
//    ApplyYY12Gate( amp, {indices[0], indices[4], indices[8], indices[12]});
//    ApplyYY12Gate( amp, {indices[1], indices[5], indices[9], indices[13]});
//    ApplyYY12Gate( amp, {indices[2], indices[6], indices[10], indices[14]});
//    ApplyYY12Gate( amp, {indices[3], indices[7], indices[11], indices[15]});
//    ApplyYY12Gate( amp, {indices[0], indices[1], indices[2], indices[3]});
//    ApplyYY12Gate( amp, {indices[4], indices[5], indices[6], indices[7]});
//    ApplyYY12Gate( amp, {indices[8], indices[9], indices[10], indices[11]});
//    ApplyYY12Gate( amp, {indices[12], indices[13], indices[14], indices[15]});
}

__attribute__((always_inline)) inline void
ApplyZGateAVX(cmplx* __restrict amp,
              const int num_qubits_amp,
              const idx_size gate_bitmask)
{
    float* __restrict t_amp = (float*)__builtin_assume_aligned(amp, 64);
    
    idx_size amp_size = 2 * (1ull << num_qubits_amp);
    idx_size a = 0;
    for (idx_size i = 0; i < amp_size; i+=8) {
        __m256 temp_amp = _mm256_load_ps (&t_amp[i]);
        
        if (_mm256_movemask_ps(_mm256_cmp_ps(temp_amp, kzeros, _CMP_EQ_OQ)) == 255)
            return;
        
        float neg[4] = {1};
        for (int j = 0; j < 4; ++j) {
            if ((1ull << (a + j)) & gate_bitmask)
                neg[i] = -1;
        }
        
        const __m256 neg_amp = {neg[0], neg[0], neg[1], neg[1], neg[2], neg[2], neg[3], neg[3]};
        temp_amp = _mm256_mul_ps(temp_amp, neg_amp);
        _mm256_store_ps(&t_amp[i], temp_amp);
        a += 4;
    }
}

__attribute__((always_inline)) inline void
ApplyProjection01AVX(cmplx* __restrict amp,
                     const int num_qubits_amp,
                     const idx_size gate_bitmask)
{
    float* __restrict t_amp = (float*)__builtin_assume_aligned(amp, 64);
    
    idx_size amp_size = 2 * (1ull << num_qubits_amp);
    idx_size a = 0;
    for (idx_size i = 0; i < amp_size; i+=8) {
        __m256 temp_amp = _mm256_load_ps (&t_amp[i]);
        
        if (_mm256_movemask_ps(_mm256_cmp_ps(temp_amp, kzeros, _CMP_EQ_OQ)) == 255)
            return;
        
        //Calculate which amps to zero
        float zero[4] = {1};
        bool all_zeros = true;
        for (int j = 0; j < 4; ++j) {
            if (!((1ull << (a + j)) & gate_bitmask))
                zero[i] = 0;
            else
                all_zeros = false;
        }
        if (all_zeros)
            _mm256_store_ps(&t_amp[i], kzeros);
        else {
            const __m256 zero_amp = {zero[0], zero[0], zero[1], zero[1], zero[2], zero[2], zero[3], zero[3]};
            __m256 temp_amp = _mm256_load_ps (&t_amp[i]);
            temp_amp = _mm256_mul_ps(temp_amp, zero_amp);
            _mm256_store_ps(&t_amp[i], temp_amp);
        }
        a += 4;
    }
}

__attribute__((always_inline)) inline void
ApplyProjection10AVX(cmplx* __restrict amp,
                     const int num_qubits_amp,
                     const idx_size gate_bitmask)
{
    float* __restrict t_amp = (float*)__builtin_assume_aligned(amp, 64);
    
    idx_size amp_size = 2 * (1ull << num_qubits_amp);
    idx_size a = 0;
    for (idx_size i = 0; i < amp_size; i+=8) {
        __m256 temp_amp = _mm256_load_ps (&t_amp[i]);
        
        if (_mm256_movemask_ps(_mm256_cmp_ps(temp_amp, kzeros, _CMP_EQ_OQ)) == 255)
            return;
        
        //Calculate which amps to zero
        float zero[4] = {1};
        bool all_zeros = true;
        for (int j = 0; j < 4; ++j) {
            if ((1ull << (a + j)) & gate_bitmask)
                zero[i] = 0;
            else
                all_zeros = false;
        }
        if (all_zeros)
            _mm256_store_ps(&t_amp[i], kzeros);
        else {
            const __m256 zero_amp = {zero[0], zero[0], zero[1], zero[1], zero[2], zero[2], zero[3], zero[3]};
            __m256 temp_amp = _mm256_load_ps (&t_amp[i]);
            temp_amp = _mm256_mul_ps(temp_amp, zero_amp);
            _mm256_store_ps(&t_amp[i], temp_amp);
        }
        a += 4;
    }
}


inline valarray<idx_size>
FindStrides (const vector<int>& gate_qubits,
             const int num_q) {
    
    valarray<idx_size> strides(gate_qubits.size());
    for (idx_size i = 0; i < strides.size(); ++i)
        strides[i] = (1ull << ((num_q - 1) - gate_qubits[i]));
    
    return strides;
}

void
ApplyNonControl1QGates(cmplx* __restrict amp,
                       const int q,
                       const int num_qubits_amp,
                       const Gate::Type gate_type,
                       const Gate& g = {});

//TODO: Test this
void
ApplyControlGate(cmplx* __restrict amp,
                 const int num_controls,
                 const vector<int>& gate_qubits,
                 const int num_qubits_amp,
                 const Gate& g,
                 const Gate::Type gate_type);

void
ApplyManyXOnSlice(cmplx* __restrict amp,
                  const idx_size num_qbits);

void
ApplyManyYOnSlice(cmplx* __restrict amp,
                  const idx_size num_qbits);

void
ApplyFWHT(cmplx* __restrict amp,
          idx_size qubits_in_cluster,
          const int num_qubits_amp,
          const Gate::Type gate_type);

#endif /* kernels1_h */
