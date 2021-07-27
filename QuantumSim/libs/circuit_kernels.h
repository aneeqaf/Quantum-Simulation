//
//  circuit_kernels.hpp
//  QuantumSimProj
//
//  Created by Aneeqa Fatima on 5/24/21.
//  Copyright © 2021 Aneeqa Fatima. All rights reserved.
//

#ifndef circuit_kernels_h
#define circuit_kernels_h

#include <algorithm>
#include <boost/container_hash/hash.hpp>
#include <iterator>
#include <unordered_set>

#include "gates.h"
#include "state_interface.h"

typedef vector<idx_size> qubits_list_t;

struct GateMovement {
    vector<Gate> gates_list;
    bool IsObstructed;
};

struct ObstructedQubit {
    qubits_list_t qubits;
    idx_size obstruction_idx;
    
    bool operator() (const ObstructedQubit& lhs, const ObstructedQubit& rhs) const {
        if (lhs.obstruction_idx == rhs.obstruction_idx)
            return lhs.qubits < rhs.qubits;
        return lhs.obstruction_idx < rhs.obstruction_idx;
    }
};

struct NonCommutingObstructionKeyHash {
    size_t operator()(const vector<idx_size>& k) const
    {
        return boost::hash_range(k.begin(), k.end());
    }
};
 
struct NonCommutingObstructionKeyEqual {
    bool operator()(const vector<idx_size>& lhs, const vector<idx_size>& rhs) const
    {
        for (idx_size i = 0; i < lhs.size() && i < rhs.size(); ++i)
        if (lhs[i] == rhs[i]) return true;
        
        return false;
    }
};

struct QubitsListKeyCmp {
    // assume lhs and rhs sorted
    bool operator()(const vector<idx_size>& lhs, const vector<idx_size>& rhs) const {
      return lhs[0] < rhs[0];
    }
};

typedef unordered_map<idx_size, shared_ptr<GateMovement>> gate_movement_unordered_map_t;

void
PrintGates(const vector<Gate>& gates,
           idx_size num_qubits,
           const QubitPartition& qp);

void
PrintGatesAndCycles(const vector<Gate>& gates,
                    const vector<idx_size>& clock_cycles);

void CheckIfNearestNeighbor(idx_size q0,
                            idx_size q1,
                            const QubitPartition& qp);

idx_size
ClusterSimilarGates(vector<Gate>& gates,
                    idx_size num_qubits,
                    const QubitPartition& qp,
                    idx_size start_idx = 0,
                    idx_size end_idx = 0);

idx_size
InsertNewCycleOnClusteredCircuit(idx_size gate_idx,
                                 idx_size num_qubits,
                                 vector<Gate>& circuit_gates,
                                 vector<Gate>& gates_to_insert,
                                 const QubitPartition& qp);

#endif /* circuit_kernels_h */
