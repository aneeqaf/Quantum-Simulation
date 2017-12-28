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
    
public:
    
    void ApplyBlockOfDiagGates(const vector<Gate>& block_gates,
                              const int total_circuit_qubits,
                              idx_size& gate_i);
    void ApplyNonCGate(const vector<int>& gate_qubits,
                      const int total_circuit_qubits,
                      const Gate& g,
                      const Gate::Type gate_type);
    void ApplyHGateOnAllAmps(const int qubits);
    void ApplyCGate(const int num_controls,
                   const vector<int>& gate_qubits,
                   const int total_circuit_qubits,
                   const Gate& g,
                   const Gate::Type gate_type);
    void ApplyMergedXYGate(const vector<Gate>& all_gates,
                           idx_size& gate_i,
                           const int total_circuit_qubits);
    void ApplyClusterOfXYHGates(const vector<Gate>& all_gates,
                                const int total_circuit_qubits,
                                idx_size& gate_i,
                                Gate::Type gate_type);
    
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
