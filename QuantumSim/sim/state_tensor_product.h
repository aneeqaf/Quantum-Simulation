//
//  state_tensor_product.hpp
//  vector_state_sim
//
//  Created by Aneeqa Fatima on 1/16/18.
//

#ifndef state_tensor_product_h
#define state_tensor_product_h

#include "config.h"
#include "state.h"

using namespace std;

class TensorProductStateVector : public GenericQuantumState
{
private:
    QubitPartition qp;
    QubitPartition::Cuts cut_type;
    static idx_size *global_to_local_a;
    static idx_size *global_to_local_b;
    static idx_size num_requested_amps;
    FullAmpStateVector *state_a;
    FullAmpStateVector *state_b;

public:
    friend class SumOfTensorsProductsStateVector;

    int FindCZGatesBetweenPartitions(bitset<128> *__restrict xCZ_bitmasks,
                                     const bitset<128> *gate_bitmasks);
    void ApplyCZGateAcrossTensorFactors(const Gate::Type CZ_D_A,
                                        const Gate::Type CZ_D_B,
                                        const int qubit_a,
                                        const int qubit_b);
    void ApplyCZGateAcrossTensorFactors(bitset<128> *__restrict xCZ_bitmasks_path0_D1D2,
                                        bitset<128> *__restrict xCZ_bitmasks_path0_D2D1,
                                        bitset<128> *__restrict xCZ_bitmasks_path1_D3D4,
                                        bitset<128> *__restrict xCZ_bitmasks_path1_D4D3);
    idx_size CountXCZGates(const bitset<128> *__restrict CZ_bitmasks);
    void ApplyXCZGateApprox(const bitset<128> *__restrict CZ_bitmasks,
                            const Gate::Type CZ_D_A,
                            const Gate::Type CZ_D_B);
    void ApplyNonCGate(const idx_size gate_qubit,
                       const Gate::Type gate_type);
    void ApplyHGateOnAllAmps(bool cycle_0 = false);
    void ApplyCGate(const idx_size num_controls,
                    const vector<idx_size> &gate_qubits,
                    const Gate &g,
                    const Gate::Type gate_type);
    void ApplyMergedXYGate(const Gate &gate1,
                           const Gate &gate2);
    void ApplyGatheredXYGatesInBlocks(bitset<128> X_bitmask,
                                      bitset<128> Y_bitmask,
                                      int th);
    int ApplyGoogleCirqGatesInBlocks(int &remaining_cz_bits,
                                     idx_size &cz_path,
                                     const idx_size cz_path_len,
                                     const idx_size suffix_size,
                                     const bitset<128> &X_bitmask,
                                     const bitset<128> &Y_bitmask,
                                     const bitset<128> &H_bitmask,
                                     const bitset<128> *__restrict CZ_bitmasks,
                                     const bitset<128> T_bitmasks[2],
                                     int th);
    void PopulateGlobalToLocalMap(vector<bitset<128>> &idxs);
    void UnpopulateGlobalToLocalMap();

    cmplx operator[](bitset<128> i) const;
    cmplx GetAmpFromGlobalState(const idx_size a,
                                const idx_size b) const;
    cmplx *GetGlobalAmpAtInterestingIdx(const idx_size *idxs = nullptr, const idx_size num_idxs = 0);
    cmplx GetGlobalAmpAtInterestingIdx(idx_size i);
    double GetMinProb();
    double GetMaxProb();
    double GetAvgProb() const;
    double GetMemUsage(bool peak = false) const;
    idx_size GetSize() const;
    idx_size GetFullStateVectorSize() const;
    int GetNumQInBlock(idx_size block) const;
    bitset<128> GetStateABitmask() const;
    bitset<128> GetStateBBitmask() const;
    idx_size GetGlobalFactorPower() const;
    const QubitPartition &GetQp() const;
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
    void CopyState(const GenericQuantumState &rhs, bool decompress = false);
    void CompressStateVector();
    void DecompressStateVector();

    void PrintStateVector(const string &outfile,
                          const int cycle_num);
    void PrintStateVector(const string extension = "");
    void PrintProbabilities(const string &out_file,
                            const int cycle_num);
    void WriteAmpToDisk(const string &filename);
    void ReadFromDisk(const string &filename);

    TensorProductStateVector() : qp(0, 0), state_a(nullptr), state_b(nullptr) {}
    TensorProductStateVector(const int qubits,
                             const QubitPartition::Cuts type,
                             const Config *config,
                             const int hcut = 0,
                             const int vcut = 0,
                             const Config::SimType sim = Config::LosslessH,
                             const bool row_major = true,
                             const bool first_part_small = false,
                             const int verb = 3);
    TensorProductStateVector(const TensorProductStateVector &rhs);
    TensorProductStateVector &operator=(const TensorProductStateVector &rhs) = delete;
    ~TensorProductStateVector();
};

#endif /* state_tensor_product */
