//
//  math_helper.h
//  compression
//
//  Created by Aneeqa Fatima on 1/19/19.
//  Copyright © 2019 Aneeqa Fatima. All rights reserved.
//

#ifndef math_helper_h
#define math_helper_h

#include <bitset>
#include <immintrin.h>

using namespace std;

#define GMX_FLOAT_NEGZERO   ({ const union { int  fi; float f; } _gmx_fzero = {-2147483648}; _gmx_fzero.f; })

using Packed128Bits = unsigned long long[2];

constexpr unsigned long long MASK_64_ONES = ~0ull;
constexpr unsigned int MASK_FLOAT_SIGN_EXP = ~((1u << 23) - 1);
constexpr unsigned int FLOAT_MANTISSA_BITS = 23;

const __m256 MASK_FLOAT_MANTISSA = (__m256)_mm256_set1_epi32((1u << 23) - 1);
const __m256 MASK_HIDDEN_BIT = (__m256)_mm256_set1_epi32(1u << 23);
const __m256 MASK_FLOAT_EXP = (__m256)_mm256_set1_epi32(MASK_FLOAT_SIGN_EXP ^ (1u << 31));
const __m256 MASK_REMOVE_HIDDEN_BIT = (__m256)_mm256_set1_epi32(~(1u << 23));
const __m256 MASK_SIGN = (__m256)_mm256_set1_epi32(1u << 31);

static inline double
Factorial(double x)
{
    double product = 1;
    
    for (size_t i = 1; i <= x; ++i)
        product *= i;
    return product;
}

static inline double
ApproxAtan(double z)
{
    const double n1 = 0.97239411f;
    const double n2 = -0.19194795f;
    return (n1 + n2 * z * z) * z;
}

static inline double
ApproxAtan2(double y,
            double x) 
{
    double ay = fabs(y), ax = fabs(x);
    int invert = ay > ax;
    double z = invert ? ax/ay : ay/ax; // [0,1]
    double th = ApproxAtan(z);        // [0,π/4]
    if(invert) th = M_PI_2 - th;       // [0,π/2]
    if(x < 0) th = M_PI - th;          // [0,π]
    th = copysign(th, y);              // [-π,π]
    return th;
}

static inline __m256i _mm256_shift_right(__m256i A,
                                         unsigned int count) {
    
    unsigned int m = (1u << count) - 1;
    __m256i mask = {0, m, m, m};
    
    __m256i last_bits = _mm256_and_si256(A, mask);
    last_bits = _mm256_slli_epi64(last_bits, 64 - count);
    last_bits = _mm256_permute4x64_epi64 (last_bits,  0b00111001);
    last_bits[3] = 0;
    
    __m256i shift_bits = _mm256_srli_epi64(A, count);
    
    return _mm256_or_si256(last_bits, shift_bits);
}

static inline __m256i _mm256_shift_left(__m256i A,
                                        unsigned int count)
{
    unsigned long long m = ((1ul << count) - 1) << (64 - count);
    __m256i mask = {(long long)m, (long long)m, (long long)m, 0};
    
    __m256i last_bits = _mm256_and_si256(A, mask);
    last_bits = _mm256_srli_epi64(last_bits, 64 - count);
    last_bits = _mm256_permute4x64_epi64 (last_bits,  0b10010000);
    last_bits[0] = 0;
    
    __m256i shift_bits = _mm256_slli_epi64(A, count);
    
    return _mm256_or_si256(last_bits, shift_bits);
}

template  <unsigned int N> __m256i _mm256_shift_right(__m256i A)
{
    return _mm256_alignr_epi8(_mm256_permute2x128_si256(A, A, _MM_SHUFFLE(2, 0, 0, 1)), A, N);
}

template <unsigned int N> __m256i _mm256_shift_left0To16(__m256i A)
{
    return _mm256_alignr_epi8(A, _mm256_permute2x128_si256(A, A, _MM_SHUFFLE(0, 0, 2, 0)), 16 - N);
}

template <unsigned int N> __m256i _mm256_shift_left16(__m256i A)
{
    return _mm256_permute2x128_si256(A, A, _MM_SHUFFLE(0, 0, 2, 0));
}

template <unsigned int N> __m256i _mm256_shift_left16To32(__m256i A)
{
    return _mm256_slli_si256(_mm256_permute2x128_si256(A, A, _MM_SHUFFLE(0, 0, 2, 0)), N - 16);
}

static inline __m256 _mm256_sq_norm_cmplx(__m256 real,
                                          __m256 imag)
{
    __m256 squared_real = _mm256_mul_ps(real, real);
    __m256 squared_imag = _mm256_mul_ps(imag, imag);
    return _mm256_add_ps(squared_real, squared_imag);
}

static inline __m256 _mm256_abs_cmplx(__m256 real,
                                      __m256 imag)
{
    __m256 squared_norm = _mm256_sq_norm_cmplx(real, imag);
    return _mm256_sqrt_ps(squared_norm);
}

static inline __m256 ldexp(__m256 value,
                           __m256i exponent)
{
    const __m128i exponentBias      = _mm_set1_epi32(127);
    __m256i       iExponent;
    __m128i       iExponentLow, iExponentHigh;
    
    iExponentHigh = _mm256_extractf128_si256(exponent, 0x1);
    iExponentLow  = _mm256_castsi256_si128(exponent);
    
    iExponentLow  = _mm_add_epi32(iExponentLow, exponentBias);
    iExponentHigh = _mm_add_epi32(iExponentHigh, exponentBias);
    
    iExponentLow  = _mm_max_epi32(iExponentLow, _mm_setzero_si128());
    iExponentHigh = _mm_max_epi32(iExponentHigh, _mm_setzero_si128());
    
    iExponentLow  = _mm_slli_epi32(iExponentLow, 23);
    iExponentHigh = _mm_slli_epi32(iExponentHigh, 23);
    iExponent     = _mm256_castsi128_si256(iExponentLow);
    iExponent     = _mm256_insertf128_si256(iExponent, iExponentHigh, 0x1);
    
    return _mm256_mul_ps(value, _mm256_castsi256_ps(iExponent));
}

static inline __m256 fma(__m256 a,
                         __m256 b,
                         __m256 c)
{
    return  _mm256_add_ps(_mm256_mul_ps(a, b), c);
}

static inline __m256 _mm256_exp_ps(__m256 x)
{
    const __m256  argscale = _mm256_set1_ps(1.44269504088896341f);
    const __m256  invargscale0 = _mm256_set1_ps(-0.693145751953125f);
    const __m256  invargscale1 = _mm256_set1_ps(-1.428606765330187045e-06f);
    const __m256  CC4 = _mm256_set1_ps(0.00136324646882712841033936f);
    const __m256  CC3 = _mm256_set1_ps(0.00836596917361021041870117f);
    const __m256  CC2 = _mm256_set1_ps(0.0416710823774337768554688f);
    const __m256  CC1 = _mm256_set1_ps(0.166665524244308471679688f);
    const __m256  CC0 = _mm256_set1_ps(0.499999850988388061523438f);
    const __m256  one = _mm256_set1_ps(1.0f);
    __m256        fexppart;
    __m256        intpart;
    __m256        y, p;
    
    // Large negative values are valid arguments to exp2(), so there are two
    // things we need to account for:
    // 1. When the exponents reaches -127, the (biased) exponent field will be
    //    zero and we can no longer multiply with it. There are special IEEE
    //    formats to handle this range, but for now we have to accept that
    //    we cannot handle those arguments. If input value becomes even more
    //    negative, it will start to loop and we would end up with invalid
    //    exponents. Thus, we need to limit or mask this.
    // 2. For VERY large negative values, we will have problems that the
    //    subtraction to get the fractional part loses accuracy, and then we
    //    can end up with overflows in the polynomial.
    //
    // For now, we handle this by forwarding the math optimization setting to
    // ldexp, where the routine will return zero for very small arguments.
    //
    // However, before doing that we need to make sure we do not call cvtR2I
    // with an argument that is so negative it cannot be converted to an integer
    // after being multiplied by argscale.
    
     x = _mm256_max_ps(x, _mm256_set1_ps(std::numeric_limits<std::int32_t>::lowest())/argscale);
    
     y = _mm256_mul_ps(x, argscale);
    
    
    fexppart  = ldexp(one,  _mm256_cvtps_epi32(y));
    intpart   =  _mm256_round_ps(y, _MM_FROUND_NINT);
    
    // Extended precision arithmetics
    x         = fma(invargscale0, intpart, x);
    x         = fma(invargscale1, intpart, x);
    
    p         = fma(CC4, x, CC3);
    p         = fma(p, x, CC2);
    p         = fma(p, x, CC1);
    p         = fma(p, x, CC0);
    p         = fma(x*x, p, x);
    x         = fma(p, fexppart, fexppart);
    return x;
}

static inline unsigned int find_most_sig_set_bit(bitset<128> x)
{
    //Check if the most signficant 64 bits have any value set
    unsigned int n = floor(log2((x >> 64).to_ulong()));
    
    return n ? n + 64 : floor(log2(x.to_ulong()));
}

static inline __m256 _mm256_mul_128_unsigned(__m256 x,
                                             bitset<128> y)
{
    const unsigned int most_sig_bit = find_most_sig_set_bit(y);
   
    __m256 mantissas = (__m256)_mm256_and_si256((__m256i)x, (__m256i)MASK_FLOAT_MANTISSA);
    __m256 exponents = (__m256)_mm256_srli_epi32(_mm256_and_si256((__m256i)x, (__m256i)MASK_FLOAT_EXP), FLOAT_MANTISSA_BITS);
    
    //Assumes exponent never underflows or overflows.
    exponents = (__m256)_mm256_add_epi32((__m256i)exponents, _mm256_set1_epi32(most_sig_bit));
    
    __m256 result = (__m256)_mm256_or_si256(_mm256_slli_epi32((__m256i)exponents, FLOAT_MANTISSA_BITS), (__m256i)mantissas);
    
    return result;
}

static inline __m256 _mm256_maskzInv_ps(__m256 x,
                                        __m256 m)
{
    __m256 lu =  _mm256_and_ps(_mm256_rcp_ps(x), m);
    lu = _mm256_mul_ps(lu, _mm256_sub_ps(_mm256_set1_ps(2.0f), _mm256_mul_ps(lu,x)));
    
    return lu;
}

static inline __m256 _mm256_atan_ps(__m256 x)
{
    const __m256 halfpi = _mm256_set1_ps(static_cast<float>(M_PI/2.0f));
    const __m256 CA17 = _mm256_set1_ps(0.002823638962581753730774f);
    const __m256 CA15 = _mm256_set1_ps(-0.01595690287649631500244f);
    const __m256 CA13 = _mm256_set1_ps(0.04250498861074447631836f);
    const __m256 CA11 = _mm256_set1_ps(-0.07489009201526641845703f);
    const __m256 CA9 = _mm256_set1_ps(0.1063479334115982055664f);
    const __m256 CA7 = _mm256_set1_ps (-0.1420273631811141967773f);
    const __m256 CA5 = _mm256_set1_ps(0.1999269574880599975585f);
    const __m256 CA3 = _mm256_set1_ps(-0.3333310186862945556640f);
    const __m256 one = _mm256_set1_ps(1.0f);
    __m256       x2, x3, x4, pA, pB;
    __m256       m, m2;
    
    m     =  _mm256_cmp_ps(x, _mm256_set1_ps(0), _CMP_LT_OQ);
    x     = _mm256_andnot_ps( _mm256_set1_ps(GMX_FLOAT_NEGZERO), x);
    m2    =  _mm256_cmp_ps(one, x, _CMP_LT_OQ);
    x     = _mm256_blendv_ps(x, _mm256_maskzInv_ps(x, m2), m2);
    
    x2    = _mm256_mul_ps(x, x);
    x3    = _mm256_mul_ps(x2, x);
    x4    = _mm256_mul_ps(x2, x2);
    pA    = fma(CA17, x4, CA13);
    pB    = fma(CA15, x4, CA11);
    pA    = fma(pA, x4, CA9);
    pB    = fma(pB, x4, CA7);
    pA    = fma(pA, x4, CA5);
    pB    = fma(pB, x4, CA3);
    pA    = fma(pA, x2, pB);
    pA    = fma(pA, x3, x);
    
    pA    = _mm256_blendv_ps(pA, halfpi-pA, m2);
    pA    = _mm256_blendv_ps(pA, -pA, m);
    
    return pA;
}

static inline __m256 _mm256_atan2_ps(__m256 y,
                                     __m256 x)
{
    const __m256 pi = _mm256_set1_ps(static_cast<float>(M_PI));
    const __m256 halfpi = _mm256_set1_ps(static_cast<float>(M_PI/2.0));
    __m256       xinv, p, aoffset;
    __m256       mask_xnz, mask_ynz, mask_xlt0, mask_ylt0;
    
    mask_xnz  = _mm256_cmp_ps(x, _mm256_set1_ps(0), _CMP_NEQ_OQ);
    mask_ynz  = _mm256_cmp_ps(y, _mm256_set1_ps(0), _CMP_NEQ_OQ);
    mask_xlt0 = _mm256_cmp_ps(x, _mm256_set1_ps(0), _CMP_LT_OQ);
    mask_ylt0 = _mm256_cmp_ps(y, _mm256_set1_ps(0), _CMP_LT_OQ);
    
    aoffset   = _mm256_andnot_ps(mask_xnz, halfpi);
    aoffset   = _mm256_andnot_ps(mask_ynz, aoffset);
    
    aoffset   = _mm256_blendv_ps(aoffset, pi, mask_xlt0);
    aoffset   = _mm256_blendv_ps(aoffset, -aoffset, mask_ylt0);
    
    xinv      = _mm256_maskzInv_ps(x, mask_xnz);
    p         = _mm256_mul_ps(y, xinv);
    p         = _mm256_atan_ps(p);
    p         = _mm256_add_ps(p, aoffset);
    
    return p;
}

static inline __m256 _mm256_fmod_ps(__m256 x,
                                    __m256 y)
{
    __m256 d = _mm256_div_ps(x, y);
    __m256 df = _mm256_floor_ps(d);
    __m256 df_m = _mm256_mul_ps(df, y);
    
    return _mm256_sub_ps(x, df_m);
}

static inline __m256 _mm256_pow_ps(__m256 x,
                                   int n)
{
    __m256 product = _mm256_set1_ps(1);
    if (n == 0)
        return product;
    
    bool reciprocal = false;
    
    if (n < 0) {
        reciprocal = true;
        n = -n;
    }
    
    for (size_t i = 1; i <= n; ++i)
        product = _mm256_mul_ps(product, x);
    
    return reciprocal ? 1/product : product;
}

#endif /* math_helper_h */
