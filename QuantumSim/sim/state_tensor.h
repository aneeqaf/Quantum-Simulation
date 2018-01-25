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

class TensorProductStateVector : public GenericQuantumState {
public:
    enum Cuts : int {Vertical, Horizontal};
    
    static idx_size Project1QBitmask(const idx_size gate_bitmask,
                                     const idx_size partition_bitmask,
                                     const int num_qubits,
                                     const bool zero_least_sig,
                                     const bool leading_ones = false);
    
    static int ProjectQubit(const int qubit_to_project,
                            const idx_size partition_bitmask,
                            const int num_qubits);
    
    static bool ProjectCZBitmask(idx_size* __restrict projected_bitmasks,
                                 const idx_size partition_bitmask,
                                 const idx_size* __restrict gate_bitmasks,
                                 const int total_circuit_qubits);
    static idx_size ScatterGlobalIndex(const idx_size i,
                               const idx_size partition_bitmask,
                               const idx_size total_qubits);
private:
    FullAmpStateVector* state_A;
    FullAmpStateVector* state_B;
    idx_size A_qubits_bitmask;
    idx_size B_qubits_bitmask;
    int num_q_A;
    int num_q_B;
    Cuts cut_type; 
    
    void HorizontalCut(int& num_qubits_A,
                      int& num_qubits_B,
                      const int total_qubits);
    void VerticalCut(int& num_qubits_A,
                    int& num_qubits_B,
                    const int total_qubits);

public:
    void FindCZGatesBetweenPartitions(vector<pair<int,idx_size>>& CZ_bitmasks,
                                      const idx_size* __restrict gate_bitmasks);
    void ApplyCZGateAcrossTensorFactors(const Gate::Type CZ_D_A,
                                        const Gate::Type CZ_D_B,
                                        const int qubit_A,
                                        const int qubit_B);
    void ApplyBlockOfDiagGates(const idx_size* __restrict CZ_bitmasks,
                               const idx_size __restrict T_bitmasks[2]);
    void ApplyNonCGate(const int gate_qubit,
                       const Gate::Type gate_type,
                       const Gate& g = {});
    void ApplyHGateOnAllAmps();
    void ApplyCGate(const int num_controls,
                    const vector<int>& gate_qubits,
                    const Gate& g,
                    const Gate::Type gate_type);
    void ApplyMergedXYGate(const Gate& gate1,
                           const Gate& gate2);
    void ApplyClusterOfXYHGates(idx_size& gate_i,
                                idx_size& odd_Xi,
                                idx_size& odd_Yi,
                                const vector<Gate>& all_gates) {};
    void ApplyXYRecursiveTransform(idx_size X_bitmask,
                                   idx_size Y_bitmask,
                                   const int th);
    
    cmplx operator[](idx_size i) const;
    double GetMinProb() const;
    double GetMaxProb() const;
    double GetAvgProb() const;
    double GetMemUsage() const;
    idx_size GetSize() const;
    idx_size GetFullStateSize() const;
    int GetStateANumQ() const;
    int GetStateBNumQ() const;
    idx_size GetGlobalFactorPower() const;
    double CalculateNormOfAmp();
    
    void Rescale();
    void ApplyGlobalICounter();
    void RescaleAndApplyGlobalICounter();
    
    void PrintStateVector(const string& outfile) const;
    void PrintStateVector() ;
    void PrintProbabilities(const string& out_file) const;
    
    TensorProductStateVector(int qubits,
                             Cuts type);
    TensorProductStateVector(const TensorProductStateVector& rhs) ;
    TensorProductStateVector& operator=(const TensorProductStateVector& rhs) = delete;
    ~TensorProductStateVector();
};


#endif /* state_tensor_h */
