//
//  XYbenchmarks.cpp
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 12/30/17.
//  Copyright © 2017 Aneeqa Fatima. All rights reserved.
//

#include "XYmicroBM.h"

template <typename function>
double Apply2MergedGatesBMHelper(cmplx* __restrict amp,
                                 const idx_size amp_size,
                                 const int total_circuit_qubits,
                                 const function& gate_func,
                                 const int total_gates)
{
    try {
        idx_size gate_bitmask = 0;
        const array<int, 2> gate_qubits = {total_circuit_qubits - 1 , total_circuit_qubits - 2};
        for (idx_size i = 0; i < 2; ++i)
            gate_bitmask |= (1ull << ((total_circuit_qubits - 1) - gate_qubits[i]));
        
        constexpr int num_indices = 4;
        array<idx_size, num_indices> indices;
        ExtractIndicesForAmp(indices.data(), gate_qubits.data() , 2 , total_circuit_qubits);
        array<idx_size, num_indices> temp_indices;
        
        amp = (cmplx*)__builtin_assume_aligned(amp, 64);
        
        clock_t begin = clock();
        for (idx_size i = 0; i < total_gates; ++i) {
            idx_size idx = 0, iter_count = 0;
            while(iter_count < (amp_size/num_indices)) {
                if ((idx & gate_bitmask) == 0) {
                    ++iter_count;
                    
                    for (idx_size i = 0; i < num_indices; ++i)
                        temp_indices[i] = indices[i] + idx;
                    
                    gate_func(amp, temp_indices);
                    
                    ++idx;
                }
                else
                    idx += (idx & gate_bitmask);
            }
        }
        clock_t end = clock();
        
        return (double(end - begin) / CLOCKS_PER_SEC);
    }
    catch(...) {
        cout << "Error while benchmarking 2 merged XY gates";
        return -1;
    }
    return -1;
}

void Apply2MergedGatesBM(cmplx* __restrict amp,
                        const idx_size amp_size,
                        const int total_circuit_qubits,
                        const int total_gates)
{
    cout << "Time taken for ApplyXX12GateBM (" + to_string(total_gates) + " X gates) : "
    << Apply2MergedGatesBMHelper(amp, amp_size, total_circuit_qubits, ApplyXX12Gate, total_gates) << "s\n";
    cout << "Time taken for ApplyXY12GateBM (" + to_string(total_gates/2) +
            " X gates and " + to_string(total_gates/2) + " Y gates): "
    << Apply2MergedGatesBMHelper(amp, amp_size, total_circuit_qubits, ApplyXY12Gate, total_gates) << "s\n";
    cout << "Time taken for ApplyYX12GateBM (" + to_string(total_gates/2) +
    " X gates and " + to_string(total_gates/2) + " Y gates): "
    << Apply2MergedGatesBMHelper(amp, amp_size, total_circuit_qubits, ApplyYX12Gate, total_gates) << "s\n";
    cout << "Time taken for ApplyYY12GateBM (" + to_string(total_gates) + " Y gates): "
    << Apply2MergedGatesBMHelper(amp, amp_size, total_circuit_qubits, ApplyYY12Gate, total_gates) << "s\n\n";
}

template <typename function>
double Apply4MergedGatesBMHelper(cmplx* __restrict amp,
                               const idx_size amp_size,
                               const int total_circuit_qubits,
                               const function& gate_func,
                               const int total_gates)
{
    try {
        idx_size gate_bitmask = 0;
        const array<int, 4> gate_qubits = {0, total_circuit_qubits - 2, 3, total_circuit_qubits - 4};
        for (idx_size i = 0; i < 4; ++i)
            gate_bitmask |= (1ull << ((total_circuit_qubits - 1) - gate_qubits[i]));
        
        const int num_indices = 16;
        array<idx_size, num_indices> indices;
        ExtractIndicesForAmp(indices.data(), gate_qubits.data() , 4 , total_circuit_qubits);
        array<idx_size, num_indices> temp_indices;
        
        amp = (cmplx*)__builtin_assume_aligned(amp, 64);
        
        clock_t begin = clock();
        for (idx_size i = 0; i < total_gates/2; ++i) {
            idx_size idx = 0, iter_count = 0;
            while(iter_count < (amp_size/num_indices)) {
                if ((idx & gate_bitmask) == 0) {
                    ++iter_count;
                    
                    for (idx_size i = 0; i < num_indices; ++i)
                        temp_indices[i] = indices[i] + idx;
                    
                    gate_func(amp, temp_indices);
                    
                    ++idx;
                }
                else
                    idx += (idx & gate_bitmask);
            }
        }
        clock_t end = clock();
        
        return (double(end - begin) / CLOCKS_PER_SEC);
    }
    catch(...) {
        cout << "Error while benchmarking 4 merged XY gates.";
        return -1;
    }
    
    return -1;
}

void Apply4MergedGatesBM(cmplx* __restrict amp,
                         const idx_size amp_size,
                         const int total_circuit_qubits,
                         const int total_gates)
{
    cout << "Time taken for Apply4X12GateBM (" + to_string(total_gates) + " X gates): "
    << Apply4MergedGatesBMHelper(amp, amp_size, total_circuit_qubits, Apply4X12Gate, total_gates) << "s\n";
    cout << "Time taken for Apply4Y12GateBM (" + to_string(total_gates) + " Y gates): "
    << Apply4MergedGatesBMHelper(amp, amp_size, total_circuit_qubits, Apply4Y12Gate, total_gates) << "s\n\n";
}
