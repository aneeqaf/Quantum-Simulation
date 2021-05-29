//
//  circuit.h
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 12/12/17.
//

#ifndef circuit_h
#define circuit_h

#include <algorithm>
#include <fstream>
#include <map>
#include <memory>
#include <iostream>
#include <set>
#include <string>
#include <stdio.h>
#include <vector>

#include "circuit_kernels.h"
#include "gates.h"
#include "state_interface.h"

using namespace std;

using idx_size = unsigned long long;

class Circuit {
private:
    static vector<string> quiddpro_func;
    static unordered_map<string, gate_generator_ptr> google_gate_funcs;
    vector<Gate> gates;
    vector<idx_size> clock_cycles;
    vector<int> classical_bits;
    int qubits;
 
public:
    bool google ;
    
    void GroupAlternateCycles();
    idx_size GroupSimilarGates();
    idx_size ClusterSimilarGates();
    idx_size RearrangexCZForPathConcludingCycle(idx_size gate_idx,
                                                idx_size marked_crossing_gates,
                                                int remaining_cz_path_bits,
                                                GateMovementUnorderedMap& gates_to_insert,
                                                const QubitPartition& qp);
    pair<int, int> MovexCZGatesRewrite(idx_size proc_prefix_bits,
                                       idx_size range_bits,
                                       idx_size branch_bits,
                                       const QubitPartition& qp,
                                       const bool nearest_neigbors);
    pair<int, int> MovexCZGates(idx_size proc_prefix_bits,
                                idx_size range_bits,
                                idx_size branch_bits,
                                const QubitPartition& bitmasks,
                                const bool nearest_neigbors = true);
    int ComputeNumberOfHighValuedQubits(int num_qubits);
    void ReadGoogleCircuitFile(const string& input_file,
                               const int depth,
                               const int add_layer_H = 0);
    void CreateGoogleCircuit(int qubits,
                             int clock_cycles);
    void WriteGeneratedCircuitFile(const string& out_file,
                                   const idx_size size_q);
    void CreateQuiddProScript(const string& out_file,
                              int layers_last_H = 0);
    
    int GetNumQubits() const;
    idx_size GetTotalNumGates() const;
    bool ClockCycleEmpty() const;
    idx_size GetNumCycles() const;
    int GateIndexForCycle(int cycle_num) const;
    Gate& GetGateFromIndex(idx_size i);
    const vector<Gate>& GetGates() const;
    int GetCycleNumForGateIdx(idx_size gate_idx) const;
    
    Circuit();
    Circuit(const Circuit& rhs);
    Circuit& operator=(const Circuit& rhs);
};

#endif /* circuit_h */
