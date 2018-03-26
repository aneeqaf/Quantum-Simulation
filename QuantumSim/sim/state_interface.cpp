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
Config::SimMode GenericQuantumState::sim_mode = Config::SimMode::Phase1;
Config::SimType GenericQuantumState::sim_type = Config::SimType::FullState;
bool GenericQuantumState::approx = false;
char GenericQuantumState::partition_to_sim = 'x';

#ifdef Parallel
int GenericQuantumState::num_threads = omp_get_num_procs();
#else
int GenericQuantumState::num_threads = 8;
#endif

GenericQuantumState::
GenericQuantumState(int n_threads): amps_of_interest({}){
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
        bitmask[cluster_qubits[i]] = 1;
    
    return bitmask;
}

void GenericQuantumState::
FormCZTGatesBitmask(bitset<128>* __restrict CZ_bitmasks /*total_circuit_qubits*/,
                    bitset<128> T_bitmasks[2],
                    idx_size& gate_i,
                    const vector<Gate>& all_gates,
                    const int total_circuit_qubits)
{
    for (int i = 0; i < total_circuit_qubits; ++i)
        CZ_bitmasks[i] = 0;
    
    FormBlockOfCZTGates(gate_i, CZ_bitmasks, T_bitmasks, all_gates, total_circuit_qubits);    
}

idx_size GenericQuantumState::
GetNumAddends() const
{
    return 0;
}

QubitPartition::
QubitPartition(int rows,
               int cols,
               const vector<bitset<128>>& blocks)
: _rows(rows), _cols(cols), _blocks(blocks) {
    const bitset<128> mask((1ull << (rows * cols)) - 1);
    bitset<128> all(0);
    for(auto& b : _blocks) {
        b != mask;
        if (b.count() == 0) {
            cerr << " Found an empty qubit block " << endl;
            exit(1);
        }
        all |= b;
    }
    if (all != mask) {
        cerr << " Some qubit is not in any block: " << (mask & ~all) << endl;
        exit(1);
    }
    for(idx_size i = 0; i < _blocks.size(); ++i) {
        for(idx_size j = 0; j < i; ++j) {
            if ( (_blocks[i] & _blocks[j]).count() >0 ) {
                cerr << " Blocks " << i << " " << j << " are not disjoint "
                << endl;
                exit(1);
            }
        }
    }
    InitMappings();
}

QubitPartition::
QubitPartition(const Cuts cut_type,
               int total_qubits,
               int cut)
{
    const int y_axis = FindDivisor(total_qubits),
    x_axis = total_qubits/y_axis,
    modified_q = total_qubits - 1;
    
    if (cut_type == QubitPartition::Cuts::Horizontal) {
        const int block_bits = cut ? cut : (ceil(x_axis/2.0)  * y_axis);
        
//        if (!cut)
//            (*this) = QubitPartition(y_axis, x_axis);
//        else {
            bitset<128> block0_bitmask = 0;
            for (int i = 0; i < block_bits; ++i)
                block0_bitmask [modified_q - i] = 1;
            (*this) = QubitPartition(y_axis, x_axis, block0_bitmask);
//        }
    }
    else {
        const int v_cut = !cut ? ceil(y_axis/2.0) : cut;
        
//        if(!v_cut)
//           (*this) = QubitPartition(x_axis, y_axis);
//        else {
            bitset<128> block0_bitmask = 0;
            for (int i = 0; i < x_axis; ++i) {
                for (int j = 0; j < v_cut; ++j)
                    block0_bitmask [modified_q - ((i * y_axis) + j)] = 1;
            }
           (*this) = QubitPartition(x_axis, y_axis, block0_bitmask);
//        }
    }
}

void QubitPartition::InitMappings() {
    _global_to_block.resize(getNumQubits(), -1);
    _global_to_local.resize(getNumQubits(), -1);
    _local_to_global.resize(getNumBlocks());
    
    for (int b = 0; b < (int)_blocks.size(); ++b) {
        int local = 0;
        for(int i = 0; i < getNumQubits(); ++i) {  // slow but OK
            if (_blocks[b][i] == 1) {
                if (_global_to_block[i] != -1) {
                    cerr << " Qubit " << i << " is in blocks " << b
                    << " and " << _global_to_block[i] << endl;
                    exit(1);
                }
                _global_to_block[i] = b;
                _global_to_local[i] = local++;
                _local_to_global[b].push_back(i);
            }
        }
    }
    
    // go over 2q-gate locations and check for cross-gates,
    // in which case mark boundary qubits
    for(int r = 0; r < _rows - 1; ++r) {
        for(int c = 0; c < _cols; ++c) {
            int q0 = r * _cols + c;
            int q1 = (r + 1) * _cols + c;
            if (_global_to_block[q0] != _global_to_block[q1]) {
                ++_numX;
                _boundary_qubits.set(q0);
                _boundary_qubits.set(q1);
            }
        }
    }
    for(int r = 0; r < _rows; ++r) {
        for(int c = 0; c < _cols - 1; ++c) {
            int q0 = r * _cols + c;
            int q1 = r * _cols + c + 1;
            if (_global_to_block[q0] != _global_to_block[q1]) {
                ++_numX;
                _boundary_qubits.set(q0);
                _boundary_qubits.set(q1);
            }
        }
    }
}

vector<idx_size> QubitPartition::
IndexScatter(const bitset<128>& idx) const
{
    vector<idx_size> local_idx(_blocks.size(), 0);
    int total_qubits = getNumQubits();
    
    for (int j = 0; j < total_qubits; ++j) {
        if (idx[j] == 0)
            continue;
        local_idx[globalToBlock(j)] |= (1ull << globalToLocal(j));
    }
    return local_idx;
}

// Move up boundary qubits upfront in each block,
// then repopulate _global_to_local accordingly
void QubitPartition::RenumberLocalQubits() {
    for(auto& block : _local_to_global) {
        partition(block.begin(), block.end(), [&](int i) {
            return _boundary_qubits[i]; });
        for(int i = 0; i < (int)block.size(); ++i) _global_to_local[block[i]] = i;
    }
}

//string QubitPartition::print() const {
//    stringstream s;
//    s << "\nQubit grid " << _rows << "x" << _cols << " with "
//    << getNumBlocks() << " blocks " << endl;
//    
//    if (!_blocks.empty()) {
//        s << " Blocks :";
//        if (_rows * _cols < 64) {
//            for (auto b : _blocks) s << " " << b.to_ullong();
//        } else {
//            s << endl;
//            for (auto b : _blocks) s << "  " << b << " " << endl;
//        }
//        s << endl;
//    }
//    
//    if (!_global_to_block.empty()) {
//        s << " Global to block : " << _global_to_block;
//    }
//    
//    if (!_global_to_local.empty()) {
//        s << " Global to local : " << _global_to_local;
//    }
//    
//    if (!_local_to_global.empty()) {
//        s << " Local to global : " << endl;
//        for (const auto& block : _local_to_global) {
//            s << "   " << block;
//        }
//    }
//    
//    if (_numX) {
//        s << " Cross-gates : " << _numX << endl;
//        s << " Boundary qubits :";
//        for(int i = 0; i < getNumQubits(); ++i) {
//            if (_boundary_qubits[i]) s << " " << i;
//        }
//        s << endl;
//    }
//    return s.str();
//}

int FindDivisor(int num)
{
    int div = 0;
    for (int i = 1; i <= floor(sqrt(num)); ++i){
        if (num % i == 0)
            div = i;
    }
    return div;
}

idx_size
Project1QBitmask(const bitset<128>& gate_bitmask,
                 const QubitPartition& block_bitmasks,
                 const int block_idx,
                 const bool is_zero_least_sig)
{
    idx_size projected_bitmask = 0;
    int modified_q = block_bitmasks.getNumQubits() - 1;
    int modified_q_partition = block_bitmasks.getNumQubitsInBlock(block_idx) - 1;

    bitset<128> gate_bitmask_copy = gate_bitmask;
    while (gate_bitmask_copy != 0) {
        int first_half = __builtin_ctzl(gate_bitmask_copy.to_ulong());
        int second_half = __builtin_ctzl((gate_bitmask_copy >> 64).to_ulong());
        const int q = gate_bitmask_copy.to_ulong() ? first_half : second_half ? 63 + second_half : 0;

        if (!is_zero_least_sig && block_bitmasks.globalToBlock(q) == block_idx)
            projected_bitmask |= 1ull << (block_bitmasks.globalToLocal(q));
        else if (is_zero_least_sig && block_bitmasks.globalToBlock(q) == block_idx)
            projected_bitmask |= 1ull << block_bitmasks.globalToLocal(q);
        
        cout << gate_bitmask_copy.to_string().substr(128-10) << endl;
        gate_bitmask_copy[q] = 0;
    }
    return projected_bitmask;
    cout << endl;
}

int
ProjectQubit(const int qubit_to_project,
             const bitset<128>& partition_bitmask,
             const int num_qubits)
{
    const int modified_q = num_qubits - 1;
    int c = 0;
    for (int i = 0; i < num_qubits; ++i) {
        bitset<128> temp = 0;
        temp[modified_q - i] = 1;
        if ((temp & partition_bitmask) != 0) {
            if (i == qubit_to_project)
                return c;
            ++c;
        }
    }
    assert(false);
    return -1;
}

bool
ProjectCZBitmask(bitset<128>* __restrict projected_bitmasks,
                 const QubitPartition& block_bitmasks,
                 const int block_idx,
                 const bitset<128>* __restrict gate_bitmasks)
{
    const bitset<128> block_bitmask = block_bitmasks.getBlockBitmask(block_idx);
    bool is_bitmask_all_0 = true;
    
    for (int i = 0; i < block_bitmasks.getNumQubits(); ++i) {
        if (block_bitmasks.globalToBlock(i) == block_idx) {
            if ((gate_bitmasks[i] & block_bitmask) != 0) {
                projected_bitmasks[block_bitmasks.globalToLocal(i)] =
                Project1QBitmask(gate_bitmasks[i] & block_bitmask, block_bitmasks, block_idx);
                is_bitmask_all_0 = false;
            }
        }
    }
    
    return !is_bitmask_all_0;
}
