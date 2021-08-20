//
//  circuit.cpp
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 12/12/17.
//

#include "circuit.h"

unordered_map<string, gate_generator_ptr> Circuit::gate_funcs;

Circuit::
Circuit(const string input_filename, idx_size num_q, idx_size depth)
: qp(nullptr), qubits(num_q), rearranged(false)
{
    gate_funcs["h"] = create_Hadamard;
    gate_funcs["t"] = create_T;
    gate_funcs["y_1_2"] = create_Y_1_2;
    gate_funcs["x_1_2"] = create_X_1_2;
    gate_funcs["cz"] = create_CZ;
    gate_funcs["rz"] = create_Z_rotation;
    
    if (input_filename.find("rearranged") != string::npos)
        rearranged = true;
    
    if (!input_filename.empty())
        ReadGoogleCircuitFile(input_filename, depth);
}

Circuit::
Circuit(const Circuit& g)
: gates(g.gates), clock_cycles(g.clock_cycles), qp(g.qp), qubits(g.qubits), rearranged(g.rearranged) {}

Circuit& Circuit::
operator=(const Circuit& g)
{
    Circuit temp(g);
    swap(qubits, temp.qubits);
    swap(gates, temp.gates);
    swap(qp, temp.qp);
    return *this;
}

void Circuit::
GroupAlternateCycles()
{
    bool saw_xy = false;
    idx_size g_i = 0;
    
    for (idx_size i = 2; i < clock_cycles.size(); ++i) {
        int last_CZT_gate = 0;
        if (i % 2 == 1)
            g_i = clock_cycles[i] - 1;
        
        else
            g_i = clock_cycles[i - 1];
        
        for (int j = 0; j < (int)(clock_cycles[i] - clock_cycles[i-1]); ++j) {
            int k = j;
            if (i % 2 == 1)
                k = -j;
            
            if (gates[g_i + k].GetType() == Gate::Type::t ||
                gates[g_i + k].GetType() == Gate::Type::cz) {
                
                if (i % 2 == 1) {
                    if(saw_xy)
                        swap(gates[g_i + k], gates[g_i - last_CZT_gate]);
                }
                else {
                    if (saw_xy)
                        swap(gates[g_i + k], gates[g_i + last_CZT_gate]);
                }
                ++last_CZT_gate;
            }
            else
                saw_xy = true;
            
        }
    }
}

void Circuit::
RecalibrateGoogleClockCycles()
{
    clock_cycles.clear();
        
    // Assume a non-diag gate incident on same qubit initiates a new cycle
    for (idx_size i = qubits; i < gates.size(); ++i) {
        if (gates[i].IsDiagonal() || gates[i].GetType() == Gate::h) {
            clock_cycles.push_back(i);
            
            for (; i < gates.size() && gates[i].IsDiagonal(); ++i) {}
            for (; i < gates.size() && !gates[i].IsDiagonal(); ++i) {}
            --i;
        }
    }
    
    clock_cycles.push_back(gates.size());
}

idx_size Circuit::
GroupSimilarGates()
{
    static idx_size count_insert = 0;

    idx_size last_CZ = 0, last_T = 0, last_X = 0, last_Y = 0;
    bool saw_CZ = false, saw_T = false, saw_X = false, saw_Y = false, saw_H = false;
    idx_size g_i = (idx_size)qubits;
    idx_size count_CZ = 0;
        
    for (idx_size j = qubits; j < gates.size()
         && (g_i + last_Y + last_X + last_T + last_CZ) < gates.size(); ++j) {
        
        if (gates[j].GetType() == Gate::Type::cz) {
            ++count_CZ;
            if(saw_Y || saw_X || saw_H) {
                g_i = j;
                last_CZ = 0; last_T = 0; last_X = 0; last_Y = 0;
                saw_T = false; saw_X = false; saw_Y = false; saw_H = false;
            }
            saw_CZ = true;
            if (saw_T || saw_X || saw_Y || saw_H) {
                if (!saw_H)
                    swap(gates[j], gates[g_i + last_CZ]);
                else {
                    ++count_insert;
                    gates.insert(gates.begin() + g_i + last_CZ, gates[j]);
                    gates.erase(gates.begin() + j + 1);
                }
            }
            
            ++last_CZ;
        }
        else if (gates[j].GetType() == Gate::Type::t) {
            if(saw_Y || saw_X) {
                g_i = j;
                last_CZ = 0; last_T = 0; last_X = 0; last_Y = 0;
                saw_CZ = false; saw_X = false; saw_Y = false; saw_H = false;
            }
            saw_T = true;
            if (saw_CZ || saw_X || saw_Y || saw_H) {
                if (!saw_H)
                   swap(gates[g_i + last_T + last_CZ], gates[j]);
                else {
                    ++count_insert;
                    gates.insert(gates.begin() + g_i + last_T + last_CZ, gates[j]);
                    gates.erase(gates.begin() + j + 1);
                }
            }
            
            ++last_T;
        }
        else if (gates[j].GetType() == Gate::Type::x_1_2) {
            saw_X = true;
            if (saw_T || saw_CZ || saw_Y || saw_H) {
                if (!saw_H)
                    swap(gates[g_i + last_X + last_T + last_CZ], gates[j]);
                else {
                    ++count_insert;
                    gates.insert(gates.begin() + g_i + last_X + last_T + last_CZ, gates[j]);
                    gates.erase(gates.begin() + j + 1);
                }
            }
            
            ++last_X;
        }
        else if (gates[j].GetType() == Gate::Type::y_1_2) {
            saw_Y = true;
            if (saw_T || saw_X || saw_CZ || saw_H) {
                if (!saw_H)
                    swap(gates[g_i + last_Y + last_X + last_T + last_CZ], gates[j]);
                else {
                    ++count_insert;
                    gates.insert(gates.begin() + g_i + last_X + last_T + last_CZ + last_Y, gates[j]);
                    gates.erase(gates.begin() + j + 1);
                }
            }
            
            ++last_Y;
        }
        else if (gates[j].GetType() == Gate::Type::h)
            saw_H = true;
    }
#ifdef PrintG
    PrintGates(gates, qubits, *qp);
    cout << "\nGate insert count: " << count_insert << endl;
#endif
    
    return (int)count_CZ;
}

idx_size Circuit::
ClusterSimilarGates()
{
    return ::ClusterSimilarGates(gates, qubits, *qp);
}

// TODO: Generalize this to other > 1q gates that cross
void Circuit::
MovexCZGatesRewrite(idx_size proc_prefix_bits,
                    idx_size range_bits,
                    idx_size branch_bits,
                    const bool nearest_neigbors)
{
    static auto MovexCZToFront = [&](idx_size i, idx_size path_size){
        idx_size count_xCZ = 0;
        for (; i < gates.size() && count_xCZ < path_size; ++i) {
            idx_size j = i;
            idx_size cycle_count_xCZ = 0;
            for (; j < gates.size() && gates[j].GetType() == Gate::Type::cz; ++j) {
                if (IsCrossingGate(j)) {
                    ++count_xCZ;
                    swap(gates[i + cycle_count_xCZ++], gates[j]);
                }
            }
            i = j;
        }
    };
    
    const idx_size num_qubits_minus_1 = qp -> getNumQubits() - 1, num_crossing_q = qp -> getNumX();
    int remaining_path_bits = static_cast<int>(proc_prefix_bits);
    Config::SimMode current_mode = Config::SimMode::ProcPrefix;
    bool continue_rearranging = proc_prefix_bits != 0;
    
    MovexCZToFront(0, remaining_path_bits);
    
#ifdef PrintG
    PrintGates(gates, qubits, *qp);
#endif
    
    for (idx_size i = 0; i < gates.size() && continue_rearranging; ++i) {
        int num_xCZ_in_cluster = 0, boundary_qubits_obstructed = 0;
        vector<idx_size> gates_to_delete;
        unordered_map<idx_size, bool> recorded_qubit;
        vector<Gate> gates_to_move;
        
        if (IsCrossingGate(i)){
            idx_size j = i;
            
            for (; j < gates.size(); ++j) {
                const auto& gate_qubits = gates[j].GetQubits();
                
                if (nearest_neigbors && gate_qubits.size() > 2) {
                    idx_size q0 = num_qubits_minus_1 - gate_qubits[0], q1 = num_qubits_minus_1 - gate_qubits[1];
                    CheckIfNearestNeighbor(q0, q1, *qp);
                }
                
                if (IsCrossingGate(j))
                    ++num_xCZ_in_cluster;
                else if (current_mode != Config::SimMode::Branch) break;
                else if (current_mode == Config::SimMode::Branch)
                    if (num_xCZ_in_cluster == num_crossing_q) break;
            }
            
            // Move the xCZ in the transitioning cycle further out to have more gates in preceding paths
            // The number of gates in the branching path should be the least
            if (remaining_path_bits <= num_xCZ_in_cluster) {
                j = i;
                
                if (remaining_path_bits < num_xCZ_in_cluster)
                    j = i + remaining_path_bits;
                
                idx_size xCZ_to_collect = 0;
                
                // Find all xCZ in cycle
                for (; j < gates.size(); ++j) {
                    
                    const auto& gate_qubits = gates[j].GetQubits();
                    
                    bool add_gate = false;
                    idx_size q0 = gate_qubits[0];
                    if (gate_qubits.size() > 0) {
                        if ((recorded_qubit.count(q0) > 0  && recorded_qubit[q0])
                            || (recorded_qubit.count(q0) > 0  && !gates[j].IsDiagonal())) {
                            add_gate = true;
                            
                            if (qp -> isBoundaryQubit(qubits - 1 - q0) && !recorded_qubit[q0])
                                ++boundary_qubits_obstructed;
                            
                            recorded_qubit[q0] = true;
                        }
                    }
                    if (gate_qubits.size() == 2) {
                        idx_size q1 = gate_qubits[1];
                        
                        if (IsCrossingGate(j)) {
                            if (xCZ_to_collect == remaining_path_bits) break;
                            
                            ++xCZ_to_collect;
                            if (recorded_qubit.count(q0) == 0) {
                                recorded_qubit[q0] = false;
                                recorded_qubit[q1] = false;
                            }
                            add_gate = true;
                        }
                        else if ((recorded_qubit.count(q1) > 0 && recorded_qubit[q1])
                                 || (recorded_qubit.count(q1) > 0 && !gates[j].IsDiagonal())) {
                            add_gate = true;
                            
                            if (qp -> isBoundaryQubit(qubits - 1 - q1) && !recorded_qubit[q1])
                                ++boundary_qubits_obstructed;
                            
                            recorded_qubit[q1] = true;
                        }
                    }
                    
                    if (add_gate) {
                        for (const auto& q : gate_qubits) {
                            if (recorded_qubit.count(q) == 0)
                                recorded_qubit[q] = false;
                        }
                        
                        gates_to_move.push_back(gates[j]);
                        gates_to_delete.push_back(j);
                    }
                    
                    if (remaining_path_bits == num_xCZ_in_cluster
                        && num_xCZ_in_cluster == xCZ_to_collect
                        && boundary_qubits_obstructed >= xCZ_to_collect
                        && current_mode != Config::SimMode::Branch) {
                        ++j;
                        goto exit_inner_loop;
                    }
                    else if (remaining_path_bits < num_xCZ_in_cluster
                             && (num_xCZ_in_cluster - remaining_path_bits) == xCZ_to_collect
                             && boundary_qubits_obstructed >= xCZ_to_collect) {
                        ++j;
                        goto exit_inner_loop;
                    }
                }
                
            exit_inner_loop:;
                
                InsertNewCycleOnClusteredCircuit(j, qubits, gates, gates_to_move, *qp);
                
                for (int k = (int)gates_to_delete.size() - 1; k >=0; --k)
                    gates.erase(gates.begin() + gates_to_delete[k]);
                
#ifdef PrintG
                PrintGates(gates, qubits, *qp);
#endif
                // if xCZ are not completely obstructed and other xCZ are seen, coalesce them and restart that cycle
                if (boundary_qubits_obstructed == 0 && xCZ_to_collect == remaining_path_bits) {
                    j -= gates_to_delete.size();
                    num_xCZ_in_cluster -= xCZ_to_collect;
                }
                // Move it back since the gates moved belong to the next path
                else if (j < gates.size() && remaining_path_bits < num_xCZ_in_cluster) {
                    j -= gates_to_delete.size();
//                    ::ClusterSimilarGates(gates, qubits, *qp, j);
                }
                // Move back but add back xCZ already processed
                else if (j < gates.size())
                    j = (j - gates_to_delete.size()) + remaining_path_bits;
            }
            
            remaining_path_bits -= num_xCZ_in_cluster;
            if (remaining_path_bits <= 0 && num_xCZ_in_cluster > 0) {
                switch (current_mode) {
                    case Config::SimMode::ProcPrefix:
                        remaining_path_bits = static_cast<int>(range_bits);
                        current_mode = Config::SimMode::Ranges;
//                        if (range_bits == 0) continue_rearranging = false;
                        break;
                    case Config::SimMode::Ranges:
                        remaining_path_bits = static_cast<int>(branch_bits);
                        current_mode = Config::SimMode::Branch;
                        if (branch_bits == 0) continue_rearranging = false;
                        break;
                    case Config::SimMode::Branch:
                        break;
                }
                MovexCZToFront(j, remaining_path_bits);
#ifdef PrintG
                PrintGates(gates, qubits, *qp);
#endif
            }
            i = j - 1;
        }
    }
    
    // Move xCZ gates to front so that terminations in the middle of the cycle don't break.
    // Alternative way could be more costly in critical simulation loop
    MovexCZToFront(0, gates.size());
    
#ifdef PrintG
    PrintGates(gates, qubits, *qp);
#endif
}


pair<int, int> Circuit::
MovexCZGates(idx_size proc_prefix_bits,
             idx_size range_bits,
             idx_size branch_bits,
             const bool nearest_neigbors)
{
    int total_xCZ_count = 0, count_CZ = 0;
    int num_q_1 = qp -> getNumQubits() - 1;
    idx_size curr_bit_counter = proc_prefix_bits;
    bool transition_cycle = false, last_cycle = false;
    int curr_path = 0;
  
    for (idx_size i = qubits; i < gates.size(); ++i) {
         if (gates[i].GetType() == Gate::Type::cz) {
             idx_size count_xCZ = 0;
             idx_size j = i;
             for (; j < gates.size() && gates[j].GetType() == Gate::Type::cz; ++j) {
                 ++count_CZ;
                 idx_size q0 = num_q_1 - gates[j].GetQubits().front(), q1 = num_q_1 - gates[j].GetQubits().back();
                 
                 if (nearest_neigbors) {
                     idx_size x0 = q0 % qp -> GetColumns(), y0 = q0 / qp -> GetColumns(),
                     x1 = q1 % qp -> GetColumns(), y1 = q1 / qp -> GetColumns();
                     
                     if (!((x0 == x1 && (y0 + 1 == y1 || y0 - 1 == y1))
                           || ((x0 + 1 == x1 || x0 - 1 == x1) && (y0 == y1)))) {
                         cerr << "\n\n2 qubit gates are not acting on nearest neighbors.\n";
                         exit(1);
                     }
                 }
                 
                 if (qp -> globalToBlock(q0) !=  qp -> globalToBlock(q1)) {
                     if (curr_bit_counter == 0 && !last_cycle) {
                         if (curr_path == 0) {
                             curr_bit_counter = range_bits;
                             curr_path = 1;
                         }
                         else if (curr_path == 1) {
                             curr_bit_counter = branch_bits;
                             curr_path = 2;
                         }
                         else if (curr_path == 2)
                             last_cycle = true;
                         transition_cycle = true;
                     }
                     swap(gates[i + count_xCZ++], gates[j]);
                     if (!last_cycle)
                         --curr_bit_counter;
                 }
             }
             
             sort(gates.begin() + i, gates.begin() + i + count_xCZ,
                  [](Gate& g1, Gate& g2){ return g1.GetQubits().front() < g2.GetQubits().front();});
             sort(gates.begin() + i, gates.begin() + i + count_xCZ,
                  [](Gate& g1, Gate& g2) {
                        if (g1.GetQubits().front() == g2.GetQubits().front())
                            return g1.GetQubits().back() > g2.GetQubits().back();
                        else
                            return false;
                  });
             
             if (transition_cycle && count_xCZ) {
                 idx_size xCZ_counter = 0, move_count = 0;
                 vector<idx_size> xCZ_q;
                 for (idx_size k = i + count_xCZ - 1; xCZ_counter != count_xCZ; --k) {
                     xCZ_q.push_back(gates[k].GetQubits().front());
                     xCZ_q.push_back(gates[k].GetQubits().back());
                     swap(gates[k], gates[j - 1 - move_count++]);
                     ++xCZ_counter;
                 }
//                 move_count = 0;
//                 idx_size non_xCZ_gates = j - i - count_xCZ;
//                 for (idx_size k = i; non_xCZ_gates != 0; ++k, --non_xCZ_gates) {
//                     for (int l = 0; l < xCZ_q.size(); ++l) {
//                         if (gates[k].qubits.back() == xCZ_q[l]
//                             || gates[k].qubits.front() == xCZ_q[l]) {
//                             xCZ_q.push_back(gates[k].qubits.front());
//                             xCZ_q.push_back(gates[k].qubits.back());
//                             gates.insert(gates.begin() + j - move_count++, gates[k]);
//                             gates.erase(gates.begin() + k);
//                             ++xCZ_counter;
//                             --k;
//                             break;
//                         }
//                     }
//                 }
//                
                 idx_size last_CZ = j - 1;
                 idx_size next_CZ = last_CZ;
            
                 for (idx_size k = j; k < gates.size()
                      && gates[k].GetType() != Gate::Type::cz; ++k)
                     ++next_CZ;
                 
                 idx_size T_gates_idx = 0;
                 for (; i < gates.size(); ++i) {
                     idx_size XYH_count = 0;
                     move_count = 0;
                     if (gates[i].GetType() == Gate::Type::t && T_gates_idx == 0)
                         T_gates_idx = i;
                     
                     if (gates[i].GetType() == Gate::Type::x_1_2
                         || gates[i].GetType() == Gate::Type::y_1_2
                         || gates[i].GetType() == Gate::Type::h) {
                         for (j = i; j < gates.size() && gates[j].GetType() != Gate::Type::cz; ++j) {
                             for (idx_size k = 0; k < xCZ_q.size(); ++k) {
                                 if (gates[j].GetQubits().back() == xCZ_q[k]) {
                                     if (j < (next_CZ - move_count)) {
                                         swap(gates[next_CZ - move_count++], gates[j]);
                                        --j;
                                     }
                                     else
                                         XYH_count = next_CZ - j + 1;
                                     break;
                                 }
                             }
                             if (XYH_count != 0) break;
                         }
                         for (idx_size k = 0; k < xCZ_counter
                              && gates[next_CZ - XYH_count - k].GetType() != Gate::Type::cz ; ++k)
                             swap(gates[last_CZ - k], gates[next_CZ - XYH_count - k]);
                         
                         if (T_gates_idx != 0) {
                             for (int k = 0; gates[T_gates_idx + k].GetType() == Gate::Type::t ; ++k)
                                 swap(gates[last_CZ - xCZ_counter + k + 1], gates[T_gates_idx + k]);
                         }
                         
                         break;
                     }
                 }
                 transition_cycle = false;
                 j = next_CZ;
            }
             
             i = j;
             total_xCZ_count += count_xCZ;
         }
    }
    
    GroupSimilarGates();
    
    return pair<int, int> (count_CZ, total_xCZ_count);
}

idx_size Circuit::
CalculateTotalNumCycles(const Config* config)
{
    return ::CalculateTotalNumCycles(qubits, config, clock_cycles, gates, *qp);
}

void Circuit::
CoalesceRzGates()
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

int Circuit::
ComputeNumberOfHighValuedQubits(int num_qubits)
{
    double num_XY_gates = 0;
    vector<int> XY_gates_per_qubit(qubits, 0);
    for (idx_size i = qubits; i < gates.size(); ++i) {
        if (gates[i].GetType() == Gate::Type::x_1_2 || gates[i].GetType() == Gate::Type::y_1_2) {
            ++num_XY_gates;
            ++XY_gates_per_qubit[gates[i].GetQubits().back()];
        }
    }
    
    double high_XY = 0, th = 0;
    for (; th < num_qubits; ++th) {
        high_XY += XY_gates_per_qubit[th];
        if (th <= (num_qubits/2 - 2) && th > (num_qubits/2 - 5)  && high_XY/num_XY_gates >= 0.40)
            break;
    }
    return th;
}

void Circuit::
WriteCircuitToFile(const string& out_file)
{
    const idx_size qubit_minus_1 = qubits - 1;
    
    ofstream file;
    file.open(out_file);
    
    file << qubits << "\n";
    
    for (idx_size i = 0; i < gates.size(); ++i) {
        file << GetCycleNumForGateIdx(i) << " "
        << gates[i].TypeToString() << " ";
        
        if (gates[i].GetQubits().size() == 2)
            file << qubit_minus_1 - gates[i].GetQubits()[1] << " ";
        
        file << qubit_minus_1 - gates[i].GetQubits()[0] << "\n";
    }
    
    file.close();
}

void Circuit::
ReadGoogleCircuitFile(const string& input_file,
                      const idx_size depth,
                      const idx_size add_layer_H)
{
    google = true;
    ifstream file;
    file.open(input_file);
    
    file >> qubits;
    
    for (int i = 0; i < qubits; ++i)
        classical_bits.push_back(0);
    
    short current_cycle = 0, prev_cycle = 0;
    string gate_type;
    
    while (file >> current_cycle >> gate_type) {
        
        if (current_cycle >= depth && depth != 0)
            break;
        if (prev_cycle != current_cycle) {
            clock_cycles.push_back(gates.size());
            prev_cycle = current_cycle;
        }
        
        if (gate_type == "cz") {
            short q1, q2;
            file >> q1 >> q2;
            const auto gate = gate_funcs[gate_type]({static_cast<float>(qubits - 1 - q1), static_cast<float>(qubits - 1 - q2)});
            gates.push_back(gate);
        }
        else if(gate_type == "rz") {
            float q, p;
            file >> q >> p;
            const auto gate = gate_funcs[gate_type]({static_cast<float>(qubits - 1 - q), static_cast<float>(p)});
            gates.push_back(gate);
        }
        else {
            short q1;
            file >> q1;
            const auto gate = gate_funcs[gate_type]({static_cast<float>(qubits - 1 - q1)});
            gates.push_back(gate);
        }
    }
    clock_cycles.push_back(gates.size());
    if (add_layer_H != 0) {
        for (int i = 0; i < add_layer_H; ++i) {
            for (int q = 0; q < qubits; ++q)
                gates.push_back(create_Hadamard({static_cast<float>(q)}));
            clock_cycles.push_back(gates.size());
        }
    }
}

void Circuit::
OptimizeCircuitArrangement(const Config* config,
                           bool write_circuit_mode)
{
    // TODO: Find a better solution here. Super naive temporary solution.
    if (!IsRearranged()) {
        vector<Gate> gates_op2 = gates;
        
        // First option circuit preprocessing
        if (!ClockCycleEmpty())
            GroupAlternateCycles();
        ClusterSimilarGates();
        
        if (config -> sim_type != Config::SimType::FullState)
            MovexCZGatesRewrite(config -> proc_prefix_bits,
                                config -> ranges_bits, config -> dfs_length,
                                config -> nearest_neighbors);
        PostProcessAfterClustering(qubits, gates, clock_cycles);
#ifdef PrintG
        PrintGates(gates, qubits, *qp);
#endif
        
        vector<Gate> gates_op1 = gates;
        vector<idx_size> clock_cycles_op1 = clock_cycles;
        idx_size num_cycles_op1 = clock_cycles.size();
        
        // Second option circuit preprocessing
        gates = gates_op2;
        ClusterSimilarGates();
        
        if (config -> sim_type != Config::SimType::FullState)
            MovexCZGatesRewrite(config -> proc_prefix_bits,
                                config -> ranges_bits, config -> dfs_length,
                                config -> nearest_neighbors);
        PostProcessAfterClustering(qubits, gates, clock_cycles);
#ifdef PrintG
        PrintGates(gates, qubits, *qp);
#endif
        idx_size num_cycles_op2 = clock_cycles.size();
        
        // Decide between two options
        if (config -> sim_type  == Config::SimType::FullState) {
            if (num_cycles_op1 < num_cycles_op2) {
                gates = gates_op1;
                clock_cycles = clock_cycles_op1;
            }
        }
        else {
            idx_size op1_cycles = ::CalculateTotalNumCycles(qubits, config, clock_cycles_op1, gates_op1, *qp),
                     op2_cycles = CalculateTotalNumCycles(config), chosen_cycles = op2_cycles;
            if (op1_cycles < op2_cycles) {
                gates = gates_op1;
                clock_cycles = clock_cycles_op1;
                chosen_cycles = op1_cycles;
            }
            
            if (write_circuit_mode) {
                cout << "Number of memory passes in circuit : " << chosen_cycles << "\n\nCircuit:\n";
                PrintGates(gates, qubits, *qp);
            }
        }
        
#ifdef PrintG
        PrintGates(gates, qubits, *qp);
#endif
        rearranged = true;
    }
}

void Circuit::
InitializeCircuitConfig(const Config* config)
{
    bool H_sims = config -> sim_type == Config::SimType::LosslessH ||
    config -> sim_type == Config::SimType::Approx1CutH || config -> sim_type == Config::SimType::ApproxCZPathH2011;
    
    qp = make_shared<QubitPartition>(H_sims ?
    QubitPartition(QubitPartition::Cuts::Horizontal, GetNumQubits(),
                   config -> row_major, config -> hcut, config -> first_part_smaller) :
    QubitPartition(QubitPartition::Cuts::Vertical, GetNumQubits(), config -> row_major,
                   config -> vcut, config -> first_part_smaller)) ;
}

pair<int, int> Circuit::
GetTwoQGateCount() const
{
    const idx_size num_qubits_minus_1 = qubits - 1;
    idx_size total_CZ = 0, total_xCZ = 0;
    
    for (idx_size i = 0; i < gates.size(); ++i) {
        idx_size j = i;
        idx_size count_xCZ_per_cycle = 0;
        for (; j < gates.size() && gates[j].GetType() == Gate::Type::cz; ++j) {
            ++total_CZ;
            const auto& gate_qubits = gates[j].GetQubits();
            idx_size q0 = num_qubits_minus_1 - gate_qubits[0], q1 = num_qubits_minus_1 - gate_qubits[1];

            if (qp -> globalToBlock(q0) !=  qp -> globalToBlock(q1)) ++count_xCZ_per_cycle;
        }
        total_xCZ += count_xCZ_per_cycle;
        i = j;
    }
    
    return pair<int, int>{total_CZ, total_xCZ};
}

int Circuit::
GetNumQubits() const
{
    return qubits;
}

idx_size Circuit::
GetTotalNumGates() const
{
    return gates.size();
}

bool Circuit::
ClockCycleEmpty() const
{
    return clock_cycles.empty();
}

int Circuit::
GateIndexForCycle(int cycle_num) const
{
    if (cycle_num < 0 || cycle_num > (int)clock_cycles.size())
        return -1;
    
    return (int)clock_cycles[cycle_num];
}

Gate& Circuit::
GetGateFromIndex(idx_size i)
{
    return gates[i];
}

idx_size Circuit::
GetNumCycles() const
{
    return clock_cycles.size();
}

const vector<Gate>& Circuit::
GetGates() const
{
    return gates;
}

int Circuit::
GetCycleNumForGateIdx(idx_size gate_idx) const
{
    return ::GetCycleNumForGateIdx(gate_idx, clock_cycles);
}

bool Circuit::
IsRearranged() const
{
    return rearranged;
}

bool Circuit::
IsCrossingGate(idx_size gate_idx) const
{
    return ::IsCrossingGate(gate_idx, qubits, gates, *qp);
}
