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

class FullAmpStateVector : public GenericQuantumState
{
private:
    double max_prob;
    double min_prob;
    cmplx *amp;
    Cramer *cramer;
    idx_size amp_size;
    idx_size global_factor_power;
    idx_size global_i_counter;
    int num_qubits;
    ZeroOptMask zero_opt_mask; // 0 is most significant
    bool all_zeros;

    bitset<128> FormBitmask(const vector<idx_size> &qubits);
    void TransferOddBitsFromHiQubitsBM(int &th,
                                       idx_size &hi_q_X_bitmask,
                                       idx_size &hi_q_Y_bitmask,
                                       idx_size &lo_q_X_bitmask,
                                       idx_size &lo_q_Y_bitmask,
                                       int &num_hi_X_bits,
                                       int &num_hi_Y_bits,
                                       const idx_size X_bitmask,
                                       const idx_size Y_bitmask);
    void TransferOddBitsFromLowQubitsBM(int &th,
                                        idx_size &hi_q_X_bitmask,
                                        idx_size &hi_q_Y_bitmask,
                                        idx_size &lo_q_X_bitmask,
                                        idx_size &lo_q_Y_bitmask,
                                        int &num_lo_X_bits,
                                        int &num_lo_Y_bits,
                                        const idx_size X_bitmask,
                                        const idx_size Y_bitmask);
    void ApplyOddGates(idx_size &X_bitmask,
                       idx_size &Y_bitmask,
                       int &num_X_bits,
                       int &num_Y_bits);
    pair<int, Gate::Type> GetMostSigOddBitForXY1_2Gates(idx_size &X_bitmask,
                                                        idx_size &Y_bitmask,
                                                        int &num_X_bits,
                                                        int &num_Y_bits);
    pair<int, int> GetLeasttSigOddBit(idx_size &X_bitmask,
                                      idx_size &Y_bitmask,
                                      int &num_X_bits,
                                      int &num_Y_bits);

public:
    void ApplyNonCGate(const idx_size gate_qubit,
                       const Gate::Type gate_type);
    void ApplyHGateOnAllAmps(bool cycle_0 = false);
    void ApplyCGate(const idx_size num_controls,
                    const vector<idx_size> &gate_qubits,
                    const Gate &g,
                    const Gate::Type gate_type);
    void ApplyMergedXYGate(const Gate &gate1,
                           const Gate &gate2);
    void ApplyClusterOfXYHGates(idx_size &gate_i,
                                idx_size &odd_Xi,
                                idx_size &odd_Yi,
                                const vector<Gate> &all_gates);
    void ApplyXYRecursiveTransform(bitset<128> X_bitmask,
                                   bitset<128> Y_bitmask,
                                   int th);
    int ApplyLoXYHAndCZTInSamePass(int &remaining_cz_bits,
                                   idx_size &cz_path,
                                   const idx_size cz_path_len,
                                   const idx_size suffix_size,
                                   const bitset<128> &X_bitmask,
                                   const bitset<128> &Y_bitmask,
                                   const bitset<128> &H_bitmask,
                                   const bitset<128> *__restrict CZ_bitmasks,
                                   const bitset<128> T_bitmasks[2],
                                   int th);
    void ApplyCZDecompositions(const int gate_qubit,
                               const Gate::Type gate_type);
    void ApplyCZDecompositionDist(const idx_size *__restrict xCZ_bitmasks);
    void ApplyQFT();

    cmplx operator[](bitset<128> i);
    cmplx GetGlobalAmpAtInterestingIdx(idx_size i);
    const cmplx *const GetAmpVector() const;
    double GetMinProb();
    double GetMaxProb();
    double GetAvgProb() const;
    double GetMemUsage() const;
    ZeroOptMask GetZeroOptMask() const;
    idx_size GetSize() const;
    idx_size GetFullStateVectorSize() const;
    int GetNumQInBlock(idx_size block) const;
    idx_size GetGlobalFactorPower() const;
    idx_size GetGlobalICounter() const;
    int GetNumQubits() const;
    double CalculateNormSquared();
    double CalculateAverageInaccuracy(double norm) const;
    double CalculateMeanEntropy() const;
    double CalculateCrossEntropy(int range) const;
    double CountZeroAmpPercentage() const;
    idx_size CountZerosInBlock(int block) const;
    bool AreAllAmpsZero() const;

    void SetOddZeroPatternAtQubit(int qubit);
    void SetEvenZeroPatternAtQubit(int qubit);
    void UnsetZeroPatternAtQubit(int qubit);
    void Normalize();
    void IncrementGlobalFactorPower();
    void IncrementGlobalICounter();
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

    FullAmpStateVector() : max_prob(numeric_limits<double>::min()), min_prob(numeric_limits<double>::max()),
                           amp(nullptr), cramer(nullptr), amp_size(0), global_factor_power(0),
                           global_i_counter(0), num_qubits(0), zero_opt_mask(num_qubits), all_zeros(false) {}

    FullAmpStateVector(const int qubits,
                       const Config *config);
    FullAmpStateVector(cmplx *a, const idx_size size);
    FullAmpStateVector(const FullAmpStateVector &rhs);
    FullAmpStateVector &operator=(const FullAmpStateVector &rhs) = delete;
    ~FullAmpStateVector();
};

#endif /* state_h */
