//
//  state_interface.h
//  vector_state_sim
//
//  Created by Aneeqa Fatima
//

#ifndef state_tree_tensor_h
#define state_tree_tensor_h

#include "state_sum_tensor.h"
#include <tuple>

using namespace std;

const int BINARY_TREE_DIM = 3;

/*
    Flattened multidimensional matrix of size D^m, where m is constant and D is bounded by D_max
*/
template <typename dtype, uint32_t m>
class MultidimensionalArray
{
    dtype *data;
    uint32_t dims[m];
    uint32_t strides[m];
    uint32_t d_max;
    bool use_gpu;

    tuple<float *, dtype *, dtype *> CpuSVD(const vector<uint32_t> &n,
                                            const uint32_t agg_dim,
                                            const bool T) const;
    tuple<float *, dtype *, dtype *> GpuSVD(const vector<uint32_t> &n,
                                            const uint32_t agg_dim,
                                            const bool T) const;
    dtype *GpuSquareMatrix(const vector<uint32_t> &n,
                           const uint32_t agg_dim,
                           const bool T) const;
    dtype *CpuSquareMatrix(const vector<uint32_t> &n,
                           const uint32_t agg_dim,
                           const bool T) const;
    dtype *PrepareMatrix(const vector<uint32_t> &n,
                         const bool T,
                         int &num_rows,
                         int &num_cols,
                         const uint32_t agg_dim = -1) const;
    void TransposeColToRow(const dtype *src, dtype *dst, uint32_t rows, uint32_t cols) const;
    void CalculateStrides();

public:
    dtype operator()(const vector<uint32_t> &idxs) const;
    void operator()(const vector<uint32_t> &idxs, dtype val);
    vector<uint32_t> Shape() const;
    size_t Size() const;
    tuple<float *, dtype *, dtype *> SVD(const vector<uint32_t> &n,
                                         const uint32_t agg_dim = -1,
                                         const bool T = false) const;
    dtype *SquareMatrix(const vector<uint32_t> &n,
                        const uint32_t agg_dim,
                        const bool T = false) const;
    void Matmul(dtype *M, const vector<uint32_t> &dims, const vector<uint32_t> &t_dims, bool left = true);
    void UpdateToGpu();
    // TODO: matmul, normalization because of svd error
    /*
    Backward Stability: Numerical SVD algorithms (like those in LAPACK, used by numpy) are backward stable.
    This means the computed SVD matrices correspond to a matrix where is an error matrix.
    The norm of this error is bounded by a small multiple of the machine epsilon times the spectral norm of where is a polynomial function of the matrix dimensions.*/

    MultidimensionalArray() : data(nullptr), dims({}), strides({}), d_max(0), use_gpu(false) {}
    MultidimensionalArray(uint32_t d_max,
                          const vector<uint32_t> &dims,
                          bool use_gpu = false);
    MultidimensionalArray(uint32_t d_max, bool use_gpu = false);
    MultidimensionalArray(const MultidimensionalArray<dtype, m> &rhs);
    MultidimensionalArray<dtype, m> &operator=(const MultidimensionalArray<dtype, m> &rhs);
    ~MultidimensionalArray();
};

template <uint32_t m, typename value_type>
class TtnNode
{
    MultidimensionalArray<value_type, m> *parent;
    MultidimensionalArray<value_type, m> *children[m];
    MultidimensionalArray<value_type, m> *state;

public:
    void ApplySVD();

    MultidimensionalArray<value_type, m> *GetParent() const;
    MultidimensionalArray<value_type, m> *GetChildren() const;
    MultidimensionalArray<value_type, m> *GetState() const;

    TtnNode() : parent(nullptr), children({}), state(nullptr) {}
    TtnNode(const MultidimensionalArray<value_type, m> *parent,
            const vector<MultidimensionalArray<value_type, m> *> &children,
            MultidimensionalArray<value_type, m> *state);
    TtnNode(const TtnNode &rhs);
    TtnNode &operator=(const TtnNode &rhs);
    ~TtnNode();
};

class TreeTensorNetwork : public GenericQuantumState
{
private:
    TtnNode<BINARY_TREE_DIM, cmplx> *root;
    vector<TtnNode<BINARY_TREE_DIM, cmplx> *> qubit_modes;

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

    cmplx operator[](bitset<128> i) const;
    cmplx *GetGlobalAmpAtInterestingIdx(const idx_size *idxs = nullptr, const idx_size num_idxs = 0);
    cmplx GetGlobalAmpAtInterestingIdx(idx_size i);
    double GetMinProb();
    double GetMaxProb();
    double GetAvgProb() const;
    double GetMemUsage(bool peak = false) const;
    idx_size GetSize() const;
    idx_size GetFullStateVectorSize() const;
    int GetNumQInBlock(idx_size block) const;
    idx_size GetGlobalFactorPower() const;
    idx_size GetNumAddends() const;
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

    TreeTensorNetwork() : root(nullptr), qubit_modes({}) {}
    TreeTensorNetwork(uint32_t max_full_state_level,
                      uint32_t d_max,
                      const vector<uint32_t> &dims,
                      const Config *config);
    TreeTensorNetwork(const TreeTensorNetwork &rhs);
    TreeTensorNetwork &operator=(const TreeTensorNetwork &rhs);
    ~TreeTensorNetwork();
};

#endif /* state_tree_tensor_h */