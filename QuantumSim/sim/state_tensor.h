//
//  state_tensor.hpp
//  vector_state_sim
//
//  Created by Aneeqa Fatima on 1/16/18.
//

#ifndef state_tensor_h
#define state_tensor_h

#include "state.h"

using namespace std;

class TensorPartitions : public GenericQuantumState {
private:
    State* state_A;
    State* state_B;
    idx_size A_qubits_bitmask;
    idx_size B_qubits_bitmask;
    
    void HorizontalCut(int& num_qubits_A,
                      int& num_qubits_B,
                      const int total_qubits);
    void VerticalCut(int& num_qubits_A,
                    int& num_qubits_B,
                    const int total_qubits);
    void PreprocessXYBlock(idx_size& gate_i,
                           vector<Gate>& stateA_XY_gates,
                           vector<Gate>& stateB_XY_gates,
                           const vector<Gate>& all_gates);
    void PreprocessCZTBlock(idx_size& gate_i,
                            vector<Gate>& stateA_CZT_gates,
                            vector<Gate>& stateB_CZT_gates,
                            vector<Gate>& across_AB_CZ_gates,
                            const vector<Gate>& all_gates);
    
public:
    enum Cuts : int {vertical, horizontal};
    
    void ApplyBlockOfDiagGates(const idx_size* __restrict CZ_bitmasks,
                               const idx_size __restrict T_bitmasks[2],
                               const int total_circuit_qubits);
    void ApplyNonCGate(const int gate_qubit,
                       const int total_circuit_qubits,
                       const Gate::Type gate_type,
                       const Gate& g = {});
    void ApplyHGateOnAllAmps();
    void ApplyCGate(const int num_controls,
                    const vector<int>& gate_qubits,
                    const int total_circuit_qubits,
                    const Gate& g,
                    const Gate::Type gate_type);
    void ApplyMergedXYGate(const Gate& gate1,
                           const Gate& gate2,
                           const int total_circuit_qubits);
    void ApplyClusterOfXYHGates(idx_size& gate_i,
                                idx_size& odd_Xi,
                                idx_size& odd_Yi,
                                const vector<Gate>& all_gates,
                                const int total_circuit_qubits);
    void ApplyXYRecursiveTransform(idx_size X_bitmask,
                                   idx_size Y_bitmask,
                                   const int total_circuit_qubits,
                                   const int th);
    
    cmplx operator[](idx_size i) const;
    double GetMinProb() const;
    double GetMaxProb() const;
    double GetAvgProb() const;
    double GetMemUsage() const;
    idx_size GetAmpSize() const;
    idx_size GetGlobalFactorPower() const;
    idx_size GetGlobalIcounter() const;
    double CalculateNormOfAmp();
    const cmplx* const GetAmp() const;
    
    void IncrementGlobalFactorPower(int num);
    void ResetGlobalFactorPower();
    void Rescale();
    void ApplyGlobalICounter();
    
    void PrintStateVector(const string& outfile) const;
    void PrintStateVector() const;
    void PrintProbabilities(const string& out_file) const;
    
    cmplx Measure0(const short qubit);
    cmplx Measure1(const short qubit);
    void Measure(const short qubit);
    
    TensorPartitions(int qubits, Cuts type);
    TensorPartitions(const TensorPartitions& rhs);
    TensorPartitions& operator=(const TensorPartitions& rhs);
    ~TensorPartitions();
};


#endif /* state_tensor_h */
