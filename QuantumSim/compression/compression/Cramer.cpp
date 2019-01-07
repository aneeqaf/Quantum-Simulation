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
       size_t num_cw,
       double probabilty_rejection,
       bool projection_v): orig_vector_size(vector_size), num_codewords(num_cw), num_zero_amps(0),
num_bits_codewords(log2(num_cw + 1)), lambda(0), projection_vector(projection_v)
{
    r = CalcCInMagnitudeUniformSpiral(probabilty_rejection) + INNER_R_SHIFT;
    R = CalcCInMagnitudeUniformSpiral(CDF_MAX_P);
    spiral_length_r = CalcExactSpiralLen(r * PI);
    num_codewords_reg = CalcCWThatFitIn256BitsReg();
    compressed_vector_UL_size = CalcNumULInCompressedVector(CalcNum256RegForSizeOfVector());
    
    if (!projection_vector)
        lambda = orig_vector_size;
    
    codewords_mappings = new complex<float>[num_codewords + 1];
    
    double total_spiral_length = CalcExactSpiralLen(R * PI) - spiral_length_r;
    codewords_spacing = total_spiral_length/num_codewords;
}

Cramer::
Cramer(const Cramer& rhs): orig_vector_size(rhs.orig_vector_size), compressed_vector_UL_size(rhs.compressed_vector_UL_size),
r(rhs.r), R(rhs.R), num_bits_codewords(rhs.num_bits_codewords), num_codewords(rhs.num_codewords),
num_codewords_reg(rhs.num_codewords_reg), codewords_spacing(rhs.codewords_spacing),
spiral_length_r(rhs.spiral_length_r), lambda(rhs.lambda), projection_vector(rhs.projection_vector)
{
    num_zero_amps.store(rhs.num_zero_amps);
    
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
UniformTransformMagnitudeAndAmp(complex<double> amp) const
{
    double original_phase = atan2(amp.imag(), amp.real());
    original_phase = original_phase < 0 ? original_phase + ( 2.0 * PI) : original_phase;
    double PT_probability = norm(amp);
    double uniform_probability = 1.0 - exp(-PT_probability * (double)lambda);
    
    double uniform_mag = sqrt(uniform_probability);
    amp = complex<double>(uniform_mag * cos(original_phase), uniform_mag * sin(original_phase));
    
    return amp;
}

complex<double> Cramer::
PTTransformMagnitudeAndAmp(complex<double> amp) const
{
    double original_phase = atan2(amp.imag(), amp.real());
    original_phase = original_phase < 0 ? original_phase + ( 2.0 * PI) : original_phase;
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
    
    return (CalcApproxSpiralLen(theta) - spiral_length_r)/codewords_spacing;
}

inline double Cramer::
CalcCWForTheta(double theta) const
{
    return (CalcApproxSpiralLen(theta) - spiral_length_r)/codewords_spacing;
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

inline double Cramer::
CalcThetaForCW(unsigned short codeword) const
{
    double spiral_length = codeword * codewords_spacing;
    
    return CalcApproxThetaForSpiralLen(spiral_length + spiral_length_r);
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

double Cramer::
CalculateLambdaFromEmpiricalCDF(const complex<float>* state_vector) const
{
    double temp_lambda = 0;
    size_t num_amps = 0;
    for (size_t i = 0; i < orig_vector_size; ++i) {
        double p = norm(state_vector[i]);
        if (p > 1.0/(orig_vector_size * orig_vector_size)) {
            ++num_amps;
            temp_lambda += p;
        }
    }
    
    temp_lambda /= (double)num_amps;
   return 1.0/temp_lambda;
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

unsigned short Cramer::
CalcNearestCWToVal(complex<double> val) const
{
    const double magnitude = abs(val);
    double phase = arg(val);
    phase = phase < 0 ? phase + (2 * PI) : phase;
   
    double temp_codeword = CalcCWForMagnitude(magnitude);
    const unsigned short floor_cw = floor(temp_codeword);
    const unsigned short ceil_cw = ceil(temp_codeword);
    
    double floor_magnitude_diff = magnitude - CalcMagnitudeForCW(floor_cw);
    double ceil_magnitude_diff =  CalcMagnitudeForCW(ceil_cw) - magnitude;
    
    unsigned short codeword = floor_magnitude_diff < ceil_magnitude_diff ? floor_cw : ceil_cw;
    
    codeword = ShiftCWToNearestPhase(phase, codeword);
    
    return codeword;
}

unsigned short Cramer::
MapValToCW(complex<double> val)
{
    complex<double> amp_UT = UniformTransformMagnitudeAndAmp(val);
    double magnitude_UT = abs(amp_UT);
    
    if (magnitude_UT <= CalcMagnitudeForC(r)) {
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
PackCWIn256BitsAVXReg(const unsigned short* codewords) const
{
    const bitset<REG_SIZE> set_UL = ~0ULL;
    bitset<REG_SIZE> cw_pack_256_bits = 0;
    
    for (size_t i = 0; i < num_codewords_reg; ++i) {
        bitset<REG_SIZE> temp = codewords[i] ;
        cw_pack_256_bits |= (temp << (i * num_bits_codewords));
   }
    
    unsigned long long ul_in_256_reg[NUM_UL_IN_REG];
    
    for (size_t i = 0;  i < NUM_UL_IN_REG; ++i)
        ul_in_256_reg[i] = ((cw_pack_256_bits & (set_UL << (i * BITS_UL))) >> (i * BITS_UL)).to_ullong();
    
    return _mm256_load_ps((float*)ul_in_256_reg);
}

void Cramer::
UnpackCWFrom256Bits(const bitset<REG_SIZE>& packed_codewords,
                    unsigned short* unpacked_codewords) const
{
    bitset<REG_SIZE> set_cw_bits = (1ull << num_bits_codewords) - 1;
    
    for (size_t i = 0; i < num_codewords_reg; ++i)
        unpacked_codewords[i] =
        ((packed_codewords & (set_cw_bits << (i * num_bits_codewords))) >> (i * num_bits_codewords)).to_ullong();
}

complex<float>* Cramer::
CramerCompress(const complex<float>* state_vector)
{
    if (projection_vector)
        lambda = CalculateLambdaFromEmpiricalCDF(state_vector);

    complex<float>* compressed_vector = nullptr;
    if (posix_memalign((void**)&compressed_vector, 64, sizeof(unsigned short) * orig_vector_size) != 0)
        throw "Unable to allocate space for compressed vector";
    
    memset(compressed_vector, 0, sizeof(unsigned short) * orig_vector_size);
    
    vector<size_t> cw_freq(num_codewords, 0);
    
    #pragma omp parallel for
    for (size_t i = 0; i < orig_vector_size; i+=4) {
        Packed4ShortArray temp = {0};
        temp[0] = MapValToCW(state_vector[i]); ++cw_freq[temp[0]];
        temp[1] = MapValToCW(state_vector[i + 1]); ++cw_freq[temp[1]];
        temp[2] = MapValToCW(state_vector[i + 2]); ++cw_freq[temp[2]];
        temp[3] = MapValToCW(state_vector[i + 3]); ++cw_freq[temp[3]];
       
        compressed_vector[i/4] = *(complex<float>*)temp;
    }
    
    PlotCWFrequency(cw_freq, log2(orig_vector_size));
    
    return compressed_vector;
}

complex<float>* Cramer::
CramerDecompress(const complex<float>* state_vector)
{
    complex<float>* decompressed_vector = nullptr;
    if (posix_memalign((void**)&decompressed_vector, 64, sizeof(complex<float>) * orig_vector_size) != 0)
        throw "Unable to allocate space for decompressed vector";
    
    memset(decompressed_vector, 0, sizeof(complex<float>) * orig_vector_size);
    
    unsigned short * __restrict compressed_vector = (unsigned short *)state_vector;
    
    #pragma omp parallel for
    for (size_t i = 0; i < orig_vector_size; ++i) {
        if (compressed_vector[i] == 0)
            decompressed_vector[i] = 0;
        else {
            double magnitude = CalcMagnitudeForCW(compressed_vector[i]);
            double theta = CalcThetaForMagnitude(magnitude);
            decompressed_vector[i] = PTTransformMagnitudeAndAmp(complex<double>(magnitude * cos(theta), magnitude * sin(theta)));
        }
    }
    
    return decompressed_vector;
}

complex<float>* Cramer::
CramerCompressAVX(const complex<float>* state_vector)
{
    if (projection_vector)
        lambda = CalculateLambdaFromEmpiricalCDF(state_vector);

    complex<float>* compressed_vector = nullptr;
    if (posix_memalign((void**)&compressed_vector, 64, sizeof(complex<float>) * compressed_vector_UL_size) != 0)
        throw "Unable to allocate space for compressed vector";
    
    memset(compressed_vector, 0, sizeof(complex<float>) * compressed_vector_UL_size);
    
    vector<size_t> cw_freq(num_codewords, 0);
    
    #pragma omp parallel for
    for (size_t i = 0; i < compressed_vector_UL_size ; i += NUM_UL_IN_REG) {
        unsigned short codewords[num_codewords_reg];
        #pragma omp parallel for
        for (size_t j = 0; j < num_codewords_reg; ++j) {
            size_t k = ((i/NUM_UL_IN_REG) * num_codewords_reg) + j;
            if (k >= orig_vector_size)
                codewords[j] = 0;
            else
                codewords[j] = MapValToCW(state_vector[k]);
            ++cw_freq[codewords[j]];
        }
        
        __m256 pack_cw = PackCWIn256BitsAVXReg(codewords);
        _mm256_store_ps((float*)&compressed_vector[i], pack_cw);
    }
    
    PlotCWFrequency(cw_freq, log2(orig_vector_size));
    
    codewords_mappings[0] = 0;
    for (size_t i = 1; i <= num_codewords; ++i) {
        double magnitude = CalcMagnitudeForCW(i);
        double theta = CalcThetaForMagnitude(magnitude);
        codewords_mappings[i] = PTTransformMagnitudeAndAmp(complex<double>(magnitude * cos(theta), magnitude * sin(theta)));
    }
    
    return compressed_vector;
}

complex<float>* Cramer::
CramerDecompressAVX(const complex<float>* state_vector)
{
    complex<float>* decompressed_vector = nullptr;
    if (posix_memalign((void**)&decompressed_vector, 64, sizeof(complex<float>) * orig_vector_size) != 0)
        throw "Unable to allocate space for decompressed vector";
    
    memset(decompressed_vector, 0, sizeof(complex<float>) * orig_vector_size);
    
    bitset<REG_SIZE> * __restrict compressed_vector = (bitset<REG_SIZE> *)state_vector;
    
    #pragma omp parallel for
    for (size_t i = 0; i < compressed_vector_UL_size/4; ++i) {
        unsigned short unpacked_codewords[num_codewords_reg];
        UnpackCWFrom256Bits(compressed_vector[i], unpacked_codewords);
        #pragma omp parallel for
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

void Cramer::
GetCWForPlotting(vector<pair<float, float>>& codewords) const
{
    for (size_t i = 0; i < num_codewords; ++i) {
        double s = spiral_length_r + (i * codewords_spacing);
        double theta = CalcApproxThetaForSpiralLen(s);
        double magnitude = theta * B;
        codewords.push_back(make_pair(magnitude * cos(theta) , magnitude * sin(theta)));
    }
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
