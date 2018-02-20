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
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "sys/time.h"

#include "kernels1.h"
#include "profile.h"

constexpr int sampling_factor = 10;

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

class GenericQuantumState {
public:
    static Data data_per_cycles;
    static vector<string> log;
    static Counts count_of_category;
    static Times time_by_category;
    
    virtual void ApplyBlockOfDiagGates(const idx_size* __restrict CZ_bitmasks,
                                       const idx_size __restrict T_bitmasks[2]) = 0;
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
    virtual void ApplyXYRecursiveTransform(idx_size X_bitmask,
                                           idx_size Y_bitmask,
                                           const int th) = 0;
    virtual idx_size FormXYGatesBitmask(idx_size& gate_i,
                                        const vector<Gate>& all_gates,
                                        const Gate::Type gate_type);
    virtual void FormCZTGatesBitmask(idx_size* __restrict CZ_bitmasks,
                                     idx_size __restrict T_bitmasks[2],
                                     idx_size& gate_i,
                                     const vector<Gate>& all_gates,
                                     const int total_circuit_qubits);
    
    virtual cmplx operator[](idx_size i) const = 0;
    virtual double GetMinProb() const = 0;
    virtual double GetMaxProb() const = 0;
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
    
    virtual void Rescale() = 0;
    virtual void ApplyGlobalICounter() = 0;
    virtual void RescaleAndApplyGlobalICounter() = 0;
    
    virtual void PrintStateVector(const string& outfile,
                                  const int cycle_num) = 0;
    virtual void PrintStateVector() = 0;
    virtual void PrintProbabilities(const string& out_file,
                                    const int cycle_num)  = 0;
        
    GenericQuantumState(){}
    virtual ~GenericQuantumState(){}
};

int FindDivisor(int num);


#endif /* state_interface_h */
