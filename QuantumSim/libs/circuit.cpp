//
//  circuit.cpp
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 12/12/17.
//

#include "circuit.h"

vector<string> Circuit::quiddpro_func;
unordered_map<string, gate_generator_ptr> Circuit::gate_funcs;

Circuit::
Circuit() : gates({}), clock_cycles({}), qubits(0)
{
    quiddpro_func.push_back("hadamard");
    quiddpro_func.push_back("sigma_x");
    quiddpro_func.push_back("sigma_y");
    quiddpro_func.push_back("sigma_z");
    quiddpro_func.push_back("");
    quiddpro_func.push_back("rx");
    quiddpro_func.push_back("ry");
    quiddpro_func.push_back("rz");
    quiddpro_func.push_back("phase");
    
    gate_funcs["h"] = create_Hadamard;
    gate_funcs["t"] = create_T;
    gate_funcs["y_1_2"] = create_Y_1_2;
    gate_funcs["x_1_2"] = create_X_1_2;
    gate_funcs["cz"] = create_CZ;
    gate_funcs["rz"] = create_Z_rotation;
}

Circuit::
Circuit(const Circuit& g)
: gates(g.gates), clock_cycles(g.clock_cycles), qubits(g.qubits) {}

Circuit& Circuit::
operator=(const Circuit& g)
{
    Circuit temp(g);
    swap(qubits, temp.qubits);
    swap(gates, temp.gates);
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
            
            if (gates[g_i + k].GetType() == Gate::Type::T ||
                gates[g_i + k].GetType() == Gate::Type::ControlZ) {
                
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
    unordered_map<idx_size, Gate::Type> gate_obstruction;
    
    clock_cycles.clear();
        
    // Assume a non-diag gate incident on same qubit initiates a new cycle
    for (idx_size i = qubits; i < gates.size(); ++i) {
        if (gates[i].GetType() == Gate::ControlZ || gates[i].GetType() == Gate::Hadamard) {
            clock_cycles.push_back(i);
            
            for (; i < gates.size() && gates[i].GetType() == Gate::Type::ControlZ; ++i) {}
            for (; i < gates.size() && gates[i].GetType() != Gate::Type::ControlZ; ++i) {}
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
        
        if (gates[j].GetType() == Gate::Type::ControlZ) {
            ++count_CZ;
            if(saw_Y || saw_X || saw_H) {
                g_i = j;
                last_CZ = 0; last_T = 0; last_X = 0; last_Y = 0;
                saw_CZ = false; saw_T = false; saw_X = false; saw_Y = false; saw_H = false;
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
        else if (gates[j].GetType() == Gate::Type::T) {
            if(saw_Y || saw_X) {
                g_i = j;
                last_CZ = 0; last_T = 0; last_X = 0; last_Y = 0;
                saw_CZ = false; saw_T = false; saw_X = false; saw_Y = false; saw_H = false;
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
        else if (gates[j].GetType() == Gate::Type::X_1_2) {
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
        else if (gates[j].GetType() == Gate::Type::Y_1_2) {
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
        else if (gates[j].GetType() == Gate::Type::Hadamard)
            saw_H = true;
    }
#ifdef PrintG
    PrintGates(gates, qubits);
    cout << "\nGate insert count: " << count_insert << endl;
#endif
    
    return (int)count_CZ;
}

idx_size Circuit::
ClusterSimilarGates()
{
    return ::ClusterSimilarGates(gates, qubits);
}

// Inserts the new gates to the relevant clusters in the next cycles
idx_size Circuit::
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
}

// Generalize this to other >1q gates that cross
pair<int, int> Circuit::
MovexCZGatesRewrite(idx_size proc_prefix_bits,
                    idx_size range_bits,
                    idx_size branch_bits,
                    const QubitPartition& qp,
                    const bool nearest_neigbors)
{
    
    const idx_size num_qubits_minus_1 = qp.getNumQubits() - 1, num_crossing_q = qp.getNumX();
    int remaining_path_bits = static_cast<int>(proc_prefix_bits), total_CZ = 0, total_xCZ = 0;
    Config::SimMode current_mode = Config::SimMode::ProcPrefix;
    bool continue_rearranging = proc_prefix_bits != 0;
    
    for (idx_size i = 0; i < gates.size() && continue_rearranging; ++i) {
        idx_size j = i;
        int num_xCZ_in_cluster = 0;
        vector<idx_size> gates_to_delete;
        unordered_map<idx_size, bool> recorded_qubit;
        vector<Gate> gates_to_move;
        
        // Find all xCZ in cycle
        for (; j < gates.size(); ++j) {
           
            const auto& gate_qubits = gates[j].GetQubits();
            
            if (nearest_neigbors && gate_qubits.size() > 2) {
                idx_size q0 = num_qubits_minus_1 - gate_qubits[0], q1 = num_qubits_minus_1 - gate_qubits[1];
                CheckIfNearestNeighbor(q0, q1, qp);
            }
            
            bool add_gate = false;
            idx_size q0 = gate_qubits[0];
            if (gate_qubits.size() > 0) {
                if ((recorded_qubit.count(q0) > 0  && recorded_qubit[q0])
                    || (recorded_qubit.count(q0) > 0  && !gates[j].IsDiagonal())) {
                    add_gate = true;
                    recorded_qubit[q0] = true;
                }
            }
            if (gate_qubits.size() == 2) {
                idx_size q1 = gate_qubits[1];

                if (qp.globalToBlock(num_qubits_minus_1 - q0) !=  qp.globalToBlock(num_qubits_minus_1 - q1)) {
                    if (num_xCZ_in_cluster >= num_crossing_q || remaining_path_bits - num_xCZ_in_cluster <= 0)
                        goto exit_inner_loop;
                    
                    ++num_xCZ_in_cluster;
                    if (recorded_qubit.count(q0) == 0) {
                        recorded_qubit[q0] = false;
                        recorded_qubit[q1] = false;
                    }
                    add_gate = true;
                }
                else if ((recorded_qubit.count(q1) > 0 && recorded_qubit[q1])
                         || (recorded_qubit.count(q1) > 0 && !gates[j].IsDiagonal())) {
                    add_gate = true;
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
        }
        
        exit_inner_loop:;
        
        if (num_xCZ_in_cluster) {
            
            InsertNewCycleOnClusteredCircuit(j, qubits, gates, gates_to_move, qp);
            
            for (int k = (int)gates_to_delete.size() - 1; k >=0; --k) {
                gates.erase(gates.begin() + gates_to_delete[k]);
                --j;
            }
            
//            idx_size end_idx = clock_cycles[GetCycleNumForGateIdx(j) + 5];
            ::ClusterSimilarGates(gates, qubits, j);
            
        #ifdef PrintG
            PrintGates(gates, qubits);
        #endif
            
            idx_size xGate_count = 0;
            for (; j < gates.size() && xGate_count < num_xCZ_in_cluster; ++j) {
                const auto& gate_qubits = gates[j].GetQubits();

                if (gate_qubits.size() > 1) {
                    idx_size q0 = qubits - 1 - gate_qubits[0], q1 =  qubits - 1  - gate_qubits[1];
                    
                    if (qp.globalToBlock(q0) !=  qp.globalToBlock(q1))
                        ++xGate_count;
                }
            }
        }
        
        // Move the xCZ in the transitioning cycle further out to have more gates in preceding paths
        // The number of gates in the branching path should be the least
        remaining_path_bits -= num_xCZ_in_cluster;
        if (remaining_path_bits <= 0 && num_xCZ_in_cluster > 0) {
            switch (current_mode) {
                case Config::SimMode::ProcPrefix:
                    remaining_path_bits = static_cast<int>(range_bits);
                    current_mode = Config::SimMode::Ranges;
                    if (range_bits == 0) continue_rearranging = false;
                    break;
                case Config::SimMode::Ranges:
                    remaining_path_bits = static_cast<int>(branch_bits);
                    current_mode = Config::SimMode::Branch;
                    if (range_bits == 0) continue_rearranging = false;
                    break;
                case Config::SimMode::Branch:
                    break;
            }
        }
        i = j;
    }
    
    
    // This final loop is mainly for counting. Swapping is not necessary because order is not important
    // since all gates are wrapped in bitmasks.
    for (idx_size i = 0; i < gates.size(); ++i) {
        idx_size j = i;
        idx_size count_xCZ_per_cycle = 0;
        for (; j < gates.size() && gates[j].GetType() == Gate::Type::ControlZ; ++j) {
            ++total_CZ;
            const auto& gate_qubits = gates[j].GetQubits();
            idx_size q0 = num_qubits_minus_1 - gate_qubits[0], q1 = num_qubits_minus_1 - gate_qubits[1];

            if (qp.globalToBlock(q0) !=  qp.globalToBlock(q1)) {
                swap(gates[j], gates[i + count_xCZ_per_cycle]);
                ++count_xCZ_per_cycle;
            }
        }
        total_xCZ += count_xCZ_per_cycle;
        i = j;
    }
#ifdef PrintG
    PrintGates(gates, qubits);
#endif
        
    return pair<int, int> (total_CZ, total_xCZ);
}


pair<int, int> Circuit::
MovexCZGates(idx_size proc_prefix_bits,
             idx_size range_bits,
             idx_size branch_bits,
             const QubitPartition& qp,
             const bool nearest_neigbors)
{
    int total_xCZ_count = 0, count_CZ = 0;
    int num_q_1 = qp.getNumQubits() - 1;
    idx_size curr_bit_counter = proc_prefix_bits;
    bool transition_cycle = false, last_cycle = false;
    int curr_path = 0;
  
    for (idx_size i = qubits; i < gates.size(); ++i) {
         if (gates[i].GetType() == Gate::Type::ControlZ) {
             idx_size count_xCZ = 0;
             idx_size j = i;
             for (; j < gates.size() && gates[j].GetType() == Gate::Type::ControlZ; ++j) {
                 ++count_CZ;
                 idx_size q0 = num_q_1 - gates[j].GetQubits().front(), q1 = num_q_1 - gates[j].GetQubits().back();
                 
                 if (nearest_neigbors) {
                     idx_size x0 = q0 % qp.GetColumns(), y0 = q0 / qp.GetColumns(),
                     x1 = q1 % qp.GetColumns(), y1 = q1 / qp.GetColumns();
                     
                     if (!((x0 == x1 && (y0 + 1 == y1 || y0 - 1 == y1))
                           || ((x0 + 1 == x1 || x0 - 1 == x1) && (y0 == y1)))) {
                         cerr << "\n\n2 qubit gates are not acting on nearest neighbors.\n";
                         exit(1);
                     }
                 }
                 
                 if (qp.globalToBlock(q0) !=  qp.globalToBlock(q1)) {
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
                      && gates[k].GetType() != Gate::Type::ControlZ; ++k)
                     ++next_CZ;
                 
                 idx_size T_gates_idx = 0;
                 for (; i < gates.size(); ++i) {
                     idx_size XYH_count = 0;
                     move_count = 0;
                     if (gates[i].GetType() == Gate::Type::T && T_gates_idx == 0)
                         T_gates_idx = i;
                     
                     if (gates[i].GetType() == Gate::Type::X_1_2
                         || gates[i].GetType() == Gate::Type::Y_1_2
                         || gates[i].GetType() == Gate::Type::Hadamard) {
                         for (j = i; j < gates.size() && gates[j].GetType() != Gate::Type::ControlZ; ++j) {
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
                              && gates[next_CZ - XYH_count - k].GetType() != Gate::Type::ControlZ ; ++k)
                             swap(gates[last_CZ - k], gates[next_CZ - XYH_count - k]);
                         
                         if (T_gates_idx != 0) {
                             for (int k = 0; gates[T_gates_idx + k].GetType() == Gate::Type::T ; ++k)
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
//    int c = 0;
//    for (auto& g : gates) {
//        ++c;
//        cout << c << " " ;
//        if(g.ids.back() == Gate::Type::Z) {
//            cout << "CZ ";
//            cout << g.qubits.front() << " " << g.qubits.back() << "\n";
//        }
//        else {
//            if (g.ids.back() == Gate::Type::X_1_2)
//    
//                cout << "X ";
//            else if (g.ids.back() == Gate::Type::Y_1_2)
//                cout << "Y ";
//            else if (g.ids.back() == Gate::Type::T)
//                cout << "T ";
//            else if (g.ids.back() == Gate::Type::Hadamard)
//                cout << "H ";
//            cout << g.qubits.back() << "\n";
//        }
//    }
//    cout << "\n\n";
    return pair<int, int> (count_CZ, total_xCZ_count);
}

int Circuit::
ComputeNumberOfHighValuedQubits(int num_qubits)
{
    double num_XY_gates = 0;
    vector<int> XY_gates_per_qubit(qubits, 0);
    for (idx_size i = qubits; i < gates.size(); ++i) {
        if (gates[i].GetType() == Gate::Type::X_1_2 || gates[i].GetType() == Gate::Type::Y_1_2) {
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
CreateGoogleCircuit(int q, int num_clock_cycles)
{
    srand(time(NULL));
    google = true;
    
    short GS_gates[3] = {Gate::Type::X_rotation, Gate::Type::Y_rotation, Gate::Type::T};
    
    int GS_gates_num = 3;
    
    //Each row represents a qubit. This is to keep track of the
    //gates applied to this qubit.
    qubits = q;
    vector<vector<idx_size>> qubit_to_gates(qubits);
  
    auto count_wires_gate = [&](vector<short>& CZ_pairs) {
        int count = 0;
        for (idx_size i = 0; i < qubit_to_gates.size(); ++i) {
            if (gates[qubit_to_gates[i].back()].GetType() == Gate::Type::ControlZ) {
                CZ_pairs.push_back(i);
                count += 1;
            }
        }
        return count;
    };
    
    vector<bool> T_gate_allowed(qubits, true);
    
    //Start by applying Hadamard Gates
    for (int i = 0; i < qubits; ++i) {
        classical_bits.push_back(0);
        gates.push_back(create_Hadamard({static_cast<float>(i)}));
        qubit_to_gates[i].push_back(gates.size());
    }
    
    clock_cycles.push_back(gates.size());
    
    for (int i = 0; i < num_clock_cycles; i+=2) {
        vector<short> current_CZ_pairs;
        current_CZ_pairs.resize((1 + (rand() % (qubits - 1))) * 2);
        
        int CZ_size = (int) current_CZ_pairs.size();
        int CZ_q = abs(qubits - CZ_size/2);
        CZ_q = rand() % CZ_q;
        for (int j = 0; j < CZ_size; ++j) {
            //            int CZ_q = rand () % (qubits - 1) ;
            //            while(!check_reoccurance(current_CZ_pairs, CZ_q, CZ_q + 1)) {
            //                CZ_q = rand() % (qubits - 1);
            //            }
            
            current_CZ_pairs[j++] = CZ_q;
            qubit_to_gates[CZ_q].push_back(gates.size());
            
            current_CZ_pairs[j] = ++CZ_q;
        }
        qubit_to_gates[CZ_q].push_back(gates.size());
        
        //control phase Gate
        int k = 0;
        for (idx_size j = 0; j < current_CZ_pairs.size()/2; ++j)
            gates.push_back(create_CZ({static_cast<float>(current_CZ_pairs[k++]), static_cast<float>(current_CZ_pairs[k++])}));
        
        if (current_CZ_pairs.size() > 0)
            clock_cycles.push_back(gates.size());
        
        /*
         • Place a Gate at qubit q only if this qubit is occupied by a CZ Gate in the previous cycle.
         • Place a T Gate at qubit q if there are no single- qubit gates in the previous cycles at
         qubit q except for the initial cycle of Hadamard gates.
         • Any Gate at qubit q should be different from the Gate at qubit q in the previous cycle.
         */
        
        vector<short> CZ_pairs;
        int qubits_for_gates = 1 + rand() % (count_wires_gate(CZ_pairs) - 1);
        vector<short> complied_qubits(qubits_for_gates, -1);
        
        for (int j = 0; j < qubits_for_gates; ++j) {
            int q = rand() % qubits_for_gates;
            while(gates[qubit_to_gates[CZ_pairs[q]].back()].GetType() != Gate::Type::ControlZ) {
                q = rand() % qubits_for_gates;
            }
            complied_qubits[j] = CZ_pairs[q];
            
            short gate_to_apply;
            if (T_gate_allowed[CZ_pairs[q]]) {
                gate_to_apply = rand() % GS_gates_num;
                while (gates[qubit_to_gates[CZ_pairs[q]].back()].GetType() == GS_gates[gate_to_apply]) {
                    gate_to_apply = rand() % GS_gates_num;
                }
                if (gate_to_apply == 2) {
                    T_gate_allowed[CZ_pairs[q]] = false;
                }
            }
            else {
                gate_to_apply = rand() % (GS_gates_num - 1);
                while (gates[qubit_to_gates[CZ_pairs[q]].back()].GetType() == GS_gates[gate_to_apply]) {
                    gate_to_apply = rand() % (GS_gates_num - 1);
                }
            }
                        
            if (gate_to_apply == 0)
                gates.push_back(create_X_1_2({static_cast<float>(CZ_pairs[q])}));
            else if (gate_to_apply == 1)
                gates.push_back(create_Y_1_2({static_cast<float>(CZ_pairs[q])}));
            else if (gate_to_apply == 2)
                gates.push_back(create_T({static_cast<float>(CZ_pairs[q])}));
            qubit_to_gates[CZ_pairs[q]].push_back(gates.size());
        }
        if(qubits_for_gates > 0)
           clock_cycles.push_back(gates.size());
    }
    
    //    g_t_q_size = gates_to_qubits.size();
    //    q_circuit -> gates_to_qubits.resize(q_circuit -> gates_to_qubits.size() + qubits);
    //    for (idx_size i = 0; i < qubits; ++i) {
    //        Gate temp = create_I();
    //        temp -> id.push_back(Gate::Type::Measurement);
    //        q_circuit -> gates.push_back(temp);
    //        q_circuit -> gates_to_qubits[g_t_q_size + i].push_back(i);
    //    }
}

void Circuit::
CreateQuiddProScript(const string& out_file,
                     int layers_last_H)
{
    ofstream file;
    file.open(out_file);
    
    file << "state = cb(\"" + to_string((int)real(classical_bits[0]))<< "\");\n";;
    
    for (idx_size q = 1; q < classical_bits.size(); ++q)
        file << "state = kron(state, cb(\"" + to_string(classical_bits[q]) << "\"));\n";
    
    for (idx_size i = 0; i < gates.size(); ++i) {
        if(i != 0)
            file << ";\n";
        int op_count = 0;
        bool control = false;
        
        if (gates[i].GetType() == Gate::Type::Measurement) {
            file << "measure_sv(" + to_string(gates[i].GetQubits()[0] + 1)
            + ", state)";
            continue;
        }
        
        if (gates[i].GetType() == Gate::Type::T) {
            file << "T = [1 0 ; 0 " + to_string(0.707106781) + "+i*" + to_string(0.707106781) + "];\n";
            file << "op" + to_string(op_count++) + " = cu_gate (T, \"";
        }
        else if (gates[i].GetType() == Gate::Type::X_1_2) {
            file << "X_1_2 = [0.5+i*0.5 0.5-i*0.5 ; 0.5-i*0.5 0.5+i*0.5];\n";
            file << "op" + to_string(op_count++) + " = cu_gate (X_1_2, \"";
        }
        else if (gates[i].GetType() == Gate::Type::Y_1_2) {
            file << "Y_1_2 = [0.5+i*0.5 -0.5-i*0.5 ; 0.5+i*0.5 0.5+i*0.5];\n";
            file << "op" + to_string(op_count++) + " = cu_gate (Y_1_2, \"";
        }
        else if (gates[i].GetType() != Gate::Type::ControlZ) {
            file << "op" + to_string(op_count++)  + " = cu_gate(" + quiddpro_func[gates[i].GetType()] + "(";
            
            if (gates[i].GetType() < Gate::Type::X_rotation || gates[i].GetType() == Gate::Type::Phase) {
                file << "1), \"";
            }
            else if (gates[i].GetType() < Gate::Type::ControlZ) {
                file << to_string(gates[i].GetTheta()[0]) + " * pi, 1), \"";
            }
        }
        
        if (gates[i].GetType() != Gate::Type::ControlZ) {
            int q = 0;
            if (control) {
                for (; q < gates[i].GetNumControls(); ++q) {
                    file << "c" + to_string(gates[i].GetQubits()[q] + 1);
                }
            }
            
            auto num_qubits = (int)gates[i].GetQubits().size();
            for (; q < num_qubits; ++q) {
                file << "x" + to_string(gates[i].GetQubits()[q] + 1);
            }
            file << "\", " + to_string(qubits) << ");\n";
        }
        else
            control = true;
        
        if(op_count > 1) {
            file << "op = ";
            for (int n = 0; n < op_count; ++n) {
                if (n != 0)
                    file << "-";
                
                file << "op" + to_string(n);
            }
            file << ";\n";
            file << "state = op * state";
        }
        else
            file << "state = op0 * state";
    }
    
    if (layers_last_H && gates.back().GetType() != Gate::Type::Hadamard) {
        for (int j = 0; j < layers_last_H; ++j) {        
            for (int i = 0; i < qubits; ++i) {
                file << ";\nop0 = cu_gate(hadamard(1), \"x" << i + 1 << "\" ," << qubits
                << ");\nstate = op0 * state";
            }
        }
    }
}

void Circuit::
WriteGeneratedCircuitFile(const string& out_file,
                          const idx_size size_q)
{
    if ( size_q == 0)
        throw "Create Circuit first by entering qubits";
    
    ofstream file;
    file.open(out_file);
    
    file << qubits << "\n";
    
    for (idx_size i = 0; i < classical_bits.size(); ++i)
        file << classical_bits[0] << " ";
    file << "\n";
    for (idx_size i = 0; i < size_q; ++i) {
        file << 0;
        file << " " ;
    }
    file << "\n";
    
    idx_size size_g = gates.size();
    
    if ( size_g == 0)
        throw "Create Circuit first by entering gates";
    
    for (idx_size i = 0; i < size_g; ++i) {
        idx_size size_r = gates[i].GetMatrix().size();
        for(size_t j = 0; j < size_r; ++j) {
            int size_c = (int)gates[i].GetMatrix()[j].size();
            for (int k = 0; k < size_c; ++k) {
                file << real(gates[i].GetMatrix()[j][k]);
                
                if (imag(gates[i].GetMatrix()[j][k]) != 0)
                    file << " + " << imag(gates[i].GetMatrix()[j][k]) << " i";
                
                if (k != size_c-1)
                    file << " , ";
                
            }
            file << " & ";
        }
        file << "\n";
        file << "g ";
        file << gates[i].GetType() << " ";
        
        file << "\nt ";
        for ( auto theta : gates[i].GetTheta())
            file << theta << " ";
        
        
        file << "\n";
        file << "q ";
        for (const auto& q : gates[i].GetQubits())
            file << q << " ";
        
        file << "\n";
        file << "c " << gates[i].GetNumControls() << "\n";
    }
    file.close();
}

void Circuit::
ReadGoogleCircuitFile(const string& input_file,
                      const int depth,
                      const int add_layer_H)
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
    for (int c = 0; c < (int)clock_cycles.size(); ++c) {
        if (gate_idx < (idx_size)GateIndexForCycle(c))
            return c;
    }
    return (int)clock_cycles.size();
}
