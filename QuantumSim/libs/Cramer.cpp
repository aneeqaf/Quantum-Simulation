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
       size_t num_cw,
       size_t num_th,
       double probabilty_rejection,
       bool projection_v): orig_vector_size(vector_size), num_bits_codewords(log2(num_cw + 1)),
num_codewords(num_cw), num_threads(num_th), num_zero_amps(0), lambda(0), k(0),
projection_vector(projection_v), dist_type(exponential)
{
    r = CalcCInMagnitudeUniformSpiral(probabilty_rejection) + INNER_R_SHIFT;
    magnitude_r = CalcMagnitudeForC(r);
    R = CalcCInMagnitudeUniformSpiral(CDF_MAX_P);
    spiral_length_r = CalcExactSpiralLen(r * PI);
    num_codewords_reg = CalcCWThatFitIn256BitsReg();
    compressed_vector_UL_size = CalcNumULInCompressedVector(CalcNum256RegForSizeOfVector());
    
    if (!projection_vector) {
        lambda = orig_vector_size;
        k = 1;
        dist_type = exponential;
    }
    
    codewords_mappings = new complex<float>[num_codewords + 1];
    
    double total_spiral_length = CalcExactSpiralLen(R * PI) - spiral_length_r;
    codewords_spacing = total_spiral_length/num_codewords;
}

Cramer::
Cramer(const Cramer& rhs): orig_vector_size(rhs.orig_vector_size), compressed_vector_UL_size(rhs.compressed_vector_UL_size),
r(rhs.r), R(rhs.R), num_bits_codewords(rhs.num_bits_codewords), num_codewords(rhs.num_codewords),
num_codewords_reg(rhs.num_codewords_reg), num_threads(rhs.num_threads), magnitude_r(rhs.magnitude_r),
num_zero_amps(rhs.num_zero_amps), codewords_spacing(rhs.codewords_spacing),
spiral_length_r(rhs.spiral_length_r), lambda(rhs.lambda), k(rhs.k), projection_vector(rhs.projection_vector),
dist_type(rhs.dist_type)
{
    if (rhs.codewords_mappings) {
        if(codewords_mappings) delete [] codewords_mappings;
        codewords_mappings = new complex<float>[num_codewords + 1];
        for (size_t i = 0; i < num_codewords; ++i)
            codewords_mappings[i] = rhs.codewords_mappings[i];
    }
    else {
        if(codewords_mappings) delete [] codewords_mappings;
        codewords_mappings = nullptr;
    }
}

Cramer::
~Cramer()
{
    if (codewords_mappings) delete [] codewords_mappings;
}

complex<double> Cramer::
CalculateCDFofErlangDist(complex<double> amp) const
{
    double PT_mag = abs(amp);
    double PT_probability = PT_mag * PT_mag;
    double Np = PT_probability * lambda;
    double uniform_probability = 0;
    
    for(size_t n = 0; n < k; ++n)
        uniform_probability += (1/Factorial(n)) * pow(Np, n) * exp(-Np);
    
    double uniform_mag = sqrt(1 - uniform_probability);
    amp = complex<double>(uniform_mag * (amp.real()/PT_mag), uniform_mag * (amp.imag()/PT_mag));
    
    return amp;
}

__m256 Cramer::
CalculateCDFofErlangDistAVX(__m256& real,
                            __m256& imag ) const
{
    __m256 PT_probs = _mm256_sq_norm_cmplx(real, imag);
    __m256 PT_mags = _mm256_sqrt_ps(PT_probs);
    __m256 Np = _mm256_mul_ps(_mm256_set1_ps(lambda), PT_probs);
    __m256 neg_NP = _mm256_sub_ps(_mm256_set1_ps(0), Np);
    
    __m256 uniform_probs = {0};
    
    for(size_t n = 0; n < k; ++n)
        uniform_probs = _mm256_add_ps(uniform_probs,
                                      _mm256_mul_ps(_mm256_mul_ps(_mm256_set1_ps(1/Factorial(n)), _mm256_pow_ps(Np, n)), _mm256_exp_ps(neg_NP)));
    
    __m256 uniform_mags = _mm256_sqrt_ps(_mm256_sub_ps(_mm256_set1_ps(1.0), uniform_probs));
    
    real = _mm256_mul_ps(uniform_mags, _mm256_div_ps(real, PT_mags));
    imag = _mm256_mul_ps(uniform_mags, _mm256_div_ps(imag, PT_mags));
    
    return uniform_mags;
}

complex<double> Cramer::
CalculateCDFofExponential(complex<double> amp) const
{
    double PT_mag = abs(amp);
    double PT_probability = PT_mag * PT_mag;
    double uniform_probability = 1.0 - exp(-PT_probability * lambda);
    
    double uniform_mag = sqrt(uniform_probability);
    amp = complex<double>(uniform_mag * (amp.real()/PT_mag), uniform_mag * (amp.imag()/PT_mag));
    
    return amp;
}

__m256 Cramer::
CalculateCDFofExponentialAVX(__m256& real,
                             __m256& imag) const
{
    __m256 PT_probs = _mm256_sq_norm_cmplx(real, imag);
    __m256 PT_mags = _mm256_sqrt_ps(PT_probs);
    __m256 neg_PT_probs = _mm256_sub_ps(_mm256_set1_ps(0), PT_probs);
    __m256 Np = _mm256_mul_ps(_mm256_set1_ps(lambda), neg_PT_probs);
    __m256 uniform_probs = _mm256_sub_ps(_mm256_set1_ps(1.0), _mm256_exp_ps(Np));
    __m256 uniform_mags = _mm256_sqrt_ps(uniform_probs);
    
    real = _mm256_mul_ps(uniform_mags, _mm256_div_ps(real, PT_mags));
    imag = _mm256_mul_ps(uniform_mags, _mm256_div_ps(imag, PT_mags));
    
    return uniform_mags;
}

complex<double> Cramer::
UniformTransformMagnitudeAndAmp(complex<double> amp) const
{
    switch (dist_type) {
        case exponential:
            return CalculateCDFofExponential(amp);
        case erlang:
            return CalculateCDFofErlangDist(amp);
        default:
            cerr << "Unknown distribution";
            throw;
            break;
    }
}

__m256 Cramer::
UniformTransformMagnitudeAndAmpAVX(__m256& real,
                                   __m256& imag) const
{
    switch (dist_type) {
        case exponential:
            return CalculateCDFofExponentialAVX(real, imag);
            break;
        case erlang:
            return CalculateCDFofErlangDistAVX(real, imag);
        default:
            cerr << "Unknown distribution";
            throw;
            break;
    }
}

complex<double> Cramer::
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
CalcCInMagnitudeUniformSpiral(double magnitude) const
{
    return magnitude/(B * PI);
}

inline double Cramer::
CalcThetaForMagnitude(double magnitude) const
{
    return magnitude/B;
}

inline double Cramer::
CalcMagnitudeForC(double c) const
{
    return B * c * PI;
}

inline double Cramer::
CalcCWForMagnitude(double magnitude) const
{
    double theta = magnitude/B;
    
    return CalcCWForTheta(theta);
}

inline __m256 Cramer::
CalcCWForMagnitudeAVX(__m256 magnitudes) const
{
    __m256 thetas = _mm256_div_ps(magnitudes, _mm256_set1_ps(B));
    
    return CalcCWForThetaAVX(thetas);
}

inline double Cramer::
CalcCWForTheta(double theta) const
{
    return (CalcApproxSpiralLen(theta) - spiral_length_r)/codewords_spacing;
}

inline __m256 Cramer::
CalcCWForThetaAVX(__m256 thetas) const
{
    return _mm256_div_ps(_mm256_sub_ps(CalcApproxSpiralLenAVX(thetas), _mm256_set1_ps(spiral_length_r)), _mm256_set1_ps(codewords_spacing));
}

inline double Cramer::
CalcMagnitudeForCW(unsigned short codeword) const
{
    double spiral_length = codeword * codewords_spacing;
    
    return CalcApproxThetaForSpiralLen(spiral_length + spiral_length_r) * B;
}

inline double Cramer::
CalcApproxSpiralLen(double theta) const
{
    return (B * theta * theta) / 2.0;
}

inline __m256 Cramer::
CalcApproxSpiralLenAVX(__m256 thetas) const
{
    return _mm256_div_ps(_mm256_mul_ps(_mm256_mul_ps(thetas, thetas), _mm256_set1_ps(B)), _mm256_set1_ps(2.0));
}

inline double Cramer::
CalcExactSpiralLen(double theta) const
{
    return 1.0/2.0 * B * ((theta * sqrt(1.0 + (theta * theta))) + log(theta + sqrt(1.0 + (theta * theta))));
}

inline double Cramer::
CalcApproxThetaForSpiralLen(double spiral_length) const
{
    return sqrt((2.0 * spiral_length) / B);
}

inline __m256 Cramer::
CalcApproxThetaForSpiralLenAVX(__m256 spiral_lengths) const
{
    return _mm256_sqrt_ps(_mm256_div_ps(_mm256_add_ps(spiral_lengths, spiral_lengths), _mm256_set1_ps(B)));
}

inline double Cramer::
CalcThetaForCW(unsigned short codeword) const
{
    double spiral_length = codeword * codewords_spacing;
    
    return CalcApproxThetaForSpiralLen(spiral_length + spiral_length_r);
}

inline __m256 Cramer::
CalcThetaForCWAVX(__m256 codewords) const
{
    __m256 spiral_lengths = _mm256_mul_ps(codewords, _mm256_set1_ps(codewords_spacing));
    
    return CalcApproxThetaForSpiralLenAVX(_mm256_add_ps(spiral_lengths, _mm256_set1_ps(spiral_length_r)));
}

inline size_t Cramer::
CalcCWThatFitIn256BitsReg() const
{
    return floor((double)REG_SIZE/(double)num_bits_codewords);
}

inline size_t Cramer::
CalcNumULInCompressedVector(size_t num_256_reg) const
{
    return num_256_reg * NUM_UL_IN_REG;
}

inline size_t Cramer::
CalcNum256RegForSizeOfVector() const
{
    return ceil((double)orig_vector_size/(double)num_codewords_reg);
}

inline double Cramer::
CalculateKFromMeanAndVariance(double mean,
                              double variance) const
{
    return (mean * mean)/variance;
}

inline double Cramer::
CalculateLambdaFromMeanAndVariance(double mean,
                                   double variance) const
{
    return mean/variance;
}

void Cramer::
CalculateKandLambdaFromEmpiricalCDF(const complex<float>* state_vector)
{
    double mean = 0, variance = 0;
    size_t num_amps = 0;
    size_t size = orig_vector_size/(1ull << 12);
    
#pragma omp parallel for reduction(+:num_amps, mean) num_threads(num_threads)
    for (size_t i = 0; i < size; ++i) {
        double p = norm(state_vector[i]);
        if (p >= 1.0/((double)orig_vector_size * (double)orig_vector_size)) {
            ++num_amps;
            mean += p;
        }
    }
    
    mean /= (double)num_amps;
    
#pragma omp parallel for reduction(+:variance) num_threads(num_threads)
    for (size_t i = 0; i < size; ++i) {
        double p = norm(state_vector[i]);
        variance += ((p - mean) * (p - mean));
    }
    
    variance /= (double)(size - 1);
    
    k = floor(CalculateKFromMeanAndVariance(mean, variance));
    lambda = CalculateLambdaFromMeanAndVariance(mean, variance);
    dist_type = k <= 1 ? exponential : erlang;
    cout << "lambda:" << lambda << endl;
    cout << "k:" << k << endl;
}

inline unsigned short Cramer::
ShiftCWToNearestPhase(double phase,
                      unsigned short codeword) const
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
    
    return _mm256_round_ps(CalcCWForThetaAVX(thetas), _MM_FROUND_TO_NEAREST_INT |_MM_FROUND_NO_EXC);
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
    complex<double> amp_UT = UniformTransformMagnitudeAndAmp(val);
    double magnitude_UT = abs(amp_UT);
    
    if (magnitude_UT <= magnitude_r) {
        ++num_zero_amps;
        return 0;
    }
    else {
        unsigned short codeword = CalcNearestCWToVal(amp_UT);
        
        if (codeword == 0) {
            ++num_zero_amps;
            return 0;
        }
        else if (codeword >= num_codewords + 1)
            return num_codewords;
        
        return codeword;
    }
}

__m256 Cramer::
MapValToCWAVX(__m256 real,
              __m256 imag)
{
    __m256 magnitudes_UT = UniformTransformMagnitudeAndAmpAVX(real, imag);
    
    if (_mm256_movemask_ps(_mm256_cmp_ps(magnitudes_UT, _mm256_set1_ps(magnitude_r), _CMP_LE_OQ)) == 255) {
        ++num_zero_amps;
        return _mm256_set1_ps(0);
    }
    else {
        __m256 codewords = CalcNearestCWToValAVX(real, imag);
        for (size_t i = 0; i < 8; ++i) {
            if (codewords[i] == 0) {
                ++num_zero_amps;
                codewords[i] = 0;
            }
            else if (codewords[i] >= num_codewords + 1)
                codewords[i] = num_codewords;
        }
        
        return codewords;
    }
}

__m256 Cramer::
PackCWIn256BitsAVXReg(const unsigned int* codewords) const
{
    size_t iters = ceil((double)num_codewords_reg/(double) NUM_UI_IN_REG);
    __m256i idxs = IDXS_CW_FOR_MASKS_UI[num_bits_codewords];
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
        temp_cw = _mm256_sllv_epi32(temp_cw, BITS_TO_STARTING_OF_UI[num_bits_codewords]);
        temp_cw = _mm256_shift_left(temp_cw, i * num_bits_codewords);
        cw = _mm256_or_si256(cw, temp_cw);
        
        idxs = _mm256_add_epi64(idxs, INCREMENT_1_UI);
    }
    
    return (__m256)cw;
}

void Cramer::
UnpackCWFrom256Bits( bitset<REG_SIZE> packed_codewords,
                    unsigned short* unpacked_codewords) const
{
    const bitset<REG_SIZE> set_cw_bits = (1ull << num_bits_codewords) - 1;
    
    for (size_t i = 0; i < num_codewords_reg; ++i) {
        unpacked_codewords[i] = (packed_codewords & set_cw_bits).to_ulong();
        packed_codewords = packed_codewords >> num_bits_codewords;
    }
}

inline __m256i Cramer::
ExtractCodewordFromAVX256Reg(__m256i& packed_codewords,
                             const __m256i& mask_cw_256) const
{
    __m256i extracted_bits = _mm256_and_si256(packed_codewords, mask_cw_256);
    packed_codewords = _mm256_shift_right(packed_codewords, num_bits_codewords);
    
    return extracted_bits;
}

inline __m256i Cramer::
ExtractCodewordFromAVX256Reg(__m256i& packed_codewords) const
{
    __m256i extracted_cws = _mm256_and_si256(packed_codewords, MASKS_CW_PER_UI_REG[num_bits_codewords]);
    __m256i cws_alighned_ul = _mm256_srlv_epi32(extracted_cws, BITS_TO_STARTING_OF_UI[num_bits_codewords]);
    
    packed_codewords = _mm256_shift_right(packed_codewords, num_bits_codewords);
    
    return cws_alighned_ul;
}

void  Cramer::
UnpackCWFrom256BitsAVX(__m256i packed_codewords,
                       unsigned short* unpacked_codewords) const
{
    size_t iters = ceil((double)num_codewords_reg/(double) NUM_UI_IN_REG);
    __m256i idxs = IDXS_CW_FOR_MASKS_UI[num_bits_codewords];
    
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

complex<float>* Cramer::
CramerCompress(complex<float>* compressed_vector,
               const complex<float>* state_vector)
{
    if (projection_vector)
        CalculateKandLambdaFromEmpiricalCDF(state_vector);
    
    if (compressed_vector == nullptr) {
        if (posix_memalign((void**)&compressed_vector, 64, sizeof(complex<float>) * compressed_vector_UL_size) != 0)
            throw "Unable to allocate space for compressed vector";
        
        memset(compressed_vector, 0, sizeof(complex<float>) * compressed_vector_UL_size);
    }
    
    size_t cw_freq[num_codewords + 1];
    memset(cw_freq, 0, sizeof(size_t) * (num_codewords + 1));
    
    size_t block_size = 8 * num_codewords_reg;
    size_t num_blocks = ceil((double)orig_vector_size/(double)block_size);
    
#pragma omp parallel for reduction(+:num_zero_amps, cw_freq) num_threads(num_threads)
    for (size_t i = 0; i < num_blocks; ++i) {
        unsigned int codewords[block_size];
        memset(codewords, 0, sizeof(unsigned short) * block_size);
        size_t block_idx = i * block_size;
        
        //Find all the codewords
        for (size_t j = 0; j < block_size; j += 8) {
            size_t idx = block_idx + j;
            if (idx >= orig_vector_size)
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
                codewords_mappings[codewords[j + k]] += state_vector[idx + k];
                ++cw_freq[codewords[j + k]];
            }
        }
        //Pack all the codewords
        for (size_t j = 0; j < block_size; j += 23) {
            __m256 pack_cw = PackCWIn256BitsAVXReg(&codewords[j]);
            size_t idx = ((block_idx + j)/num_codewords_reg) * NUM_UL_IN_REG;;
            if (block_idx + j >= orig_vector_size)
                break;
            _mm256_store_ps((float*)&compressed_vector[idx], pack_cw);
        }
    }
    
    //    #pragma omp parallel for num_threads(num_threads)
    for (size_t i = 0; i <= num_codewords; ++i)
        codewords_mappings[i] /= cw_freq[i];
    
    return compressed_vector;
}

complex<float>* Cramer::
CramerDecompress(complex<float>* decompressed_vector,
                 const complex<float>* state_vector)
{
    if (decompressed_vector == nullptr) {
        if (posix_memalign((void**)&decompressed_vector, 64, sizeof(complex<float>) * orig_vector_size) != 0)
            throw "Unable to allocate space for decompressed vector";
        
        memset(decompressed_vector, 0, sizeof(complex<float>) * orig_vector_size);
    }
    
    __m256i* __restrict compressed_vector = (__m256i *)state_vector;
    size_t num_cw = ceil((double)num_codewords_reg / (double)NUM_SHORT_IN_REG) * NUM_SHORT_IN_REG;
    size_t iters = compressed_vector_UL_size/NUM_UL_IN_REG;
    
#pragma omp parallel for num_threads(num_threads)
    for (size_t i = 0; i < iters; ++i) {
        unsigned short unpacked_codewords[num_cw];
        UnpackCWFrom256BitsAVX(compressed_vector[i], unpacked_codewords);
        for (size_t j = 0; j < num_codewords_reg; ++j) {
            size_t k = (i * num_codewords_reg) + j;
            if (k >= orig_vector_size)
                continue;
            else
                decompressed_vector[k] = codewords_mappings[unpacked_codewords[j]];
        }
    }
    
    return decompressed_vector;
}

size_t Cramer::
GetCompressedVectorSize() const
{
    return compressed_vector_UL_size + num_codewords;
}

double Cramer::
GetMinInnerRadius() const
{
    return CalcMagnitudeForC(r);
}

double Cramer::
GetMaxOuterRadius() const
{
    return CalcMagnitudeForC(R);
}

size_t Cramer::
GetNumOfCW() const
{
    return num_codewords;
}

size_t Cramer::
GetNumValsMappedToZero() const
{
    return num_zero_amps;
}

double Cramer::
GetFactorOfDistBetweenTurns() const
{
    return B;
}

double Cramer::
GetDistBetweenCW() const
{
    return codewords_spacing;
}

double Cramer:: GetLog2Lambda() const
{
    return log2(lambda);
}

int Cramer::
GetKForGammaDist() const
{
    return k;
}
