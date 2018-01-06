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
