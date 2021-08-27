//
//  state_interface.h
//  vector_state_sim
//
//  Created by Aneeqa Fatima on 1/16/18.
//

#ifndef state_interface_h
#define state_interface_h

#include <stdio.h>

#include <array>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include "sys/time.h"
#include <unordered_map>

#include "kernels1.h"
#include "profile.h"
#include "config.h"
#include "Cramer.h"

using namespace std;

constexpr int SAMPLING_FACTOR = 1;

typedef struct DataPerCycle {
    vector<ul> cycles;
    vector<ul> XY_gates;
    vector<ul> CZ_gates;
    vector<ul> T_gates;
    vector<ul> xCZ_H;
    vector<ul> xCZ_V;
    vector<ul> addends;
    vector<ul> memory;
    
    DataPerCycle():cycles({}), XY_gates({}), CZ_gates({}), T_gates({}), xCZ_H({}), xCZ_V({}),
    addends({}), memory({}){}
} Data;


// This class allows one to map global indices in a qubit grid
// to partition indices and back. It can renumber qubits in blocks
// so that boundary qubits appear upfront.
class QubitPartition {
    
public:
    enum Cuts : int {Vertical, Horizontal};

    // two blocks, as balanced as possible (may use a Z cut)
    QubitPartition(int rows,
                   int cols)
    : QubitPartition(rows, cols,
    bitset<128>((1ull << (rows * cols / 2)) - 1) << ((rows * cols) - (rows * cols / 2))) {
    }
    
    // two blocks, of which one is specified, the other is its complement
    QubitPartition(int rows,
                   int cols,
                   bitset<128> block0)
    : _rows(rows), _cols(cols), _blocks(1, block0) {
        if (rows * cols > 64) {
            cerr << " More than 64 qubits not supported here yet" << endl;
            exit(1);
        }
        bitset<128> mask = 0;//((1ull << (rows * cols)) - 1);
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
    
    QubitPartition(int rows, int cols, const vector<bitset<128>>& blocks);
    
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
                      idx_size q) const {
        return _local_to_global[block][q];
    }
    
    // Scatters the global amp index into local amp indices.
    vector<idx_size> IndexScatter(const bitset<128>& i);
    idx_size IndexScatter(const bitset<128>& i,
                          const idx_size block_idx);
    // move up boundary qubits up front in each block
    void RenumberLocalQubits();
    
private:
    int _rows = 0;
    int _cols = 0;
    vector<bitset<128>> _blocks;
    vector<int> _global_to_block;
    vector<int> _global_to_local;
    vector<vector<int>> _local_to_global;  // block, local -> global
    int _numX = 0;
    bitset<128> _boundary_qubits;  // global indices
    
    void InitMappings();
};

inline ostream& operator<<(ostream& o, const QubitPartition& qp) {
    return o << qp.print();
}


class GenericQuantumState {    
public:
    static vector<array<complex<float>*, 2>> compressed_vector_ptrs;
    
    static Data data_per_cycles;
    static vector<string> log;
    static Counts count_of_category;
    static Times time_by_category;
    static int num_threads;
    static Config::SimType sim_type;
    static char partition_to_sim;
    static bool book_keep;
    bool compressed;
    bool block_compression;

    virtual void ApplyNonCGate(const idx_size gate_qubit,
                               const Gate::Type gate_type) = 0;
    virtual void ApplyHGateOnAllAmps(bool cycle_0 = false) = 0;
    virtual void ApplyCGate(const idx_size num_controls,
                            const vector<idx_size>& gate_qubits,
                            const Gate& g,
                            const Gate::Type gate_type) = 0;
    virtual void ApplyMergedXYGate(const Gate& gate1,
                                   const Gate& gate2) = 0;
    virtual void ApplyClusterOfXYHGates(idx_size& gate_i,
                                        idx_size& odd_Xi,
                                        idx_size& odd_Yi,
                                        const vector<Gate>& all_gates) = 0;
    virtual void ApplyXYRecursiveTransform(bitset<128> X_bitmask,
                                           bitset<128> Y_bitmask,
                                           int th) = 0;
    virtual int ApplyLoXYHAndCZTInSamePass(int& remaining_cz_bits,
                                           idx_size& cz_path,
                                           const idx_size cz_path_len,
                                           const idx_size suffix_size,
                                           const bitset<128>& X_bitmask,
                                           const bitset<128>& Y_bitmask,
                                           const bitset<128>& H_bitmask,
                                           const bitset<128>* __restrict CZ_bitmasks,
                                           const bitset<128> T_bitmasks[2],
                                           int th) = 0;
    
    virtual cmplx operator[](bitset<128> i) = 0;
    virtual cmplx GetGlobalAmpAtInterestingIdx(idx_size i) = 0;
    virtual double GetMinProb()  = 0;
    virtual double GetMaxProb()  = 0;
    virtual double GetAvgProb() const = 0;
    virtual double GetMemUsage() const = 0;
    virtual idx_size GetSize() const = 0;
    virtual idx_size GetFullStateVectorSize() const = 0;
    virtual int GetNumQInBlock(idx_size block) const = 0;
    virtual idx_size GetGlobalFactorPower() const = 0;
    virtual double CalculateNormSquared() = 0;
    virtual double CalculateAverageInaccuracy(double norm) const = 0;
    virtual double CalculateMeanEntropy() const = 0;
    virtual double CalculateCrossEntropy(int range) const = 0;
    virtual double CountZeroAmpPercentage() const = 0;
    virtual idx_size GetNumAddends() const;
    virtual idx_size CountZerosInBlock(int block) const = 0;
    virtual bool AreAllAmpsZero() const = 0;
    
    virtual void Normalize() = 0;
    virtual void ResetAmpVector() = 0;
    virtual void Rescale() = 0;
    virtual void ApplyGlobalICounter() = 0;
    virtual void RescaleAndApplyGlobalICounter() = 0;
    virtual void CopyState(const GenericQuantumState& rhs) = 0;
    virtual void CopyMemberVars(const GenericQuantumState& rhs) = 0;
    virtual void CompressStateVector(idx_size num_codewords,
                                     double p_rejection) = 0;
    virtual void DecompressStateVector() = 0;
    virtual void DecompressAndCopyAnotherState(const GenericQuantumState& rhs) = 0;
    
    virtual void PrintStateVector(const string& outfile,
                                  const int cycle_num) = 0;
    virtual void PrintStateVector() = 0;
    virtual void PrintProbabilities(const string& out_file,
                                    const int cycle_num)  = 0;
    virtual void WriteAmpToDisk(const string& filename) = 0;
    virtual void ReadFromDisk(const string& filename) = 0;
        
    GenericQuantumState(): compressed(false){}
    GenericQuantumState(int n_threads);
    virtual ~GenericQuantumState(){}
};

int FindDivisor(int num);
idx_size Project1QBitmask(bitset<128> gate_bitmask,
                          const QubitPartition& block_bitmasks,
                          const int block_idx,
                          const bool is_zero_least_sig = false);
bool ProjectCZBitmask(bitset<128> projected_bitmasks[],
                      const QubitPartition& block_bitmasks,
                      const int block_idx,
                      const bitset<128>* gate_bitmasks);


#endif /* state_interface_h */
