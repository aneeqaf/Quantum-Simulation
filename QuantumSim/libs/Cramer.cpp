//
//  Cramer.cpp
//  compression
//
//  Created by Aneeqa Fatima on 1/2/19.
//  Copyright © 2019 Aneeqa Fatima. All rights reserved.
//

#include "Cramer.h"

Cramer::
    Cramer(size_t vector_size,
           size_t num_codewords,
           size_t num_threads,
           double probabilty_rejection,
           size_t num_sectors /* Cannot be 0 and should be powers of 2*/,
           bool projection_v)
{
    /*
     The probability of rejection are the smaller amps that we map to 0.
     Since the range of the radius of the spiral is from [0,1], the probability of rejection
     defines the lower limit or inner radius of the spiral.

     Spiral formula : r = a * theta
     */
    config.orig_vector_size = vector_size;
    config.num_bits_sector = static_cast<size_t>(log2(num_sectors));
    config.num_bits_codewords = static_cast<size_t>(log2(num_codewords));
    config.num_bits_encoding = config.num_bits_sector + config.num_bits_codewords;
    config.num_total_codewords = num_codewords * num_sectors;
    config.num_threads = num_threads;
    config.num_zero_amps = 0;
    config.num_sectors = num_sectors;
    config.num_turnings = NUM_TURNINGS_CW[config.num_bits_codewords];
    config.projection_vector = projection_v;
    config.dist_type = gamma;
    config.num_codewords_per_sector = (1ull << config.num_bits_codewords);

    config.magnitude_r = CalcMagnitudeForTheta(probabilty_rejection);
    config.A = CalcSizeParameterInUniformSpiral(CDF_MAX_P, 2 * PI * config.num_turnings);
    config.spiral_length_r = CalcExactSpiralLen(probabilty_rejection);
    double total_spiral_length = CalcExactSpiralLen(2 * PI * config.num_turnings) - config.spiral_length_r;
    config.codewords_spacing = total_spiral_length / config.num_codewords_per_sector;

    config.num_codewords_reg = CalcCWThatFitIn256BitsReg();
    config.compressed_vector_UL_size = CalcNumULInCompressedVector(CalcNum256RegForSizeOfVector());

    global_context.codewords_mappings = new atomic<complex<float>>[config.num_codewords_per_sector];
    global_context.cw_freq = new atomic<size_t>[config.num_codewords_per_sector];
    global_context.codeword_all_amps = -1;
    k = 0;
    lambda = 0;
    kAndLambdaInitialized = false;

    new_global_context.codewords_mappings = new atomic<complex<float>>[config.num_codewords_per_sector];
    new_global_context.cw_freq = new atomic<size_t>[config.num_codewords_per_sector];
    new_global_context.codeword_all_amps = -1;

    memset(global_context.codewords_mappings, 0, sizeof(complex<float>) * config.num_codewords_per_sector);
    memset(global_context.cw_freq, 0, sizeof(size_t) * config.num_codewords_per_sector);
    memset(new_global_context.codewords_mappings, 0, sizeof(complex<float>) * config.num_codewords_per_sector);
    memset(new_global_context.cw_freq, 0, sizeof(size_t) * config.num_codewords_per_sector);

    sector_factors = new complex<float>[num_sectors];
    memset(sector_factors, 0, sizeof(complex<float>) * num_sectors);

    // Angle is 0 in the last sector and outermost ring.
    const double phi = 2.0 * PI / config.num_sectors;

    for (size_t i = 0; i < config.num_sectors; ++i)
        sector_factors[i] = complex<float>(cos(phi * i), sin(phi * i));
}

Cramer::
    Cramer(const Cramer &rhs) : config(rhs.config)
{
    global_context.codewords_mappings = new atomic<complex<float>>[config.num_codewords_per_sector];
    global_context.cw_freq = new atomic<size_t>[config.num_codewords_per_sector];
    new_global_context.codewords_mappings = new atomic<complex<float>>[config.num_codewords_per_sector];
    new_global_context.cw_freq = new atomic<size_t>[config.num_codewords_per_sector];
    sector_factors = new complex<float>[config.num_sectors];

    for (size_t i = 0; i < config.num_codewords_per_sector; ++i)
        global_context.codewords_mappings[i].store(rhs.global_context.codewords_mappings[i].load());

    for (size_t i = 0; i < config.num_codewords_per_sector; ++i)
        new_global_context.codewords_mappings[i].store(rhs.new_global_context.codewords_mappings[i].load());

    for (size_t i = 0; i < config.num_codewords_per_sector; ++i)
        new_global_context.cw_freq[i].store(rhs.new_global_context.cw_freq[i].load());

    for (size_t i = 0; i < config.num_codewords_per_sector; ++i)
        new_global_context.cw_freq[i].store(rhs.new_global_context.cw_freq[i].load());

    for (size_t i = 0; i < config.num_sectors; ++i)
        sector_factors[i] = rhs.sector_factors[i];

    global_context.codeword_all_amps = rhs.global_context.codeword_all_amps;
    k = rhs.k;
    lambda = rhs.lambda;
    kAndLambdaInitialized.store(rhs.kAndLambdaInitialized.load());

    new_global_context.codeword_all_amps = rhs.new_global_context.codeword_all_amps;
}

Cramer &Cramer::
operator=(const Cramer &rhs)
{
    Cramer temp(rhs);
    swap(config, temp.config);
    swap(global_context, temp.global_context);
    swap(new_global_context, temp.new_global_context);
    swap(sector_factors, temp.sector_factors);
    kAndLambdaInitialized.exchange(temp.kAndLambdaInitialized);

    return *this;
}

Cramer::
    ~Cramer()
{
    if (global_context.codewords_mappings)
        delete[] global_context.codewords_mappings;
    if (global_context.cw_freq)
        delete[] global_context.cw_freq;
    if (new_global_context.codewords_mappings)
        delete[] new_global_context.codewords_mappings;
    if (new_global_context.cw_freq)
        delete[] new_global_context.cw_freq;
    if (sector_factors)
        delete[] sector_factors;
}

double Cramer::
    CalculateCDFofGammaDist(complex<double> &amp) const
{
    double PT_mag = abs(amp);
    PT_mag = PT_mag > 0 ? PT_mag : 1;
    double PT_probability = PT_mag * PT_mag;
    double Np = PT_probability * lambda;
    double uniform_probability = gammp(k, Np);
    double uniform_mag = sqrt(uniform_probability);
    amp = complex<double>(uniform_mag * (amp.real() / PT_mag), uniform_mag * (amp.imag() / PT_mag));

    return uniform_mag;
}

__m256 Cramer::
    CalculateCDFofGammaDistAVX(__m256 &real,
                               __m256 &imag) const
{
    __m256 PT_probs = _mm256_sq_norm_cmplx(real, imag);
    __m256 PT_mags = _mm256_sqrt_ps(PT_probs);
    __m256 mask = _mm256_cmp_ps(PT_mags, _mm256_setzero_ps(), _CMP_EQ_OS);
    PT_mags = _mm256_or_ps(_mm256_and_ps(mask, _mm256_set1_ps(1)), _mm256_andnot_ps(mask, PT_mags));
    __m256 Np = _mm256_mul_ps(_mm256_set1_ps(lambda), PT_probs);
    __m256 uniform_probs = _mm256_gammp_ps(_mm256_set1_ps(k), Np);
    __m256 uniform_mags = _mm256_sqrt_ps(uniform_probs);

    real = _mm256_mul_ps(uniform_mags, _mm256_div_ps(real, PT_mags));
    imag = _mm256_mul_ps(uniform_mags, _mm256_div_ps(imag, PT_mags));

    return uniform_mags;
}

inline double Cramer::
    CalculateCDFofErlangDist(complex<double> &amp) const
{
    double PT_mag = abs(amp);
    PT_mag = PT_mag > 0 ? PT_mag : 1;
    double PT_probability = PT_mag * PT_mag;
    double Np = PT_probability * lambda;
    double uniform_probability = 0;

    for (size_t n = 0; n < k; ++n)
        uniform_probability += (1 / Factorial(n)) * pow(Np, n) * exp(-Np);

    double uniform_mag = sqrt(1 - uniform_probability);
    amp = complex<double>(uniform_mag * (amp.real() / PT_mag), uniform_mag * (amp.imag() / PT_mag));

    return uniform_mag;
}

inline __m256 Cramer::
    CalculateCDFofErlangDistAVX(__m256 &real,
                                __m256 &imag) const
{
    __m256 PT_probs = _mm256_sq_norm_cmplx(real, imag);
    __m256 PT_mags = _mm256_sqrt_ps(PT_probs);
    __m256 mask = _mm256_cmp_ps(PT_mags, _mm256_setzero_ps(), _CMP_EQ_OS);
    PT_mags = _mm256_or_ps(_mm256_and_ps(mask, _mm256_set1_ps(1)), _mm256_andnot_ps(mask, PT_mags));
    __m256 Np = _mm256_mul_ps(_mm256_set1_ps(lambda), PT_probs);
    __m256 neg_NP = _mm256_sub_ps(_mm256_set1_ps(0), Np);

    __m256 uniform_probs = {0};

    for (size_t n = 0; n < k; ++n)
        uniform_probs = _mm256_add_ps(uniform_probs,
                                      _mm256_mul_ps(_mm256_mul_ps(_mm256_set1_ps(1 / Factorial(n)),
                                                                  _mm256_pow_int_ps(Np, n)),
                                                    _mm256_exp_ps(neg_NP)));

    __m256 uniform_mags = _mm256_sqrt_ps(_mm256_sub_ps(_mm256_set1_ps(1.0), uniform_probs));

    real = _mm256_mul_ps(uniform_mags, _mm256_div_ps(real, PT_mags));
    imag = _mm256_mul_ps(uniform_mags, _mm256_div_ps(imag, PT_mags));

    return uniform_mags;
}

inline double Cramer::
    CalculateCDFofExponential(complex<double> &amp) const
{
    double PT_mag = abs(amp);
    PT_mag = PT_mag > 0 ? PT_mag : 1;
    double PT_probability = PT_mag * PT_mag;
    double uniform_probability = 1.0 - exp(-PT_probability * lambda);

    double uniform_mag = sqrt(uniform_probability);
    amp = complex<double>(uniform_mag * (amp.real() / PT_mag), uniform_mag * (amp.imag() / PT_mag));

    return uniform_mag;
}

inline __m256 Cramer::
    CalculateCDFofExponentialAVX(__m256 &real,
                                 __m256 &imag) const
{
    __m256 PT_probs = _mm256_sq_norm_cmplx(real, imag);
    __m256 PT_mags = _mm256_sqrt_ps(PT_probs);
    __m256 mask = _mm256_cmp_ps(PT_mags, _mm256_setzero_ps(), _CMP_EQ_OS);
    PT_mags = _mm256_or_ps(_mm256_and_ps(mask, _mm256_set1_ps(1)), _mm256_andnot_ps(mask, PT_mags));
    __m256 neg_PT_probs = _mm256_sub_ps(_mm256_set1_ps(0), PT_probs);
    __m256 Np = _mm256_mul_ps(_mm256_set1_ps(lambda), neg_PT_probs);
    __m256 uniform_probs = _mm256_sub_ps(_mm256_set1_ps(1.0), _mm256_exp_ps(Np));
    __m256 uniform_mags = _mm256_sqrt_ps(uniform_probs);

    real = _mm256_mul_ps(uniform_mags, _mm256_div_ps(real, PT_mags));
    imag = _mm256_mul_ps(uniform_mags, _mm256_div_ps(imag, PT_mags));

    return uniform_mags;
}

inline double Cramer::
    UniformTransformMagnitudeAndAmp(complex<double> &amp) const
{
    switch (config.dist_type)
    {
    case exponential:
        return CalculateCDFofExponential(amp);
    case erlang:
        return CalculateCDFofErlangDist(amp);
    case gamma:
        return CalculateCDFofGammaDist(amp);
    default:
        cerr << "Unknown distribution";
        throw;
        break;
    }
}

inline __m256 Cramer::
    UniformTransformMagnitudeAndAmpAVX(__m256 &real,
                                       __m256 &imag) const
{
    switch (config.dist_type)
    {
    case exponential:
        return CalculateCDFofExponentialAVX(real, imag);
        break;
    case erlang:
        return CalculateCDFofErlangDistAVX(real, imag);
    case gamma:
        return CalculateCDFofGammaDistAVX(real, imag);
    default:
        cerr << "Unknown distribution";
        throw;
        break;
    }
}

inline complex<double> Cramer::
    PTTransformMagnitudeAndAmp(complex<double> amp) const
{
    double original_phase = atan2(amp.imag(), amp.real());
    double uniform_probability = norm(amp);
    double PT_probability = -log(abs(1.0 - uniform_probability)) / lambda;

    double PT_mag = sqrt(PT_probability);
    amp = complex<double>(PT_mag * cos(original_phase), PT_mag * sin(original_phase));

    return amp;
}

inline double Cramer::
    CalcSizeParameterInUniformSpiral(double magnitude, double theta) const
{
    return magnitude / theta;
}

inline double Cramer::
    CalcThetaForMagnitude(double magnitude) const
{
    return magnitude / config.A;
}

inline double Cramer::
    CalcMagnitudeForTheta(double theta) const
{
    return config.A * theta;
}

inline double Cramer::
    CalcCWForMagnitude(double magnitude) const
{
    double theta = magnitude / config.A;

    return CalcCWForTheta(theta);
}

inline __m256 Cramer::
    CalcCWForMagnitudeAVX(__m256 magnitudes) const
{
    __m256 thetas = _mm256_div_ps(magnitudes, _mm256_set1_ps(config.A));

    return CalcCWForThetaAVX(thetas);
}

inline double Cramer::
    CalcCWForTheta(double theta) const
{
    return (CalcApproxSpiralLen(theta) - config.spiral_length_r) / config.codewords_spacing;
}

inline __m256 Cramer::
    CalcCWForThetaAVX(__m256 thetas) const
{
    return _mm256_div_ps(_mm256_sub_ps(CalcApproxSpiralLenAVX(thetas), _mm256_set1_ps(config.spiral_length_r)),
                         _mm256_set1_ps(config.codewords_spacing));
}

inline double Cramer::
    CalcMagnitudeForCW(unsigned short codeword) const
{
    double spiral_length = codeword * config.codewords_spacing;

    return CalcApproxThetaForSpiralLen(spiral_length + config.spiral_length_r) * config.A;
}

inline double Cramer::
    CalcApproxSpiralLen(double theta) const
{
    return (config.A * theta * theta) / 2.0;
}

// TODO: Has NAN issues with 0
inline __m256 Cramer::
    CalcApproxSpiralLenAVX(__m256 thetas) const
{
    return _mm256_div_ps(_mm256_mul_ps(_mm256_mul_ps(thetas, thetas), _mm256_set1_ps(config.A)), _mm256_set1_ps(2.0));
}

inline double Cramer::
    CalcExactSpiralLen(double theta) const
{
    return (config.A / 2.0) * ((theta * sqrt(1.0 + (theta * theta))) + log(theta + sqrt(1.0 + (theta * theta))));
}

inline __m256 Cramer::
    CalcExactSpiralLenAVX(__m256 theta) const
{
    __m256 sqrt_vals = _mm256_sqrt_ps(_mm256_add_ps(_mm256_set1_ps(1), _mm256_mul_ps(theta, theta)));
    __m256 theta_plus_sqrt_vals = _mm256_add_ps(theta, sqrt_vals);
    __m256 theta_times_sqrt_vals = _mm256_mul_ps(theta, sqrt_vals);
    __m256 right_side_mul_val = _mm256_add_ps(theta_times_sqrt_vals, _mm256_log_ps(theta_plus_sqrt_vals));

    return _mm256_mul_ps(_mm256_div_ps(_mm256_set1_ps(config.A), _mm256_set1_ps(2)), right_side_mul_val);
}

inline double Cramer::
    CalcApproxThetaForSpiralLen(double spiral_length) const
{
    return sqrt((2.0 * spiral_length) / config.A);
}

inline __m256 Cramer::
    CalcApproxThetaForSpiralLenAVX(__m256 spiral_lengths) const
{
    return _mm256_sqrt_ps(_mm256_div_ps(_mm256_add_ps(spiral_lengths, spiral_lengths), _mm256_set1_ps(config.A)));
}

inline double Cramer::
    CalcThetaForCW(unsigned short codeword) const
{
    double spiral_length = codeword * config.codewords_spacing;

    return CalcApproxThetaForSpiralLen(spiral_length + config.spiral_length_r);
}

inline __m256 Cramer::
    CalcThetaForCWAVX(__m256 codewords) const
{
    __m256 spiral_lengths = _mm256_mul_ps(codewords, _mm256_set1_ps(config.codewords_spacing));

    return CalcApproxThetaForSpiralLenAVX(_mm256_add_ps(spiral_lengths, _mm256_set1_ps(config.spiral_length_r)));
}

inline complex<float> Cramer::
    CalcValForCW(unsigned short codeword) const
{
    auto magnitude = CalcMagnitudeForCW(codeword);
    auto theta = CalcThetaForCW(codeword);
    return complex<float>(magnitude * cos(theta), magnitude * sin(theta));
}

inline size_t Cramer::
    CalcCWThatFitIn256BitsReg() const
{
    return floor((double)REG_SIZE / (double)config.num_bits_encoding);
}

inline size_t Cramer::
    CalcNumULInCompressedVector(size_t num_256_reg) const
{
    return num_256_reg * NUM_UL_IN_REG;
}

inline size_t Cramer::
    CalcNum256RegForSizeOfVector() const
{
    return ceil((double)config.orig_vector_size / (double)config.num_codewords_reg);
}

inline size_t Cramer::
    CalcNum256RegForSizeOfBlock(size_t block_size) const
{
    return ceil((double)block_size / (double)config.num_codewords_reg);
}

inline double Cramer::
    CalcKFromMeanAndVar(double mean,
                        double variance) const
{
    return (mean * mean) / variance;
}

inline double Cramer::
    CalcLambdaFromMeanAndVar(double mean,
                             double variance) const
{
    return mean / variance;
}

void Cramer::
    CalcKandLambdaFromEmpiricalCDF(const complex<float> *state_vector,
                                   const size_t block_size)
{
    double mean = 0, variance = 1e-10;
    CalculateMeanAndVariance(mean, variance,
                             1.0 / (double)(block_size * block_size),
                             state_vector, block_size, config.num_threads);

    if (variance == 0)
    {
        lambda = block_size;
        k = 1;
        config.dist_type = exponential;
    }
    else
    {
        k = CalcKFromMeanAndVar(mean, variance);
        lambda = CalcLambdaFromMeanAndVar(mean, variance);
        config.dist_type = k > 0.9 ? exponential : gamma;
    }
}

void Cramer::
    InitilizeDistributionParameters(const complex<float> *state_vector,
                                    const size_t state_vector_size,
                                    const bool calculate)
{
    if (state_vector && (config.projection_vector || calculate))
        CalcKandLambdaFromEmpiricalCDF(state_vector, state_vector_size);
    else
    {
        lambda = state_vector_size;
        k = 1;
        config.dist_type = exponential;
    }
    kAndLambdaInitialized.store(true);
}

inline unsigned short Cramer::
    ShiftCWToNearestPhase(double phase,
                          double codeword) const
{
    double theta = CalcThetaForCW(codeword);
    double theta_mod2 = fmod(theta, 2.0 * PI);
    double diff_phase = phase - theta_mod2;
    theta += diff_phase;

    return round(CalcCWForTheta(theta));
}

__m256 Cramer::
    ShiftCWToNearestPhaseAVX(__m256 phases,
                             __m256 codewords) const
{
    __m256 thetas = CalcThetaForCWAVX(codewords);

    __m256 theta_mod2 = _mm256_fmod_ps(thetas, _mm256_set1_ps(2.0 * PI));
    __m256 diff_phases = _mm256_sub_ps(phases, theta_mod2);
    thetas = _mm256_add_ps(diff_phases, thetas);

    return _mm256_round_ps(CalcCWForThetaAVX(thetas), _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
}

__m256 Cramer::
    CalcNearestCWToValAVX(__m256 real,
                          __m256 imag)
{
    __m256 magnitudes = _mm256_abs_cmplx(real, imag);
    __m256 phases = Sleef_atan2f8_u10avx2(imag, real);
    __m256 mask_atan2 = _mm256_cmp_ps(phases, _mm256_setzero_ps(), _CMP_GE_OQ);
    __m256 mask1_atan2 = _mm256_cmp_ps(phases, _mm256_setzero_ps(), _CMP_LT_OQ);
    phases = _mm256_or_ps(_mm256_and_ps(phases, mask_atan2),
                          _mm256_and_ps(mask1_atan2, _mm256_add_ps(phases, _mm256_set1_ps(2.0 * PI))));
    __m256 codewords = ShiftCWToNearestPhaseAVX(phases, CalcCWForMagnitudeAVX(magnitudes));

    // Move the codewords so they are within range
    size_t num_codewords = config.num_codewords_per_sector - 1;
    __m256 mask1 = _mm256_cmp_ps(codewords, _mm256_setzero_ps(), _CMP_LE_OQ);
    __m256 mask2 = _mm256_cmp_ps(_mm256_set1_ps(num_codewords), codewords, _CMP_LT_OQ);
    __m256 mask12 = _mm256_or_ps(mask1, mask2);
    codewords = _mm256_or_ps(_mm256_and_ps(mask2, _mm256_set1_ps(num_codewords)),
                             _mm256_andnot_ps(mask12, codewords));

    return codewords;
}

unsigned short Cramer::
    CalcNearestCWToVal(complex<double> val) const
{
    const double magnitude = abs(val);
    double phase = ApproxAtan2(val.imag(), val.real()); // arg(val);
    phase = phase < 0 ? phase + (2 * PI) : phase;

    return ShiftCWToNearestPhase(phase, CalcCWForMagnitude(magnitude));
}

__m256 Cramer::
    CalcNearestCWToValAVX(__m256 real,
                          __m256 imag) const
{
    const __m256 magnitudes = _mm256_abs_cmplx(real, imag);
    __m256 phases = Sleef_atan2f8_u10avx2(imag, real);
    for (size_t i = 0; i < 8; ++i)
        phases[i] = phases[i] < 0 ? phases[i] + (2 * PI) : phases[i];

    return ShiftCWToNearestPhaseAVX(phases, CalcCWForMagnitudeAVX(magnitudes));
}

unsigned short Cramer::
    MapValToCW(complex<double> val)
{
    double magnitude_UT = UniformTransformMagnitudeAndAmp(val);

    if (magnitude_UT <= config.magnitude_r)
        return 0;
    else
    {
        unsigned short codeword = CalcNearestCWToVal(val);

        if (codeword == 0)
            return 0;
        else if (codeword >= config.num_codewords_per_sector)
            return config.num_codewords_per_sector;

        return codeword;
    }
}

__m256 Cramer::
    MapValToCWAVX(__m256 real,
                  __m256 imag)
{
    __m256 magnitudes_UT = UniformTransformMagnitudeAndAmpAVX(real, imag);

    if (_mm256_movemask_ps(_mm256_cmp_ps(magnitudes_UT, _mm256_set1_ps(config.magnitude_r), _CMP_LE_OQ)) == 255)
        return _mm256_set1_ps(0);
    else
    {
        __m256 codewords = CalcNearestCWToValAVX(real, imag);

        __m256 mask1 = _mm256_cmp_ps(codewords, _mm256_setzero_ps(), _CMP_LE_OQ);
        __m256 mask2 = _mm256_cmp_ps(_mm256_set1_ps(config.num_codewords_per_sector - 1), codewords, _CMP_LT_OQ);
        __m256 mask12 = _mm256_or_ps(mask1, mask2);
        codewords = _mm256_or_ps(_mm256_and_ps(mask2, _mm256_set1_ps(config.num_codewords_per_sector - 1)),
                                 _mm256_andnot_ps(mask12, codewords));

        return codewords;
    }
}

__m256i Cramer::
    PackCWIn256BitsAVXReg(const unsigned int *codewords) const
{
    // TODO: make it work for arbitrary number of cw
    size_t iters = ceil((double)config.num_codewords_reg / (double)NUM_UI_IN_REG);
    __m256i idxs = IDXS_CW_FOR_MASKS_UI[config.num_bits_encoding];
    unsigned int idxs_iter[NUM_UI_IN_REG];
    __m256i cw = {0};

    for (size_t i = 0; i < iters; ++i)
    {
        _mm256_store_ps((float *)idxs_iter, (__m256)idxs);

        __m256i temp1_cw = {0}, temp2_cw = {0};
        temp1_cw[0] = codewords[idxs_iter[0]];
        temp1_cw[1] = codewords[idxs_iter[2]];
        temp1_cw[2] = codewords[idxs_iter[4]];
        temp1_cw[3] = codewords[idxs_iter[6]];

        temp2_cw[0] = codewords[idxs_iter[1]];
        temp2_cw[1] = codewords[idxs_iter[3]];
        temp2_cw[2] = codewords[idxs_iter[5]];
        temp2_cw[3] = codewords[idxs_iter[7]];
        temp2_cw = _mm256_slli_epi64(temp2_cw, BITS_UI);

        __m256i temp_cw = _mm256_or_si256(temp1_cw, temp2_cw);
        temp_cw = _mm256_sllv_epi32(temp_cw, BITS_TO_STARTING_OF_UI[config.num_bits_encoding]);
        temp_cw = _mm256_shift_left(temp_cw, i * config.num_bits_encoding);
        cw = _mm256_or_si256(cw, temp_cw);

        idxs = _mm256_add_epi64(idxs, INCREMENT_1_UI);
    }

    return cw;
}

void Cramer::
    UnpackCWFrom256Bits(bitset<REG_SIZE> packed_codewords,
                        unsigned short *unpacked_codewords) const
{
    const bitset<REG_SIZE> set_cw_bits = (1ull << config.num_bits_encoding) - 1;

    for (size_t i = 0; i < config.num_codewords_reg; ++i)
    {
        unpacked_codewords[i] = (packed_codewords & set_cw_bits).to_ulong();
        packed_codewords = packed_codewords >> config.num_bits_encoding;
    }
}

inline __m256i Cramer::
    ExtractCodewordFromAVX256Reg(__m256i &packed_codewords,
                                 const __m256i &mask_cw_256) const
{
    __m256i extracted_bits = _mm256_and_si256(packed_codewords, mask_cw_256);
    packed_codewords = _mm256_shift_right(packed_codewords, config.num_bits_encoding);

    return extracted_bits;
}

inline __m256i Cramer::
    ExtractCodewordFromAVX256Reg(__m256i &packed_codewords) const
{
    __m256i extracted_cws = _mm256_and_si256(packed_codewords, MASKS_CW_PER_UI_REG[config.num_bits_encoding]);
    __m256i cws_aligned_ul = _mm256_srlv_epi32(extracted_cws, BITS_TO_STARTING_OF_UI[config.num_bits_encoding]);

    packed_codewords = _mm256_shift_right(packed_codewords, config.num_bits_encoding);

    return cws_aligned_ul;
}

void Cramer::
    UnpackCWFrom256BitsAVX(__m256i packed_codewords,
                           unsigned int *unpacked_codewords) const
{
    size_t iters = ceil((double)config.num_codewords_reg / (double)NUM_UI_IN_REG);
    __m256i idxs = IDXS_CW_FOR_MASKS_UI[config.num_bits_encoding];

    for (size_t i = 0; i < iters; ++i)
    {
        __m256i extracted_cws = ExtractCodewordFromAVX256Reg(packed_codewords);
        unsigned int idxs_iter[NUM_UI_IN_REG];
        unsigned int cws[NUM_UI_IN_REG];
        _mm256_store_ps((float *)idxs_iter, (__m256)idxs);
        _mm256_store_ps((float *)cws, (__m256)extracted_cws);

        unpacked_codewords[idxs_iter[0]] = cws[0];
        unpacked_codewords[idxs_iter[1]] = cws[1];
        unpacked_codewords[idxs_iter[2]] = cws[2];
        unpacked_codewords[idxs_iter[3]] = cws[3];
        unpacked_codewords[idxs_iter[4]] = cws[4];
        unpacked_codewords[idxs_iter[5]] = cws[5];
        unpacked_codewords[idxs_iter[6]] = cws[6];
        unpacked_codewords[idxs_iter[7]] = cws[7];

        idxs = _mm256_add_epi64(idxs, INCREMENT_1_UI);
    }
}

__attribute__((always_inline)) inline void Cramer::
    PackCWBlocksCrossingBoundaries(__m256i *compressed_vector,
                                   const size_t xtra_leading_cw,
                                   const size_t xtra_trailing_cw,
                                   const size_t compressed_v_offset,
                                   const unsigned int *codewords)
{
    __m256i mask1 = _mm256_shift_left({ALL_ONES_REG}, xtra_leading_cw * config.num_bits_encoding);
    __m256i mask2 = _mm256_shift_right({ALL_ONES_REG}, xtra_trailing_cw * config.num_bits_encoding);
    __m256i mask = _mm256_and_ps(mask1, mask2);

    __m256i packed_cw = PackCWIn256BitsAVXReg(codewords);

    __m256i compressed_reg = _mm256_load_ps((float *)(compressed_vector + compressed_v_offset));
    compressed_reg = _mm256_and_ps(~mask, compressed_reg);
    packed_cw = _mm256_and_ps(mask, packed_cw);
    compressed_reg = _mm256_or_ps(compressed_reg, packed_cw);
    _mm256_store_ps((float *)(compressed_vector + compressed_v_offset), compressed_reg);
}

complex<float> *Cramer::
    CramerBlockCompress(complex<float> *compressed_vector,
                        /* If bigger than a block then pointer should be initial_address + block_begin */
                        const complex<float> *state_vector,
                        const size_t block_offset, // Needed for calculating indices in compressed state
                        const size_t block_size)
{
    assert(kAndLambdaInitialized.load());

    if (compressed_vector == nullptr)
    {
        // Compressed vector is always allocated in full so that it occupies contiguous memory
        if (posix_memalign((void **)&compressed_vector, 64, sizeof(complex<float>) * config.compressed_vector_UL_size) != 0)
            throw "Unable to allocate space for compressed vector";

        memset(compressed_vector, 0, sizeof(complex<float>) * config.compressed_vector_UL_size);
    }

    assert(block_size % 8 == 0);

    const size_t leaked_cw = (block_offset % config.num_codewords_reg);
    const size_t total_cw = leaked_cw + block_size;
    const size_t total_cw_byte_multiple = total_cw + (total_cw % NUM_UI_IN_REG);
    unsigned int codewords[total_cw_byte_multiple];
    memset(codewords, 0, sizeof(unsigned int) * total_cw_byte_multiple);

    // Find all the codewords
    // Start from zero instead of leaked_cw because alignment causes bad memory access
    for (size_t j = 0; j < block_size; j += 8)
    {
        if (block_offset + j >= config.orig_vector_size)
            break;

        // Load 8 amps
        const __m256 temps_amps0 = _mm256_load_ps((float *)&state_vector[j]);
        const __m256 temps_amps1 = _mm256_load_ps((float *)&state_vector[j + 4]);
        const __m256 perm_amps0 = _mm256_permutevar8x32_ps(temps_amps0, _mm256_set_epi32(7, 5, 3, 1, 6, 4, 2, 0));
        const __m256 perm_amps1 = _mm256_permutevar8x32_ps(temps_amps1, _mm256_set_epi32(6, 4, 2, 0, 7, 5, 3, 1));
        const __m256 real = _mm256_blend_ps(perm_amps0, perm_amps1, 0b11110000);
        __m256 imag = _mm256_blend_ps(perm_amps0, perm_amps1, 0b00001111);
        imag = _mm256_permutevar8x32_ps(imag, _mm256_set_epi32(3, 2, 1, 0, 7, 6, 5, 4));
        __m256 cws = MapValToCWAVX(real, imag);

        _mm256_storeu_ps((float *)&codewords[leaked_cw + j], (__m256)_mm256_cvtps_epi32(cws));

        for (size_t k = 0; k < 8; ++k)
        {
            assert(codewords[leaked_cw + j + k] < config.num_codewords_per_sector);
            assert(block_offset + j + k < config.orig_vector_size);
            auto c = codewords[leaked_cw + j + k];
            new_global_context.codewords_mappings[c]
                .store(new_global_context.codewords_mappings[c].load() + state_vector[j + k]);
            ++new_global_context.cw_freq[c];
        }
    }

    // Pack all the codewords
    //  Overlapping codewords
    //  ________------|---------|------_______
    const size_t num_256_in_block = CalcNum256RegForSizeOfBlock(block_size + leaked_cw);
    const size_t compressed_idx = (block_offset - leaked_cw) / config.num_codewords_reg;
    size_t xtra_trailing_cw = 0, xtra_leading_cw = leaked_cw, remaining_cw = leaked_cw + block_size;

    for (size_t i = 0, j = 0; i < num_256_in_block; ++i, j += config.num_codewords_reg)
    {
        assert(compressed_idx + i < ceil((double)config.compressed_vector_UL_size / 4.0)); // 256 bit reg hold four 64 bit UL
        if (remaining_cw < config.num_codewords_reg)
            xtra_trailing_cw = config.num_codewords_reg - remaining_cw;
        PackCWBlocksCrossingBoundaries((__m256i *)compressed_vector, xtra_leading_cw, xtra_trailing_cw, compressed_idx + i, &codewords[j]);
        xtra_leading_cw = 0;
        remaining_cw -= config.num_codewords_reg;
    }

    return compressed_vector;
}

complex<float> *Cramer::
    CramerBlockDecompress(/* If bigger than a block then pointer should be initial_address + block_begin */
                          complex<float> *decompressed_vector,
                          const complex<float> *state_vector,
                          const size_t block_offset,
                          const size_t block_size)
{
    // TODO: unauthorized memory access because num_reg_in_block x num_cw_in_reg > block_size
    if (state_vector == nullptr)
        throw "No compressed input";

    if (decompressed_vector == nullptr)
    {
        if (posix_memalign((void **)&decompressed_vector, 64, sizeof(complex<float>) * block_size) != 0)
            throw "Unable to allocate space for decompressed vector";

        memset(decompressed_vector, 0, sizeof(complex<float>) * block_size);
    }

    __m256i *__restrict compressed_vector = (__m256i *)__builtin_assume_aligned(state_vector, 64);
    size_t num_zero_amps = 0;
    size_t starting_cw_reg = block_offset % config.num_codewords_reg;
    const size_t compressed_idx = (block_offset - starting_cw_reg) / config.num_codewords_reg;
    const size_t end_offset = block_offset + block_size;

    size_t num_256_in_block = CalcNum256RegForSizeOfBlock(block_size + starting_cw_reg);
    size_t k = block_offset;

    unsigned int unpacked_codewords[config.num_codewords_reg + 1];
    memset(unpacked_codewords, 0, sizeof(unsigned int) * config.num_codewords_reg);
    for (size_t i = 0; i < num_256_in_block; ++i)
    {
        UnpackCWFrom256BitsAVX(compressed_vector[compressed_idx + i], unpacked_codewords);

        for (size_t j = starting_cw_reg; j < config.num_codewords_reg && k < end_offset; ++j, ++k)
        {
            assert(k - block_offset < block_size);
            assert(k < config.orig_vector_size);
            assert(unpacked_codewords[j] < config.num_total_codewords);

            if (unpacked_codewords[j] == 0)
                ++num_zero_amps;
            auto cw = unpacked_codewords[j] & (config.num_codewords_per_sector - 1);
            auto sector_cw = unpacked_codewords[j] >> config.num_bits_codewords;
            decompressed_vector[k - block_offset] = sector_factors[sector_cw] * global_context.codewords_mappings[cw].load();
        }
        starting_cw_reg = 0;
    }

    config.num_zero_amps = num_zero_amps;

    return decompressed_vector;
}

void Cramer::
    CramerBlockSectorSwitch(complex<float> *state_vector,
                            const unsigned short *volatile sectors,
                            const size_t block_offset,
                            const size_t block_size)
{
    if (state_vector == nullptr)
        throw "No compressed input";

    int codewords_all = global_context.codeword_all_amps;

    __m256i *__restrict compressed_vector = (__m256i *)state_vector;
    size_t sector_count = 0;
    size_t starting_cw_reg = block_offset % config.num_codewords_reg;
    const size_t starting_idxs = block_offset - starting_cw_reg;
    const size_t compressed_idx = starting_idxs / config.num_codewords_reg;

    size_t num_256_in_block = CalcNum256RegForSizeOfBlock(block_size + starting_cw_reg);

    unsigned int unpacked_codewords[config.num_codewords_reg + 1];
    memset(unpacked_codewords, 0, sizeof(unsigned int) * config.num_codewords_reg);

    for (size_t i = 0; i < num_256_in_block; ++i)
    {
        UnpackCWFrom256BitsAVX(compressed_vector[compressed_idx + i], unpacked_codewords);

        size_t cw_reg = 0;
        // Codewords from entire registers are unpacked but only the ones that belong
        // to the amplitude block are modified
        for (size_t j = starting_cw_reg; j < config.num_codewords_reg && sector_count < block_size; ++j)
        {
            if (codewords_all != -1)
                unpacked_codewords[j] = codewords_all;

            // Do not need to update mapping because CZT don't change magnitude.
            // K and Lambda are only relevant for compression.
            auto prev_sector = unpacked_codewords[j] >> config.num_bits_codewords;
            unpacked_codewords[j] = (unpacked_codewords[j] & (config.num_codewords_per_sector - 1)) | (((prev_sector + sectors[sector_count]) % config.num_sectors) << config.num_bits_codewords);

            // unpacked_codewords[j] &= config.num_codewords_per_sector - 1;
            //                cout << j << ":" << sectors[sector_count] << ", ";
            ++sector_count;
            ++cw_reg;
        }

        size_t xtra_trailing_cw = 0;
        if (sector_count >= block_size)
            xtra_trailing_cw = config.num_codewords_reg - (starting_cw_reg + cw_reg);

        PackCWBlocksCrossingBoundaries(compressed_vector, starting_cw_reg, xtra_trailing_cw, compressed_idx + i, unpacked_codewords);

        starting_cw_reg = 0;
    }
}

void Cramer::
    CommitGlobalContext()
{
#pragma omp parallel for num_threads(config.num_threads)
    for (size_t c = 0; c < config.num_codewords_per_sector; ++c)
    {
        if (new_global_context.codewords_mappings[c].load() != complex<float>(0, 0))
        {
            assert(new_global_context.cw_freq[c] > 0);
            auto new_cw_mapping = new_global_context.codewords_mappings[c].load();
            auto real_avg = new_cw_mapping.real() / new_global_context.cw_freq[c];
            auto imag_avg = new_cw_mapping.imag() / new_global_context.cw_freq[c];
            global_context.codewords_mappings[c].store({real_avg, imag_avg});
            global_context.cw_freq[c] = 0;
        }
    }

    // TODO: Figure out how to set this for QFT
    global_context.codeword_all_amps = -1;
    kAndLambdaInitialized.store(false);
    memset(new_global_context.codewords_mappings, 0, sizeof(complex<float>) * config.num_codewords_per_sector);
    memset(new_global_context.cw_freq, 0, sizeof(size_t) * config.num_codewords_per_sector);
}

complex<float> *Cramer::
    CramerCompress(complex<float> *compressed_vector,
                   complex<float> *state_vector)
{
    InitilizeDistributionParameters(state_vector, config.orig_vector_size);

    if (compressed_vector == nullptr)
    {
        if (posix_memalign((void **)&compressed_vector, 64, sizeof(complex<float>) * config.compressed_vector_UL_size) != 0)
            throw "Unable to allocate space for compressed vector";

        memset(compressed_vector, 0, sizeof(complex<float>) * config.compressed_vector_UL_size);
    }

    size_t block_size = config.num_threads >= 8 ? config.num_threads * config.num_codewords_reg : 8 * config.num_codewords_reg;
    size_t num_blocks = config.orig_vector_size / block_size;

#pragma omp parallel for num_threads(config.num_threads)
    for (size_t i = 0; i < num_blocks; ++i)
    {
        size_t block_idx = i * block_size;
        CramerBlockCompress(compressed_vector, state_vector + block_idx, block_idx, block_size);
    }

    size_t remaining_block_size = config.orig_vector_size - (num_blocks * block_size);
    if (remaining_block_size > 0)
        CramerBlockCompress(compressed_vector, state_vector + (num_blocks * block_size), (num_blocks * block_size), remaining_block_size);

    CommitGlobalContext();

    return compressed_vector;
}

complex<float> *Cramer::
    CramerDecompress(complex<float> *decompressed_vector,
                     const complex<float> *state_vector)
{
    size_t block_size = config.num_threads >= 8 ? config.num_threads * config.num_codewords_reg : 8 * config.num_codewords_reg;
    size_t num_blocks = config.orig_vector_size / block_size;

    if (decompressed_vector == nullptr)
    {
        if (posix_memalign((void **)&decompressed_vector, 64, sizeof(complex<float>) * config.orig_vector_size) != 0)
            throw "Unable to allocate space for decompressed vector";

        memset(decompressed_vector, 0, sizeof(complex<float>) * config.orig_vector_size);
    }

    if (global_context.codeword_all_amps != -1)
    {
        auto cw = global_context.codewords_mappings[global_context.codeword_all_amps].load();

        if (cw == complex<float>(0, 0))
        {
            memset(decompressed_vector, 0, sizeof(complex<float>) * config.orig_vector_size);
            decompressed_vector[0] = 1;
        }
        else
        {
            for (size_t i = 0; i < config.orig_vector_size; ++i)
            {
                decompressed_vector[i] = cw;
            }
        }
        return decompressed_vector;
    }

#pragma omp parallel for num_threads(config.num_threads)
    for (size_t i = 0; i < num_blocks; ++i)
    {
        size_t block_idx = i * block_size;
        CramerBlockDecompress(decompressed_vector + block_idx, state_vector, block_idx, block_size);
    }

    size_t remaining_block_size = config.orig_vector_size - (num_blocks * block_size);
    if (remaining_block_size > 0)
        CramerBlockDecompress(decompressed_vector + (num_blocks * block_size), state_vector, (num_blocks * block_size), remaining_block_size);

    return decompressed_vector;
}

complex<float> *Cramer::
    SetAllAmpsToZero(complex<float> *state_vector)
{
    global_context.codeword_all_amps = 0;
    global_context.codewords_mappings[global_context.codeword_all_amps] = 0;

    if (state_vector == nullptr)
    {
        if (posix_memalign((void **)&state_vector, 64, sizeof(complex<float>) * config.compressed_vector_UL_size) != 0)
            throw "Unable to allocate space for compressed vector";

        memset(state_vector, 0, sizeof(complex<float>) * config.compressed_vector_UL_size);
    }

    return state_vector;
}

complex<float> *Cramer::
    SetAllAmpsToOne(complex<float> *state_vector)
{
    global_context.codeword_all_amps = config.num_codewords_per_sector - 1;
    global_context.codewords_mappings[global_context.codeword_all_amps] = 1;

    // Delay writing of codewords.
    if (state_vector == nullptr)
    {
        if (posix_memalign((void **)&state_vector, 64, sizeof(complex<float>) * config.compressed_vector_UL_size) != 0)
            throw "Unable to allocate space for compressed vector";

        memset(state_vector, 0, sizeof(complex<float>) * config.compressed_vector_UL_size);
    }

    return state_vector;
}

void Cramer::
    Rescale(const __m256 rescaling)
{
    float *__restrict t_cw_map = (float *)__builtin_assume_aligned(global_context.codewords_mappings, 64);

#pragma omp parallel for num_threads(config.num_threads)
    for (size_t i = 0; i < config.num_codewords_per_sector; i += 4)
    {
        __m256 t = _mm256_load_ps(t_cw_map + (2 * i));
        t = _mm256_mul_ps(t, rescaling);
        _mm256_store_ps(t_cw_map + (2 * i), t);
    }
}

size_t Cramer::
    GetCompressedVectorSize() const
{
    return config.compressed_vector_UL_size + config.num_codewords_per_sector;
}

double Cramer::
    GetMinInnerRadius() const
{
    return config.magnitude_r;
}

double Cramer::
    GetMaxOuterRadius() const
{
    return CalcMagnitudeForTheta(config.A);
}

size_t Cramer::
    GetNumOfCW() const
{
    return config.num_total_codewords;
}

size_t Cramer::
    GetNumValsMappedToZero() const
{
    return config.num_zero_amps;
}

size_t Cramer::
    GetGlobalCodeword() const
{
    return global_context.codeword_all_amps;
}

double Cramer::
    GetFactorOfDistBetweenTurns() const
{
    return CalcMagnitudeForTheta(config.A) / config.num_turnings;
}

double Cramer::
    GetDistBetweenCW() const
{
    return config.codewords_spacing;
}
