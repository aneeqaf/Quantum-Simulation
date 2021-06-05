//
//  circuit_kernels.cpp
//  QuantumSimProj
//
//  Created by Aneeqa Fatima on 5/24/21.
//  Copyright © 2021 Aneeqa Fatima. All rights reserved.
//

#include "circuit_kernels.h"

void
PrintGates(const vector<Gate>& gates, idx_size num_qubits)
{
    cout << endl;
    for (idx_size q = 0; q < num_qubits; ++q) {
        cout << q << " | ";
        for (idx_size i = 0; i < gates.size(); ++i) {
            const Gate& g = gates[i];
            if (g.GetQubits().size() == 1 && g.GetQubits()[0] == q) {
                if (g.GetType() == Gate::Type::X_1_2)
                    cout << "X ";
                else if (g.GetType() == Gate::Type::Y_1_2)
                    cout << "Y ";
                else if (g.GetType() == Gate::Type::T)
                    cout << "D ";
                else if (g.GetType() == Gate::Type::Hadamard)
                    cout << "H ";
            }
            else if (g.GetQubits().size() == 2 && (g.GetQubits()[0] == q || g.GetQubits()[1] == q)) {
                if(g.GetType() == Gate::Type::ControlZ)
                    cout << "D ";
            }
        }
        cout << "\n";
    }
//    cout << endl;
//    for (idx_size q = 0; q < num_qubits; ++q) {
//        cout << q << " | ";
//        for (idx_size i = 0; i < gates.size(); ++i) {
//            const Gate& g = gates[i];
//            if (g.GetQubits().size() == 1 && g.GetQubits()[0] == q) {
//                if (g.GetType() == Gate::Type::X_1_2)
//                    cout << i << ":X ";
//                else if (g.GetType() == Gate::Type::Y_1_2)
//                    cout << i << ":Y ";
//                else if (g.GetType() == Gate::Type::T)
//                    cout << i << ":T ";
//                else if (g.GetType() == Gate::Type::Hadamard)
//                    cout << i << ":H ";
//            }
//            else if (g.GetQubits().size() == 2 && (g.GetQubits()[0] == q || g.GetQubits()[1] == q)) {
//                if(g.GetType() == Gate::Type::ControlZ)
//                    cout << i << ":CZ ";
//            }
//        }
//        cout << "\n";
//    }
    cout << "\n";
    Gate::Type prev_type = gates[0].GetType();
    for (idx_size i = 0; i < gates.size(); ++i) {
        if (prev_type != gates[i].GetType()) {
            cout << endl;
            prev_type = gates[i].GetType();
        }
        const Gate& g = gates[i];
        if(g.GetType() == Gate::Type::ControlZ)
            cout << i << ":CZ(" << g.GetQubits()[0] << "," << g.GetQubits()[1] << ") ";
        else if (g.GetType() == Gate::Type::X_1_2)
            cout << i << ":X(" << g.GetQubits()[0] << ") ";
        else if (g.GetType() == Gate::Type::Y_1_2)
            cout << i << ":Y(" << g.GetQubits()[0] << ") ";
        else if (g.GetType() == Gate::Type::T)
            cout << i << ":T(" << g.GetQubits()[0] << ") ";
        else if (g.GetType() == Gate::Type::Hadamard)
            cout << i << ":H(" << g.GetQubits()[0] << ") ";
    }
    cout << "\n\n";
}

void
PrintGatesAndCycles(const vector<Gate>& gates,
                    const vector<idx_size>& clock_cycles)
{
    idx_size j = 0;
    for (idx_size i = 0; i < clock_cycles.size(); ++i) {
        cout << i << endl;
        
        for(;j < clock_cycles[i]; ++j) {
            auto& g = gates[j];
            if(g.GetType() == Gate::Type::ControlZ)
                cout << "CZ ";
            else if (g.GetType() == Gate::Type::X_1_2)
                cout << "X ";
            else if (g.GetType() == Gate::Type::Y_1_2)
                cout << "Y ";
            else if (g.GetType() == Gate::Type::T)
                cout << "T ";
            else if (g.GetType() == Gate::Type::Hadamard)
                cout << "H ";
        }
        cout << "\n";
    }
    cout << "\n\n";
}

void
CheckIfNearestNeighbor(idx_size q0,
                       idx_size q1,
                       const QubitPartition& qp)
{
    idx_size x0 = q0 % qp.GetColumns(), y0 = q0 / qp.GetColumns(),
    x1 = q1 % qp.GetColumns(), y1 = q1 / qp.GetColumns();
    
    if (!((x0 == x1 && (y0 + 1 == y1 || y0 - 1 == y1))
          || ((x0 + 1 == x1 || x0 - 1 == x1) && (y0 == y1)))) {
        cerr << "\n\n2 qubit gates are not acting on nearest neighbors.\n";
        exit(1);
    }
}

idx_size
ClusterSimilarGates(vector<Gate>& gates,
                    idx_size num_qubits,
                    idx_size start_idx,
                    idx_size end_idx)
{
#ifdef PrintG
    PrintGates(gates, num_qubits);
#endif
    idx_size count_2q_gates = 0;
    bool qs_obstructed[num_qubits];
    memset(qs_obstructed, 0, num_qubits * sizeof(bool));
    
    if (end_idx == 0) end_idx = gates.size();
        
    idx_size last_swap = start_idx + 1;
    for (idx_size i = start_idx; i < end_idx; i = last_swap++) {
        const Gate& curr_gate = gates[i];
        idx_size num_q_obstructed = 0;
        
        for (idx_size j = i + 1; j < end_idx && num_q_obstructed < num_qubits; ++j) {
            assert(last_swap <= j);
            
            bool q_obstructed = false;
            for (auto q : gates[j].GetQubits())
                if (qs_obstructed[q]) {
                    q_obstructed = true;
                    break;
                }
            
            if (!q_obstructed && curr_gate.GetType() == gates[j].GetType()) {
//                for (auto q : gates[j].GetQubits()) {
//                    if (!qs_obstructed[q]) {
//                        qs_obstructed[q] = true;
//                        ++num_q_obstructed;
//                    }
//                }
                if (j > last_swap) {
                    gates.insert(gates.begin() + last_swap, gates[j]);
                    gates.erase(gates.begin() + j + 1);
                }
                ++last_swap;
            }
            else if (!q_obstructed && curr_gate.IsDiagonal() && gates[j].IsDiagonal())
            {
                /*
                 If the gate is diagonal and the qubit is obstructed move it closer to the diagonal cluster but don't increase last swap.
                 */
                if (j > last_swap) {
                    gates.insert(gates.begin() + last_swap, gates[j]);
                    gates.erase(gates.begin()  + j + 1);
                }
            }
            else if (curr_gate.IsDiagonal() && !gates[j].IsDiagonal()) {
                /*
                 If the current gate is diagonal, then the qubit is only obstructed if the gate is not diagonal.
                 */
                for (auto q : gates[j].GetQubits()) {
                    if (!qs_obstructed[q]) {
                        qs_obstructed[q] = true;
                        ++num_q_obstructed;
                    }
                }
            }
            else if (!curr_gate.IsDiagonal() && curr_gate.GetType() != gates[j].GetType()) {
                /*
                 If the current gate is not diagonal, then only move similar gates towards current gate.
                 The similar gate should appear before other types of gates on a qubit.
                 */
                for (auto q : gates[j].GetQubits()) {
                    if (!qs_obstructed[q]) {
                        qs_obstructed[q] = true;
                        ++num_q_obstructed;
                    }
                }
            }
        }
        memset(qs_obstructed, 0, num_qubits * sizeof(bool));
        if (curr_gate.GetQubits().size() == 2)
            count_2q_gates += last_swap - i;
    }
    
    for (idx_size i = start_idx; i < end_idx; ++i) {
        idx_size j = i;
        if (gates[i].IsDiagonal()) {
            if (gates[j].GetType() != Gate::Type::ControlZ) {
                for (; j < end_idx && gates[j].GetType() != Gate::Type::ControlZ; ++j) {}
                
                idx_size count_CZ = 0;
                for (; j < end_idx && gates[j].GetType() == Gate::Type::ControlZ; ++j)
                    swap(gates[i + count_CZ++], gates[j]);
            }
            else {
                for (; j < end_idx && gates[j].IsDiagonal(); ++j) {}
            }
        }
        i = j;
    }
    
#ifdef PrintG
    PrintGates(gates, num_qubits);
#endif
    
    return count_2q_gates;
}

idx_size
InsertNewCycleOnClusteredCircuit(idx_size gate_idx,
                                 idx_size num_qubits,
                                 vector<Gate>& circuit_gates,
                                 vector<Gate>& gates_to_insert,
                                 const QubitPartition& qp)
{
    /*
     Rearrange sub-circuit into the desired order xCZ->diag gates->non-diag gates
     */
    ClusterSimilarGates(gates_to_insert, num_qubits);
//    for (idx_size i = 0; i < gates_to_insert.size(); ++i) {
//        for (idx_size j = i; j < gates_to_insert.size() && gates_to_insert[j].GetType() == Gate::Type::ControlZ; ++j) {
//            const auto& gate_qubits = gates_to_insert[j].GetQubits();
//
//            if (gate_qubits.size() > 1
//                &&  (qp.globalToBlock(num_qubits - 1 - gate_qubits[0]) !=  qp.globalToBlock(num_qubits - 1 - gate_qubits[1])))
//                swap(gates_to_insert[j], gates_to_insert[i]);
//        }
//    }

    for (idx_size i = 0; i < gates_to_insert.size(); ++i)
        circuit_gates.insert(circuit_gates.begin() + gate_idx + i, gates_to_insert[i]);
    
    return gate_idx + gates_to_insert.size();

}
