//
//  state_autoconv.hpp
//  vector_state_sim
//
//  Created by Aneeqa Fatima on 1/20/18.
//

#ifndef state_autoconv_h
#define state_autoconv_h

#include "state_sum_tensor.h"

using namespace std;

class AdaptiveStateVector : public GenericQuantumState {
private:
    FullAmpStateVector* full_state;
    SumOfTensorsProductsStateVector* sumOfTensors;
    int total_q;
     
public:
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
    idx_size GetFullStateVectorSize() const;
    idx_size GetGlobalFactorPower() const;
    double CalculateNormSquared();
    double CalculateAverageInaccuracy(double norm) const;
    double CalculateMeanEntropy() const;
    double CalculateCrossEntropy(int range) const;
    
    void Rescale();
    void ApplyGlobalICounter();
    void RescaleAndApplyGlobalICounter();
    
    void PrintStateVector(const string& outfile) const;
    void PrintStateVector() ;
    void PrintProbabilities(const string& out_file) const;
    
    AdaptiveStateVector(const int qubits,
                        const SumOfTensorsProductsStateVector::SimType type,
                        const int cut_size = 0);
    AdaptiveStateVector(const AdaptiveStateVector& rhs) = delete;
    AdaptiveStateVector& operator=(const AdaptiveStateVector& rhs) = delete;
    ~AdaptiveStateVector();
};



#endif /* state_autoconv_h */
