//
//  state.hpp
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 12/13/17.
//  Copyright © 2017 Aneeqa Fatima. All rights reserved.
//

#ifndef state_h
#define state_h

#include <stdio.h>
#include <stdio.h>
#include <ctime>
#include <fstream>
#include "kernals.h"

using namespace std;

class state {
private:
    valarray<cmplx> amp;
    idx_size global_factor_power;
    
public:
    
    void ApplyBlockOfDiagGates(const vector<gate>& block_gates,
                              const int qubits,
                              idx_size& gate_i);
    void ApplyGateFWHT();
    void ApplyNonCGate(const vector<int>& gate_qubits,
                      const int qubits,
                      const gate& g,
                      const gate::Gates gate_type);
    void ApplyHGateOnAllAmps(const int qubits);
    void ApplyCGate(const int num_controls,
                   const vector<int>& gate_qubits,
                   const int qubits,
                   const gate& g,
                   const gate::Gates gate_type);
    void ApplyTwoMergedXYGate(vector<gate>& gates,
                              idx_size gate1,
                              idx_size gate2,
                              const int qubits);
    
    double GetMinProb() const;
    double GetMaxProb() const;
    double GetAvgProb() const;
    double GetMemUsage() const;
    idx_size GetGlobalFactorPower() const;
    float CalculateNormOfAmp();
    const valarray<cmplx>& GetAmp() const;
    
    void IncrementGlobalFactorPower(int num);
    void ResetGlobalFactorPower();
    cmplx ComputeRescalingFactor();
    
    void PrintStateVector(const string& outfile) const;
    void PrintStateVector() const;
    void PrintProbabilities(const string& out_file) const;
    
    cmplx Measure0(const short qubit);
    cmplx Measure1(const short qubit);
    void Measure(const short qubit);
    
    state(int qubits);
    state(valarray<cmplx>& a);
    state(const state& rhs);
    state& operator=(const state& rhs);
};

#endif /* state_h */
