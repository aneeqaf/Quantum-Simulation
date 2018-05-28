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
bool GenericQuantumState::book_keep = true;
Config::SimType GenericQuantumState::sim_type = Config::SimType::FullState;
char GenericQuantumState::partition_to_sim = 'x';

#ifdef Parallel
int GenericQuantumState::num_threads = omp_get_num_procs();
#else
int GenericQuantumState::num_threads = 8;
#endif

ostream& operator<<(ostream& o, const vector<int>& v) {
    for (auto k : v) o << k << " ";
    return o << endl;
}

GenericQuantumState::
GenericQuantumState(int n_threads){
    num_threads = n_threads;
}

bitset<128> GenericQuantumState::
FormXYHGatesBitmask(idx_size& gate_i,
                  const vector<Gate>& all_gates,
                  const Gate::Type gate_type)
{
//    vector<int> cluster_qubits = FormBlockOfXYHGates(gate_i, gate_type, all_gates);
    
    bitset<128> bitmask = 0;
    for(;gate_i < all_gates.size(); ++gate_i) {
        const auto& gt = all_gates[gate_i];
        
        if(gt.ids.back() == gate_type)
            bitmask[gt.qubits.back()] = 1;
        else break;
    }
//    for (idx_size i = 0; i < cluster_qubits.size(); ++i)
//        bitmask[cluster_qubits[i]] = 1;
//
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
        b &= mask;
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
               bool row_major,
               int cut,
               bool first_part_small)
{
    int y_axis = FindDivisor(total_qubits),
    x_axis = total_qubits/y_axis,
    num_q_1 = total_qubits - 1;
    
    if (!row_major)
        swap(y_axis, x_axis);
    
    if (cut_type == QubitPartition::Cuts::Horizontal) {
        int right_cut = cut ? cut : (first_part_small ? y_axis - ceil(y_axis/2.0) : ceil(y_axis/2.0));
        if ((cut > y_axis - cut) && first_part_small)
            right_cut = y_axis - cut;
        const int block_bits = right_cut * x_axis;
        
//        if (!cut)
//            (*this) = QubitPartition(y_axis, x_axis);
//        else {
            bitset<128> block0_bitmask = 0;
            for (int i = 0; i < block_bits; ++i)
                block0_bitmask [num_q_1 - i] = 1;
            (*this) = QubitPartition(y_axis, x_axis, block0_bitmask);
//        }
    }
    else {
        int v_cut = cut ? cut : (first_part_small ? x_axis - ceil(x_axis/2.0) : ceil(x_axis/2.0));
        if ((v_cut > x_axis - v_cut) && first_part_small)
            v_cut = x_axis - v_cut;
//        if(!v_cut)
//           (*this) = QubitPartition(x_axis, y_axis);
//        else {
            bitset<128> block0_bitmask = 0;
            for (int i = 0; i < y_axis; ++i) {
                for (int j = 0; j < v_cut; ++j)
                    block0_bitmask [num_q_1 - ((i * x_axis) + j)] = 1;
            }
           (*this) = QubitPartition(y_axis, x_axis, block0_bitmask);
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

idx_size QubitPartition::
IndexScatter(const bitset<128>& idx,
             const int block_idx)
{
    idx_size local_idx = 0;
    int total_qubits = getNumQubits();

    for (int j = 0; j < total_qubits; ++j) {
        if (idx[j] == 0)
            continue;
        if (globalToBlock(j) == block_idx)
            local_idx |= (1ull << globalToLocal(j));
    }
    return local_idx;
}

vector<idx_size> QubitPartition::
IndexScatter(const bitset<128>& idx)
{
    //    int total_qubits = getNumQubits();
    //
    //    if (!_global_to_local_amp_idxs.count(idx)){
    //        _global_to_local_amp_idxs[idx].resize(_blocks.size(), 0);
    //        for (int j = 0; j < total_qubits; ++j) {
    //            if (idx[j] == 0)
    //                continue;
    //            _global_to_local_amp_idxs[idx][globalToBlock(j)] |= (1ull << globalToLocal(j));
    //        }
    //    }
    //    return _global_to_local_amp_idxs[idx];
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
            return !_boundary_qubits[i]; });
        for(int i = 0; i < (int)block.size(); ++i) _global_to_local[block[i]] = i;
    }
}

string
QubitPartition::print(int verb) const {
    
    if (verb == 0)
        return "";
    
    int qubits = getNumQubits();
    stringstream s;
    s << "Qubit grid " << _rows << "x" << _cols << " with "
    << getNumBlocks() << " blocks " << endl;
    
    if (!_global_to_local.empty()) {
        for (int i = 0; i < _rows * _cols; i+= _cols) {
            s << "    ";
            for (int j = 0; j < _cols; ++j)
                s << setw(2) << i + j
                <<  char(_global_to_block[qubits - (i + j) - 1] + 94) << "  ";
            s << "\n";
        }
    }
    if (verb > 3) {
        if (!_blocks.empty()) {
            s << " Blocks :";
            if (_rows * _cols < 64) {
                for (auto b : _blocks) s << " " << b.to_string().substr(128 - getNumQubits());
            } else {
                s << endl;
                for (auto b : _blocks) s << "  " << b.to_string().substr(128 - getNumQubits()) << " " << endl;
            }
            s << endl;
        }
    }
//
//    if (!_global_to_block.empty()) {
//        s << " Global to block : \n";
//        for (int i = 0; i < _rows * _cols; i+= _cols) {
//            s << "  ";
//            for (int j = 0; j < _cols; ++j)
//                s << _global_to_block[i + j] << " ";
//            s << "\n";
//        }
//        s << "\n";
//    }
    
    if (!_global_to_local.empty()) {
       if (verb > 3) {
            s << " Global to local : \n";
            for (int i = 0; i < _rows * _cols; i+= _cols) {
                s << "    ";
                for (int j = 0; j < _cols; ++j)
                    s << setw(2) << _global_to_local[i + j]
                    <<  char(_global_to_block[i + j] + 94) << "  ";
                s << "\n";
            }
        }
    }
    
    if (verb > 3) {
        if (!_local_to_global.empty()) {
            s << " Local to global : " << endl;
            for (const auto& block : _local_to_global) {
                s << "   " << block;
            }
        }
    }
    
    
    if (_numX) {
        s << " Cross-gates : " << _numX << endl;
        if (verb > 3) {
            s << " Boundary qubits :";
            for(int i = 0; i < getNumQubits(); ++i) {
                if (_boundary_qubits[i]) s << " " << i;
            }
            s << endl;
        }
    }
    return s.str();
}

int FindDivisor(int num)
{
    int div = 0;
    for (int i = 1; i <= floor(sqrt(num)); ++i){
        if (num % i == 0)
            div = i;
    }
    return div;
}

// Returns the local gate bitmask that is a projection of the global gate bitmask
idx_size
Project1QBitmask(bitset<128> gate_bitmask,
                 const QubitPartition& qp,
                 const int block_idx,
                 const bool is_zero_least_sig)
{
    idx_size projected_bitmask = 0;
    const int num_q1 = qp.getNumQubits() - 1;
    const int num_qb1 = qp.getNumQubitsInBlock(block_idx) - 1;

    while (gate_bitmask != 0) {
        const idx_size first_half = gate_bitmask.to_ulong();
        const idx_size second_half = (gate_bitmask >> 64).to_ulong();
        const int q = first_half ? __builtin_ctzl(first_half) : second_half ? 63 + __builtin_ctzl(second_half) : 0;

        const int global_q_idx = is_zero_least_sig ? num_q1 - q : q;
        
        if (qp.globalToBlock(global_q_idx) == block_idx) {
            int local_q_idx = qp.globalToLocal(global_q_idx);
            if (is_zero_least_sig) {
                local_q_idx = num_qb1 - local_q_idx;
            }
            projected_bitmask |= 1ull << local_q_idx;
        }

        gate_bitmask[q] = 0;
    }
    
    return projected_bitmask;
}

bool
ProjectCZBitmask(bitset<128> projected_bitmasks[],
                 const QubitPartition& qp,
                 const int block_idx,
                 const bitset<128>* gate_bitmasks)
{
    const bitset<128> block_bitmask = qp.getBlockBitmask(block_idx);
    bool is_bitmask_all_0 = true;
    
    for (int i = 0; i < qp.getNumQubits(); ++i) {
        if ((qp.globalToBlock(i) == block_idx) && ((gate_bitmasks[i] & block_bitmask) != 0)) {
            projected_bitmasks[qp.globalToLocal(i)] = Project1QBitmask(gate_bitmasks[i] & block_bitmask,
                                                                       qp, block_idx);
            is_bitmask_all_0 = false;
        }
    }
    return !is_bitmask_all_0;
}

