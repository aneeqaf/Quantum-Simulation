//
//  state_interface.h
//  vector_state_sim
//
//  Created by Aneeqa Fatima on 1/16/18.
//

#ifndef state_interface_h
#define state_interface_h

#include <stdio.h>

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

using namespace std;

constexpr int sampling_factor = 1;

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
        const bitset<128> mask((1ull << (rows * cols)) - 1);
        _blocks.push_back(mask ^ (mask & block0));
        InitMappings();
    }
    
    QubitPartition(const Cuts cut_type,
                   int total_qubits,
                   int cut = 0);
    
    QubitPartition(int rows, int cols, const vector<bitset<128>>& blocks);
    
    string print(int verb = 3) const;
    
    int getNumQubits() const { return _rows * _cols; }
    int getNumBlocks() const { return static_cast<int>(_blocks.size()); }
    int getNumQubitsInBlock(int i) const { return (int)_blocks[i].count(); }
    int getNumX() const { return _numX; }
    bitset<128> getBlockBitmask(int i) const { return _blocks[i]; }
    
    int globalToBlock(int q) const { return _global_to_block[q]; }
    int globalToLocal(int q) const { return _global_to_local[q]; }
    int localToGlobal(int block,
                      int q) const {
        return _local_to_global[block][q];
    }
    
    // Scatters the global amp index into local amp indices.
    vector<idx_size> IndexScatter(const bitset<128>& i);
    idx_size IndexScatter(const bitset<128>& i,
                          const int block_idx);
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
    static Data data_per_cycles;
    static vector<string> log;
    static Counts count_of_category;
    static Times time_by_category;
    static int num_threads;
    static Config::SimMode sim_mode;
    static Config::SimType sim_type;
    static idx_size approx;
    vector<cmplx> amps_of_interest;
    static char partition_to_sim;
    
    virtual int ApplyBlockOfDiagGates(string& cz_bits,
                                      idx_size prefix_size,
                                       const bitset<128>* __restrict CZ_bitmasks,
                                       const bitset<128> T_bitmasks[2]) = 0;
    virtual void ApplyNonCGate(const int gate_qubit,
                               const Gate::Type gate_type,
                               const Gate& g = {}) = 0;
    virtual void ApplyHGateOnAllAmps() = 0;
    virtual void ApplyCGate(const int num_controls,
                            const vector<int>& gate_qubits,
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
                                           const int th) = 0;
    virtual bitset<128> FormXYGatesBitmask(idx_size& gate_i,
                                        const vector<Gate>& all_gates,
                                        const Gate::Type gate_type);
    virtual void FormCZTGatesBitmask(bitset<128>* __restrict CZ_bitmasks,
                                     bitset<128> T_bitmasks[2],
                                     idx_size& gate_i,
                                     const vector<Gate>& all_gates,
                                     const int total_circuit_qubits);
    
    virtual cmplx operator[](bitset<128> i) = 0;
    virtual cmplx GetGlobalAmpAtInterestingIdx(idx_size i) = 0;
    virtual double GetMinProb()  = 0;
    virtual double GetMaxProb()  = 0;
    virtual double GetAvgProb() const = 0;
    virtual double GetMemUsage() const = 0;
    virtual idx_size GetSize() const = 0;
    virtual idx_size GetFullStateVectorSize() const = 0;
    virtual idx_size GetGlobalFactorPower() const = 0;
    virtual double CalculateNormSquared() = 0;
    virtual double CalculateAverageInaccuracy(double norm) const = 0;
    virtual double CalculateMeanEntropy() const = 0;
    virtual double CalculateCrossEntropy(int range) const = 0;
    virtual void Normalize() = 0;
    virtual double CountZeroAmpPercentage() const = 0;
    virtual idx_size GetNumAddends() const;
    
    virtual void ResetAmpVector() = 0;
    virtual void Rescale() = 0;
    virtual void ApplyGlobalICounter() = 0;
    virtual void RescaleAndApplyGlobalICounter() = 0;
    
    virtual void PrintStateVector(const string& outfile,
                                  const int cycle_num) = 0;
    virtual void PrintStateVector() = 0;
    virtual void PrintProbabilities(const string& out_file,
                                    const int cycle_num)  = 0;
        
    GenericQuantumState(): amps_of_interest({}){}
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
