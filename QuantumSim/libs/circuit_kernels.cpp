//
//  circuit_kernels.cpp
//  QuantumSimProj
//
//  Created by Aneeqa Fatima on 5/24/21.
//  Copyright © 2021 Aneeqa Fatima. All rights reserved.
//

#include "circuit_kernels.h"

bool
IsCrossingGate(idx_size gate_idx,
               idx_size qubits,
               const vector<Gate>& gates,
               const QubitPartition& qp)
{
    const auto& gate_qubits = gates[gate_idx].GetQubits();
    return gate_qubits.size() > 1 &&
        (qp.globalToBlock(qubits - 1 - gate_qubits[0]) !=  qp.globalToBlock(qubits - 1 - gate_qubits[1]));
}

int
GetCycleNumForGateIdx(idx_size gate_idx, const vector<idx_size>& clock_cycles)
{
    for (int c = 0; c < (int)clock_cycles.size(); ++c) {
        if (gate_idx < clock_cycles[c])
            return c;
    }
    return (int)clock_cycles.size();
}

void
PrintGates(const vector<Gate>& gates, idx_size num_qubits, const QubitPartition& qp)
{
    cout << endl;
    for (idx_size q = 0; q < num_qubits; ++q) {
        cout << q << " | ";
        for (idx_size i = 0; i < gates.size(); ++i) {
            const Gate& g = gates[i];
            if (g.GetQubits().size() == 1 && g.GetQubits()[0] == q) {
                if (g.GetType() == Gate::Type::x_1_2)
                    cout << "X ";
                else if (g.GetType() == Gate::Type::y_1_2)
                    cout << "Y ";
                else if (g.GetType() == Gate::Type::t)
                    cout << "D ";
                else if (g.GetType() == Gate::Type::h)
                    cout << "H ";
            }
            else if (g.GetQubits().size() == 2 && (g.GetQubits()[0] == q || g.GetQubits()[1] == q)) {
                if(g.GetType() == Gate::Type::cz)
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
        if(g.GetType() == Gate::Type::cz) {
            const auto& gate_qubits = gates[i].GetQubits();
            if (qp.globalToBlock(num_qubits - 1 - gate_qubits[0]) !=  qp.globalToBlock(num_qubits - 1 - gate_qubits[1])) {
                cout << i << ":xCZ(" << g.GetQubits()[0] << "," << g.GetQubits()[1] << ") ";
            }
            else {
                cout << i << ":CZ(" << g.GetQubits()[0] << "," << g.GetQubits()[1] << ") ";
            }
        }
        else if (g.GetType() == Gate::Type::x_1_2)
            cout << i << ":X(" << g.GetQubits()[0] << ") ";
        else if (g.GetType() == Gate::Type::y_1_2)
            cout << i << ":Y(" << g.GetQubits()[0] << ") ";
        else if (g.GetType() == Gate::Type::t)
            cout << i << ":T(" << g.GetQubits()[0] << ") ";
        else if (g.GetType() == Gate::Type::h)
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
            if(g.GetType() == Gate::Type::cz)
                cout << "CZ ";
            else if (g.GetType() == Gate::Type::x_1_2)
                cout << "X ";
            else if (g.GetType() == Gate::Type::y_1_2)
                cout << "Y ";
            else if (g.GetType() == Gate::Type::t)
                cout << "T ";
            else if (g.GetType() == Gate::Type::h)
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
CalculateTotalNumCycles(const idx_size num_qubits,
                        const Config* config,
                        const vector<idx_size>& clock_cycles,
                        const vector<Gate>& gates,
                        const QubitPartition& qp)
{
    if (config -> sim_type == Config::SimType::FullState)
    {
        return clock_cycles.size();
    }
    else {
        array<idx_size, 3> proc_ranges_branch_mempass {0, 0, 0};
        int current_mode = Config::ProcPrefix;
        idx_size current_path_bits = config -> proc_prefix_bits, memory_passes = 0;
        for (idx_size i = 0; i < gates.size(); ++i) {
            if (i == 0 && memory_passes == 0 && gates[i].GetType() == Gate::Type::h) {
                i += num_qubits - 1;
                ++memory_passes;
                continue;
            }
            else if (gates[i].IsDiagonal()) {
                ++memory_passes;
                for(; i < gates.size() && gates[i].IsDiagonal(); ++i) { 
                    if (IsCrossingGate(i, num_qubits, gates, qp)) {
                        if (current_path_bits <= 0) {
                            proc_ranges_branch_mempass[current_mode] = memory_passes;
                            memory_passes = 0;

                            switch (current_mode) {
                                case Config::ProcPrefix:
                                    current_path_bits = config -> ranges_bits;
                                    ++current_mode;
                                    break;
                                case Config::Ranges:
                                    current_path_bits = config -> dfs_length;
                                    ++current_mode;
                                    break;
                                default:
                                    break;
                            }
                        }
                        --current_path_bits;
                    }
                }
            }
            else {
                while (i < gates.size() && !gates[i].IsDiagonal()) {
                    if (i + 1 < gates.size() && !gates[i].IsDiagonal() && !gates[i + 1].IsDiagonal()) {
                        ++memory_passes;
                        i += 2;
                    }
                    else {
                        ++memory_passes;
                        ++i;
                    }
                }
            }
            --i;
        }
        proc_ranges_branch_mempass[current_mode] = memory_passes;
        return  proc_ranges_branch_mempass[0] + ((1ull << config -> ranges_bits) * proc_ranges_branch_mempass[1])
                + ((1ull << config -> ranges_bits) * (1ull << config -> dfs_length) * proc_ranges_branch_mempass[2]);
    }
}

void
CoalesceRzGates(vector<Gate>& gates)
{
    // Assume gates are already clustered
    for (idx_size i = 0; i < gates.size(); ++i) {
        unordered_map<idx_size, float> rz_idxs;
        idx_size j = i;
        // First collect all the gates
        for (; j < gates.size() && gates[i].GetType() == Gate::Type::rz; ++j) {
            auto q = gates[j].GetQubits()[0];
            if (rz_idxs.count(q) == 0)  rz_idxs[q] = gates[j].GetTheta()[0];
            else {
                rz_idxs[q] += gates[j].GetTheta()[0];
                gates.erase(gates.begin() + j);
            }
        }
        // Update the gates in the cluster to have the cumulative phases
        for (j = i; j < gates.size() && gates[i].GetType() == Gate::Type::rz; ++j) {
            auto q = gates[j].GetQubits()[0];
            gates[j] = Gate(Gate::Type::rz, 0, true, {gates[j].GetQubits()[0]}, {rz_idxs[q]});
        }
        i = j;
    }
}

void
PostProcessAfterClustering(idx_size num_qubits,
                           vector<Gate>& gates,
                           vector<idx_size>& clock_cycles,
                           bool google)
{
    clock_cycles.clear();
        
    idx_size prev_non_diag_start = 0;
    idx_size prev_num_1q_gates = 0;
    // Assume a non-diag gate incident on same qubit initiates a new cycle
    for (idx_size i = num_qubits; i < gates.size(); ++i) {
        idx_size num_1q_gates = 0;
        bitset<128> obstructed_qubits;

        if (gates[i].IsDiagonal() || gates[i].GetType() == Gate::h) {
            clock_cycles.push_back(i);
            
            for (; i < gates.size() && gates[i].IsDiagonal(); ++i) {
                auto gate_qubits = gates[i].GetQubits();
                for (auto q : gate_qubits) obstructed_qubits[q] = 1;
            }
            
            idx_size non_diag_start = i;
            for (; i < gates.size() && !gates[i].IsDiagonal(); ++i) {
                auto gate_qubits = gates[i].GetQubits();
                // google gates have a layer of H that completely obstructs and will not allow moving
                // potential gates.
                if (google && gates[i].GetType() == Gate::Type::h) break;
                if (gate_qubits.size() == 1) ++num_1q_gates;
                for (auto q : gate_qubits) obstructed_qubits[q] = 1;
            }
                        
            // If there was an odd 1q gate in previous cycle, see if it can be moved in current cycle
            if (prev_num_1q_gates % 2 == 1) {
                idx_size gate_idx_to_move = 0;
                for (idx_size j = prev_non_diag_start; j < i; ++j) {
                    if (!obstructed_qubits[gates[j].GetQubits()[0]]) {
                        gate_idx_to_move = j;
                        ++num_1q_gates;
                        clock_cycles[clock_cycles.size() - 1] -= 1;
                        break;
                    }
                }
                if (gate_idx_to_move != 0) {
                    gates.insert(gates.begin() + i, gates[gate_idx_to_move]);
                    gates.erase(gates.begin() + gate_idx_to_move);
                }
            }
            
            
            prev_non_diag_start = non_diag_start;
            
            if (google && gates[i].GetType() == Gate::Type::h) {
                prev_num_1q_gates = 0;
                for (; i < gates.size() && gates[i].GetType() == Gate::Type::h; ++i) {}
            }
            else prev_num_1q_gates = num_1q_gates;

            --i;
        }
    }
    
    clock_cycles.push_back(gates.size());
}

void MoveLastLayerOfHGatesForFusion(idx_size num_qubits,
                                    vector<Gate>& gates,
                                    vector<idx_size>& clock_cycles)
{
    idx_size num_gates = gates.size();
    
    idx_size starting_idx = 0;
    for (size_t i = 0; i < gates.size() ; ++i) {
        if (gates[i].GetType() != Gate::Type::h) break;
        ++starting_idx;
    }
    
    for (idx_size i = num_gates - 1; i > starting_idx ; --i) {
        if (gates[i].GetType() == Gate::Type::h) {
            Gate gate = gates[i];
            idx_size q = gate.GetQubits()[0];
            idx_size i_cycle_num = GetCycleNumForGateIdx(i, clock_cycles);

            for (idx_size j = i - 1; j > 0 ; --j) {
                auto gate_qubits = gates[j].GetQubits();
                if (gate_qubits[0] == q || (gate_qubits.size() == 2 && gate_qubits[1] == q)){
                    // Assume H gates are at the end of the cycle
                    idx_size cycle_num = GetCycleNumForGateIdx(j, clock_cycles);
                    idx_size end = clock_cycles[cycle_num];
                    if (cycle_num != i_cycle_num) {
                        gates.erase(gates.begin() + i);
                        gates.insert(gates.begin() + end, gate);
                        for (idx_size c = cycle_num; c < i_cycle_num; ++c)
                            clock_cycles[c] += 1;
                       
                        ++i;
                    }
                    break;
                }
            }            
        }
    }
}

// TODO: The way the gates are arranged affects the clustering. Not sure if there is a way to overcome that
// TODO: Currently, can only have one type of 1q non-diag gate incident on a qubit
idx_size
ClusterSimilarGates(vector<Gate>& gates,
                    idx_size num_qubits,
                    const QubitPartition& qp,
                    idx_size start_idx,
                    idx_size end_idx)
{
    static idx_size count_insert = 0;
#ifdef PrintG
    PrintGates(gates, num_qubits, qp);
#endif
    idx_size count_2q_gates = 0;
        
    if (end_idx == 0) end_idx = gates.size();
        
    idx_size last_swap = start_idx + 1;
    for (idx_size i = start_idx; i < end_idx; i = last_swap++) {
        const Gate& curr_gate = gates[i];
        idx_size num_q_obstructed = 0, consecutive_gates = 0;
        bitset<128> qs_obstructed = 0;
        
        for (idx_size j = i + 1; j < end_idx && num_q_obstructed < num_qubits; ++j) {
            assert(last_swap <= j);
            const auto& gate_qubits = gates[j].GetQubits();
            const idx_size num_gate_qubits = gate_qubits.size();
            
            bool q_obstructed = qs_obstructed[gate_qubits[0]]
                                || (num_gate_qubits == 2 && qs_obstructed[gate_qubits[1]]);
            
            if (!q_obstructed && curr_gate.GetType() == gates[j].GetType()) {
//                for (auto q : gates[j].GetQubits()) {
//                    if (!qs_obstructed[q]) {
//                        qs_obstructed[q] = true;
//                        ++num_q_obstructed;
//                    }
//                }
                if (j > last_swap) {
                    ++count_insert;
                    gates.insert(gates.begin() + last_swap, gates[j]);
                    gates.erase(gates.begin() + j + 1);
                }
                ++last_swap;
            }
            else if (!q_obstructed && curr_gate.IsDiagonal() && gates[j].IsDiagonal())
            {
                /*
                 If the gate is diagonal and the qubit is not obstructed move it closer to the diagonal cluster but don't increase last swap.
                 */
                if (j > last_swap) {
                    ++count_insert;
                    gates.insert(gates.begin() + last_swap, gates[j]);
                    gates.erase(gates.begin()  + j + 1);
                }
                ++consecutive_gates;
            }
            else {
                idx_size q0 = gate_qubits[0], q1 = num_gate_qubits == 2 ? gate_qubits[1] : 0;
               
                if (curr_gate.IsDiagonal() && !gates[j].IsDiagonal()) {
                    /*
                     If the current gate is diagonal, then the qubit is only obstructed if the gate is not diagonal.
                     */
                    if (!qs_obstructed[q0]) {
                        qs_obstructed[q0] = true;
                        ++num_q_obstructed;
                    }
                    if (num_gate_qubits == 2 && !qs_obstructed[q1]) {
                        qs_obstructed[q1] = true;
                        ++num_q_obstructed;
                    }
                }
                else if (!curr_gate.IsDiagonal() && curr_gate.GetType() != gates[j].GetType()) {
                    /*
                     If the current gate is not diagonal, then only move similar gates towards current gate.
                     The similar gate should appear before other types of gates on a qubit.
                     */
                    // Move obstructing gate closer for better grouping
//                    if (j > last_swap && ((num_gate_qubits == 1 && !qs_obstructed[q0])
//                                          || (num_gate_qubits == 2 && !qs_obstructed[q0] && !qs_obstructed[q1]))) {
//                        ++count_insert;
//                        gates.insert(gates.begin() + last_swap + consecutive_gates++, gates[j]);
//                        gates.erase(gates.begin()  + j + 1);
//                    }
                    if (!qs_obstructed[q0]) {
                        qs_obstructed[q0] = true;
                        ++num_q_obstructed;
                    }
                    if (num_gate_qubits == 2 && !qs_obstructed[q1]) {
                        qs_obstructed[q1] = true;
                        ++num_q_obstructed;
                    }
                    
                }
            }
        }
        if (curr_gate.GetQubits().size() == 2)
            count_2q_gates += last_swap - i;
    }
    
//    // TODO : Move all diagonal gates to front. Order shouldn't matter.
//    for (idx_size i = start_idx; i < end_idx; ++i) {
//        idx_size j = i;
//        if (gates[i].IsDiagonal()) {
//                idx_size count_diag = 0;
//                for (; j < end_idx && gates[j].IsDiagonal(); ++j)
//                    swap(gates[i + count_diag++], gates[j]);
//            else {
//                for (; j < end_idx && gates[j].IsDiagonal(); ++j) {}
//            }
//        }
//        i = j;
//    }
    
#ifdef PrintG
    PrintGates(gates, num_qubits, qp);
    cout << "\nGate insert count: " << count_insert << endl;
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
    ClusterSimilarGates(gates_to_insert, num_qubits, qp);

    for (idx_size i = 0; i < gates_to_insert.size(); ++i)
        circuit_gates.insert(circuit_gates.begin() + gate_idx + i, gates_to_insert[i]);
    
    return gate_idx + gates_to_insert.size();

}

// Inserts the new gates to the relevant clusters in the next cycles
/*idx_size Circuit::
RearrangexCZForPathConcludingCycle(idx_size gate_idx,
                                   idx_size marked_crossing_gates,
                                   gate_movement_unordered_map_t& gates_to_insert,
                                   const vector<idx_size>& gates_to_delete,
                                   const QubitPartition& qp)
{
    if (gates_to_insert.size() == 0) return gate_idx;
    
    set<idx_size> obstructed_qubits;
    set<ObstructedQubit, ObstructedQubit> qubit_obstruction_idx;
    
    idx_size i = gate_idx;
    // Find the obstruction indices to move out the xCZ gates as far as possible
    for (; i < gates.size()
                && qubit_obstruction_idx.size() != marked_crossing_gates; ++i) {
        if (gates[i].IsDiagonal()) {
            const auto& gate_qubits = gates[i].GetQubits();
                        
            // A diagonal gate impedes the movement of another diagonal gate
            // if it has been already been obstructed
            for (const auto& q : gate_qubits) {
                if (obstructed_qubits.count(q) == 0 &&
                    gates_to_insert.count(q) > 0
                     && gates_to_insert[q] -> IsObstructed) {
                    const auto& insertion_gate_qubits =
                                gates_to_insert[q] -> gates_list[0].GetQubits();
                    
                    for (const auto q1 : insertion_gate_qubits)
                        obstructed_qubits.insert(q1);
                    qubit_obstruction_idx.insert({insertion_gate_qubits, i});
                    break;
                }
            }
        }
        else if (!gates[i].IsDiagonal()) {
            const auto& gate_qubits = gates[i].GetQubits();
            
            for (const auto& q : gate_qubits) {
                if (obstructed_qubits.count(q) == 0 && gates_to_insert.count(q) > 0) {
                    const auto& insertion_gate_qubits = gates_to_insert[q] -> gates_list[0].GetQubits();
                    
                    for (const auto q1 : insertion_gate_qubits)
                        obstructed_qubits.insert(q1);
                    qubit_obstruction_idx.insert({insertion_gate_qubits, i});
                    break;
                }
            }
        }
    }
    
    // If an obstruction does not exist, append it to the end.
    for (const auto& g : gates_to_insert) {
        const auto& gate_qubits = g.second -> gates_list[0].GetQubits();
        for (const auto& q : gate_qubits) {
            if (obstructed_qubits.count(q) == 0) {
                const auto& insertion_gate_qubits = gates_to_insert[q] -> gates_list[0].GetQubits();
                
                for (const auto q1 : insertion_gate_qubits)
                    obstructed_qubits.insert(q1);
                qubit_obstruction_idx.insert({insertion_gate_qubits, gates.size()});
                break;
            }
        }
    }

    int rearrangement_gate = -1;
    for (const auto& q : qubit_obstruction_idx) {
        vector<Gate>& gates_list = gates_to_insert[q.qubits[0]] -> gates_list;
        if (gates_to_insert.size() > 0) {
            ++rearrangement_gate;
            InsertNewCycleOnClusteredCircuit(q.obstruction_idx, qubits, gates, gates_list, qp);
        }
    }
    
    if (rearrangement_gate != -1)
        ::ClusterSimilarGates(gates, qubits, gate_idx);

    
    // Deleting after is fine because all insertions are happening at or after the deletion idx.
    // Deletion indices need to be be descending order.
    for (int i = (int)gates_to_delete.size() - 1; i >=0; --i) {
        gates.erase(gates.begin() + gates_to_delete[i]);
        --gate_idx;
    }
    
#ifdef PrintG
    PrintGates(gates, qubits);
#endif
    
    idx_size xGate_count = 0;
    for (; gate_idx < gates.size() && xGate_count < marked_crossing_gates; ++gate_idx) {
        const auto& gate_qubits = gates[gate_idx].GetQubits();

        if (gate_qubits.size() > 1) {
            idx_size q0 = qubits - 1 - gate_qubits[0], q1 =  qubits - 1  - gate_qubits[1];
            
            if (qp.globalToBlock(q0) !=  qp.globalToBlock(q1))
                ++xGate_count;
        }
    }
    
    return gate_idx - 1;
}*/
