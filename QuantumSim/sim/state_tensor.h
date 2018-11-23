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
private:
    QubitPartition qp;
    QubitPartition::Cuts cut_type;
    static idx_size* global_to_local_a;
    static idx_size* global_to_local_b;
    static idx_size num_requested_amps;
    FullAmpStateVector* state_a;
    FullAmpStateVector* state_b;
    
    void HandleCZApprox(const bitset<128>* __restrict CZ_bitmasks);
    
public:
    friend class SumOfTensorsProductsStateVector;
    
    int FindCZGatesBetweenPartitions(bitset<128>* __restrict xCZ_bitmasks,
                                      const bitset<128>* gate_bitmasks);
    void ApplyCZGateAcrossTensorFactors(const Gate::Type CZ_D_A,
                                        const Gate::Type CZ_D_B,
                                        const int qubit_a,
                                        const int qubit_b);
    void ApplyCZGateAcrossTensorFactors(bitset<128>* __restrict xCZ_bitmasks_path0_D1D2,
                                        bitset<128>* __restrict xCZ_bitmasks_path0_D2D1,
                                        bitset<128>* __restrict xCZ_bitmasks_path1_D3D4,
                                        bitset<128>* __restrict xCZ_bitmasks_path1_D4D3);
    int ApplyBlockOfDiagGates(string& cz_bits,
                              idx_size prefix_size,
                              const bitset<128>* __restrict CZ_bitmasks,
                              const bitset<128>  T_bitmasks[2],
                              const bitset<128>& H_bitmask,
                              const bool last_cycle = false);
    idx_size CountXCZGates(const bitset<128>* __restrict CZ_bitmasks);
    void ApplyXCZGateApprox(const bitset<128>* __restrict CZ_bitmasks,
                            const Gate::Type CZ_D_A,
                            const Gate::Type CZ_D_B);
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
    
    cmplx operator[](bitset<128> i);
    cmplx GetAmpFromGlobalState(const idx_size a,
                                const idx_size b) const;
    cmplx GetGlobalAmpAtInterestingIdx(idx_size i);
    double GetMinProb();
    double GetMaxProb();
    double GetAvgProb() const;
    double GetMemUsage() const;
    idx_size GetSize() const;
    idx_size GetFullStateVectorSize() const;
    int GetNumQInBlock(idx_size block) const;
    bitset<128> GetStateABitmask() const;
    bitset<128> GetStateBBitmask() const;
    idx_size GetGlobalFactorPower() const;
    const QubitPartition& GetQp() const;
    double CountZeroAmpPercentage() const;
    double CalculateNormSquared();
    double CalculateAverageInaccuracy(double norm) const;
    double CalculateMeanEntropy() const;
    double CalculateCrossEntropy(int range) const;
    idx_size CountZerosInBlock(int block) const;
    bool AreAllAmpsZero() const;
    
    void Normalize();
    void ResetAmpVector();
    void Rescale();
    void ApplyGlobalICounter();
    void RescaleAndApplyGlobalICounter();
    void CopyState(const GenericQuantumState& rhs);
    void CopyMemberVars(const GenericQuantumState& rhs);
    
    void PrintStateVector(const string& outfile,
                          const int cycle_num);
    void PrintStateVector() ;
    void PrintProbabilities(const string& out_file,
                            const int cycle_num) ;
    void WriteAmpToDisk(const string& filename);
    void ReadFromDisk(const string& filename);
    
    TensorProductStateVector(): qp(0, 0) {
        state_a = new FullAmpStateVector();
        state_b = new FullAmpStateVector();
    }
    TensorProductStateVector(const int qubits,
                             const QubitPartition::Cuts type,
                             const int hcut = 0,
                             const int vcut = 0,
                             const Config::SimType sim = Config::LosslessH,
                             const bool row_major = true,
                             const bool first_part_small = false,
                             const int verb = 3);
    TensorProductStateVector(const TensorProductStateVector& rhs) ;
    TensorProductStateVector& operator=(const TensorProductStateVector& rhs) = delete;
    ~TensorProductStateVector();
};



#endif /* state_tensor_h */
