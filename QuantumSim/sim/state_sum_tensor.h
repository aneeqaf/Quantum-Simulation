//
//  state_sum_tensor.hpp
//  vector_state_sim
//
//  Created by Aneeqa Fatima on 1/18/18.
//

#ifndef state_sum_tensor_h
#define state_sum_tensor_h

#include "config.h"
#include "state_tensor.h"

using namespace std;

class SumOfTensorsProductsStateVector : public GenericQuantumState {
private:
    Config::SimType sim_type;
    vector<TensorProductStateVector*> tensor_addends;
    idx_size num_addends;
    
public:
    int ApplyBlockOfDiagGates(string& cz_bits,
                               const bitset<128>* __restrict CZ_bitmasks,
                               const bitset<128> T_bitmasks[2]);
    inline int ApplyXCZGatesExact(const bitset<128>* __restrict CZ_bitmasks);
    inline int ApplyXCZGatesForDist(string& cz_bits,
                                    const bitset<128>* __restrict CZ_bitmasks);
    int FormGatesBitmaskXCZ(bool& terminate,
                            string& cz_bits,
                            bitset<128>* __restrict xCZ_bitmasks_path0_D1D2,
                            bitset<128>* __restrict xCZ_bitmasks_path0_D2D1,
                            bitset<128>* __restrict xCZ_bitmasks_path1_D3D4,
                            bitset<128>* __restrict xCZ_bitmasks_path1_D4D3,
                            const bitset<128>* __restrict CZ_bitmasks);
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
    FullAmpStateVector* ConvertSumOfTensorsToState();
    FullAmpStateVector* ConvertSumOfTensorsToStateAVX();
    int ApplyXCZGatesExactTemp(string& cz_bits,
                           const bitset<128>* __restrict CZ_bitmasks);
    
    cmplx operator[](bitset<128> i) const;
    double GetMinProb() const;
    double GetMaxProb() const;
    double GetAvgProb() const;
    double GetMemUsage() const;
    Config::SimType GetSimType() const;
    idx_size GetNumAddends() const;
    idx_size GetSize() const;
    idx_size GetFullStateVectorSize() const;
    idx_size GetGlobalFactorPower() const;
    int GetStateANumQ() const;
    int GetStateBNumQ() const;
    double CalculateNormSquared();
    double CalculateAverageInaccuracy(double norm) const;
    double CalculateMeanEntropy() const;
    double CalculateCrossEntropy(int range) const;
    double CalculateMeanEntropyHCuts() const;
    double CalculateCrossEntropyHCuts(int range) const;
    double CalculateMeanEntropy2Cuts() const;
    double CalculateCrossEntropy2Cuts(int range) const;
    void Normalize();
    idx_size CountZeroAmp() const;
    void ResetAmpVector();

    void Rescale();
    void ApplyGlobalICounter();
    void RescaleAndApplyGlobalICounter();
    
    void PrintStateVector(const string& outfile,
                          const int cycle_num);
    void PrintStateVector() ;
    void PrintProbabilities(const string& out_file,
                            const int cycle_num) ;
    
    SumOfTensorsProductsStateVector(const int qubits,
                                    const Config::SimType type,
                                    const int hcut = 0,
                                    const int vcut = 0);
    SumOfTensorsProductsStateVector(const SumOfTensorsProductsStateVector& rhs);
    SumOfTensorsProductsStateVector& operator=(const SumOfTensorsProductsStateVector& rhs);
    ~SumOfTensorsProductsStateVector();
};



#endif /* state_sum_tensor_h */
