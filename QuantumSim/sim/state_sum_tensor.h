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
    
    int HandlexCZApplication(int& remaining_cz_bits,
                             idx_size& cz_path,
                             const idx_size cz_path_len,
                             const idx_size suffix_size,
                             const bitset<128>* __restrict CZ_bitmasks);
    
public:
    
    inline int ApplyXCZGatesExact(const bitset<128>* __restrict CZ_bitmasks);
    inline int ApplyXCZGatesForDist(int& remaining_cz_bits,
                                    idx_size& cz_path,
                                    const idx_size cz_path_len,
                                    const idx_size suffix_size,
                                    const bitset<128>* __restrict CZ_bitmasks);
    int FormGatesBitmaskXCZ(bool& terminate,
                            int& remaining_cz_bits,
                            idx_size& cz_path,
                            bitset<128>* __restrict xCZ_bitmasks_path0_D1D2,
                            bitset<128>* __restrict xCZ_bitmasks_path0_D2D1,
                            bitset<128>* __restrict xCZ_bitmasks_path1_D3D4,
                            bitset<128>* __restrict xCZ_bitmasks_path1_D4D3,
                            const idx_size cz_path_len,
                            const idx_size suffix_size,
                            const bitset<128>* __restrict CZ_bitmasks);
    void ApplyNonCGate(const idx_size gate_qubit,
                       const Gate::Type gate_type);
    void ApplyHGateOnAllAmps(bool cycle_0 = false);
    void ApplyCGate(const idx_size num_controls,
                    const vector<idx_size>& gate_qubits,
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
    int ApplyLoXYHAndCZTInSamePass(int& remaining_cz_bits,
                                   idx_size& cz_path,
                                   const idx_size cz_path_len,
                                   const idx_size suffix_size,
                                   const bitset<128>& X_bitmask,
                                   const bitset<128>& Y_bitmask,
                                   const bitset<128>& H_bitmask,
                                   const bitset<128>* __restrict CZ_bitmasks,
                                   const bitset<128> T_bitmasks[2],
                                   int th);
    void PopulateGlobalToLocalMap(vector<bitset<128>>& idxs);
    void UnpopulateGlobalToLocalMap();
    FullAmpStateVector* ConvertSumOfTensorsToState();
    FullAmpStateVector* ConvertSumOfTensorsToStateAVX();
    
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
    double CountZeroAmpPercentage() const;
    idx_size CountZerosInBlock(int block) const;
    bool AreAllAmpsZero() const;
    
    void Normalize();
    void ResetAmpVector();
    void Rescale();
    void ApplyGlobalICounter();
    void RescaleAndApplyGlobalICounter();
    void CopyState(const GenericQuantumState& rhs);
    void CopyMemberVars(const GenericQuantumState& rhs);
    void CompressStateVector(idx_size num_codewords,
                             double p_rejection);
    void DecompressStateVector();
    void DecompressAndCopyAnotherState(const GenericQuantumState& rhs);
    
    void PrintStateVector(const string& outfile,
                          const int cycle_num);
    void PrintStateVector() ;
    void PrintProbabilities(const string& out_file,
                            const int cycle_num) ;
    void WriteAmpToDisk(const string& filename);
    void ReadFromDisk(const string& filename);
    
    SumOfTensorsProductsStateVector(): num_addends(1) {
        tensor_addends.push_back(new TensorProductStateVector());}
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
