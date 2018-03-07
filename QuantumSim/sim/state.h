//
//  state.h
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 12/13/17.
//

#ifndef state_h
#define state_h

#include <immintrin.h>

#include "state_interface.h"

using namespace std;

class FullAmpStateVector : public GenericQuantumState {
private:
    double max_prob;
    double min_prob;
    cmplx* amp;
    idx_size amp_size;
    idx_size global_factor_power;
    idx_size global_i_counter;
    int num_qubits;
    
    bitset<128> FormBitmask(const vector<int>& qubits);
    
public:
    
    int ApplyBlockOfDiagGates(string& cz_bits,
                               const bitset<128>* __restrict CZ_bitmasks,
                               const bitset<128> T_bitmasks[2]);
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
                                const vector<Gate>& all_gates);
    void ApplyXYRecursiveTransform(bitset<128> X_bitmask,
                                   bitset<128> Y_bitmask,
                                   const int th);
    void ApplyCZDecompositions(const int gate_qubit,
                               const Gate::Type gate_type);
    
    cmplx operator[](bitset<128> i) const;
    const cmplx* const GetAmpVector() const;
    double GetMinProb() const;
    double GetMaxProb() const;
    double GetAvgProb() const;
    double GetMemUsage() const;
    idx_size GetSize() const;
    idx_size GetFullStateVectorSize() const;
    idx_size GetGlobalFactorPower() const;
    idx_size GetGlobalICounter() const;
    int GetNumQubits() const;
    double CalculateNormSquared();
    double CalculateAverageInaccuracy(double norm) const;
    double CalculateMeanEntropy() const;
    double CalculateCrossEntropy(int range) const;
    void Normalize();
    void IncrementGlobalFactorPower();
    void IncrementGlobalICounter();
    idx_size CountZeroAmp() const;
    
    void Rescale();
    void ApplyGlobalICounter();
    void RescaleAndApplyGlobalICounter();
    
    void PrintStateVector(const string& outfile,
                          const int cycle_num);
    void PrintStateVector();
    void PrintProbabilities(const string& out_file,
                            const int cycle_num) ;
    
    FullAmpStateVector(const int qubits);
    FullAmpStateVector(cmplx* a, const idx_size size);
    FullAmpStateVector(const FullAmpStateVector& rhs);
    FullAmpStateVector& operator=(const FullAmpStateVector& rhs) = delete;
    ~FullAmpStateVector();
};

#endif /* state_h */
