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

typedef struct DataPerCycle
{
    vector<ul> cycles;
    vector<ul> XY_gates;
    vector<ul> CZ_gates;
    vector<ul> T_gates;
    vector<ul> xCZ_H;
    vector<ul> xCZ_V;
    vector<ul> addends;
    vector<ul> memory;

    DataPerCycle() : cycles({}), XY_gates({}), CZ_gates({}), T_gates({}), xCZ_H({}), xCZ_V({}),
                     addends({}), memory({}) {}
} Data;

class GenericQuantumState
{
public:
    static vector<array<complex<float> *, 2>> compressed_vector_ptrs;

    static Data data_per_cycles;
    static vector<string> log;
    static Counts count_of_category;
    static Times time_by_category;
    static int num_threads;
    static Config::SimType sim_type;
    static char partition_to_sim;
    static bool book_keep;
    bool compressed;

    virtual void ApplyNonCGate(const idx_size gate_qubit,
                               const Gate::Type gate_type) = 0;
    virtual void ApplyHGateOnAllAmps(bool cycle_0 = false) = 0;
    virtual void ApplyCGate(const idx_size num_controls,
                            const vector<idx_size> &gate_qubits,
                            const Gate &g,
                            const Gate::Type gate_type) = 0;
    virtual void ApplyMergedXYGate(const Gate &gate1,
                                   const Gate &gate2) = 0;
    virtual void ApplyClusterOfXYHGates(idx_size &gate_i,
                                        idx_size &odd_Xi,
                                        idx_size &odd_Yi,
                                        const vector<Gate> &all_gates) = 0;
    virtual void ApplyXYRecursiveTransform(bitset<128> X_bitmask,
                                           bitset<128> Y_bitmask,
                                           int th) = 0;
    virtual int ApplyLoXYHAndCZTInSamePass(int &remaining_cz_bits,
                                           idx_size &cz_path,
                                           const idx_size cz_path_len,
                                           const idx_size suffix_size,
                                           const bitset<128> &X_bitmask,
                                           const bitset<128> &Y_bitmask,
                                           const bitset<128> &H_bitmask,
                                           const bitset<128> *__restrict CZ_bitmasks,
                                           const bitset<128> T_bitmasks[2],
                                           int th) = 0;

    virtual cmplx operator[](bitset<128> i) = 0;
    virtual cmplx GetGlobalAmpAtInterestingIdx(idx_size i) = 0;
    virtual double GetMinProb() = 0;
    virtual double GetMaxProb() = 0;
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
    virtual void CopyState(const GenericQuantumState &rhs) = 0;
    virtual void CopyMemberVars(const GenericQuantumState &rhs) = 0;
    virtual void CompressStateVector() = 0;
    virtual void DecompressStateVector() = 0;
    virtual void DecompressAndCopyAnotherState(const GenericQuantumState &rhs) = 0;

    virtual void PrintStateVector(const string &outfile,
                                  const int cycle_num) = 0;
    virtual void PrintStateVector(const string extension = "") = 0;
    virtual void PrintProbabilities(const string &out_file,
                                    const int cycle_num) = 0;
    virtual void WriteAmpToDisk(const string &filename) = 0;
    virtual void ReadFromDisk(const string &filename) = 0;

    GenericQuantumState() : compressed(false) {}
    GenericQuantumState(int n_threads);
    virtual ~GenericQuantumState() {}
};

#endif /* state_interface_h */
