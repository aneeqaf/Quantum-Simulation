//
//  state_tree_tensor.cpp
//  vector_state_sim
//
//  Created by Aneeqa Fatima
//

#include "state_tree_tensor.h"
#include <cuda_runtime.h>
#include <cusolverDn.h>
#include <lapacke.h>
#include <cblas.h>

template <typename dtype, uint32_t m>
MultidimensionalArray<dtype, m>::MultidimensionalArray(uint32_t d_max,
                                                       const vector<uint32_t> &dims,
                                                       bool use_gpu)
    : data(nullptr), d_max(d_max), use_gpu(use_gpu)
{
    allocate_aligned_mem(data, pow(d_max, m));

    for (uint32_t i = 0; i < m; ++i)
        this->dims[i] = dims[i];

    CalculateStrides();
}

template <typename dtype, uint32_t m>
MultidimensionalArray<dtype, m>::MultidimensionalArray(uint32_t d_max, bool use_gpu)
    : data(nullptr), d_max(d_max), use_gpu(use_gpu)
{
    allocate_aligned_mem(data, pow(d_max, m));
    memset(dims, 0, m * sizeof(uint32_t));
    memset(strides, 0, m * sizeof(uint32_t));
}

template <typename dtype, uint32_t m>
MultidimensionalArray<dtype, m>::MultidimensionalArray(const MultidimensionalArray<dtype, m> &rhs)
    : data(nullptr), d_max(rhs.d_max), use_gpu(rhs.use_gpu)
{
    allocate_aligned_mem(data, pow(d_max, m));
    memcpy(data, rhs.data, pow(d_max, m) * sizeof(dtype));
    memcpy(dims, rhs.dims, m * sizeof(uint32_t));
    memcpy(strides, rhs.strides, m * sizeof(uint32_t));
}

template <typename dtype, uint32_t m>
MultidimensionalArray<dtype, m> &MultidimensionalArray<dtype, m>::operator=(const MultidimensionalArray<dtype, m> &rhs)
{
    MultidimensionalArray temp(rhs);
    swap(data, temp.data);
    swap(dims, temp.dims);
    swap(strides, temp.strides);
    swap(d_max, temp.d_max);
    swap(use_gpu, temp.use_gpu);
    return *this;
}

template <typename dtype, uint32_t m>
MultidimensionalArray<dtype, m>::~MultidimensionalArray()
{
    free(data);
    data = nullptr;
}

template <typename dtype, uint32_t m>
dtype MultidimensionalArray<dtype, m>::operator()(const vector<uint32_t> &idxs) const
{
    assert(idxs.size() == m);
    for (size_t i = 0; i < m; i++)
    {
        assert(idxs[i] < dims[i] and idxs[i] < d_max);
    }

    uint32_t idx = 0;
    for (size_t i = 0; i < m; i++)
    {
        idx += idxs[i] * strides[i];
    }
    return data[idx];
}

template <typename dtype, uint32_t m>
void MultidimensionalArray<dtype, m>::operator()(const vector<uint32_t> &idxs, dtype val)
{
    assert(idxs.size() == m);
    for (size_t i = 0; i < m; i++)
    {
        assert(idxs[i] < dims[i] and idxs[i] < d_max);
    }

    uint32_t idx = 0;
    for (size_t i = 0; i < m; i++)
    {
        idx += idxs[i] * strides[i];
    }
    data[idx] = val;
}

template <typename dtype, uint32_t m>
vector<uint32_t> MultidimensionalArray<dtype, m>::Shape() const
{
    return vector<uint32_t>(dims, dims + m);
}

template <typename dtype, uint32_t m>
size_t MultidimensionalArray<dtype, m>::Size() const
{
    uint32_t size = 1;
    for (size_t i = 0; i < m; i++)
    {
        size *= dims[i];
    }

    return size;
}

// Transpose a rows x cols column-major matrix into a row-major matrix
// col-major src of shape (rows x cols) → row-major dst of shape (cols x rows)
template <typename dtype, uint32_t m>
void MultidimensionalArray<dtype, m>::TransposeColToRow(const dtype *src, dtype *dst, uint32_t rows, uint32_t cols)
{
    const float alpha = 1.0f;
    if constexpr (std::is_same_v<dtype, std::complex<float>>)
    {
        cblas_comatcopy(CblasColMajor,
                        CblasTrans,
                        rows,
                        cols,
                        &alpha,
                        reinterpret_cast<const float *>(src),
                        rows,
                        reinterpret_cast<float *>(dst),
                        cols);
    }
    else
    {
        cblas_somatcopy(CblasColMajor, // column-major ordering
                        CblasTrans,    // transpose
                        rows,          // number of rows in src
                        cols,          // number of cols in src
                        &alpha,        // alpha (scale factor)
                        src,
                        rows, // leading dimension of src
                        dst,
                        cols); // leading dimension of dst
    }
}

template <typename dtype, uint32_t m>
void MultidimensionalArray<dtype, m>::CalculateStrides()
{
    if (m >= 2)
    {
        strides[m - 2] = 1;
        strides[m - 1] = dims[m - 2];
        uint32_t current_stride = dims[m - 2] * dims[m - 1];
        for (int i = (int)m - 3; i >= 0; --i)
        {
            strides[i] = current_stride;
            current_stride *= dims[i];
        }
    }
    else
    {
        uint32_t current_stride = 1;
        for (uint32_t i = 0; i < m; ++i)
        {
            strides[i] = current_stride;
            current_stride *= dims[i];
        }
    }
}

template <typename dtype, uint32_t m>
void MultidimensionalArray<dtype, m>::UpdateToGpu()
{
    assert(!use_gpu);
    use_gpu = true;
}

template <typename dtype, uint32_t m>
tuple<float *, dtype *, dtype *> MultidimensionalArray<dtype, m>::SVD(const vector<uint32_t> &n,
                                                                      const uint32_t agg_dim,
                                                                      const bool T)
{
    if (use_gpu)
        return GpuSVD(n, agg_dim, T);
    return CpuSVD(n, agg_dim, T);
}

template <typename dtype, uint32_t m>
dtype *MultidimensionalArray<dtype, m>::PrepareMatrix(const vector<uint32_t> &n,
                                                      const bool T,
                                                      int &num_rows,
                                                      int &num_cols,
                                                      const uint32_t agg_dim)
{
    int agg = agg_dim == -1 || dims[agg_dim] == 0 ? 1 : dims[agg_dim];

    dtype *a_copy;

    if (T)
    {
        num_cols = dims[n[0]] * agg;
        num_rows = dims[n[1]];
        allocate_aligned_mem(a_copy, num_rows * num_cols);

        // transpose each matrix and then stack horizontally
        if (strides[n[1]] == 1)
        {
            memcpy(a_copy, data, num_rows * num_cols * sizeof(dtype));
        }
        else
        {
            for (uint32_t i = 0; i < agg; ++i)
            {
                TransposeColToRow(data + i * strides[agg_dim], a_copy + i * strides[agg_dim], dims[n[0]], dims[n[1]]);
            }
        }
    }
    else
    {
        // stack matrices horizontally
        num_cols = dims[n[1]] * agg;
        num_rows = dims[n[0]];

        allocate_aligned_mem(a_copy, num_rows * num_cols);

        memcpy(a_copy, data, num_rows * num_cols * sizeof(dtype));
    }

    return a_copy;
}

// The function only works when agg_dim is 0 and the dims in n are contiguous. Does not work generally.
template <typename dtype, uint32_t m>
tuple<float *, dtype *, dtype *> MultidimensionalArray<dtype, m>::CpuSVD(const vector<uint32_t> &n,
                                                                         const uint32_t agg_dim,
                                                                         const bool T)
{
    assert(agg_dim <= 0);
    // SVD only works on 2D matrices
    assert(n.size() == 2);
    assert(n[1] == n[0] + 1);
    assert(!use_gpu);

    int num_rows, num_cols;
    dtype *a_copy = PrepareMatrix(n, T, num_rows, num_cols, agg_dim);

    float *s = new float[num_cols];
    dtype *u = new dtype[num_rows * num_cols];
    dtype *vt = new dtype[num_cols * num_cols];
    vector<float> superb(num_cols - 1);

    lapack_int info = LAPACKE_cgesvd(
        LAPACK_COL_MAJOR,
        'S', 'S',
        num_rows, num_cols,
        reinterpret_cast<lapack_complex_float *>(a_copy),
        num_rows,
        s,
        reinterpret_cast<lapack_complex_float *>(u),
        num_rows,
        reinterpret_cast<lapack_complex_float *>(vt),
        num_cols,
        superb.data());

    if (info > 0)
    {
        cerr << "The algorithm computing SVD failed to converge." << endl;
        exit(1);
    }

    free(a_copy);
    return make_tuple(s, u, vt);
}

template <typename dtype, uint32_t m>
tuple<float *, dtype *, dtype *> MultidimensionalArray<dtype, m>::GpuSVD(const vector<uint32_t> &n,
                                                                         const uint32_t agg_dim,
                                                                         const bool T)
{
    assert(agg_dim <= 0);
    assert(n.size() == 2);
    assert(n[1] == n[0] + 1);
    assert(use_gpu);

    int num_rows, num_cols;
    dtype *a_col = PrepareMatrix(n, T, num_rows, num_cols, agg_dim);

    float *s = new float[num_cols];
    dtype *u = new dtype[num_rows * num_cols];
    dtype *vt = new dtype[num_cols * num_cols];

    cusolverDnHandle_t cusolverH = NULL;
    cusolverDnCreate(&cusolverH);

    cuComplex *d_A = NULL;
    float *d_S = NULL;
    cuComplex *d_U = NULL;
    cuComplex *d_VT = NULL;
    int *devInfo = NULL;
    cuComplex *d_work = NULL;
    float *rwork = NULL;

    cudaMalloc((void **)&d_A, sizeof(cuComplex) * num_rows * num_cols);
    cudaMalloc((void **)&d_S, sizeof(float) * num_cols);
    cudaMalloc((void **)&d_U, sizeof(cuComplex) * num_rows * num_cols);
    cudaMalloc((void **)&d_VT, sizeof(cuComplex) * num_cols * num_cols);
    cudaMalloc((void **)&devInfo, sizeof(int));
    cudaMalloc((void **)&rwork, sizeof(float) * 5 * num_cols);

    cudaMemcpy(d_A, a_col, sizeof(dtype) * num_rows * num_cols, cudaMemcpyHostToDevice);

    int lwork = 0;
    cusolverDnCgesvd_bufferSize(cusolverH, num_rows, num_cols, &lwork);

    cudaMalloc((void **)&d_work, sizeof(cuComplex) * lwork);

    // 'S' for thin U (num_rows x num_cols, ldu=num_rows) and thin VT (num_cols x num_cols, ldvt=num_cols)
    // Both returned in column-major by cuSOLVER
    cusolverDnCgesvd(cusolverH, 'S', 'S', num_rows, num_cols, d_A, num_rows, d_S, d_U, num_rows, d_VT, num_cols, d_work, lwork, rwork, devInfo);

    cudaMemcpy(s, d_S, sizeof(float) * num_cols, cudaMemcpyDeviceToHost);

    // cuSOLVER returns U and VT in column-major.
    cudaMemcpy(u, d_U, sizeof(dtype) * num_rows * num_cols, cudaMemcpyDeviceToHost);
    cudaMemcpy(vt, d_VT, sizeof(dtype) * num_cols * num_cols, cudaMemcpyDeviceToHost);

    cudaFree(d_A);
    cudaFree(d_S);
    cudaFree(d_U);
    cudaFree(d_VT);
    cudaFree(d_work);
    cudaFree(rwork);
    cudaFree(devInfo);
    cusolverDnDestroy(cusolverH);

    free(a_col);
    return make_tuple(s, u, vt);
}