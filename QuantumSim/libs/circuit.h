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
    static unordered_map<string, gate_generator_ptr> gate_funcs;
    vector<Gate> gates;
    vector<idx_size> clock_cycles;
    vector<int> classical_bits;
    shared_ptr<QubitPartition> qp;
    idx_size qubits;
    bool rearranged;
 
public:
    bool google ;
    
    void GroupAlternateCycles();
    void RecalibrateGoogleClockCycles();
    idx_size GroupSimilarGates();
    idx_size ClusterSimilarGates();
    pair<int, int> MovexCZGatesRewrite(idx_size proc_prefix_bits,
                                       idx_size range_bits,
                                       idx_size branch_bits,
                                       const bool nearest_neigbors);
    pair<int, int> MovexCZGates(idx_size proc_prefix_bits,
                                idx_size range_bits,
                                idx_size branch_bits,
                                const bool nearest_neigbors = true);
    void CoalesceRzGates();
    int ComputeNumberOfHighValuedQubits(int num_qubits);
    void ReadGoogleCircuitFile(const string& input_file,
                               const idx_size depth,
                               const idx_size add_layer_H = 0);
    void WriteCircuitToFile(const string& out_file);
    void OptimizeCircuitArrangement(const Config* config);
    void InitializeCircuitConfig(const Config* config);
    
    int GetNumQubits() const;
    idx_size GetTotalNumGates() const;
    bool ClockCycleEmpty() const;
    idx_size GetNumCycles() const;
    int GateIndexForCycle(int cycle_num) const;
    Gate& GetGateFromIndex(idx_size i);
    const vector<Gate>& GetGates() const;
    int GetCycleNumForGateIdx(idx_size gate_idx) const;
    pair<int, int> GetTwoQGateCount() const;
    bool isRearranged() const;
    bool isCrossingGate(idx_size gate_idx) const;
    
    Circuit(const string input_filename, idx_size num_q, idx_size depth);
    Circuit(const Circuit& rhs);
    Circuit& operator=(const Circuit& rhs);
};

#endif /* circuit_h */
