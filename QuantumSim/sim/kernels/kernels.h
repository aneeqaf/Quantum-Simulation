//
//  kernels.h
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 12/11/17.
//

#ifndef kernals_h
#define kernals_h

#include "kernelsAVX.h"

using namespace std;

__attribute__((always_inline)) inline void
ApplyX12Gate(cmplx* __restrict amp,
              const idx_size* indices)
{
    cmplx temp_amp[2] = {amp[indices[0]], amp[indices[1]]};
    
    if (amp[indices[0]] == cmplx(0,0) && amp[indices[1]] == cmplx(0,0))
        return;
    
    amp[indices[0]] = (temp_amp[0]*X12[0][0]) + (temp_amp[1]*X12[0][1]);
    amp[indices[1]] = (temp_amp[0]*X12[1][0]) + (temp_amp[1]*X12[1][1]);
}

__attribute__((always_inline)) inline void
ApplyY12Gate(cmplx* __restrict amp,
             const idx_size* indices)
{
    cmplx temp_amp[2] = {amp[indices[0]], amp[indices[1]]};
    
    if (amp[indices[0]] == cmplx(0,0) && amp[indices[1]] == cmplx(0,0))
        return;
    
    amp[indices[0]] = (temp_amp[0]*Y12[0][0]) + (temp_amp[1]*Y12[0][1]);
    amp[indices[1]] = (temp_amp[0]*Y12[1][0]) + (temp_amp[1]*Y12[1][1]);
}
              
__attribute__((always_inline)) inline void
ApplyXX12Gate(cmplx* __restrict amp,
              const idx_size* indices /*4*/)
{
    const cmplx a[4] = {amp[indices[0]], amp[indices[1]], amp[indices[2]], amp[indices[3]]};
    
    if (amp[indices[0]] == cmplx(0,0) && amp[indices[1]] == cmplx(0,0)
        && amp[indices[2]] == cmplx(0,0) && amp[indices[3]] == cmplx(0,0))
        return;
    
    const auto t0 = a[0] + a[3];
    const auto t1 = a[1] + a[2];
    const auto t2 = ki * (a[0] - a[3]);
    const auto t3 = ki * (a[1] - a[2]);

    amp[indices[0]] = t1 + t2;
    amp[indices[1]] = t0 + t3;
    amp[indices[2]] = t0 - t3;
    amp[indices[3]] = t1 - t2;
}

__attribute__((always_inline)) inline void
ApplyYY12Gate(cmplx* __restrict amp,
              const idx_size* indices /*4*/)
{
    const cmplx a[4] = {amp[indices[0]], amp[indices [1]], amp[indices[2]], amp[indices[3]]};
    
    if (amp[indices[0]] == cmplx(0,0) && amp[indices[1]] == cmplx(0,0)
        && amp[indices[2]] == cmplx(0,0) && amp[indices[3]] == cmplx(0,0))
        return;
    
    const auto t0 = a[0] + a[3];
    const auto t1 = a[0] - a[3];
    const auto t2 = a[1] + a[2];
    const auto t3 = a[1] - a[2];
    
    amp[indices[0]] = t0 - t2;
    amp[indices[1]] = t1 + t3;
    amp[indices[2]] = t1 - t3;
    amp[indices[3]] = t0 + t2;
}

__attribute__((always_inline)) inline void
ApplyXY12Gate(cmplx* __restrict amp,
              const idx_size* indices /*4*/)
{
    const cmplx a[4] = { amp[indices[0]], amp[indices [1]], amp[indices[2]], amp[indices[3]]};
    
    if (amp[indices[0]] == cmplx(0,0) && amp[indices[1]] == cmplx(0,0)
        && amp[indices[2]] == cmplx(0,0) && amp[indices[3]] == cmplx(0,0))
        return;
    
    const auto t0 = a[0] - a[1];
    const auto t1 = a[2] - a[3];
    const auto t2 = a[0] + a[1];
    const auto t3 = a[2] + a[3];

    amp[indices[0]] = (ki * t0) + t1;
    amp[indices[1]] = (ki * t2) + t3;
    amp[indices[2]] = t0 + (ki * t1);
    amp[indices[3]] = t2 + (ki * t3);
}

__attribute__((always_inline)) inline void
ApplyYX12Gate(cmplx* __restrict amp,
              const idx_size* indices /*4*/)
{
    const cmplx a[4] = { amp[indices[0]], amp[indices [1]], amp[indices[2]], amp[indices[3]]};
    
    if (amp[indices[0]] == cmplx(0,0) && amp[indices[1]] == cmplx(0,0)
        && amp[indices[2]] == cmplx(0,0) && amp[indices[3]] == cmplx(0,0))
        return;
    
    const auto t0 = (ki * a[0]) + a[1];
    const auto t1 = a[0] + (ki * a[1]);
    const auto t2 = (ki * a[2]) + a[3];
    const auto t3 = a[2] + (ki * a[3]);
    
    amp[indices[0]] = t0 - t2;
    amp[indices[1]] = t1 - t3;
    amp[indices[2]] = t0 + t2;
    amp[indices[3]] = t1 + t3;
}

inline void
ApplyGateOnAmps(cmplx* __restrict amp,
                const idx_size* indices,
                const idx_size indices_size,
                const Gate::Type gate_type,
                const Gate& q_gate)
{
    cmplx temp_amp[indices_size];
    for (idx_size i = 0; i < indices_size; ++i)
        temp_amp[i] = amp[indices[i]];
    
    if (indices_size <= 2 && !(amp[indices[0]] == cmplx(0,0) && amp[indices[1]] == cmplx(0,0))) {
        switch (gate_type) {
            case (Gate::Type::X_1_2):{
                amp[indices[0]] = (temp_amp[0]*X12[0][0]) + (temp_amp[1]*X12[0][1]);
                amp[indices[1]] = (temp_amp[0]*X12[1][0]) + (temp_amp[1]*X12[1][1]);
                break;
            }
            case (Gate::Type::Y_1_2):{
                amp[indices[0]] = (temp_amp[0]*Y12[0][0]) + (temp_amp[1]*Y12[0][1]);
                amp[indices[1]] = (temp_amp[0]*Y12[1][0]) + (temp_amp[1]*Y12[1][1]);
                break;
            }
            case (Gate::Type::Z):{
                amp[indices[1]] *= -1;
                break;
            }
            case (Gate::Type::CZ_D1):{
                amp[indices[1]] *= -1;
                break;
            }
            case (Gate::Type::CZ_D2):{
                amp[indices[0]] = 0;
                break;
            }
            case (Gate::Type::CZ_D3):{
                break;
            }
            case (Gate::Type::CZ_D4):{
                amp[indices[1]] = 0;
                break;
            }
            case (Gate::Type::T):{
                amp[indices[1]] *= T[1][1];
                break;
            }
            case (Gate::Type::X):{
                swap(amp[indices[0]], amp[indices[1]]);
                break;
            }
            case (Gate::Type::Y):{
                amp[indices[0]] = temp_amp[1] * Y[0][1];
                amp[indices[1]] = temp_amp[0] * Y[1][0];
                break;
            }
            default: {
                vector<cmplx> m_temp(temp_amp, temp_amp + sizeof(temp_amp) / sizeof(temp_amp[0]));
                m_temp = matrix_v_mult(q_gate.rows, m_temp);
                for (idx_size i = 0; i < indices_size; ++i)
                    amp[indices[i]] = temp_amp[i];
                break;
            }
        }
    }
    else {
        vector<cmplx> m_temp(temp_amp, temp_amp + sizeof(temp_amp) / sizeof(temp_amp[0]));
        m_temp = matrix_v_mult(q_gate.rows, m_temp);
        for (idx_size i = 0; i < indices_size; ++i)
            amp[indices[i]] = temp_amp[i];
    }
}

void
GroupCZGates(idx_size* __restrict qubits_CZ_bitmasks,
             const int num_qubits_amp,
             const vector<int>& gate_qubits);

void
GroupTGates(idx_size* __restrict T_bitmasks,
            const int num_qubits_amp,
            const vector<int>& gate_qubits);

void
ExtractIndicesForAmp(idx_size* strides,
                     idx_size gate_qubits,
                     const int num_qubits_amp,
                     const idx_size starting_idx = 0);

void
FormBlockOfCZTGates(idx_size& gate_i,
                    bitset<128>* __restrict CZ_bitmasks,
                    bitset<128>* __restrict T_bitmasks,
                    const vector<Gate>& cluster,
                    const int num_qubits_amp);

vector<int>
FormBlockOfXYHGates(idx_size& gate_i,
                    const Gate::Type gate_type,
                    const vector<Gate>& all_gates);

void
FormBlockOfXYHGates(vector<Gate>& cluster,
                    idx_size& gate_i,
                    const Gate::Type gate_type,
                    const vector<Gate>& all_gates);
void
ApplyBlockOfCZTGates(cmplx* __restrict amp,
                     const int num_qubits_amp,
                     const idx_size* __restrict CZ_bitmasks,
                     const idx_size* __restrict T_bitmasks);

void
ApplyCZDecomposition(cmplx* __restrict amp,
                     const int num_qubits_amp,
                     const int gate_qubit,
                     const Gate::Type gate_type);

void
Apply1QXYGates(cmplx* __restrict amp,
               const int q,
               const int num_qubits,
               const Gate::Type gate_type,
               const int num_threads);

void
Apply2MergedXY12Gates(Gate gate1,
                      Gate gate2,
                      cmplx* __restrict amp,
                      const int num_qubits_amp);

idx_size
XYFastTransformIterative(cmplx* __restrict amp,
                         idx_size X_bitmask,
                         idx_size Y_bitmask,
                         const int num_qubits,
                         const int num_threads,
                         const int th);

idx_size
ApplyHighQXYGates(cmplx* __restrict amp,
                  idx_size& X_bitmask,
                  idx_size& Y_bitmask,
                  const int num_qubits);

idx_size
XYFastTransform(cmplx* __restrict amp,
                idx_size X_bitmask,
                idx_size Y_bitmask,
                const int num_qubits,
                const int num_threads,
                const int th = 16);

#endif /* kernels_h */
