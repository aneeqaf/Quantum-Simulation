//
//  state.h
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 12/13/17.
//

#ifndef state_h
#define state_h

#include <cstdlib>
#include <ctime>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>

#include "kernels1.h"

using namespace std;

class State {
private:
    cmplx* amp;
    idx_size amp_size;
    idx_size global_factor_power;
    idx_size global_i_counter;
    
public:
    
    void ApplyBlockOfDiagGates(idx_size& gate_i,
                               const vector<Gate>& block_gates,
                               const int total_circuit_qubits);
    void ApplyNonCGate(const vector<int>& gate_qubits,
                       const int total_circuit_qubits,
                       const Gate::Type gate_type,
                       const Gate& g = {});
    void ApplyHGateOnAllAmps(const int qubits);
    void ApplyCGate(const int num_controls,
                   const vector<int>& gate_qubits,
                   const int total_circuit_qubits,
                   const Gate& g,
                   const Gate::Type gate_type);
    void ApplyMergedXYGate(idx_size& gate_i,
                           const vector<Gate>& all_gates,
                           const int total_circuit_qubits);
    void ApplyClusterOfXYHGates(idx_size& gate_i,
                                idx_size& odd_Xi,
                                idx_size& odd_Yi,
                                const vector<Gate>& all_gates,
                                const int total_circuit_qubits);
    
    double GetMinProb() const;
    double GetMaxProb() const;
    double GetAvgProb() const;
    double GetMemUsage() const;
    idx_size GetAmpSize() const;
    idx_size GetGlobalFactorPower() const;
    float CalculateNormOfAmp();
    const cmplx* const GetAmp() const;
    
    void IncrementGlobalFactorPower(int num);
    void ResetGlobalFactorPower();
    void Rescale();
    
    void PrintStateVector(const string& outfile) const;
    void PrintStateVector() const;
    void PrintProbabilities(const string& out_file) const;
    
    cmplx Measure0(const short qubit);
    cmplx Measure1(const short qubit);
    void Measure(const short qubit);
    
    State(int qubits);
    State(cmplx* a, idx_size size);
    State(const State& rhs);
    State& operator=(const State& rhs);
    ~State();
};

#endif /* state_h */
