#include <iostream>
#include <vector>
#include <cassert>
#include <complex>
#include <random>

// Include the implementation directly to instantiate templates for testing
// This avoids linker errors since the template definitions are in the .cpp file
#include "../sim/state_tree_tensor.cpp"

using namespace std;

void TestMultidimensionalArray()
{
    cout << "Running MultidimensionalArray tests..." << endl;

    const uint32_t m = 3;
    uint32_t d_max = 4;

    // 1. Instantiation with no dims (zeros)
    MultidimensionalArray<std::complex<float>, m> tensor(d_max, {2, 3, 2});

    // 2. Check Initial State - dims should be 0
    vector<uint32_t> shape = tensor.Shape();
    assert(shape.size() == m);

    assert(shape[0] == 2);
    assert(shape[1] == 3);
    assert(shape[2] == 2);
    assert(tensor.Size() == 2 * 3 * 2);

    // 4. Set and Get Values - implicitly verifies strides are correct after Expand
    // Row-major strides for {2,3,2}: {6, 2, 1}
    // idx2 = {1,2,1} -> flat index = 1*6 + 2*2 + 1*1 = 11
    vector<uint32_t> idx1 = {0, 0, 0};
    vector<uint32_t> idx2 = {1, 2, 1};

    std::complex<float> val1(1.0f, 2.0f);
    std::complex<float> val2(3.0f, 4.0f);

    tensor(idx1, val1);
    tensor(idx2, val2);

    assert(tensor(idx1) == val1);
    assert(tensor(idx2) == val2);

    // 5. Constructor with explicit dims sets strides correctly
    vector<uint32_t> dims = {2, 3, 2};
    MultidimensionalArray<std::complex<float>, m> tensor2(d_max, dims);
    tensor2(idx1, val1);
    tensor2(idx2, val2);
    assert(tensor2(idx1) == val1);
    assert(tensor2(idx2) == val2);

    // 6. Copy Constructor
    MultidimensionalArray<std::complex<float>, m> tensor_copy(tensor);
    assert(tensor_copy(idx1) == val1);
    assert(tensor_copy(idx2) == val2);

    // Modify copy, original should stay same
    tensor_copy(idx1, val2);
    assert(tensor_copy(idx1) == val2);
    assert(tensor(idx1) == val1);

    cout << "MultidimensionalArray tests passed!" << endl;
}

// Shared verification for both CPU and GPU SVD results.
// U is num_rows x min_dim col-major (ldu=num_rows).
// Vt is min_dim x num_cols col-major with leading dimension ldvt.
// ldvt defaults to min_dim (the T=false case where ldvt==num_cols==min_dim).
// When transposed=true the SVD was computed on A^T, so tensor rows/cols are swapped.
void VerifySVDResult(float *s, complex<float> *u, complex<float> *vt,
                     MultidimensionalArray<complex<float>, 3> &tensor,
                     int num_rows, int num_cols, int min_dim,
                     int ldvt = -1, bool transposed = false)
{
    if (ldvt == -1)
        ldvt = min_dim;

    const float eps = 1e-4f;

    // 1. Singular values are non-negative and in descending order
    for (int i = 0; i < min_dim; i++)
        assert(s[i] >= -eps);
    for (int i = 0; i < min_dim - 1; i++)
        assert(s[i] >= s[i + 1] - eps);

    // 2. Reconstruction: A ≈ U * diag(s) * Vt
    // When transposed, the decomposed matrix is A^T so tensor indices are swapped.
    for (int i = 0; i < num_rows; i++)
    {
        for (int j = 0; j < num_cols; j++)
        {
            complex<float> recon(0, 0);
            for (int k = 0; k < min_dim; k++)
                recon += u[k * num_rows + i] * complex<float>(s[k], 0) * vt[j * ldvt + k];
            complex<float> orig = transposed
                                      ? tensor({(uint32_t)0, (uint32_t)j, (uint32_t)i})
                                      : tensor({(uint32_t)0, (uint32_t)i, (uint32_t)j});
            assert(abs(orig - recon) < eps);
        }
    }

    // 3. U is semi-unitary: U^H * U = I_{min_dim x min_dim}
    for (int i = 0; i < min_dim; i++)
    {
        for (int j = 0; j < min_dim; j++)
        {
            complex<float> dot(0, 0);
            for (int k = 0; k < num_rows; k++)
                dot += conj(u[i * num_rows + k]) * u[j * num_rows + k];
            float expected = (i == j) ? 1.0f : 0.0f;
            assert(abs(dot - complex<float>(expected, 0)) < eps);
        }
    }

    // 4. Vt is semi-unitary: Vt * Vt^H = I_{min_dim x min_dim}
    // Vt is col-major with leading dimension ldvt, so Vt[k][j] = vt[j*ldvt + k].
    for (int i = 0; i < min_dim; i++)
    {
        for (int j = 0; j < min_dim; j++)
        {
            complex<float> dot(0, 0);
            for (int k = 0; k < num_cols; k++)
                dot += vt[k * ldvt + i] * conj(vt[k * ldvt + j]);
            float expected = (i == j) ? 1.0f : 0.0f;
            assert(abs(dot - complex<float>(expected, 0)) < eps);
        }
    }
}

void TestMultidimensionalArraySVD()
{
    cout << "Running MultidimensionalArray SVD tests..." << endl;

    const uint32_t m = 3;
    uint32_t d_max = 20;

    const int num_rows = 20, num_cols = 16, min_dim = min(num_rows, num_cols);
    vector<uint32_t> dims = {1, num_rows, num_cols};
    MultidimensionalArray<complex<float>, m> tensor(d_max, dims);

    // Random initialization
    mt19937 rng(42);
    normal_distribution<float> dist(0.0f, 10.0f);
    for (uint32_t r = 0; r < num_rows; r++)
        for (uint32_t c = 0; c < num_cols; c++)
        {
            tensor({0u, r, c}, complex<float>(dist(rng), dist(rng)));
            // cout << tensor({0u, r, c});
        }

    // CPU
    cout << "  CPU SVD..." << endl;
    auto [s_cpu, u_cpu, vt_cpu] = tensor.SVD({1, 2}, 0);
    VerifySVDResult(s_cpu, u_cpu, vt_cpu, tensor, num_rows, num_cols, min_dim);
    delete[] s_cpu;
    delete[] u_cpu;
    delete[] vt_cpu;

    tensor.UpdateToGpu();
    // GPU (skipped if no CUDA device is available)
    int device_count = 0;
    cudaGetDeviceCount(&device_count);
    if (device_count > 0)
    {
        cout << "  GPU SVD..." << endl;
        auto [s_gpu, u_gpu, vt_gpu] = tensor.SVD({1, 2}, 0);

        VerifySVDResult(s_gpu, u_gpu, vt_gpu, tensor, num_rows, num_cols, min_dim);
        delete[] s_gpu;
        delete[] u_gpu;
        delete[] vt_gpu;
    }
    else
    {
        cout << "  GPU SVD skipped (no CUDA device found)" << endl;
    }

    cout << "MultidimensionalArray SVD tests passed!" << endl;
}

void TestMultidimensionalArrayTransposedSVD()
{
    cout << "Running MultidimensionalArray Transposed SVD tests..." << endl;

    const uint32_t m = 3;
    uint32_t d_max = 20;

    const int orig_rows = 16, orig_cols = 16;
    const int num_rows_T = orig_cols;
    const int num_cols_T = orig_rows;
    const int min_dim_T = min(num_rows_T, num_cols_T);
    const int ldvt_T = num_cols_T;

    vector<uint32_t> dims = {1, (uint32_t)orig_rows, (uint32_t)orig_cols};
    MultidimensionalArray<complex<float>, m> tensor(d_max, dims);

    mt19937 rng(42);
    normal_distribution<float> dist(0.0f, 10.0f);
    for (uint32_t r = 0; r < orig_rows; r++)
        for (uint32_t c = 0; c < orig_cols; c++)
            tensor({0u, r, c}, complex<float>(dist(rng), dist(rng)));

    // CPU transposed SVD
    cout << "  CPU Transposed SVD..." << endl;
    auto [s_T_cpu, u_T_cpu, vt_T_cpu] = tensor.SVD({1, 2}, 0, true);
    VerifySVDResult(s_T_cpu, u_T_cpu, vt_T_cpu, tensor,
                    num_rows_T, num_cols_T, min_dim_T, ldvt_T, true);

    // Singular values of A and A^T must match those from the non-transposed SVD.
    auto [s_cpu, u_cpu, vt_cpu] = tensor.SVD({1, 2}, 0, false);
    const float eps = 1e-4f;
    for (int k = 0; k < min_dim_T; k++)
        assert(abs(s_cpu[k] - s_T_cpu[k]) < eps);
    delete[] s_cpu;
    delete[] u_cpu;
    delete[] vt_cpu;

    delete[] s_T_cpu;
    delete[] u_T_cpu;
    delete[] vt_T_cpu;

    tensor.UpdateToGpu();
    int device_count = 0;
    cudaGetDeviceCount(&device_count);
    if (device_count > 0)
    {
        cout << "  GPU Transposed SVD..." << endl;
        auto [s_T_gpu, u_T_gpu, vt_T_gpu] = tensor.SVD({1, 2}, 0, true);
        VerifySVDResult(s_T_gpu, u_T_gpu, vt_T_gpu, tensor,
                        num_rows_T, num_cols_T, min_dim_T, ldvt_T, true);
        delete[] s_T_gpu;
        delete[] u_T_gpu;
        delete[] vt_T_gpu;
    }
    else
    {
        cout << "  GPU Transposed SVD skipped (no CUDA device found)" << endl;
    }

    cout << "MultidimensionalArray Transposed SVD tests passed!" << endl;
}

int main()
{
    TestMultidimensionalArray();
    TestMultidimensionalArraySVD();
    TestMultidimensionalArrayTransposedSVD();
    return 0;
}
