//
//  state_interface.cpp
//  vector_state_sim
//
//  Created by Aneeqa Fatima on 1/18/18.
//  Copyright © 2018 Aneeqa Fatima. All rights reserved.
//

#include "state_interface.h"

Times GenericQuantumState::time_by_category({});
Counts GenericQuantumState::count_of_category({});
vector<string> GenericQuantumState::log({});
Data GenericQuantumState::data_per_cycles({});

#ifdef Xcode
int GenericQuantumState::num_threads = 8;
#else
int GenericQuantumState::num_threads = omp_get_num_procs();
#endif

GenericQuantumState::
GenericQuantumState(int n_threads) {
    num_threads = n_threads;
}

bitset<128> GenericQuantumState::
FormXYGatesBitmask(idx_size& gate_i,
                  const vector<Gate>& all_gates,
                  const Gate::Type gate_type)
{
    vector<int> cluster_qubits = FormBlockOfXYHGates(gate_i, gate_type, all_gates);
    
    bitset<128> bitmask = 0;
    
    for (idx_size i = 0; i < cluster_qubits.size(); ++i)
        bitmask |= 1ull << cluster_qubits[i];
    
    return bitmask;
}

void GenericQuantumState::
FormCZTGatesBitmask(bitset<128>* __restrict CZ_bitmasks /*total_circuit_qubits*/,
                    bitset<128> __restrict T_bitmasks[2],
                    idx_size& gate_i,
                    const vector<Gate>& all_gates,
                    const int total_circuit_qubits)
{
    for (int i = 0; i < total_circuit_qubits; ++i)
        CZ_bitmasks[i] = 0;
    
    FormBlockOfCZTGates(gate_i, CZ_bitmasks, T_bitmasks, all_gates, total_circuit_qubits);    
}

int FindDivisor(int num)
{
    int div = 0;
    for (int i = 1; i <= floor(sqrt(num)); ++i){
        if (num % i == 0)
            div = i;
    }
    return div;
}
