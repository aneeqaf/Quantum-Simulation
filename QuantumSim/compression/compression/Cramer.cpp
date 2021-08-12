//
//  Cramer.cpp
//  compression
//
//  Created by Aneeqa Fatima on 1/2/19.
//  Copyright © 2019 Aneeqa Fatima. All rights reserved.
//

#include "graphing.h"

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
    config.num_bits_encoding =  config.num_bits_sector + config.num_bits_codewords;
    config.num_total_codewords = num_codewords * num_sectors;
    config.num_threads = num_threads;
    config.num_zero_amps = 0;
    config.num_sectors = num_sectors;
    config.num_turnings = NUM_TURNINGS_CW[config.num_bits_codewords];
    config.projection_vector = projection_v;
    config.dist_type = exponential;
    config.map_codewords_sector.resize(config.num_turnings);

    config.magnitude_r = CalcMagnitudeForTheta(probabilty_rejection);
    config.A = CalcSizeParameterInUniformSpiral(CDF_MAX_P, 2 * PI * config.num_turnings);
    config.spiral_length_r = CalcExactSpiralLen(probabilty_rejection);
    double total_spiral_length = CalcExactSpiralLen(2 * PI * config.num_turnings) - config.spiral_length_r;
    config.codewords_spacing = (total_spiral_length/(1ull << config.num_bits_codewords));
    
    config.num_codewords_reg = CalcCWThatFitIn256BitsReg();
    config.compressed_vector_UL_size = CalcNumULInCompressedVector(CalcNum256RegForSizeOfVector());
    
    if (!config.projection_vector) {
        block_context.lambda = config.orig_vector_size;
        block_context.k = 1;
        config.dist_type = exponential;
    }
    
    global_context.codewords_mappings = new complex<float>[config.num_total_codewords];
    block_context.active = false;
    block_context.variance = 0;
    block_context.mean = 0;
    block_context.codewords_mappings = nullptr;
    block_context.cw_freq = nullptr;
    
    size_t cw_count = 0;
    config.map_codewords_sector[0] = cw_count;
    double radius_1 = config.A * 2.0 * PI;
    double phase_sector = 2.0 * PI/num_sectors;
    for (size_t i = 1; i < config.num_turnings; ++i) {
        double arc_length_ring =  i * radius_1 * phase_sector;
        size_t num_cw_on_arc = (arc_length_ring / config.codewords_spacing);
        cw_count += num_cw_on_arc;
        config.map_codewords_sector[i] = cw_count;
    }
}

//Cramer::
//Cramer(const Cramer& rhs): config(rhs.config), block_context(rhs.block_context)
//{
//    if (rhs.global_context.codewords_mappings) {
//        if(global_context.codewords_mappings) delete [] global_context.codewords_mappings;
//        global_context.codewords_mappings = new complex<float>[config.num_total_codewords + 1];
//        for (size_t i = 0; i < config.num_total_codewords; ++i)
//            global_context.codewords_mappings[i] = rhs.global_context.codewords_mappings[i];
//    }
//    else {
//        if(global_context.codewords_mappings) delete [] global_context.codewords_mappings;
//        global_context.codewords_mappings = nullptr;
//    }
//}

Cramer::
~Cramer()
{
    if (global_context.codewords_mappings) delete [] global_context.codewords_mappings;
    if (block_context.codewords_mappings) delete [] block_context.codewords_mappings;
    if (block_context.cw_freq) delete [] block_context.cw_freq;
}

double Cramer::
CalculateCDFofGammaDist(complex<double>& amp) const
{
    double PT_mag = abs(amp);
    PT_mag = PT_mag > 0 ? PT_mag : 1;
    double PT_probability = PT_mag * PT_mag;
    double Np = PT_probability * block_context.lambda;
    double uniform_probability = gammp(block_context.k, Np) ;
    double uniform_mag = sqrt(uniform_probability);
    amp = complex<double>(uniform_mag * (amp.real()/PT_mag), uniform_mag * (amp.imag()/PT_mag));
    
    return uniform_mag;
}

__m256 Cramer::
CalculateCDFofGammaDistAVX(__m256& real,
                           __m256& imag ) const
{
    __m256 PT_probs = _mm256_sq_norm_cmplx(real, imag);
    __m256 PT_mags = _mm256_sqrt_ps(PT_probs);
    __m256 mask = _mm256_cmp_ps(PT_mags, _mm256_setzero_ps(), _CMP_EQ_OS);
    PT_mags = _mm256_or_ps(_mm256_and_ps(mask, _mm256_set1_ps(1)),  _mm256_andnot_ps(mask, PT_mags));
    __m256 Np = _mm256_mul_ps(_mm256_set1_ps(block_context.lambda), PT_probs);
    __m256 uniform_probs = _mm256_gammp_ps(_mm256_set1_ps(block_context.k), Np);
    __m256 uniform_mags = _mm256_sqrt_ps(uniform_probs);
    
    real = _mm256_mul_ps(uniform_mags, _mm256_div_ps(real, PT_mags));
    imag = _mm256_mul_ps(uniform_mags, _mm256_div_ps(imag, PT_mags));
    
    return uniform_mags;
}

inline double Cramer::
CalculateCDFofErlangDist(complex<double>& amp) const
{
    double PT_mag = abs(amp);
    PT_mag = PT_mag > 0 ? PT_mag : 1;
    double PT_probability = PT_mag * PT_mag;
    double Np = PT_probability * block_context.lambda;
    double uniform_probability = 0;
    
    for(size_t n = 0; n < block_context.k; ++n)
        uniform_probability += (1/Factorial(n)) * pow(Np, n) * exp(-Np);
    
    double uniform_mag = sqrt(1 - uniform_probability);
    amp = complex<double>(uniform_mag * (amp.real()/PT_mag), uniform_mag * (amp.imag()/PT_mag));
    
    return uniform_mag;
}

inline __m256 Cramer::
CalculateCDFofErlangDistAVX(__m256& real,
                            __m256& imag ) const
{
    __m256 PT_probs = _mm256_sq_norm_cmplx(real, imag);
    __m256 PT_mags = _mm256_sqrt_ps(PT_probs);
    __m256 mask = _mm256_cmp_ps(PT_mags, _mm256_setzero_ps(), _CMP_EQ_OS);
    PT_mags = _mm256_or_ps(_mm256_and_ps(mask, _mm256_set1_ps(1)),  _mm256_andnot_ps(mask, PT_mags));
    __m256 Np = _mm256_mul_ps(_mm256_set1_ps(block_context.lambda), PT_probs);
    __m256 neg_NP = _mm256_sub_ps(_mm256_set1_ps(0), Np);
    
    __m256 uniform_probs = {0};
    
    for(size_t n = 0; n < block_context.k; ++n)
        uniform_probs = _mm256_add_ps(uniform_probs,
                                      _mm256_mul_ps(_mm256_mul_ps(_mm256_set1_ps(1/Factorial(n)),
                                                                  _mm256_pow_int_ps(Np, n)), _mm256_exp_ps(neg_NP)));
    
    __m256 uniform_mags = _mm256_sqrt_ps(_mm256_sub_ps(_mm256_set1_ps(1.0), uniform_probs));
    
    real = _mm256_mul_ps(uniform_mags, _mm256_div_ps(real, PT_mags));
    imag = _mm256_mul_ps(uniform_mags, _mm256_div_ps(imag, PT_mags));
    
    return uniform_mags;
}

inline double Cramer::
CalculateCDFofExponential(complex<double>& amp) const
{
    double PT_mag = abs(amp);
    PT_mag = PT_mag > 0 ? PT_mag : 1;
    double PT_probability = PT_mag * PT_mag;
    double uniform_probability = 1.0 - exp(-PT_probability * block_context.lambda);
    
    double uniform_mag = sqrt(uniform_probability);
    amp = complex<double>(uniform_mag * (amp.real()/PT_mag), uniform_mag * (amp.imag()/PT_mag));
    
    return uniform_mag;
}

inline __m256 Cramer::
CalculateCDFofExponentialAVX(__m256& real,
                             __m256& imag) const
{
    __m256 PT_probs = _mm256_sq_norm_cmplx(real, imag);
    __m256 PT_mags = _mm256_sqrt_ps(PT_probs);
    __m256 mask = _mm256_cmp_ps(PT_mags, _mm256_setzero_ps(), _CMP_EQ_OS);
    PT_mags = _mm256_or_ps(_mm256_and_ps(mask, _mm256_set1_ps(1)),  _mm256_andnot_ps(mask, PT_mags));
    __m256 neg_PT_probs = _mm256_sub_ps(_mm256_set1_ps(0), PT_probs);
    __m256 Np = _mm256_mul_ps(_mm256_set1_ps(block_context.lambda), neg_PT_probs);
    __m256 uniform_probs = _mm256_sub_ps(_mm256_set1_ps(1.0), _mm256_exp_ps(Np));
    __m256 uniform_mags = _mm256_sqrt_ps(uniform_probs);
    
    real = _mm256_mul_ps(uniform_mags, _mm256_div_ps(real, PT_mags));
    imag = _mm256_mul_ps(uniform_mags, _mm256_div_ps(imag, PT_mags));
    
    return uniform_mags;
}

inline double Cramer::
UniformTransformMagnitudeAndAmp(complex<double>& amp) const
{
    switch (config.dist_type) {
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
UniformTransformMagnitudeAndAmpAVX(__m256& real,
                                   __m256& imag) const
{
    switch (config.dist_type) {
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
    double PT_probability = -log(abs(1.0 - uniform_probability)) / block_context.lambda;
    
    double PT_mag = sqrt(PT_probability);
    amp = complex<double>(PT_mag * cos(original_phase), PT_mag * sin(original_phase));
    
    return amp;
}

inline double Cramer::
CalcSizeParameterInUniformSpiral(double magnitude, double theta) const
{
    return magnitude/theta;
}

inline double Cramer::
CalcThetaForMagnitude(double magnitude) const
{
    return magnitude/config.A;
}

inline double Cramer::
CalcMagnitudeForTheta(double theta) const
{
    return config.A * theta;
}

inline double Cramer::
CalcCWForMagnitude(double magnitude) const
{
    double theta = magnitude/config.A;
    
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
    return (CalcApproxSpiralLen(theta) - config.spiral_length_r)/config.codewords_spacing;
}

inline __m256 Cramer::
CalcCWForThetaAVX(__m256 thetas) const
{
    return _mm256_div_ps(_mm256_sub_ps(CalcApproxSpiralLenAVX(thetas), _mm256_set1_ps(config.spiral_length_r)),
                         _mm256_set1_ps(config.codewords_spacing));
}

inline __m256 Cramer::
CalcCWForPhaseAndMagnitudeAVX(__m256 magnitudes, __m256 phases)
{
    double radius_1 = config.A * 2.0 * PI;
    __m256 turnings = _mm256_div_ps(magnitudes, _mm256_set1_ps(radius_1));
    turnings =  _mm256_round_ps(turnings, _MM_FROUND_TO_NEAREST_INT |_MM_FROUND_NO_EXC);
    __m256 radius_of_turning = _mm256_mul_ps(_mm256_set1_ps(radius_1), turnings);
    __m256 phase_per_slice = _mm256_div_ps(_mm256_set1_ps(config.codewords_spacing), radius_of_turning);
    __m256 codewords = _mm256_round_ps(_mm256_div_ps(phases, phase_per_slice),
                                       _MM_FROUND_TO_NEAREST_INT |_MM_FROUND_NO_EXC);
    for (size_t i = 0; i < NUM_UI_IN_REG; ++i)
        codewords[i] +=  config.map_codewords_sector[turnings[i] - 1];
    
    return codewords;
}

inline double Cramer::
CalcMagnitudeForCW(unsigned short codeword) const
{
    double spiral_length = codeword * config.codewords_spacing;
    
    return CalcApproxThetaForSpiralLen(spiral_length + config.spiral_length_r);
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
    return (config.A/2.0) * ((theta * sqrt(1.0 + (theta * theta))) + log(theta + sqrt(1.0 + (theta * theta))));
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

inline size_t Cramer::
CalcCWThatFitIn256BitsReg() const
{
    return floor((double)REG_SIZE/(double)config.num_bits_encoding);
}

inline size_t Cramer::
CalcNumULInCompressedVector(size_t num_256_reg) const
{
    return num_256_reg * NUM_UL_IN_REG;
}

inline size_t Cramer::
CalcNum256RegForSizeOfVector() const
{
    return ceil((double)config.orig_vector_size/(double)config.num_codewords_reg);
}

inline size_t Cramer::
CalcNum256RegForSizeOfBlock(size_t block_size) const
{
    return ceil((double)block_size/(double)config.num_codewords_reg);
}

inline double Cramer::
CalcKFromMeanAndVar(double mean,
                    double variance) const
{
    return (mean * mean)/variance;
}

inline double Cramer::
CalcLambdaFromMeanAndVar(double mean,
                         double variance) const
{
    return mean/variance;
}

void Cramer::
CalcKandLambdaFromEmpiricalCDF(const complex<float>* state_vector,
                               const size_t block_size)
{
    CalculateMeanAndVariance(block_context.mean, block_context.variance,
                             1.0/((double)config.orig_vector_size * (double)config.orig_vector_size),
                             state_vector, block_size);
    
    block_context.k = CalcKFromMeanAndVar(block_context.mean, block_context.variance);
    block_context.lambda = CalcLambdaFromMeanAndVar(block_context.mean, block_context.variance);
    config.dist_type = block_context.k == 1 ? exponential : gamma;
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
    
    return  _mm256_round_ps(CalcCWForThetaAVX(thetas), _MM_FROUND_TO_NEAREST_INT |_MM_FROUND_NO_EXC);
}

__m256 Cramer::
GetAdjustedPhaseAVX(__m256 phases, __m256 phase_sectors) const
{
    __m256 size_of_sectors = _mm256_div_ps(_mm256_set1_ps(2.0 * PI), _mm256_set1_ps(config.num_sectors));
    
    __m256 adjustment = _mm256_mul_ps(phase_sectors, size_of_sectors) ;
    
    return _mm256_sub_ps(phases, adjustment);
}

__m256 Cramer::
CalcNearestCWToValWithEncodedSectorAVX(__m256 real,
                                       __m256 imag)
{
    __m256 magnitudes = _mm256_abs_cmplx(real, imag);
    __m256 phases = Sleef_atan2f8_u10(imag, real);
    __m256 mask_atan2 = _mm256_cmp_ps(phases, _mm256_setzero_ps(), _CMP_GE_OQ);
    __m256 mask1_atan2 = _mm256_cmp_ps(phases, _mm256_setzero_ps(), _CMP_LT_OQ);
    phases = _mm256_or_ps(_mm256_and_ps(phases, mask_atan2),
                          _mm256_and_ps(mask1_atan2, _mm256_add_ps(phases, _mm256_set1_ps(2.0 * PI))));
    __m256 size_of_sectors = _mm256_div_ps(_mm256_set1_ps(2.0 * PI), _mm256_set1_ps(config.num_sectors));
    __m256 phase_sectors = _mm256_floor_ps(_mm256_div_ps(phases, size_of_sectors));
    __m256 codewords = ShiftCWToNearestPhaseAVX(phases, CalcCWForMagnitudeAVX(magnitudes));
    
    // Move the codewords so they are within range
    size_t num_codewords = (1ull << config.num_bits_codewords) - 1;
    __m256 mask1 = _mm256_cmp_ps(codewords, _mm256_setzero_ps(), _CMP_LE_OQ);
    __m256 mask2 = _mm256_cmp_ps(_mm256_set1_ps(num_codewords), codewords, _CMP_LT_OQ);
    __m256 mask12 = _mm256_or_ps(mask1, mask2);
    codewords = _mm256_or_ps(_mm256_and_ps(mask2, _mm256_set1_ps(num_codewords)),
                             _mm256_andnot_ps(mask12, codewords));
    
    __m256 shifted_phase_sector = _mm256_cvtepi32_ps(_mm256_slli_epi32(_mm256_cvtps_epi32(phase_sectors),
                                                                       static_cast<int>(config.num_bits_codewords)));
    
    return  _mm256_add_ps(shifted_phase_sector, codewords);
}

unsigned short Cramer::
CalcNearestCWToVal(complex<double> val) const
{
    const double magnitude = abs(val);
    double phase = ApproxAtan2(val.imag(), val.real());// arg(val);
    phase = phase < 0 ? phase + (2 * PI) : phase;
    
    return ShiftCWToNearestPhase(phase, CalcCWForMagnitude(magnitude));
}

__m256 Cramer::
CalcNearestCWToValAVX(__m256 real,
                      __m256 imag) const
{
    const __m256 magnitudes = _mm256_abs_cmplx(real, imag);
    __m256 phases = _mm256_atan2_ps(imag, real);
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
    else {
        unsigned short codeword = CalcNearestCWToVal(val);
        
        if (codeword == 0)
            return 0;
        else if (codeword >= config.num_total_codewords)
            return config.num_total_codewords;
        
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
    else {
        __m256 codewords = CalcNearestCWToValWithEncodedSectorAVX(real, imag);
        
        __m256 mask1 = _mm256_cmp_ps(codewords, _mm256_setzero_ps(), _CMP_LE_OQ);
        __m256 mask2 = _mm256_cmp_ps(_mm256_set1_ps(config.num_total_codewords - 1), codewords, _CMP_LT_OQ);
        __m256 mask12 = _mm256_or_ps(mask1, mask2);
        codewords = _mm256_or_ps(_mm256_and_ps(mask2, _mm256_set1_ps(config.num_total_codewords - 1)),
                                 _mm256_andnot_ps(mask12, codewords));
        
        return codewords;
    }
}

__m256i Cramer::
PackCWIn256BitsAVXReg(const unsigned int* codewords) const
{
    size_t iters = ceil((double)config.num_codewords_reg/(double) NUM_UI_IN_REG);
    __m256i idxs = IDXS_CW_FOR_MASKS_UI[config.num_bits_encoding];
    unsigned int idxs_iter[NUM_UI_IN_REG];
    __m256i cw = {0};
    
    for (size_t i = 0; i < iters; ++i) {
        _mm256_store_ps((float*)idxs_iter, (__m256)idxs);
        
        __m256i temp1_cw = {0}, temp2_cw = {0};
        temp1_cw [0] = codewords[idxs_iter[0]];
        temp1_cw [1] = codewords[idxs_iter[2]];
        temp1_cw [2] = codewords[idxs_iter[4]];
        temp1_cw [3] = codewords[idxs_iter[6]];
        
        temp2_cw [0] = codewords[idxs_iter[1]];
        temp2_cw [1] = codewords[idxs_iter[3]];
        temp2_cw [2] = codewords[idxs_iter[5]];
        temp2_cw [3] = codewords[idxs_iter[7]];
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
UnpackCWFrom256Bits( bitset<REG_SIZE> packed_codewords,
                    unsigned short* unpacked_codewords) const
{
    const bitset<REG_SIZE> set_cw_bits = (1ull << config.num_bits_encoding) - 1;
    
    for (size_t i = 0; i < config.num_codewords_reg; ++i) {
        unpacked_codewords[i] = (packed_codewords & set_cw_bits).to_ulong();
        packed_codewords = packed_codewords >> config.num_bits_encoding;
    }
}

inline __m256i Cramer::
ExtractCodewordFromAVX256Reg(__m256i& packed_codewords,
                             const __m256i& mask_cw_256) const
{
    __m256i extracted_bits = _mm256_and_si256(packed_codewords, mask_cw_256);
    packed_codewords = _mm256_shift_right(packed_codewords, config.num_bits_encoding);
    
    return extracted_bits;
}

inline __m256i Cramer::
ExtractCodewordFromAVX256Reg(__m256i& packed_codewords) const
{
    __m256i extracted_cws = _mm256_and_si256(packed_codewords, MASKS_CW_PER_UI_REG[config.num_bits_encoding]);
    __m256i cws_alighned_ul = _mm256_srlv_epi32(extracted_cws, BITS_TO_STARTING_OF_UI[config.num_bits_encoding]);
    
    packed_codewords = _mm256_shift_right(packed_codewords, config.num_bits_encoding);
    
    return cws_alighned_ul;
}

void  Cramer::
UnpackCWFrom256BitsAVX(__m256i packed_codewords,
                       unsigned int* unpacked_codewords) const
{
    size_t iters = ceil((double)config.num_codewords_reg/(double) NUM_UI_IN_REG);
    __m256i idxs = IDXS_CW_FOR_MASKS_UI[config.num_bits_encoding];
    
    for (size_t i = 0; i < iters; ++i) {
        __m256i extracted_cws = ExtractCodewordFromAVX256Reg(packed_codewords);
        unsigned int idxs_iter[NUM_UI_IN_REG];
        unsigned int cws[NUM_UI_IN_REG];
        _mm256_store_ps((float*)idxs_iter, (__m256)idxs);
        _mm256_store_ps((float*)cws, (__m256)extracted_cws);
        
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

//complex<float>* Cramer::
//CramerCompress(complex<float>* compressed_vector,
//               const complex<float>* state_vector)
//{
//    if (projection_vector)
//        CalcKandLambdaFromEmpiricalCDF(state_vector);
//
//    if (compressed_vector == nullptr) {
//        if (posix_memalign((void**)&compressed_vector, 64, sizeof(complex<float>) * compressed_vector_UL_size) != 0)
//            throw "Unable to allocate space for compressed vector";
//
//        memset(compressed_vector, 0, sizeof(complex<float>) * compressed_vector_UL_size);
//    }
//
//    size_t cw_freq[num_codewords + 1];
//    memset(cw_freq, 0, sizeof(size_t) * (num_codewords + 1));
//
////    #pragma omp parallel for reduction(+:num_zero_amps, cw_freq) num_threads(num_threads)
//    for (size_t i = 0; i < orig_vector_size ; i += num_codewords_reg) {
//        unsigned int codewords[num_codewords_reg];
//        //        memset(codewords, 0, sizeof(unsigned short) * num_codewords_reg);
//        for (size_t j = 0; j < num_codewords_reg; ++j) {
//            if (i + j >= orig_vector_size) {
//                codewords[j] = 0;
//                continue;
//            }
//            codewords[j] = MapValToCW(state_vector[i + j]);
//            codewords_mappings[codewords[j]] += state_vector[i + j];
//            ++cw_freq[codewords[j]];
//        }
//
//        __m256i pack_cw = PackCWIn256BitsAVXReg(codewords);
//        size_t idx = (i/num_codewords_reg) * NUM_UL_IN_REG;
//        _mm256_store_ps((float*)&compressed_vector[idx], (__m256)pack_cw);
//    }
//
//    //    #pragma omp parallel for num_threads(num_threads)
//    for (size_t i = 0; i <= num_codewords; ++i)
//        codewords_mappings[i] /= cw_freq[i];
//
//    return compressed_vector;
//}

void Cramer::
InitiateBlockContext()
{
    block_context.active = true;
    block_context.cw_freq = new size_t[config.num_total_codewords + 1];
    memset(block_context.cw_freq, 0, (config.num_total_codewords + 1) * sizeof(size_t));
    block_context.codewords_mappings = new complex<float>[config.num_total_codewords];
    block_context.mean = 0;
    block_context.variance = 0;
    block_context.k = 0;
    block_context.lambda = 0;
}

complex<float>* Cramer::
CramerBlockCompress(complex<float>* compressed_vector,
                    unsigned int* codewords,
                    const complex<float>* state_vector /* starting point of block: i * block_size */,
                    const size_t block_idx,
                    const size_t block_size)
{
    if (!block_context.active) {
        InitiateBlockContext();
        CalcKandLambdaFromEmpiricalCDF(state_vector + block_idx, block_size);
    }
    
    if (compressed_vector == nullptr) {
        if (posix_memalign((void**)&compressed_vector, 64, sizeof(complex<float>) * config.compressed_vector_UL_size) != 0)
            throw "Unable to allocate space for compressed vector";
        
        memset(compressed_vector, 0, sizeof(complex<float>) * config.compressed_vector_UL_size);
    }
    
    if (block_size < 8 * config.num_codewords_reg)
        throw "Compression not supported for very small block sizes.\n";
        
    
//    unsigned int codewords[block_size];
//    memset(codewords, 0, sizeof(unsigned int) * block_size);
    
    //Find all the codewords
    for (size_t j = 0; j < block_size; j += 8) {
        size_t idx = block_idx + j;
        if (idx >= config.orig_vector_size)
            break;
        
        //Load 8 amps
        const __m256 temps_amps0 = _mm256_load_ps((float*)&state_vector[idx]);
        const __m256 temps_amps1 = _mm256_load_ps((float*)&state_vector[idx + 4]);
        const __m256 perm_amps0 = _mm256_permutevar8x32_ps(temps_amps0, _mm256_set_epi32(7, 5, 3, 1, 6, 4, 2, 0));
        const __m256 perm_amps1 = _mm256_permutevar8x32_ps(temps_amps1, _mm256_set_epi32(6, 4, 2, 0, 7, 5, 3, 1));
        const __m256 real = _mm256_blend_ps(perm_amps0, perm_amps1, 0b11110000);
        __m256 imag = _mm256_blend_ps(perm_amps0, perm_amps1, 0b00001111);
        imag = _mm256_permutevar8x32_ps(imag, _mm256_set_epi32(3, 2, 1, 0, 7, 6, 5 , 4));
        __m256 cws = MapValToCWAVX(real, imag);
        
        _mm256_storeu_ps((float*)&codewords[j], (__m256)_mm256_cvtps_epi32(cws));
        
        for (size_t k = 0; k < 8; ++k) {
            block_context.codewords_mappings[codewords[j + k]] += state_vector[idx + k];
            ++block_context.cw_freq[codewords[j + k]];
        }
    }
    //Pack all the codewords
    for (size_t j = 0; j < block_size; j += config.num_codewords_reg) {
        size_t idx = ((block_idx + j)/config.num_codewords_reg) * NUM_UL_IN_REG;
        __m256i pack_cw = PackCWIn256BitsAVXReg(&codewords[j]);
        if (block_idx + j >= config.orig_vector_size)
            break;
        _mm256_store_ps((float*)&compressed_vector[idx], (__m256)pack_cw);
    }
    
    return compressed_vector;
}

complex<float>* Cramer::
CramerBlockDecompress(complex<float>* decompressed_vector,
                      const complex<float>* state_vector,
                      const size_t block_idx,
                      const size_t block_size)
{
    if (decompressed_vector == nullptr) {
        if (posix_memalign((void**)&decompressed_vector, 64, sizeof(complex<float>) * config.orig_vector_size) != 0)
            throw "Unable to allocate space for decompressed vector";
        
        memset(decompressed_vector, 0, sizeof(complex<float>) * config.orig_vector_size);
    }
    
    size_t num_256_in_block = CalcNum256RegForSizeOfBlock(block_size);
    
    if (num_256_in_block < NUM_UL_IN_REG)
        throw "Block size is too small";
    
    __m256i* __restrict compressed_vector = (__m256i *)state_vector;
//    size_t num_extra_cw = block_idx % config.num_codewords_reg;
    size_t compressed_idx = (block_idx - (block_idx % config.num_codewords_reg))/config.num_codewords_reg;
    size_t starting_idxs = block_idx - (block_idx % config.num_codewords_reg);
    
#pragma omp parallel for reduction(+:num_zero_amps) num_threads(num_threads)
    for (size_t i = 0; i < num_256_in_block; ++i) {
        unsigned int unpacked_codewords[config.num_codewords_reg + 1];
        UnpackCWFrom256BitsAVX(compressed_vector[compressed_idx + i], unpacked_codewords);
        for (size_t j = 0; j < config.num_codewords_reg; ++j) {
            size_t k = starting_idxs + (i * config.num_codewords_reg) + j;
            if (k >= (block_idx + block_size))
                continue;
            else if (k >= block_idx){
                if (unpacked_codewords[j] == 0) ++config.num_zero_amps;
                decompressed_vector[k] = global_context.codewords_mappings[unpacked_codewords[j]];
            }
        }
    }
    
    return decompressed_vector;
}

void Cramer::
CommitBlockContext()
{
    //    #pragma omp parallel for num_threads(num_threads)
    for (size_t i = 0; i < config.num_total_codewords; ++i) {
        if (block_context.cw_freq[i] > 0)
            block_context.codewords_mappings[i] /= block_context.cw_freq[i];
        global_context.codewords_mappings[i] = block_context.codewords_mappings[i];
    }
    block_context.active = false;
//    block_context.mean = 0;
//    block_context.variance = 0;
//    block_context.k = 0;
//    block_context.lambda = 0;
//
    delete [] block_context.cw_freq;
    delete [] block_context.codewords_mappings;
    block_context.cw_freq = nullptr;
    block_context.codewords_mappings = nullptr;
}

complex<float>* Cramer::
CramerCompress(complex<float>* compressed_vector,
               const complex<float>* state_vector)
{
    InitiateBlockContext();
    
    if (config.projection_vector)
        CalcKandLambdaFromEmpiricalCDF(state_vector, config.orig_vector_size);
    
    if (compressed_vector == nullptr) {
        if (posix_memalign((void**)&compressed_vector, 64, sizeof(complex<float>) * config.compressed_vector_UL_size) != 0)
            throw "Unable to allocate space for compressed vector";
        
        memset(compressed_vector, 0, sizeof(complex<float>) * config.compressed_vector_UL_size);
    }
    
    vector<vector<complex<float>>> cw_map_plots(config.num_total_codewords + 1);
    vector<vector<float>>error_bins(2, vector<float>(config.num_codewords_reg, 0));
    vector<vector<pair<short, size_t>>> cw_idx;
    vector<float> phase_error_per_cw(config.num_total_codewords + 1, 0);
    vector<float> plot_displacements(config.num_total_codewords + 1, 0);
    
    size_t block_size = 8 * config.num_codewords_reg;
    size_t num_blocks = ceil((double)config.orig_vector_size/(double)block_size);
    
    //#pragma omp parallel for reduction(+: cw_freq) num_threads(num_threads)
    for (size_t i = 0; i < num_blocks; ++i) {
        size_t block_idx = i * block_size;
        unsigned int codewords[block_size];
        memset(codewords, 0, sizeof(unsigned int) * block_size);
          
        CramerBlockCompress(compressed_vector, codewords, state_vector, block_idx, block_size);

        // Graphing code
        for (size_t j = 0; j < block_size; j += 8) {
            size_t idx = block_idx + j;
            for (size_t k = 0; k < 8; ++k)
                cw_map_plots[codewords[j+k]].push_back(state_vector[idx + k]);
        }
        for (size_t j = 0; j < block_size; j += config.num_codewords_reg) {
            vector<pair<short, size_t>> temp_cw_idx(config.num_codewords_reg);
            for (size_t k = 0; k < config.num_codewords_reg; ++k)
                temp_cw_idx[k] = make_pair(codewords[j + k], block_idx + j + k);
            sort(temp_cw_idx.begin(), temp_cw_idx.end(),
                 [](pair<size_t, size_t>first, pair<size_t, size_t>second)
                 {
                return first.first < second.first;
            });
            cw_idx.push_back(temp_cw_idx);
        }
    }
    
    //    for (int i = 0 ; i <= num_total_codewords; ++i)
    //        PlotAmpsAroundCW(cw_map_plots[i], codewords_mappings[i], i);
//    PlotCWFrequency(block_context.cw_freq, config.num_total_codewords, log2(config.orig_vector_size));
    
    for (size_t i = 0; i < cw_idx.size(); ++i) {
        for (size_t j = 0; j < cw_idx[i].size(); ++j) {
            if (cw_idx[i][j].second < config.orig_vector_size){
                float p_real = arg(state_vector[cw_idx[i][j].second]);
                p_real = p_real < 0 ? p_real + (2 * PI) : p_real;
                float p_after = arg(block_context.codewords_mappings[cw_idx[i][j].first]);
                p_after = p_after < 0 ? p_after + (2 * PI) : p_after;
                float p_diff = abs(p_after - p_real);
                p_diff = p_diff > PI ? (2 * PI) - p_diff : p_diff;
                error_bins[0][j] += abs(abs(state_vector[cw_idx[i][j].second])
                                        - abs(block_context.codewords_mappings[cw_idx[i][j].first]));
                error_bins[1][j] += p_diff;
                phase_error_per_cw[cw_idx[i][j].first] += p_diff;
            }
        }
    }
    for (size_t i = 0; i < config.num_codewords_reg; ++i) {
        error_bins[0][i] /= cw_idx.size();
        error_bins[1][i] /= cw_idx.size();
    }
    for (size_t i = 0; i < config.num_total_codewords; ++i)
        if (block_context.cw_freq[i] > 0)
            phase_error_per_cw[i] /= block_context.cw_freq[i];
//    PlotErrorBins(error_bins);
//    PlotPhaseError(phase_error_per_cw, config.num_total_codewords);
    
    CommitBlockContext();
    
    return compressed_vector;
}

complex<float>* Cramer::
CramerDecompress(complex<float>* decompressed_vector,
                 const complex<float>* state_vector)
{
    size_t block_size = 256;// config.num_codewords_reg * 8;
    size_t iters = config.orig_vector_size / block_size;
    
    if (decompressed_vector == nullptr) {
        if (posix_memalign((void**)&decompressed_vector, 64, sizeof(complex<float>) * config.orig_vector_size) != 0)
            throw "Unable to allocate space for decompressed vector";
        
        memset(decompressed_vector, 0, sizeof(complex<float>) * config.orig_vector_size);
    }
    
    for (size_t i = 0; i < iters; ++i)
        CramerBlockDecompress(decompressed_vector, state_vector, i * block_size, block_size);
    
    return decompressed_vector;
}

size_t Cramer::
GetCompressedVectorSize() const
{
    return config.compressed_vector_UL_size + config.num_total_codewords;
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

double Cramer:: GetLog2Lambda() const
{
    return log2(block_context.lambda);
}

double Cramer::
GetKForGammaDist() const
{
    return block_context.k;
}


void Cramer::
GetCWForPlotting(vector<pair<float, float>>& codewords) const
{
    for (size_t i = 0; i < config.num_total_codewords; ++i) {
        double s = config.spiral_length_r + (i * config.codewords_spacing);
        double theta = CalcApproxThetaForSpiralLen(s);
        double magnitude = theta * config.A;
        codewords.push_back(make_pair(magnitude * cos(theta) , magnitude * sin(theta)));
    }
}

