//
//  AVXkernels.h
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 12/22/17.
//

#ifndef kernelsAVX_h
#define kernelsAVX_h

#include <complex>
#include <immintrin.h>

using namespace std;

using cmplx = complex<float>;
using idx_size = size_t;

static const __m256 kM256CmplxNeg1 = _mm256_setr_ps(1.0, 1.0, 1.0, 1.0, 1.0, -1.0, 1.0, -1.0);
static const __m256 kM256CmplxNeg2 = _mm256_setr_ps(1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0);
static const __m256 kM256Neg = _mm256_setr_ps(1.0, 1.0, 1.0, 1.0, -1.0, -1.0, -1.0, -1.0);
static const __m128 kM128CmplxNeg = _mm_setr_ps(1.0, -1.0, 1.0, -1.0);

__attribute__((always_inline)) inline void
ApplyXX12GateAVX(cmplx* __restrict amp,
                 const array<idx_size, 4> indices)
{
//    cmplx a[4] = {amp[indices[0]], amp[indices[1]], amp[indices[2]], amp[indices[3]]};
//    __m128 a0 = _mm_setr_ps(real(a[0]), imag(a[0]), real(a[1]), imag(a[1]));
//    __m128 a1 = _mm_setr_ps(real(a[3]), imag(a[3]), real(a[2]), imag(a[2]));
//    __m128 t = _mm_add_ps(a0, a1);
//    __m128 t1 = _mm_sub_ps(a0, a1);
//    t1 = _mm_permute_ps(t1, 0x5);
//    t1 = _mm_mul_ps(t1, kM128CmplxNeg);
//    t = _mm_permute_ps(t, 0b10110001);
//
//    __m128 val1 = _mm_add_ps(t, t1);
//    __m128 val2 = _mm_sub_ps(t, t1);
//    float* t_amp1 = (float*)&val1;
//    float* t_amp2 = (float*)&val2;
//
//    amp[indices[0]] = cmplx(t_amp1[0], t_amp1[1]);
//    amp[indices[1]] = cmplx(t_amp1[2], t_amp1[3]);
//    amp[indices[2]] = cmplx(t_amp2[0], t_amp1[1]);
//    amp[indices[3]] = cmplx(t_amp2[2], t_amp1[3]);
    
        cmplx a[4] = {amp[indices[0]], amp[indices[1]], amp[indices[2]], amp[indices[3]]};
        __m256 a0 = _mm256_setr_ps(real(a[0]), imag(a[0]), real(a[1]), imag(a[1]),
                                    imag(a[0]), real(a[0]), imag(a[1]), real(a[1]));
        __m256 a1 = _mm256_setr_ps(real(a[3]), imag(a[3]), real(a[2]), imag(a[2]),
                                   -imag(a[3]), -real(a[3]), -imag(a[2]), -real(a[2]));
        __m256 t = _mm256_add_ps(a0, a1);
        t = _mm256_mul_ps(t, kM256CmplxNeg1);
        __m256 t1 = _mm256_permute2f128_ps(t, t, 1);
        t = _mm256_fmadd_ps(kM256Neg, t, t1);

        float* t_amp = (float*)&t;

        amp[indices[0]] = cmplx(t_amp[0], t_amp[1]);
        amp[indices[1]] = cmplx(t_amp[2], t_amp[3]);
        amp[indices[2]] = cmplx(t_amp[4], t_amp[5]);
        amp[indices[3]] = cmplx(t_amp[6], t_amp[7]);
}

inline void
ApplyXY12GateAVX(cmplx* __restrict amp,
                 const array<idx_size, 4> indices)
{
//    cmplx a[4] = { amp[indices[0]], amp[indices [1]], amp[indices[2]], amp[indices[3]]};
//    __m128 a0 = _mm_setr_ps(real(a[0]), imag(a[0]), real(a[2]), imag(a[2]));
//    __m128 a1 = _mm_setr_ps(real(a[1]), imag(a[1]), real(a[3]), imag(a[3]));
//    __m128 t = _mm_add_ps(a0, a1);
//    __m128 t1 = _mm_sub_ps(a0, a1);
//
//    __m128 first_assign = _mm_permute_ps(t, 0b01001011);
//    first_assign = _mm_mul_ps(first_assign, kM128CmplxNeg);
//    __m128 sec_assign = _mm_permute_ps(t1, 0b01001011);
//    sec_assign = _mm_mul_ps(sec_assign, kM128CmplxNeg);
//    neg = _mm_permute_ps(neg, 0b00111001);
//    __m128 third_assign = _mm_permute_ps(t, 0b00011110);
//    first_assign = _mm_mul_ps(first_assign, kM128CmplxNeg);
//    __m128 fourth_assign = _mm_permute_ps(t1, 0b00011110);
//    sec_assign = _mm_mul_ps(sec_assign, kM128CmplxNeg);
//
//    float* b = (float*)&first_assign;
//    float* c = (float*)&sec_assign;
//    float* d = (float*)&third_assign;
//    float* e = (float*)&fourth_assign;
//
//    first_assign = _mm_setr_ps(b[0], b[1], c[0], c[1]);
//    sec_assign = _mm_setr_ps(b[2], b[3], c[2], c[3]);
//    third_assign = _mm_setr_ps(d[0], d[1], d[0], e[1]);
//    fourth_assign = _mm_setr_ps(d[2], d[3], e[2], e[3]);
//    first_assign = _mm_add_ps(first_assign, sec_assign);
//    sec_assign = _mm_add_ps(third_assign, fourth_assign);
//
//    amp[indices[0]] = cmplx(c[0], c[1]);
//    amp[indices[1]] = cmplx(b[0], b[1]);
//    amp[indices[2]] = cmplx(e[0], e[1]);
//    amp[indices[3]] = cmplx(d[0], d[1]);
    
    cmplx a[4] = { amp[indices[0]], amp[indices [1]], amp[indices[2]], amp[indices[3]]};
    __m256 a0 = _mm256_setr_ps(real(a[0]), imag(a[0]), real(a[2]), imag(a[2]),
                               imag(a[0]), real(a[0]), imag(a[2]), real(a[2]));
    __m256 a1 = _mm256_setr_ps(real(a[1]), imag(a[1]), real(a[3]), imag(a[3]),
                               -imag(a[1]), -real(a[1]), -imag(a[3]), -real(a[3]));
    __m256 t = _mm256_add_ps(a0, a1);
    __m256 t1 = _mm256_permute_ps(t, 0x5);
    t1 = _mm256_mul_ps(t1, kM256CmplxNeg2);

    t1 = _mm256_permute_ps(t1, 0b10110001);
    t = _mm256_add_ps(t, t1);

    float* result = (float*)&t;

    amp[indices[0]] = cmplx(result[6], result[7]);
    amp[indices[1]] = cmplx(result[2], result[3]);
    amp[indices[2]] = cmplx(result[4], result[5]);
    amp[indices[3]] = cmplx(result[0], result[1]);
}

inline void
ApplyYY12GateAVX(cmplx* __restrict amp,
                 const array<idx_size, 4> indices)
{
    cmplx a[4] = { amp[indices[0]], amp[indices [1]], amp[indices[2]], amp[indices[3]]};
    __m128 a0 = _mm_setr_ps(real(a[0]), imag(a[0]), real(a[1]), imag(a[1]));
    __m128 a1 = _mm_setr_ps(real(a[3]), imag(a[3]), real(a[2]), imag(a[2]));
    __m128 t = _mm_add_ps(a0, a1);
    __m128 t1 = _mm_sub_ps(a0, a1);
    t = _mm_permute_ps(t, 0x5);
    t = _mm_mul_ps(t, kM128CmplxNeg);
    t1 = _mm_permute_ps(t1, 0x5);
    t1 = _mm_mul_ps(t1, kM128CmplxNeg);

    float* t_amp1 = (float*)&t;
    float* t_amp2 = (float*)&t1;

    a0 = _mm_setr_ps(t_amp1[0], t_amp1[1], t_amp2[0], t_amp2[1]);
    a1 = _mm_setr_ps(t_amp1[2], t_amp1[3], t_amp2[2], t_amp2[3]);
    t = _mm_add_ps(a0, a1);
    t1 = _mm_sub_ps(a0, a1);

    amp[indices[0]] = cmplx(t_amp2[0], t_amp1[1]);
    amp[indices[1]] = cmplx(t_amp1[2], t_amp1[3]);
    amp[indices[2]] = cmplx(t_amp2[0], t_amp1[1]);
    amp[indices[3]] = cmplx(t_amp2[2], t_amp1[3]);
}

inline void
ApplyYX12GateAVX(cmplx* __restrict amp,
                 const array<idx_size, 4> indices)
{
    cmplx a[4] = { amp[indices[0]], amp[indices [1]], amp[indices[2]], amp[indices[3]]};
    __m256 a0 = _mm256_setr_ps(real(a[0]), imag(a[0]), real(a[1]), imag(a[1]),
                               real(a[2]), imag(a[2]), real(a[3]), imag(a[3]));
    __m256 a1 = _mm256_setr_ps(imag(a[1]), real(a[1]), imag(a[0]), real(a[0]),
                               imag(a[3]), real(a[3]), imag(a[2]), real(a[2]));
    a1 = _mm256_mul_ps(a1, kM256CmplxNeg2);
    __m256 res = _mm256_add_ps(a0, a1);
    
    float* f_res = (float*)&res;
    
    amp[indices[0]] = cmplx(f_res[1], f_res[1]);
    amp[indices[1]] = cmplx(f_res[0], f_res[0]);
    amp[indices[2]] = cmplx(f_res[3], f_res[3]);
    amp[indices[3]] = cmplx(f_res[2], f_res[2]);
}


#endif /* kernelsAVX_h */
