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
//    ApplyXX12Gate( amp, {indices[0], indices[4], indices[8], indices[12]});
//    ApplyXX12Gate( amp, {indices[1], indices[5], indices[9], indices[13]});
//    ApplyXX12Gate( amp, {indices[2], indices[6], indices[10], indices[14]});
//    ApplyXX12Gate( amp, {indices[3], indices[7], indices[11], indices[15]});
//    ApplyXX12Gate( amp, {indices[0], indices[1], indices[2], indices[3]});
//    ApplyXX12Gate( amp, {indices[4], indices[5], indices[6], indices[7]});
//    ApplyXX12Gate( amp, {indices[8], indices[9], indices[10], indices[11]});
//    ApplyXX12Gate( amp, {indices[12], indices[13], indices[14], indices[15]});
//    
    const cmplx a[16] = {amp[indices[0]], amp[indices[1]], amp[indices[2]], amp[indices[3]],
        amp[indices[4]], amp[indices[5]], amp[indices[6]], amp[indices[7]],
        amp[indices[8]], amp[indices[9]], amp[indices[10]], amp[indices[11]],
        amp[indices[12]], amp[indices[13]], amp[indices[14]], amp[indices[15]]
    };
    auto t = a[0] + a[12];
    auto t1 = a[4] + a[8];
    auto t2 = ki * (a[0] - a[12]);
    auto t3 = ki * (a[4] - a[8]);
    
    amp[indices[0]] = t1 + t2;
    amp[indices[4]] = t + t3;
    amp[indices[8]] = t - t3;
    amp[indices[12]] = t1 - t2;
    
    t = a[1] + a[13];
    t1 = a[5] + a[9];
    t2 = ki * (a[1] - a[13]);
    t3 = ki * (a[5] - a[9]);
    
    amp[indices[1]] = t1 + t2;
    amp[indices[5]] = t + t3;
    amp[indices[9]] = t - t3;
    amp[indices[13]] = t1 - t2;
    
    t = a[2] + a[14];
    t1 = a[6] + a[10];
    t2 = ki * (a[2] - a[14]);
    t3 = ki * (a[6] - a[10]);
    
    amp[indices[2]] = t1 + t2;
    amp[indices[6]] = t + t3;
    amp[indices[10]] = t - t3;
    amp[indices[14]] = t1 - t2;
    
    t = a[3] + a[15];
    t1 = a[7] + a[11];
    t2 = ki * (a[3] - a[15]);
    t3 = ki * (a[7] - a[11]);
    
    amp[indices[3]] = t1 + t2;
    amp[indices[7]] = t + t3;
    amp[indices[11]] = t - t3;
    amp[indices[15]] = t1 - t2;
    
    
    const cmplx b[16] = {amp[indices[0]], amp[indices[1]], amp[indices[2]], amp[indices[3]],
        amp[indices[4]], amp[indices[5]], amp[indices[6]], amp[indices[7]],
        amp[indices[8]], amp[indices[9]], amp[indices[10]], amp[indices[11]],
        amp[indices[12]], amp[indices[13]], amp[indices[14]], amp[indices[15]]
    };
    
     t = b[0] + b[3];
     t1 = b[1] + b[2];
     t2 = ki * (b[0] - b[3]);
     t3 = ki * (b[1] - b[2]);
    
    amp[indices[0]] = t1 + t2;
    amp[indices[1]] = t + t3;
    amp[indices[2]] = t - t3;
    amp[indices[3]] = t1 - t2;
    
    t = b[4] + b[7];
    t1 = b[5] + b[6];
    t2 = ki * (b[4] - b[7]);
    t3 = ki * (b[5] - b[6]);
    
    amp[indices[4]] = t1 + t2;
    amp[indices[5]] = t + t3;
    amp[indices[6]] = t - t3;
    amp[indices[7]] = t1 - t2;
    
    t = b[8] + b[11];
    t1 = b[9] + b[10];
    t2 = ki * (b[8] - b[11]);
    t3 = ki * (b[9] - b[10]);
    
    amp[indices[8]] = t1 + t2;
    amp[indices[9]] = t + t3;
    amp[indices[10]] = t - t3;
    amp[indices[11]] = t1 - t2;
    
    t = b[12] + b[15];
    t1 = b[13] + b[14];
    t2 = ki * (b[12] - b[15]);
    t3 = ki * (b[13] - b[14]);
    
    amp[indices[12]] = t1 + t2;
    amp[indices[13]] = t + t3;
    amp[indices[14]] = t - t3;
    amp[indices[15]] = t1 - t2;
    
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
