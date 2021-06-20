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
ApplyHGate(cmplx* __restrict amp,
             const idx_size* indices)
{
    cmplx temp_amp[2] = {amp[indices[0]], amp[indices[1]]};
    
    if (amp[indices[0]] == cmplx(0,0) && amp[indices[1]] == cmplx(0,0))
        return;
    
    amp[indices[0]] = temp_amp[0] + temp_amp[1];
    amp[indices[1]] = temp_amp[0] - temp_amp[1];
}

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
ApplyHHGate(cmplx* __restrict amp,
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
    
    amp[indices[0]] = t0 + t2;
    amp[indices[1]] = t1 - t3;
    amp[indices[2]] = t1 + t3;
    amp[indices[3]] = t0 - t2;
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

__attribute__((always_inline)) inline void
ApplyYY12HHGate(cmplx* __restrict amp,
                const idx_size* indices /*4*/)
{    
    if (amp[indices[0]] == cmplx(0,0) && amp[indices[1]] == cmplx(0,0)
        && amp[indices[2]] == cmplx(0,0) && amp[indices[3]] == cmplx(0,0))
        return;
    
    amp[indices[1]] = -amp[indices[1]];
    amp[indices[2]] = -amp[indices[2]];
}

__attribute__((always_inline)) inline void
ApplyXX12HHGate(cmplx* __restrict amp,
                const idx_size* indices /*4*/)
{
    cmplx a[4] = {amp[indices[0]], amp[indices[1]], amp[indices[2]], amp[indices[3]]};
    
    if (amp[indices[0]] == cmplx(0,0) && amp[indices[1]] == cmplx(0,0)
        && amp[indices[2]] == cmplx(0,0) && amp[indices[3]] == cmplx(0,0))
        return;
    
    auto t0 = a[0] + a[3];
    auto t1 = a[1] + a[2];
    auto t2 = ki * (a[0] - a[3]);
    auto t3 = ki * (a[1] - a[2]);

    amp[indices[0]] = t1 + t0;
    amp[indices[1]] = t2 - t3;
    amp[indices[2]] = t2 + t3;
    amp[indices[3]] = t1 - t0;
}

__attribute__((always_inline)) inline void
ApplyXY12HHGate(cmplx* __restrict amp,
                const idx_size* indices /*4*/)
{
    cmplx a[4] = { amp[indices[0]], amp[indices [1]], amp[indices[2]], amp[indices[3]]};
    
    if (amp[indices[0]] == cmplx(0,0) && amp[indices[1]] == cmplx(0,0)
        && amp[indices[2]] == cmplx(0,0) && amp[indices[3]] == cmplx(0,0))
        return;
    
    auto t0 = (ki * a[0]) + a[2];
    auto t1 = a[0] + (ki * a[2]);
    auto t2 = (-ki * a[1]) - a[3];
    auto t3 = a[1] + (ki * a[3]);

    amp[indices[0]] = t0 + t1;
    amp[indices[1]] = t2 - t3;
    amp[indices[2]] = t0 - t1;
    amp[indices[3]] = t3 + t2;
}

__attribute__((always_inline)) inline void
ApplyYX12HHGate(cmplx* __restrict amp,
                const idx_size* indices /*4*/)
{
    cmplx a[4] = { amp[indices[0]], amp[indices [1]], amp[indices[2]], amp[indices[3]]};
    
    if (amp[indices[0]] == cmplx(0,0) && amp[indices[1]] == cmplx(0,0)
        && amp[indices[2]] == cmplx(0,0) && amp[indices[3]] == cmplx(0,0))
        return;
    
    auto t0 = (ki * a[0]) + a[1];
    auto t1 = a[0] + (ki * a[1]);
    auto t2 = (-ki * a[2]) - a[3];
    auto t3 = a[2] + (ki * a[3]);
    
    amp[indices[0]] = t0 + t1;
    amp[indices[1]] = t0 - t1 ;
    amp[indices[2]] = t2 - t3;
    amp[indices[3]] = t3 + t2;
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
            case (Gate::Type::x_1_2):{
                amp[indices[0]] = (temp_amp[0]*X12[0][0]) + (temp_amp[1]*X12[0][1]);
                amp[indices[1]] = (temp_amp[0]*X12[1][0]) + (temp_amp[1]*X12[1][1]);
                break;
            }
            case (Gate::Type::y_1_2):{
                amp[indices[0]] = (temp_amp[0]*Y12[0][0]) + (temp_amp[1]*Y12[0][1]);
                amp[indices[1]] = (temp_amp[0]*Y12[1][0]) + (temp_amp[1]*Y12[1][1]);
                break;
            }
            case (Gate::Type::z):{
                amp[indices[1]] *= -1;
                break;
            }
            case (Gate::Type::cz_d1):{
                amp[indices[1]] *= -1;
                break;
            }
            case (Gate::Type::cz_d2):{
                amp[indices[0]] = 0;
                break;
            }
            case (Gate::Type::cz_d3):{
                break;
            }
            case (Gate::Type::cz_d4):{
                amp[indices[1]] = 0;
                break;
            }
            case (Gate::Type::t):{
                amp[indices[1]] *= T[1][1];
                break;
            }
            case (Gate::Type::x):{
                swap(amp[indices[0]], amp[indices[1]]);
                break;
            }
            case (Gate::Type::y):{
                amp[indices[0]] = temp_amp[1] * Y[0][1];
                amp[indices[1]] = temp_amp[0] * Y[1][0];
                break;
            }
            default: {
                vector<cmplx> m_temp(temp_amp, temp_amp + sizeof(temp_amp) / sizeof(temp_amp[0]));
                m_temp = matrix_v_mult(q_gate.GetMatrix(), m_temp);
                for (idx_size i = 0; i < indices_size; ++i)
                    amp[indices[i]] = temp_amp[i];
                break;
            }
        }
    }
    else {
        vector<cmplx> m_temp(temp_amp, temp_amp + sizeof(temp_amp) / sizeof(temp_amp[0]));
        m_temp = matrix_v_mult(q_gate.GetMatrix(), m_temp);
        for (idx_size i = 0; i < indices_size; ++i)
            amp[indices[i]] = temp_amp[i];
    }
}

template<typename function>
void
Apply2MergedGatesHelper(cmplx* __restrict amp,
                        const idx_size gate_qubits,
                        const int num_qubits_amp,
                        const function& gate_func,
                        const idx_size add = 1);

void
ApplyHighQ2MergedGatesInParallel(cmplx* __restrict amp,
                                const int num_threads,
                                const idx_size gate_qubits,
                                const int num_qubits_amp,
                                void (*gate_func)(cmplx*, const idx_size*),
                                const idx_size add = 1);

__attribute__((always_inline)) inline int
ApplyMergedXYGates(cmplx* __restrict amp,
                   int gate_type,
                   const idx_size gates_bitmask,
                   const int num_qubits,
                   const int num_threads = 0,
                   const bool HHGate = false,
                   const bool parallel = false)
{
    if (HHGate)
        gate_type += 4;

    static void (*XYApplicationFuncs[8])(cmplx* , const idx_size*) = {ApplyXX12Gate, ApplyXY12Gate,
        ApplyYY12Gate, ApplyYX12Gate, ApplyXX12HHGate, ApplyXY12HHGate, ApplyYY12HHGate, ApplyYX12HHGate};
    static void (*XYApplicationAVXFuncs[8])(cmplx* , const idx_size*) = {ApplyXX12GateAVX, ApplyXY12GateAVX,
        ApplyYY12GateAVX, ApplyYX12GateAVX, ApplyXX12HHGateAVX, ApplyXY12HHGateAVX,
        ApplyYY12HHGateAVX, ApplyYX12HHGateAVX};
    
    int num_gates_collected = (__builtin_ctzl(gates_bitmask) < num_qubits - 1
                && __builtin_ctzl(gates_bitmask ^ (1ull << __builtin_ctzl(gates_bitmask))) < num_qubits - 2) ? 4 : 1;
    void (**XYApplicationToUse)(cmplx* , const idx_size*) = num_gates_collected == 4 ? XYApplicationAVXFuncs : XYApplicationFuncs;
    
    if (parallel)
        ApplyHighQ2MergedGatesInParallel(amp, num_threads, gates_bitmask, num_qubits,
                                XYApplicationToUse[gate_type], num_gates_collected);
    else
        Apply2MergedGatesHelper(amp, gates_bitmask, num_qubits,
                            XYApplicationToUse[gate_type], num_gates_collected);

    return gate_type != 6 ? 0 : -2;
}

void
GroupCZGates(idx_size* __restrict qubits_CZ_bitmasks,
             const int num_qubits_amp,
             const vector<idx_size>& gate_qubits);

void
GroupTGates(idx_size* __restrict T_bitmasks,
            const int num_qubits_amp,
            const vector<idx_size>& gate_qubits);

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

vector<idx_size>
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
Apply1QXYHGates(cmplx* __restrict amp,
                const int num_threads,
                const int q,
                const int num_qubits,
                const Gate::Type gate_type);

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


pair<idx_size, int>
XYFastTransform(cmplx* __restrict amp,
                idx_size X_bitmask,
                idx_size Y_bitmask,
                const int num_qubits,
                const int num_threads,
                const int th);

pair<idx_size, int>
ApplyXYHIterativelyInParallel(cmplx* __restrict amp,
                             idx_size X_bitmask,
                             idx_size Y_bitmask,
                             idx_size H_bitmask,
                             const int num_qubits,
                              const int num_threads);

void ApplyHGatesRecursively(cmplx* __restrict amp,
                            int num_qubits,
                            int num_threads,
                            idx_size gate_bm);

void ApplyHighHGatesIterativelyInParallel(cmplx* __restrict amp,
                                            int num_qubits,
                                            int num_threads,
                                            idx_size gate_bm);

pair<idx_size, int>
ApplyHighXYHGatesByBitReversal(cmplx* __restrict amp,
                               idx_size X_bitmask,
                               idx_size Y_bitmask,
                               idx_size H_bitmask,
                               const int num_qubits,
                               const int num_low_qubits,
                               const int num_threads);

//void ApplyHGatesRecursively(cmplx* __restrict amp,
//                            int num_qubits,
//                            int num_threads,
//                            int current_q);

#endif /* kernels_h */
