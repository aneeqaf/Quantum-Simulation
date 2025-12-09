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
#include <climits>

using namespace std;

#define GMX_FLOAT_NEGZERO ({ const union { int  fi; float f; } _gmx_fzero = {-2147483648}; _gmx_fzero.f; })

using Packed128Bits = unsigned long long[2];

constexpr unsigned long long MASK_64_ONES = ~0ull;
constexpr unsigned int MASK_FLOAT_SIGN_EXP = ~((1u << 23) - 1);
constexpr unsigned int FLOAT_MANTISSA_BITS = 23;
constexpr unsigned int NGAU = 18;
constexpr unsigned int ASWITCH = 100;
const float EPS = numeric_limits<float>::epsilon();
const float FPMIN = numeric_limits<float>::min() / EPS;

constexpr float GAMMA_Y[18] = {0.0021695375159141994,
                               0.011413521097787704, 0.027972308950302116, 0.051727015600492421,
                               0.082502225484340941, 0.12007019910960293, 0.16415283300752470,
                               0.21442376986779355, 0.27051082840644336, 0.33199876341447887,
                               0.39843234186401943, 0.46931971407375483, 0.54413605556657973,
                               0.62232745288031077, 0.70331500465597174, 0.78649910768313447,
                               0.87126389619061517, 0.95698180152629142};

constexpr float GAMMA_W[18] = {0.0055657196642445571,
                               0.012915947284065419, 0.020181515297735382, 0.027298621498568734,
                               0.034213810770299537, 0.040875750923643261, 0.047235083490265582,
                               0.053244713977759692, 0.058860144245324798, 0.064039797355015485,
                               0.068745323835736408, 0.072941885005653087, 0.076598410645870640,
                               0.079687828912071670, 0.082187266704339706, 0.084078218979661945,
                               0.085346685739338721, 0.085983275670394821};

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
    double z = invert ? ax / ay : ay / ax; // [0,1]
    double th = ApproxAtan(z);             // [0,π/4]
    if (invert)
        th = M_PI_2 - th; // [0,π/2]
    if (x < 0)
        th = M_PI - th;   // [0,π]
    th = copysign(th, y); // [-π,π]
    return th;
}

static inline __m256i _mm256_shift_right(__m256i A,
                                         size_t count)
{
    if (count >= 64)
    {
        if (count < 128)
        {
            count -= 64;
            A[0] = 0;
            A = _mm256_permute4x64_epi64(A, 0b00111001);
        }
        else if (count < 192)
        {
            count -= 128;
            A[0] = 0;
            A = _mm256_permute4x64_epi64(A, 0b00001110);
        }
        else if (count < 256)
        {
            count -= 192;
            A[0] = 0;
            A = _mm256_permute4x64_epi64(A, 0b00000011);
        }
        else
            return {0};
    }

    long long m = (1ul << count) - 1;
    __m256i mask = {0, m, m, m};

    __m256i last_bits = _mm256_and_si256(A, mask);
    last_bits = _mm256_slli_epi64(last_bits, 64 - static_cast<int>(count));
    last_bits = _mm256_permute4x64_epi64(last_bits, 0b00111001);
    last_bits[3] = 0;

    __m256i shift_bits = _mm256_srli_epi64(A, static_cast<int>(count));

    return _mm256_or_si256(last_bits, shift_bits);
}

static inline __m256i _mm256_shift_left(__m256i A,
                                        size_t count)
{
    if (count >= 64)
    {
        if (count < 128)
        {
            count -= 64;
            A[3] = 0;
            A = _mm256_permute4x64_epi64(A, 0b10010011);
        }
        else if (count < 192)
        {
            count -= 128;
            A[3] = 0;
            A = _mm256_permute4x64_epi64(A, 0b01001111);
        }
        else if (count < 256)
        {
            count -= 192;
            A[3] = 0;
            A = _mm256_permute4x64_epi64(A, 0b00111111);
        }
        else
            return {0};
    }

    long long m = ((1ul << count) - 1) << (64 - count);
    __m256i mask = {m, m, m, 0};

    __m256i last_bits = _mm256_and_si256(A, mask);
    last_bits = _mm256_srli_epi64(last_bits, 64 - static_cast<int>(count));
    last_bits = _mm256_permute4x64_epi64(last_bits, 0b10010000);
    last_bits[0] = 0;

    __m256i shift_bits = _mm256_slli_epi64(A, static_cast<int>(count));

    return _mm256_or_si256(last_bits, shift_bits);
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

static inline __m256 _mm256_abs_ps(__m256 x)
{
    return _mm256_andnot_ps(_mm256_set1_ps(GMX_FLOAT_NEGZERO), x);
}

static inline __m256 ldexp(__m256 value,
                           __m256i exponent)
{
    const __m128i exponentBias = _mm_set1_epi32(127);
    __m256i iExponent;
    __m128i iExponentLow, iExponentHigh;

    iExponentHigh = _mm256_extractf128_si256(exponent, 0x1);
    iExponentLow = _mm256_castsi256_si128(exponent);

    iExponentLow = _mm_add_epi32(iExponentLow, exponentBias);
    iExponentHigh = _mm_add_epi32(iExponentHigh, exponentBias);

    iExponentLow = _mm_max_epi32(iExponentLow, _mm_setzero_si128());
    iExponentHigh = _mm_max_epi32(iExponentHigh, _mm_setzero_si128());

    iExponentLow = _mm_slli_epi32(iExponentLow, 23);
    iExponentHigh = _mm_slli_epi32(iExponentHigh, 23);
    iExponent = _mm256_castsi128_si256(iExponentLow);
    iExponent = _mm256_insertf128_si256(iExponent, iExponentHigh, 0x1);

    return _mm256_mul_ps(value, _mm256_castsi256_ps(iExponent));
}

static inline __m256 fma(__m256 a,
                         __m256 b,
                         __m256 c)
{
    return _mm256_add_ps(_mm256_mul_ps(a, b), c);
}

static inline __m256 _mm256_exp_ps(__m256 x)
{
    const static __m256 argscale = _mm256_set1_ps(1.44269504088896341f);
    const static __m256 invargscale0 = _mm256_set1_ps(-0.693145751953125f);
    const static __m256 invargscale1 = _mm256_set1_ps(-1.428606765330187045e-06f);
    const static __m256 CC4 = _mm256_set1_ps(0.00136324646882712841033936f);
    const static __m256 CC3 = _mm256_set1_ps(0.00836596917361021041870117f);
    const static __m256 CC2 = _mm256_set1_ps(0.0416710823774337768554688f);
    const static __m256 CC1 = _mm256_set1_ps(0.166665524244308471679688f);
    const static __m256 CC0 = _mm256_set1_ps(0.499999850988388061523438f);
    const static __m256 one = _mm256_set1_ps(1.0f);
    __m256 fexppart;
    __m256 intpart;
    __m256 y, p;

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

    x = _mm256_max_ps(x, _mm256_set1_ps(std::numeric_limits<std::int32_t>::lowest()) / argscale);

    y = _mm256_mul_ps(x, argscale);

    fexppart = ldexp(one, _mm256_cvtps_epi32(y));
    intpart = _mm256_round_ps(y, _MM_FROUND_NINT);

    // Extended precision arithmetics
    x = fma(invargscale0, intpart, x);
    x = fma(invargscale1, intpart, x);

    p = fma(CC4, x, CC3);
    p = fma(p, x, CC2);
    p = fma(p, x, CC1);
    p = fma(p, x, CC0);
    p = fma(x * x, p, x);
    x = fma(p, fexppart, fexppart);
    return x;
}

static inline unsigned int find_most_sig_set_bit(bitset<128> x)
{
    // Check if the most signficant 64 bits have any value set
    unsigned int n = floor(log2((x >> 64).to_ulong()));

    return n ? n + 64 : floor(log2(x.to_ulong()));
}

static inline __m256 _mm256_mul_128_unsigned(__m256 x,
                                             bitset<128> y)
{
    const unsigned int most_sig_bit = find_most_sig_set_bit(y);

    __m256 mantissas = (__m256)_mm256_and_si256((__m256i)x, (__m256i)MASK_FLOAT_MANTISSA);
    __m256 exponents = (__m256)_mm256_srli_epi32(_mm256_and_si256((__m256i)x, (__m256i)MASK_FLOAT_EXP), FLOAT_MANTISSA_BITS);

    // Assumes exponent never underflows or overflows.
    exponents = (__m256)_mm256_add_epi32((__m256i)exponents, _mm256_set1_epi32(most_sig_bit));

    __m256 result = (__m256)_mm256_or_si256(_mm256_slli_epi32((__m256i)exponents, FLOAT_MANTISSA_BITS), (__m256i)mantissas);

    return result;
}

static inline __m256 _mm256_maskzInv_ps(__m256 x,
                                        __m256 m)
{
    __m256 lu = _mm256_and_ps(_mm256_rcp_ps(x), m);
    lu = _mm256_mul_ps(lu, _mm256_sub_ps(_mm256_set1_ps(2.0f), _mm256_mul_ps(lu, x)));

    return lu;
}

static inline __m256 _mm256_frexp_ps(__m256 value,
                                     __m256i &exponent)
{
    const __m256 exponentMask = _mm256_castsi256_ps(_mm256_set1_epi32(0x7F800000));
    const __m256 mantissaMask = _mm256_castsi256_ps(_mm256_set1_epi32(0x807FFFFF));
    const __m256 half = _mm256_set1_ps(0.5);
    const __m128i exponentBias = _mm_set1_epi32(126); // add 1 to make our definition identical to frexp()
    __m256i iExponent;
    __m128i iExponentLow, iExponentHigh;

    iExponent = _mm256_castps_si256(_mm256_and_ps(value, exponentMask));
    iExponentHigh = _mm256_extractf128_si256(iExponent, 0x1);
    iExponentLow = _mm256_castsi256_si128(iExponent);
    iExponentLow = _mm_srli_epi32(iExponentLow, 23);
    iExponentHigh = _mm_srli_epi32(iExponentHigh, 23);
    iExponentLow = _mm_sub_epi32(iExponentLow, exponentBias);
    iExponentHigh = _mm_sub_epi32(iExponentHigh, exponentBias);
    iExponent = _mm256_castsi128_si256(iExponentLow);
    exponent = _mm256_insertf128_si256(iExponent, iExponentHigh, 0x1);

    return _mm256_or_ps(_mm256_and_ps(value, mantissaMask), half);
}

static inline __m256 _mm256_inv_ps(__m256 x)
{
    __m256 lu = _mm256_rcp_ps(x);
    return _mm256_mul_ps(lu, _mm256_sub_ps(_mm256_set1_ps(2.0f), _mm256_mul_ps(lu, x)));
}

static inline __m256 _mm256_log_ps(__m256 x)
{
    const static __m256 one = _mm256_set1_ps(1.0f);
    const static __m256 two = _mm256_set1_ps(2.0f);
    const static __m256 invsqrt2 = _mm256_set1_ps(1.0f / sqrt(2.0f));
    const static __m256 corr = _mm256_set1_ps(0.693147180559945286226764f);
    const static __m256 CL9 = _mm256_set1_ps(0.2371599674224853515625f);
    const static __m256 CL7 = _mm256_set1_ps(0.285279005765914916992188f);
    const static __m256 CL5 = _mm256_set1_ps(0.400005519390106201171875f);
    const static __m256 CL3 = _mm256_set1_ps(0.666666567325592041015625f);
    const static __m256 CL1 = _mm256_set1_ps(2.0f);
    __m256 fExp, x2, p;
    __m256 m;
    __m256i iExp;

    x = _mm256_frexp_ps(x, iExp);
    fExp = _mm256_cvtepi32_ps(iExp);

    m = _mm256_cmp_ps(x, invsqrt2, _CMP_LT_OQ);
    // Adjust to non-IEEE format for x<1/sqrt(2): exponent -= 1, mantissa *= 2.0
    fExp = _mm256_sub_ps(fExp, _mm256_and_ps(one, m));
    x = _mm256_mul_ps(x, _mm256_blendv_ps(one, two, m));

    x = _mm256_mul_ps(_mm256_sub_ps(x, one), _mm256_inv_ps(_mm256_add_ps(x, one)));
    x2 = _mm256_mul_ps(x, x);

    p = fma(CL9, x2, CL7);
    p = fma(p, x2, CL5);
    p = fma(p, x2, CL3);
    p = fma(p, x2, CL1);
    p = fma(p, x, corr * fExp);

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

static inline __m256 _mm256_pow_int_ps(__m256 x,
                                       size_t n)
{
    __m256 product = _mm256_set1_ps(1);

    while (n > 0)
    {
        // If n is odd, multiply x with result
        if (n & 1)
            product = _mm256_mul_ps(product, x);

        n = n >> 1;              // n = n/2
        x = _mm256_mul_ps(x, x); // Changes x to x^2
    }

    return product;
}

static inline __m256 _mm256_pow_ps(__m256 x,
                                   __m256 n)
{
    return _mm256_exp_ps(_mm256_mul_ps(x, _mm256_log_ps(n)));
}

//
// Lanczos approximation to the gamma function.
//
// found on http://www.rskey.org/gamma.htm
//
static inline double gamm(double x)
{
    double ret = (1.000000000190015 +
                  76.18009172947146 / (x + 1) +
                  -86.50532032941677 / (x + 2) +
                  24.01409824083091 / (x + 3) +
                  -1.231739572450155 / (x + 4) +
                  1.208650973866179e-3 / (x + 5) +
                  -5.395239384953e-6 / (x + 6));

    return ret * sqrt(2 * M_PI) / x * pow(x + 5.5, x + .5) * exp(-x - 5.5);
}

static inline __m256 _mm256_gamm_ps(__m256 x)
{
    const __m256 c0 = _mm256_set1_ps(1.000000000190015);
    const __m256 c1 = _mm256_div_ps(_mm256_set1_ps(76.18009172947146), _mm256_add_ps(x, _mm256_set1_ps(1)));
    const __m256 c2 = _mm256_div_ps(_mm256_set1_ps(-86.50532032941677), _mm256_add_ps(x, _mm256_set1_ps(2)));
    const __m256 c3 = _mm256_div_ps(_mm256_set1_ps(24.01409824083091), _mm256_add_ps(x, _mm256_set1_ps(3)));
    const __m256 c4 = _mm256_div_ps(_mm256_set1_ps(-1.231739572450155), _mm256_add_ps(x, _mm256_set1_ps(4)));
    const __m256 c5 = _mm256_div_ps(_mm256_set1_ps(1.208650973866179e-3), _mm256_add_ps(x, _mm256_set1_ps(5)));
    const __m256 c6 = _mm256_div_ps(_mm256_set1_ps(-5.395239384953e-6), _mm256_add_ps(x, _mm256_set1_ps(6)));

    __m256 ret = _mm256_add_ps(c1, c0);
    ret = _mm256_add_ps(_mm256_add_ps(c2, c3), ret);
    ret = _mm256_add_ps(_mm256_add_ps(c4, c5), ret);
    ret = _mm256_add_ps(c6, ret);

    return _mm256_mul_ps(_mm256_mul_ps(_mm256_mul_ps(ret, _mm256_div_ps(_mm256_sqrt_ps(_mm256_set1_ps(2 * M_PI)), x)),
                                       _mm256_exp_ps(_mm256_sub_ps(_mm256_sub_ps(_mm256_set1_ps(0), x), _mm256_set1_ps(5.5)))),
                         _mm256_pow_ps(_mm256_add_ps(x, _mm256_set1_ps(5.5)), _mm256_add_ps(x, _mm256_set1_ps(1))));
}

static inline double gammln(double xx)
{
    double x, tmp, ser;
    const static double cof[6] = {76.18009172947146, -86.50532032941677,
                                  24.01409824083091, -1.231739572450155, 0.1208650973866179e-2, -0.5395239384953e-5};

    x = xx - 1.0;
    tmp = x + 5.5;
    tmp -= (x + 0.5) * log(tmp);
    ser = 1.000000000190015;
    for (int j = 0; j <= 5; j++)
    {
        x += 1.0;
        ser += cof[j] / x;
    }
    return -tmp + log(2.5066282746310005 * ser);
}

static inline __m256 _mm256_gammln_ps(__m256 xx)
{
    const static __m256 one = _mm256_set1_ps(1);
    const static __m256 rand = _mm256_set1_ps(2.5066282746310005);
    const static __m256 five_point_five = _mm256_set1_ps(5.5);
    const static __m256 half = _mm256_set1_ps(0.5);
    const static __m256 cof[6] = {_mm256_set1_ps(76.18009172947146), _mm256_set1_ps(-86.50532032941677),
                                  _mm256_set1_ps(24.01409824083091), _mm256_set1_ps(-1.231739572450155), _mm256_set1_ps(0.1208650973866179e-2),
                                  _mm256_set1_ps(-0.5395239384953e-5)};

    __m256 x = _mm256_sub_ps(xx, one);
    __m256 tmp = _mm256_add_ps(x, five_point_five);
    tmp = _mm256_sub_ps(tmp, _mm256_mul_ps(_mm256_add_ps(x, half), _mm256_log_ps(tmp)));
    __m256 ser = _mm256_set1_ps(1.000000000190015);

    for (int i = 0; i <= 5; ++i)
    {
        x = _mm256_add_ps(x, one);
        ser = _mm256_add_ps(_mm256_div_ps(cof[i], x), ser);
    }

    return _mm256_sub_ps(_mm256_log_ps(_mm256_mul_ps(rand, ser)), tmp);
}

static inline float gser(const float a,
                         const float x)
{
    float gln = gammln(a);
    float ap = a, del = 1.0 / a, sum = del;
    for (;;)
    {
        ++ap;
        del *= x / ap;
        sum += del;
        if (fabs(del) < fabs(sum) * EPS)
        {
            return sum * exp(-x + a * log(x) - gln);
        }
    }
}

static inline __m256 _mm256_gser_ps(__m256 a,
                                    __m256 x,
                                    __m256 mask)
{
    const static __m256 one = _mm256_set1_ps(1);
    const static __m256 eps = _mm256_set1_ps(EPS);

    const __m256 gln = _mm256_gammln_ps(a);
    __m256 ap = a;
    __m256 del = _mm256_div_ps(one, a);
    __m256 sum = del;

    do
    {
        ap = _mm256_add_ps(one, ap);
        del = _mm256_mul_ps(_mm256_div_ps(x, ap), del);
        sum = _mm256_add_ps(sum, del);
    } while (_mm256_movemask_ps(_mm256_and_ps(
                 _mm256_cmp_ps(_mm256_abs_ps(del), _mm256_mul_ps(_mm256_abs_ps(sum), eps), _CMP_LT_OQ), mask)) != _mm256_movemask_ps(mask));

    return _mm256_mul_ps(sum, _mm256_exp_ps(_mm256_sub_ps(_mm256_sub_ps(_mm256_mul_ps(a, _mm256_log_ps(x)), x), gln)));
}

static inline float gcf(const float a,
                        const float x)
{
    float gln = gammln(a);
    float b = x + 1.0 - a;
    float c = 1.0 / FPMIN;
    float d = 1.0 / b;
    float h = d;
    for (int i = 1;; i++)
    {
        float an = -i * (i - a);
        b += 2.0;
        d = an * d + b;
        if (fabs(d) < FPMIN)
        {
            cout << "!";
            d = FPMIN;
        }
        c = b + an / c;
        if (fabs(c) < FPMIN)
        {
            cout << "!";
            c = FPMIN;
        }
        d = 1.0 / d;
        float del = d * c;
        h *= del;
        if (fabs(del - 1.0) <= EPS)
            break;
    }
    return exp(-x + a * log(x) - gln) * h;
}

static inline __m256 _mm256_gcf_ps(__m256 a,
                                   __m256 x,
                                   __m256 mask)
{
    static const __m256 one = _mm256_set1_ps(1);
    static const __m256 two = _mm256_set1_ps(2);
    static const __m256 fpmin = _mm256_set1_ps(FPMIN);
    static const __m256 eps = _mm256_set1_ps(EPS);

    const __m256 gln = _mm256_gammln_ps(a);
    __m256 b = _mm256_sub_ps(_mm256_add_ps(x, one), a);
    __m256 c = _mm256_div_ps(one, fpmin);
    __m256 d = _mm256_div_ps(one, b);
    __m256 h = d;

    for (size_t i = 1; i < 1000; ++i)
    {
        __m256 i_t = _mm256_set1_ps(i);
        __m256 an = _mm256_mul_ps(i_t, _mm256_sub_ps(a, i_t));
        b = _mm256_add_ps(b, two);
        d = _mm256_add_ps(_mm256_mul_ps(an, d), b);
        __m256 mask_d = _mm256_cmp_ps(_mm256_abs_ps(d), fpmin, _CMP_LT_OQ);
        d = _mm256_or_ps(_mm256_and_ps(mask_d, fpmin), _mm256_andnot_ps(mask_d, d));
        c = _mm256_add_ps(b, _mm256_div_ps(an, c));
        __m256 mask_c = _mm256_cmp_ps(_mm256_abs_ps(c), fpmin, _CMP_LT_OQ);
        c = _mm256_or_ps(_mm256_and_ps(mask_c, fpmin), _mm256_andnot_ps(mask_c, c));
        d = _mm256_div_ps(one, d);
        __m256 del = _mm256_mul_ps(d, c);
        h = _mm256_mul_ps(h, del);
        if (_mm256_movemask_ps(_mm256_and_ps(
                _mm256_cmp_ps(_mm256_abs_ps(_mm256_sub_ps(del, one)), eps, _CMP_LE_OQ), mask)) == _mm256_movemask_ps(mask))
            break;
    }

    return _mm256_mul_ps(_mm256_exp_ps(_mm256_sub_ps(_mm256_sub_ps(_mm256_mul_ps(a, _mm256_log_ps(x)), x), gln)), h);
}

static inline float gammpapprox(float a,
                                float x)
{
    float xu, t, sum, ans;
    float a1 = a - 1.0, lna1 = log(a1), sqrta1 = sqrt(a1);
    float gln = gammln(a);
    if (x > a1)
        xu = max(a1 + 11.5 * sqrta1, x + 6.0 * sqrta1);
    else
        xu = max(0., min(a1 - 7.5 * sqrta1, x - 5.0 * sqrta1));
    sum = 0;
    for (size_t j = 0; j < NGAU; j++)
    {
        t = x + (xu - x) * GAMMA_Y[j];
        sum += GAMMA_Y[j] * exp(-(t - a1) + a1 * (log(t) - lna1));
    }
    ans = sum * (xu - x) * exp(a1 * (lna1 - 1.) - gln);
    return ans > 0.0 ? 1.0 - ans : -ans;
}

static inline __m256 _mm256_gammpapprox_ps(__m256 a,
                                           __m256 x,
                                           __m256 mask)
{
    static const __m256 one = _mm256_set1_ps(1);
    static const __m256 eleven_5 = _mm256_set1_ps(11.5);
    static const __m256 six = _mm256_set1_ps(6);
    static const __m256 five = _mm256_set1_ps(5);
    static const __m256 seven_5 = _mm256_set1_ps(7.5);

    const __m256 gln = _mm256_gammln_ps(a);
    const __m256 a1 = _mm256_sub_ps(a, one);
    const __m256 lna1 = _mm256_log_ps(a1);
    const __m256 sqrta1 = _mm256_sqrt_ps(a1);

    __m256 xu;

    // fix this
    if (_mm256_movemask_ps(_mm256_cmp_ps(a1, x, _CMP_LT_OQ)) == 255)
        xu = _mm256_max_ps(_mm256_add_ps(_mm256_mul_ps(sqrta1, eleven_5), a1),
                           _mm256_add_ps(_mm256_mul_ps(sqrta1, six), x));
    else
        xu = _mm256_max_ps(_mm256_setzero_ps(),
                           _mm256_min_ps(_mm256_sub_ps(a1, _mm256_mul_ps(sqrta1, seven_5)),
                                         _mm256_sub_ps(x, _mm256_mul_ps(sqrta1, five))));

    __m256 sum = _mm256_setzero_ps();
    for (size_t j = 0; j < NGAU; j++)
    {
        const __m256 t = _mm256_add_ps(_mm256_mul_ps(_mm256_set1_ps(GAMMA_Y[j]), _mm256_sub_ps(xu, x)), x);
        const __m256 e_val = _mm256_exp_ps(_mm256_sub_ps(_mm256_mul_ps(a1, _mm256_sub_ps(_mm256_log_ps(t), lna1)), _mm256_sub_ps(t, a1)));
        sum = _mm256_add_ps(sum, _mm256_mul_ps(_mm256_set1_ps(GAMMA_Y[j]), e_val));
    }

    __m256 ans = _mm256_mul_ps(sum,
                               _mm256_mul_ps(_mm256_sub_ps(xu, x),
                                             _mm256_exp_ps(_mm256_sub_ps(_mm256_mul_ps(a1, _mm256_sub_ps(lna1, one)), gln))));

    __m256 ans_mask = _mm256_cmp_ps(_mm256_setzero_ps(), ans, _CMP_LT_OQ);
    ans = _mm256_or_ps(_mm256_and_ps(ans_mask, _mm256_sub_ps(one, ans)),
                       _mm256_andnot_ps(ans_mask, _mm256_sub_ps(_mm256_setzero_ps(), ans)));

    return ans;
}

static inline float gammp(const float a,
                          const float x)
{
    if (x < 0.0 || a <= 0.0)
        throw("bad args in gammp");
    if (x == 0.0)
        return 0.0;
    else if ((size_t)a >= ASWITCH)
        return gammpapprox(a, x);
    else if (x < a + 1.0)
        return gser(a, x);
    else
        return 1.0 - gcf(a, x);
}

static inline __m256 _mm256_gammp_ps(__m256 a,
                                     __m256 x)
{
    static const __m256 one = _mm256_set1_ps(1);

    if (_mm256_movemask_ps(_mm256_cmp_ps(x, _mm256_setzero_ps(), _CMP_EQ_OQ)) == 255)
        return _mm256_setzero_ps();

    __m256 mask2 = _mm256_cmp_ps(x, _mm256_add_ps(a, one), _CMP_LT_OQ);
    __m256 mask3 = _mm256_andnot_ps(mask2, (__m256)_mm256_set1_epi32(~0));
    __m256 second = _mm256_setzero_ps(), third = _mm256_setzero_ps();

    if (_mm256_movemask_ps(mask2) != 0)
        second = _mm256_gser_ps(a, x, mask2);
    if (_mm256_movemask_ps(mask3) != 0)
        third = _mm256_sub_ps(one, _mm256_gcf_ps(a, x, mask3));
    //    float result_test[8] = {0};
    //    for (int i = 0; i < 8; ++i) {
    //        result_test[i] = gammp(a[i], x[i]);
    //    }

    __m256 result = _mm256_or_ps(_mm256_and_ps(second, mask2), _mm256_and_ps(third, mask3));

    //    cout << "test:" << endl;
    //    for (int i = 0; i < 8; ++i) {
    //        cout << result_test[i] << " : " << result[i] << endl;
    //    }

    return result;
}

static inline void
CalculateMeanAndVariance(double &mean,
                         double &variance,
                         const double threshold,
                         const atomic<complex<float>> *input /* pointer to beginning of input block */,
                         const size_t input_size,
                         const size_t num_threads)
{
    size_t non_zero_amps = 0;
#pragma omp parallel for reduction(+ : non_zero_amps, mean) num_threads(num_threads)
    for (size_t i = 0; i < input_size; ++i)
    {
        double p = norm(input[i].load());
        if (p > threshold)
        {
            ++non_zero_amps;
            mean += p;
        }
        if (mean > ULONG_MAX - 100)
        {
            mean /= non_zero_amps;
            non_zero_amps = 1;
        }
    }
    assert(mean != NAN);
    assert(mean < INFINITY);

    mean /= non_zero_amps;

    variance = 0;
    non_zero_amps = 1;
#pragma omp parallel for reduction(+ : variance) num_threads(num_threads)
    for (size_t i = 0; i < input_size; ++i)
    {
        double p = norm(input[i].load());
        if (p > threshold)
        {
            ++non_zero_amps;
            variance += ((p - mean) * (p - mean));
        }
        if (variance > ULONG_MAX - 100)
        {
            variance /= non_zero_amps;
            non_zero_amps = 1;
        }
    }
    assert(variance != NAN);
    assert(variance < INFINITY);
    assert(variance != 0);

    variance /= (non_zero_amps - 1);
}

static inline void
CalculateMeanAndVariance(double &mean,
                         double &variance,
                         const double threshold,
                         const complex<float> *input /* pointer to beginning of input block */,
                         const size_t input_size,
                         const size_t num_threads)
{
    double non_zero_amps = 1e-8;
    // #pragma omp parallel for reduction(+ : non_zero_amps, mean) num_threads(num_threads)
    for (size_t i = 0; i < input_size; ++i)
    {
        double p = norm(input[i]);
        if (p > 0)
        {
            ++non_zero_amps;
            mean += p;
        }
        if (mean > 1e10)
        {
            mean /= non_zero_amps;
            non_zero_amps = 1;
        }
    }

    mean /= non_zero_amps;
    assert(!isnan(mean));
    assert(!isinf(mean));

    variance = 0;
    non_zero_amps = 1e-8;
    // #pragma omp parallel for reduction(+ : variance) num_threads(num_threads)
    for (size_t i = 0; i < input_size; ++i)
    {
        double p = norm(input[i]);
        if (p > 0)
        {
            ++non_zero_amps;
            variance += ((p - mean) * (p - mean));
        }
        if (variance > 1e10)
        {
            variance /= non_zero_amps;
            non_zero_amps = 1;
        }
    }
    variance /= non_zero_amps;

    assert(!isnan(variance));
    assert(!isinf(variance));
}

#endif /* math_helper_h */
