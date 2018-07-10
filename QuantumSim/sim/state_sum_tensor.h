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
    vector<TensorProductStateVector*> tensor_addends;
    idx_size num_addends;
    
    int HandlexCZApplication(string& cz_bits,
                             idx_size prefix_size,
                             const bitset<128>* __restrict CZ_bitmasks);
    
public:
    int ApplyBlockOfDiagGates(string& cz_bits,
                              idx_size prefix_size,
                              const bitset<128>* __restrict CZ_bitmasks,
                              const bitset<128> T_bitmasks[2],
                              const bitset<128>& H_bitmask,
                              const bool last_cycle);
    inline int ApplyXCZGatesExact(const bitset<128>* __restrict CZ_bitmasks);
    inline int ApplyXCZGatesForDist(string& cz_bits,
                                    idx_size prefix_size,
                                    const bitset<128>* __restrict CZ_bitmasks);
    int FormGatesBitmaskXCZ(bool& terminate,
                            string& cz_bits,
                            idx_size prefix_size,
                            bitset<128>* __restrict xCZ_bitmasks_path0_D1D2,
                            bitset<128>* __restrict xCZ_bitmasks_path0_D2D1,
                            bitset<128>* __restrict xCZ_bitmasks_path1_D3D4,
                            bitset<128>* __restrict xCZ_bitmasks_path1_D4D3,
                            const bitset<128>* __restrict CZ_bitmasks);
    void ApplyNonCGate(const int gate_qubit,
                       const Gate::Type gate_type,
                       const Gate& g = {});
    void ApplyHGateOnAllAmps(bool not_cycle_0 = false);
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
                                   int th);
    int ApplyLoXYHAndCZTInSamePass(string& cz_bits,
                                   idx_size prefix_size,
                                   const bitset<128>& X_bitmask,
                                   const bitset<128>& Y_bitmask,
                                   const bitset<128>& H_bitmask,
                                   const bitset<128>* __restrict CZ_bitmasks,
                                   const bitset<128> T_bitmasks[2],
                                   int th,
                                   bool last_cycle = false);
    void PopulateGlobalToLocalMap(vector<bitset<128>>& idxs);
    void UnpopulateGlobalToLocalMap();
    FullAmpStateVector* ConvertSumOfTensorsToState();
    FullAmpStateVector* ConvertSumOfTensorsToStateAVX();
    void CopyState(const SumOfTensorsProductsStateVector& rhs);
    void CopyMemberVars(const SumOfTensorsProductsStateVector& rhs);

    cmplx operator[](bitset<128> i);
    cmplx GetGlobalAmpAtInterestingIdx(idx_size i);
    double GetMinProb();
    double GetMaxProb();
    double GetAvgProb() const;
    double GetMemUsage() const;
    Config::SimType GetSimType() const;
    idx_size GetNumAddends() const;
    idx_size GetSize() const;
    idx_size GetFullStateVectorSize() const;
    int GetNumQInBlock(idx_size block) const;
    idx_size GetGlobalFactorPower() const;
    double CalculateNormSquared();
    double CalculateAverageInaccuracy(double norm) const;
    double CalculateMeanEntropy() const;
    double CalculateCrossEntropy(int range) const;
    double CalculateMeanEntropyHCuts() const;
    double CalculateCrossEntropyHCuts(int range) const;
    double CalculateMeanEntropy2Cuts() const;
    double CalculateCrossEntropy2Cuts(int range) const;
    idx_size CountZerosInBlock(int block);
    double CountZeroAmpPercentage() const;
    bool AreAllAmpsZeros() const;
    
    void Normalize();
    void ResetAmpVector();
    void Rescale();
    void ApplyGlobalICounter();
    void RescaleAndApplyGlobalICounter();
    
    void PrintStateVector(const string& outfile,
                          const int cycle_num);
    void PrintStateVector() ;
    void PrintProbabilities(const string& out_file,
                            const int cycle_num) ;
    void WriteAmpToDisk(const string& filename);
    void ReadFromDisk(const string& filename);
    void SetMemberVariables(const GenericQuantumState& amp);
    
    SumOfTensorsProductsStateVector(): num_addends(0) {
        tensor_addends.push_back(new TensorProductStateVector()); }
    SumOfTensorsProductsStateVector(const int qubits,
                                    const Config::SimType type,
                                    const int hcut = 0,
                                    const int vcut = 0,
                                    const bool row_major = true,
                                    const bool first_part_small = false,
                                    const int verb = 3);
    SumOfTensorsProductsStateVector(const SumOfTensorsProductsStateVector& rhs);
    SumOfTensorsProductsStateVector& operator=(const SumOfTensorsProductsStateVector& rhs);
    ~SumOfTensorsProductsStateVector();
};



#endif /* state_sum_tensor_h */
