//
//  qubit_partition.h
//  vector_state_sim
//
//  Created by Aneeqa Fatima on 9/22/21.
//  Copyright © 2021 Aneeqa Fatima. All rights reserved.
//

#ifndef qubit_partition_h
#define qubit_partition_h

#include <bitset>
#include <complex>
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <sstream>
#include <vector>
#include <algorithm>

using namespace std;
using cmplx = complex<float>;
using idx_size = unsigned long long;

// This class allows one to map global indices in a qubit grid
// to partition indices and back. It can renumber qubits in blocks
// so that boundary qubits appear upfront.
class QubitPartition
{

public:
    enum Cuts : int
    {
        Vertical,
        Horizontal
    };

    // two blocks, as balanced as possible (may use a Z cut)
    QubitPartition(int rows,
                   int cols)
        : QubitPartition(rows, cols,
                         bitset<128>((1ull << (rows * cols / 2)) - 1) << ((rows * cols) - (rows * cols / 2)))
    {
    }

    // two blocks, of which one is specified, the other is its complement
    QubitPartition(int rows,
                   int cols,
                   bitset<128> block0)
        : _rows(rows), _cols(cols), _blocks(1, block0)
    {
        if (rows * cols > 64)
        {
            cerr << " More than 64 qubits not supported here yet" << endl;
            exit(1);
        }
        bitset<128> mask = 0; //((1ull << (rows * cols)) - 1);
        for (int i = 0; i < (rows * cols); ++i)
            mask[i] = 1;
        _blocks.push_back(mask ^ (mask & block0));
        InitMappings();
    }

    QubitPartition(const Cuts cut_type,
                   int total_qubits,
                   bool row_major = true,
                   int cut = 0,
                   bool first_part_small = false);

    QubitPartition(int rows, int cols, const vector<bitset<128>> &blocks);

    string print(int verb = 3) const;

    int GetRows() const { return _rows; }
    int GetColumns() const { return _cols; }
    int getNumQubits() const { return _rows * _cols; }
    int getNumBlocks() const { return static_cast<int>(_blocks.size()); }
    int getNumQubitsInBlock(idx_size i) const { return (int)_blocks[i].count(); }
    int getNumX() const { return _numX; }
    bitset<128> getBlockBitmask(int i) const { return _blocks[i]; }
    bool isBoundaryQubit(idx_size i) const { return _boundary_qubits[i]; }

    int globalToBlock(idx_size q) const { return _global_to_block[q]; }
    int globalToLocal(idx_size q) const { return _global_to_local[q]; }
    int localToGlobal(idx_size block,
                      idx_size q) const
    {
        return _local_to_global[block][q];
    }

    // Scatters the global amp index into local amp indices.
    vector<idx_size> IndexScatter(const bitset<128> &i);
    idx_size IndexScatter(const bitset<128> &i,
                          const idx_size block_idx) const;
    // move up boundary qubits up front in each block
    void RenumberLocalQubits();

private:
    int _rows = 0;
    int _cols = 0;
    vector<bitset<128>> _blocks;
    vector<int> _global_to_block;
    vector<int> _global_to_local;
    vector<vector<int>> _local_to_global; // block, local -> global
    int _numX = 0;
    bitset<128> _boundary_qubits; // global indices

    void InitMappings();
};

inline ostream &operator<<(ostream &o, const QubitPartition &qp)
{
    return o << qp.print();
}

int FindDivisor(int num);
idx_size Project1QBitmask(bitset<128> gate_bitmask,
                          const QubitPartition &block_bitmasks,
                          const int block_idx,
                          const bool is_zero_least_sig = false);
bool ProjectCZBitmask(bitset<128> projected_bitmasks[],
                      const QubitPartition &block_bitmasks,
                      const int block_idx,
                      const bitset<128> *gate_bitmasks);

#endif /* qubit_partition_h */
