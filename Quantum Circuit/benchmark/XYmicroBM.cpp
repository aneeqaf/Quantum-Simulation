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
                                 int& i,
                                 const idx_size amp_size,
                                 const int total_circuit_qubits,
                                 const function& gate_func)
{
    idx_size gate_bitmask = 0, idx = 0, iter_count = 0;
    const array<int, 2> gate_qubits = {i, ++i};
    for (idx_size i = 0; i < 2; ++i)
        gate_bitmask |= (1ull << ((total_circuit_qubits - 1) - gate_qubits[i]));
    
    constexpr int num_indices = 4;
    array<idx_size, num_indices> indices;
    ExtractIndicesForAmp(indices.data(), gate_qubits.data() , 2 , total_circuit_qubits);
    array<idx_size, num_indices> temp_indices;
    
    amp = (cmplx*)__builtin_assume_aligned(amp, 64);
    
    clock_t begin = clock();
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
    clock_t end = clock();
    return double(end - begin) / CLOCKS_PER_SEC;
}

template <typename function>
double Apply2MergedGatesBMAllQ(cmplx* __restrict amp,
                               const idx_size amp_size,
                               const int total_circuit_qubits,
                               const function& gate_func)
{
    try {
        double total_time = 0;
        for (int i = 0; (i + 1) < total_circuit_qubits; ++i)
            total_time += Apply2MergedGatesBMHelper(amp, i, amp_size, total_circuit_qubits, gate_func);
        
        return total_time;
    }
    catch(...) {
        cout << "Error while benchmarking 2 merged XY gates";
        return -1;
    }
    return -1;
}

template <typename function>
double Apply2MergedGatesBMHighQ(cmplx* __restrict amp,
                               const idx_size amp_size,
                               const int total_circuit_qubits,
                               const function& gate_func)
{
    try {
        double total_time = 0;
        for (int i = total_circuit_qubits - 10; (i + 1) < total_circuit_qubits ; ++i)
            total_time += Apply2MergedGatesBMHelper(amp, i, amp_size, total_circuit_qubits, gate_func);
        
        return total_time;
    }
    catch(...) {
        cout << "Error while benchmarking 2 merged XY gates";
        return -1;
    }
    return -1;
}

template <typename function>
double Apply2MergedGatesBMLowQ(cmplx* __restrict amp,
                                const idx_size amp_size,
                                const int total_circuit_qubits,
                                const function& gate_func)
{
    try {
        double total_time = 0;
        for (int i = 0; (i + 1) < 10 ; ++i)
            total_time += Apply2MergedGatesBMHelper(amp, i, amp_size, total_circuit_qubits, gate_func);
        
        return total_time;
    }
    catch(...) {
        cout << "Error while benchmarking 2 merged XY gates";
        return -1;
    }
    return -1;
}

void Apply2MergedGatesBM(cmplx* __restrict amp,
                        const idx_size amp_size,
                        const int total_circuit_qubits)
{
    int total_gates = total_circuit_qubits % 2 == 1 ? total_circuit_qubits - 1 : total_circuit_qubits;
    cout << "ApplyXX12GateBM on all qubits (" + to_string(total_gates) + " X gates) : "
    << Apply2MergedGatesBMAllQ(amp, amp_size, total_circuit_qubits, ApplyXX12Gate) << "s\n";
    cout << "ApplyXY12GateBM on all qubits (" + to_string(total_gates/2) +
            " X gates and " + to_string(total_gates/2) + " Y gates): "
    << Apply2MergedGatesBMAllQ(amp, amp_size, total_circuit_qubits, ApplyXY12Gate) << "s\n";
    cout << "ApplyYX12GateBM on all qubits (" + to_string(total_gates/2) +
    " X gates and " + to_string(total_gates/2) + " Y gates): "
    << Apply2MergedGatesBMAllQ(amp, amp_size, total_circuit_qubits, ApplyYX12Gate) << "s\n";
    cout << "ApplyYY12GateBM on all qubits (" + to_string(total_gates) + " Y gates): "
    << Apply2MergedGatesBMAllQ(amp, amp_size, total_circuit_qubits, ApplyYY12Gate) << "s\n";
    cout << "ApplyXX12GateBM on low qubits (" + to_string(10) + " X gates) : "
    << Apply2MergedGatesBMLowQ(amp, amp_size, total_circuit_qubits, ApplyXX12Gate) << "s\n";
    cout << "ApplyXX12GateBM on high qubits (" + to_string(10) + " X gates) : "
    << Apply2MergedGatesBMHighQ(amp, amp_size, total_circuit_qubits, ApplyXX12Gate) << "s\n\n";
}

template <typename function>
double Apply4MergedGatesBMHelper(cmplx* __restrict amp,
                               const idx_size amp_size,
                               const int total_circuit_qubits,
                               const function& gate_func)
{
    try {
        double total_time = 0;
        for (int i = 0; (i + 4) < total_circuit_qubits; ++i) {
            idx_size gate_bitmask = 0, idx = 0, iter_count = 0;
            const array<int, 4> gate_qubits = {i, ++i, ++i, ++i};
            for (idx_size i = 0; i < 4; ++i)
                gate_bitmask |= (1ull << ((total_circuit_qubits - 1) - gate_qubits[i]));
            
            const int num_indices = 16;
            array<idx_size, num_indices> indices;
            ExtractIndicesForAmp(indices.data(), gate_qubits.data() , 4 , total_circuit_qubits);
            array<idx_size, num_indices> temp_indices;
            
            amp = (cmplx*)__builtin_assume_aligned(amp, 64);
            
            clock_t begin = clock();
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
            clock_t end = clock();
            total_time += double(end - begin) / CLOCKS_PER_SEC;
        }
        
        return total_time;
    }
    catch(...) {
        cout << "Error while benchmarking 4 merged XY gates.";
        return -1;
    }
    
    return -1;
}

void Apply4MergedGatesBM(cmplx* __restrict amp,
                         const idx_size amp_size,
                         const int total_circuit_qubits)
{
    int total_gates = total_circuit_qubits % 4 == 0 ? total_circuit_qubits :
                        total_circuit_qubits - (total_circuit_qubits % 4);
    cout << "Apply4X12GateBM on all qubits (" + to_string(total_gates) + " X gates): "
    << Apply4MergedGatesBMHelper(amp, amp_size, total_circuit_qubits, Apply4X12Gate) << "s\n";
    cout << "Apply4Y12GateBM on all qubits (" + to_string(total_gates) + " Y gates): "
    << Apply4MergedGatesBMHelper(amp, amp_size, total_circuit_qubits, Apply4Y12Gate) << "s\n\n";
}
