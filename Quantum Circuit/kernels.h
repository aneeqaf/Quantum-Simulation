//
//  kernels.h
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 12/11/17.
//

#ifndef kernals_h
#define kernals_h

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdlib>
#include <complex>
#include <cstring>
#include <ctime>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <memory>
#include <stdalign.h>
#include <stdio.h>
#include <stdlib.h>
#include <valarray>
#include <vector>
#include <unistd.h>
#include <utility>

#include "kernelsAVX.h"
#include "matrix.h"
#include "gates.h"

using namespace std;

constexpr float kH = 0.707106781;

using idx_size = size_t;

constexpr cmplx kTGate[8] = {1, {kH, kH}, {0,1}, {-kH, kH},
    -1, {-kH, -kH}, {0, -1}, {kH, -kH}};
constexpr cmplx kSqrtCZGate[4] = {1, {0,1}, -1, {0,-1}};
constexpr cmplx ki = {0,1};


__attribute__((always_inline)) inline void
ApplyXX12Gate(const array<idx_size, 4> indices,
              cmplx* __restrict amp)
{
    const cmplx a[4] = {amp[indices[0]], amp[indices[1]], amp[indices[2]], amp[indices[3]]};
    const auto t = a[0] + a[3];
    const auto t1 = a[1] + a[2];
    const auto t2 = ki * (a[0] - a[3]);
    const auto t3 = ki * (a[1] - a[2]);

    amp[indices[0]] = t1 + t2;
    amp[indices[1]] = t + t3;
    amp[indices[2]] = t - t3;
    amp[indices[3]] = t1 - t2;
}

__attribute__((always_inline)) inline void
ApplyXY12Gate(const array<idx_size, 4> indices,
              cmplx* __restrict amp)
{
    const cmplx a[4] = { amp[indices[0]], amp[indices [1]], amp[indices[2]], amp[indices[3]]};
    const auto t = a[0] - a[1];
    const auto t1 = a[2] - a[3];
    const auto t2 = a[0] + a[1];
    const auto t3 = a[2] + a[3];

    amp[indices[0]] = (ki * t) + t1;
    amp[indices[1]] = (ki * t2) + t3;
    amp[indices[2]] = t + (ki * t1);
    amp[indices[3]] = t2 + (ki * t3);
}

__attribute__((always_inline)) inline void
ApplyYY12Gate(const array<idx_size, 4> indices,
              cmplx* __restrict amp)
{
    cmplx a[4] = { amp[indices[0]], amp[indices [1]], amp[indices[2]], amp[indices[3]]};
    auto t = ki * (a[0] + a[3]);
    auto t1 = ki * (a[0] - a[3]);
    auto t2 = ki * (a[1] + a[2]);
    auto t3 = ki * (a[1] - a[2]);

    amp[indices[0]] = t - t2;
    amp[indices[1]] = t1 + t3;
    amp[indices[2]] = t1 - t3;
    amp[indices[3]] = t + t2;
}

__attribute__((always_inline)) inline void
ApplyYX12Gate(const array<idx_size, 4> indices,
              cmplx* __restrict amp)
{
    cmplx a[4] = { amp[indices[0]], amp[indices [1]], amp[indices[2]], amp[indices[3]]};
    auto t = (ki * a[0]) + a[1];
    auto t1 = a[0] + (ki * a[1]);
    auto t2 = (ki * a[2]) + a[3];
    auto t3 = a[2] + (ki * a[3]);
    
    amp[indices[0]] = t - t2;
    amp[indices[1]] = t1 - t3;
    amp[indices[2]] = t + t2;
    amp[indices[3]] = t1 + t3;
}

template <typename function>
__attribute__((always_inline)) inline void
Apply4YX12Gate(cmplx*  __restrict amp_slice,
               const function& gate_func1,
               const function& gate_func2)
{
    gate_func1({0, 4, 8, 12}, amp_slice);
    gate_func1({1, 5, 9, 13}, amp_slice);
    gate_func1({2, 6, 10, 14}, amp_slice);
    gate_func1({3, 7, 11, 15}, amp_slice);
    
    gate_func2({0, 1, 2, 3}, amp_slice);
    gate_func2({4, 5, 6, 7}, amp_slice);
    gate_func2({8, 9, 10, 11}, amp_slice);
    gate_func2({12, 13, 14, 15}, amp_slice);
}

inline void
ApplyGateOnAmps(const idx_size* indices,
                const idx_size indices_size,
                const Gate::Type gate_type,
                const Gate& q_gate,
                cmplx* __restrict amp)
{
    cmplx temp_amp[indices_size];
    for (idx_size i = 0; i < indices_size; ++i)
        temp_amp[i] = amp[indices[i]];
    
    if (indices_size >= 2 && !(amp[indices[0]] == cmplx(0,0) && amp[indices[1]] == cmplx(0,0))) {
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
GroupCZGates(valarray<idx_size>& qubits_CZ_bitmasks,
             const int total_circuit_qubits,
             const vector<int>& gate_qubits);

void
GroupTGates(array<idx_size, 2>& T_bitmasks,
            const int total_circuit_qubits,
            const vector<int>& gate_qubits);

void
ExtractIndicesForAmp(idx_size* strides,
                     const int* gate_qubits,
                     const idx_size gate_qubits_size,
                     const int total_circuit_qubits,
                     const idx_size starting_idx = 0);

void
FormBlockOfCZTGates(idx_size& gate_i,
                    valarray<idx_size>& CZ_bitmasks,
                    array<idx_size, 2>& T_bitmasks,
                    const vector<Gate>& cluster,
                    const int total_circuit_qubits);

void
FormBlockOfXYHGates(vector<Gate>& cluster,
                    idx_size& gate_i,
                    const vector<Gate>& all_gates);
void
ApplyBlockOfCZTGates(cmplx* __restrict amp,
                     const idx_size amp_size,
                     const int total_circuit_qubits,
                     const valarray<idx_size>& CZ_bitmasks,
                     const array<idx_size, 2>& T_bitmasks);

void
ApplyNonControl1QGates(cmplx* __restrict amp,
                       const idx_size amp_size,
                       const int q,
                       const int total_circuit_qubits,
                       const Gate& g,
                       const Gate::Type gate_type);

template<typename function>
void
Apply2MergedXY12GatesHelper(cmplx* __restrict amp,
                            const int* gate_qubits,
                            const int total_circuit_qubits,
                            const idx_size size,
                            const function& gate_func);

void
Apply2MergedXY12Gates(Gate& gate1,
                      Gate& gate2,
                      cmplx* __restrict amp,
                      const idx_size amp_size,
                      const int total_circuit_qubits);

#endif /* kernels_h */
