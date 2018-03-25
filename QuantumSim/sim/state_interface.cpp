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

QubitPartition::QubitPartition(int rows, int cols,
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
    for(int i = 0; i < _blocks.size(); ++i) {
        for(int j = 0; j < i; ++j) {
            if ( (_blocks[i] & _blocks[j]).count() >0 ) {
                cerr << " Blocks " << i << " " << j << " are not disjoint "
                << endl;
                exit(1);
            }
        }
    }
    InitMappings();
}

void QubitPartition::InitMappings() {
    _global_to_block.resize(getNumQubits(), -1);
    _global_to_local.resize(getNumQubits(), -1);
    _local_to_global.resize(getNumBlocks());
    
    for (int b = 0; b < _blocks.size(); ++b) {
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

// Move up boundary qubits upfront in each block,
// then repopulate _global_to_local accordingly
void QubitPartition::RenumberLocalQubits() {
    for(auto& block : _local_to_global) {
        partition(block.begin(), block.end(), [&](int i) {
            return _boundary_qubits[i]; });
        for(int i = 0; i < block.size(); ++i) _global_to_local[block[i]] = i;
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
Project1QBitmask(const bitset<128> gate_bitmask,
                 const bitset<128> partition_bitmask,
                 const int num_qubits,
                 const bool zero_least_sig,
                 const bool leading_ones)
{
    const int modified_q = num_qubits - 1;
    idx_size projected_bitmask = 0, c = 0;
    if (!leading_ones) {
        if (!zero_least_sig) {
            for (int i = modified_q; i >= 0; --i) {
                bitset<128> temp = 0;
                temp[modified_q - i] = 1;
                if ((temp & partition_bitmask) != 0) {
                    if ((temp & gate_bitmask) != 0)
                        projected_bitmask |= 1ull << c;
                    ++c;
                }
            }
        }
        else {
            for (int i = 0; i < num_qubits; ++i) {
                bitset<128> temp = 0, temp1 = 0;
                temp[modified_q - i] = 1;
                temp1[i] = 1;
                if ((temp & partition_bitmask) != 0) {
                    if ((temp1 & gate_bitmask) != 0)
                        projected_bitmask |= 1ull << c;
                    ++c;
                }
            }
        }
    }
    else {
        assert(false);
        //        return gate_bitmask & partition_bitmask;
    }//TODO:FIX
    
    return projected_bitmask;
}

int
ProjectQubit(const int qubit_to_project,
             const bitset<128> partition_bitmask,
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
                 const bitset<128> partition_bitmask,
                 const bitset<128>* __restrict gate_bitmasks,
                 const int total_circuit_qubits)
{
    const int modified_q = total_circuit_qubits - 1;
    idx_size c = 0;
    bool bitmask_0 = true;
    for (int i = modified_q; i >= 0; --i) {
        bitset<128> temp = 0;
        temp[modified_q - i] = 1;
        if ((temp & partition_bitmask) != 0) {
            if ((gate_bitmasks[modified_q - i] & partition_bitmask) != 0) {
                projected_bitmasks[c] = Project1QBitmask(gate_bitmasks[modified_q - i] & partition_bitmask,
                                                         partition_bitmask, total_circuit_qubits,
                                                         false);
                bitmask_0 = false;
            }
            ++c;
        }
    }
    return !bitmask_0;
}

bitset<128>
ScatterGlobalIndex(const bitset<128> i,
                   const bitset<128> partition_bitmask,
                   const idx_size total_qubits)
{
    bitset<128> local_idx = 0;
    int count = 0;
    
    for (idx_size j = 0; j < total_qubits; ++j) {
        bitset<128> j_bit = 0;
        j_bit[j] = 1;
        if ((j_bit & partition_bitmask) == 0) continue;
        
        if ((j_bit & i) != 0) local_idx[count++] = 1;
        else ++count;
    }
    return local_idx;
}

void
HorizontalCut(bitset<128>& a_qubits_bitmask,
              bitset<128>& b_qubits_bitmask,
              int& num_qubits_a,
              int& num_qubits_b,
              const int total_qubits,
              const int cut)
{
//    num_qubits_a = !cut ? ceil(total_qubits/2) : cut;
//    const int modified_q = total_qubits - 1;
//
//    for (int i = 0; i < num_qubits_a; ++i)
//        a_qubits_bitmask [modified_q - i] = 1;
//
//    for (int i = num_qubits_a; i < total_qubits; ++i) {
//        b_qubits_bitmask [modified_q - i] = 1;
//        ++num_qubits_b;
//    }
    const int x_axis_sz = FindDivisor(total_qubits), y_axis_sz = total_qubits/x_axis_sz,
    modified_q = total_qubits - 1;
    num_qubits_a = cut ? cut : (ceil(x_axis_sz/2.0)  * y_axis_sz);
    
    for (int i = 0; i < num_qubits_a; ++i)
        a_qubits_bitmask [modified_q - i] = 1;
        
    for (int i = num_qubits_a ; i < total_qubits; ++i) {
        b_qubits_bitmask [modified_q - i] = 1;
        ++num_qubits_b;
    }
}

void
VerticalCut(bitset<128>& a_qubits_bitmask,
            bitset<128>& b_qubits_bitmask,
            int& num_qubits_a,
            int& num_qubits_b,
            const int total_qubits,
            const int cut)
{
    const int x_axis_sz = FindDivisor(total_qubits), y_axis_sz = total_qubits/x_axis_sz,
    modified_q = total_qubits - 1, v_cut = !cut ? ceil(y_axis_sz/2.0) : cut;
    
    for (int i = 0; i < x_axis_sz; ++i) {
        for (int j = 0; j < v_cut; ++j) {
            a_qubits_bitmask [modified_q - ((i * y_axis_sz) + j)] = 1;
            ++num_qubits_a;
        }
        for (int j = v_cut; j < y_axis_sz; ++j) {
            b_qubits_bitmask [modified_q - ((i * y_axis_sz) + j)] = 1;
            ++num_qubits_b;
        }
    }
}


