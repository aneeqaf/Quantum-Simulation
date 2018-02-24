//
//  state_tensor.hpp
//  vector_state_sim
//
//  Created by Aneeqa Fatima on 1/16/18.
//

#ifndef state_tensor_h
#define state_tensor_h

#include "config.h"
#include "state.h"

using namespace std;

class TensorProductStateVector : public GenericQuantumState {
public:
    enum Cuts : int {Vertical, Horizontal};

private:
    bitset<128> a_qubits_bitmask;
    bitset<128> b_qubits_bitmask;
    int num_q_a;
    int num_q_b;
    Cuts cut_type;
    Config::SimType sim_type;
    
    void HorizontalCut(int& num_qubits_a,
                       int& num_qubits_b,
                       const int total_qubits,
                       const int cut = 0);
    void VerticalCut(int& num_qubits_a,
                     int& num_qubits_b,
                     const int total_qubits,
                     const int cut = 0);

public:
    FullAmpStateVector* state_a;
    FullAmpStateVector* state_b;
    
    void FindCZGatesBetweenPartitions(vector<pair<int,bitset<128>>>& CZ_bitmasks,
                                      const bitset<128>* __restrict gate_bitmasks);
    void ApplyCZGateAcrossTensorFactors(const Gate::Type CZ_D_A,
                                        const Gate::Type CZ_D_B,
                                        const int qubit_a,
                                        const int qubit_b);
    bool ApplyBlockOfDiagGates(string& cz_bits,
                               const bitset<128>* __restrict CZ_bitmasks,
                               const bitset<128> __restrict T_bitmasks[2]);
    idx_size CountXCZGates(const bitset<128>* __restrict CZ_bitmasks);
    void ApplyXCZGateApprox(const bitset<128>* __restrict CZ_bitmasks,
                            const Gate::Type CZ_D_A,
                            const Gate::Type CZ_D_B);
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
    void ApplyXYRecursiveTransform(bitset<128> X_bitmask,
                                   bitset<128> Y_bitmask,
                                   const int th);
    
    cmplx operator[](idx_size i) const;
    cmplx GetAmpFromGlobalState(const idx_size a,
                                const idx_size b) const;
    double GetMinProb() const;
    double GetMaxProb() const;
    double GetAvgProb() const;
    double GetMemUsage() const;
    idx_size GetSize() const;
    idx_size GetFullStateVectorSize() const;
    int GetStateANumQ() const;
    int GetStateBNumQ() const;
    bitset<128> GetStateABitmask() const;
    bitset<128> GetStateBBitmask() const;
    idx_size GetGlobalFactorPower() const;
    double CalculateNormSquared();
    double CalculateAverageInaccuracy(double norm) const;
    double CalculateMeanEntropy() const;
    double CalculateCrossEntropy(int range) const;
    void Normalize();

    void Rescale();
    void ApplyGlobalICounter();
    void RescaleAndApplyGlobalICounter();
    
    void PrintStateVector(const string& outfile,
                          const int cycle_num);
    void PrintStateVector() ;
    void PrintProbabilities(const string& out_file,
                            const int cycle_num) ;
    
    TensorProductStateVector(const int qubits,
                             const Cuts type,
                             const int hcut = 0,
                             const int vcut = 0,
                             const Config::SimType sim = Config::LosslessH);
    TensorProductStateVector(const TensorProductStateVector& rhs) ;
    TensorProductStateVector& operator=(const TensorProductStateVector& rhs) = delete;
    ~TensorProductStateVector();
};

idx_size Project1QBitmask(const bitset<128> gate_bitmask,
                          const bitset<128> partition_bitmask,
                          const int num_qubits,
                          const bool zero_least_sig,
                          const bool leading_ones = false);

int ProjectQubit(const int qubit_to_project,
                 const bitset<128> partition_bitmask,
                 const int num_qubits);

bool ProjectCZBitmask(bitset<128>* __restrict projected_bitmasks,
                      const bitset<128> partition_bitmask,
                      const bitset<128>* __restrict gate_bitmasks,
                      const int total_circuit_qubits);

bitset<128> ScatterGlobalIndex(const idx_size i,
                            const idx_size partition_bitmask,
                            const idx_size total_qubits);


#endif /* state_tensor_h */
