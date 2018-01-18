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

#include "kernels1.h"

class GenericQuantumState {
public:
    
    virtual void ApplyBlockOfDiagGates(const idx_size* __restrict CZ_bitmasks,
                                       const idx_size __restrict T_bitmasks[2],
                                       const int total_circuit_qubits) = 0;
    virtual void ApplyNonCGate(const int gate_qubit,
                               const int total_circuit_qubits,
                               const Gate::Type gate_type,
                               const Gate& g = {}) = 0;
    virtual void ApplyHGateOnAllAmps() = 0;
    virtual void ApplyCGate(const int num_controls,
                            const vector<int>& gate_qubits,
                            const int total_circuit_qubits,
                            const Gate& g,
                            const Gate::Type gate_type) = 0;
    virtual void ApplyMergedXYGate(const Gate& gate1,
                                   const Gate& gate2,
                                   const int total_circuit_qubits) = 0;
    virtual void ApplyClusterOfXYHGates(idx_size& gate_i,
                                        idx_size& odd_Xi,
                                        idx_size& odd_Yi,
                                        const vector<Gate>& all_gates,
                                        const int total_circuit_qubits) = 0;
    virtual void ApplyXYRecursiveTransform(idx_size X_bitmask,
                                           idx_size Y_bitmask,
                                           const int total_circuit_qubits,
                                           const int th) = 0;
    virtual idx_size FormXGatesBitmask(idx_size& gate_i,
                                       const vector<Gate>& all_gates);
    virtual idx_size FormYGatesBitmask(idx_size& gate_i,
                                       const vector<Gate>& all_gates);
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
    virtual idx_size GetAmpSize() const = 0;
    virtual idx_size GetGlobalFactorPower() const = 0;
    virtual idx_size GetGlobalIcounter() const = 0;
    virtual double CalculateNormOfAmp() = 0;
    virtual const cmplx* const GetAmp() const = 0;
    
    virtual void IncrementGlobalFactorPower(int num) = 0;
    virtual void ResetGlobalFactorPower() = 0;
    virtual void Rescale() = 0;
    virtual void ApplyGlobalICounter() = 0;
    
    virtual void PrintStateVector(const string& outfile) const = 0;
    virtual void PrintStateVector() const = 0;
    virtual void PrintProbabilities(const string& out_file) const = 0;
    
//    virtual cmplx Measure0(const short qubit);
//    virtual cmplx Measure1(const short qubit);
//    virtual void Measure(const short qubit);
    
    GenericQuantumState(){};
    virtual ~GenericQuantumState(){};
};


#endif /* state_interface_h */
