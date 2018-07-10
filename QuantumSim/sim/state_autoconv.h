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
    int ApplyBlockOfDiagGates(string& cz_bits,
                              idx_size prefix_size,
                              const bitset<128>* __restrict CZ_bitmasks,
                              const bitset<128>  T_bitmasks[2],
                              const bitset<128>& H_bitmask,
                              const bool last_cycle = false);
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
    void CopyState(const AdaptiveStateVector& rhs);
    
    cmplx operator[](bitset<128> i);
    cmplx GetGlobalAmpAtInterestingIdx(idx_size i);
    double GetMinProb();
    double GetMaxProb();
    double GetAvgProb() const;
    double GetMemUsage() const;
    idx_size GetSize() const;
    idx_size GetFullStateVectorSize() const;
    int GetNumQInBlock(idx_size block) const;
    idx_size GetGlobalFactorPower() const;
    idx_size GetNumAddends() const;
    idx_size CountZerosInBlock(int block);
    double CountZeroAmpPercentage() const;
    double CalculateNormSquared();
    double CalculateAverageInaccuracy(double norm) const;
    double CalculateMeanEntropy() const;
    double CalculateCrossEntropy(int range) const;
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
    
    AdaptiveStateVector(const int qubits,
                        const Config::SimType type,
                        const int hcut = 0,
                        const int vcut = 0,
                        const bool row_major = true,
                        const bool first_part_small = false,
                        const int verb = 3);
    AdaptiveStateVector(const AdaptiveStateVector& rhs);
    AdaptiveStateVector& operator=(const AdaptiveStateVector& rhs);
    ~AdaptiveStateVector();
};



#endif /* state_autoconv_h */
