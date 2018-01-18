//
//  state_interface.cpp
//  vector_state_sim
//
//  Created by Aneeqa Fatima on 1/18/18.
//  Copyright © 2018 Aneeqa Fatima. All rights reserved.
//

#include "state_interface.h"

idx_size GenericQuantumState::
FormXGatesBitmask(idx_size& gate_i,
                  const vector<Gate>& all_gates)
{
    vector<int> Xcluster_qubits;
    Xcluster_qubits = FormBlockOfXYHGates(gate_i, Gate::Type::X_1_2, all_gates);
    
    idx_size X_bitmask = 0;
    
    for (idx_size i = 0; i < Xcluster_qubits.size(); ++i)
        X_bitmask |= 1ull << Xcluster_qubits[i];
    
    return X_bitmask;
}

idx_size GenericQuantumState::
FormYGatesBitmask(idx_size& gate_i,
                  const vector<Gate>& all_gates)
{
    vector<int> Ycluster_qubits;
    Ycluster_qubits = FormBlockOfXYHGates(gate_i, Gate::Type::Y_1_2, all_gates);
    
    idx_size Y_bitmask = 0;
    
    for (idx_size i = 0; i < Ycluster_qubits.size(); ++i)
        Y_bitmask |= 1ull << Ycluster_qubits[i];
    
    return Y_bitmask;
}

void GenericQuantumState::
FormCZTGatesBitmask(idx_size* __restrict CZ_bitmasks /*total_circuit_qubits*/,
                    idx_size __restrict T_bitmasks[2],
                    idx_size& gate_i,
                    const vector<Gate>& all_gates,
                    const int total_circuit_qubits)
{
    for (int i = 0; i < total_circuit_qubits; ++i)
        CZ_bitmasks[i] = 0;
    
    FormBlockOfCZTGates(gate_i, CZ_bitmasks, T_bitmasks, all_gates, total_circuit_qubits);
    
}
