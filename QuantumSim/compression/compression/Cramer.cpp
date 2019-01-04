//
//  Cramer.cpp
//  compression
//
//  Created by Aneeqa Fatima on 1/2/19.
//  Copyright © 2019 Aneeqa Fatima. All rights reserved.
//

#include "Cramer.h"

Cramer::
Cramer(idx_size vector_size,
       idx_size num_cw,
       double probabilty_rejection,
       double dist_bw_turns): k_largest_vals({}), avg_near_zero_val(0), orig_vector_size(vector_size), compressed_vector_size(0),
num_codewords(num_cw), num_zero_amps(0), factor_dist_bw_turns(dist_bw_turns)
{
    auto make_even = [](idx_size c) {
        if (c % 2 != 0)
            c -= 1;
        if (c == 0)
            c += 2;
        return c;
    };

    r = make_even(CalculateCInMagnitudeUniformSpiral(probabilty_rejection)) + INNER_R_SHIFT;
    R = make_even(CalculateCInMagnitudeUniformSpiral(CDF_MAX_P));
    spiral_length_r = CalculateExactSpiralLength(r * PI);
    
    double total_spiral_length = CalculateExactSpiralLength(R * PI) - spiral_length_r;
    codewords_spacing = total_spiral_length/num_codewords;
}

Cramer::
Cramer(const Cramer& rhs)
{
    
}

cmplxd Cramer::
UniformTransformMagnitudeAndAmp(cmplxd amp) const
{
    double original_phase = atan2(amp.imag(), amp.real());
    original_phase = original_phase < 0 ? original_phase + ( 2.0 * PI) : original_phase;
    double PT_probability = norm(amp);
    double uniform_probability = 1.0 - exp(-PT_probability * (double)orig_vector_size);
    
    double uniform_mag = sqrt(uniform_probability);
    amp = cmplxd(uniform_mag * cos(original_phase), uniform_mag * sin(original_phase));
    
    return amp;
}

cmplxd Cramer::
PTTransformMagnitudeAndAmp(cmplxd amp) const
{
    double original_phase = atan2(amp.imag(), amp.real());
    original_phase = original_phase < 0 ? original_phase + ( 2.0 * PI) : original_phase;
    double uniform_probability = norm(amp);
    double PT_probability = -log(abs(1.0 - uniform_probability)) / orig_vector_size;
    
    double PT_mag = sqrt(PT_probability);
    amp = cmplxd(PT_mag * cos(original_phase), PT_mag * sin(original_phase));
    
    return amp;
}

inline double Cramer::
CalculateCInMagnitudeUniformSpiral(double magnitude) const
{
    return magnitude/(factor_dist_bw_turns * PI);
}

inline double Cramer::
CalculateThetaGivenMagnitude(double magnitude) const
{
    return magnitude/factor_dist_bw_turns;
}

inline double Cramer::
CalculateMagnitudeGivenC(double c) const
{
    return factor_dist_bw_turns * c * PI;
}

inline double Cramer::
CalculatCWGivenMagnitude(double magnitude) const
{
    double theta = magnitude/factor_dist_bw_turns;
    
    return (CalculateApproxSpiralLength(theta) - spiral_length_r)/codewords_spacing;
}

inline double Cramer::
CalculatCWGivenTheta(double theta) const
{
    return (CalculateApproxSpiralLength(theta) - spiral_length_r)/codewords_spacing;
}

inline double Cramer::
CalculateMagnitudeGivenCW(unsigned short codeword) const
{
    double spiral_length = codeword * codewords_spacing;
    
    return CalculateApproxThetaGivenSpiralLength(spiral_length + spiral_length_r) * factor_dist_bw_turns;
}

inline double Cramer::
CalculateApproxSpiralLength(double theta) const
{
    return (factor_dist_bw_turns * theta * theta) / 2.0;
}

inline double Cramer::
CalculateExactSpiralLength(double theta) const
{
    return 1.0/2.0 * factor_dist_bw_turns * ((theta * sqrt(1.0 + (theta * theta))) + log(theta + sqrt(1.0 + (theta * theta))));
}

inline double Cramer::
CalculateApproxThetaGivenSpiralLength(double spiral_length) const
{
    return sqrt((2.0 * spiral_length) / factor_dist_bw_turns);
}

inline double Cramer::
CalculateThetaGivenCW(unsigned short codeword) const
{
    double spiral_length = codeword * codewords_spacing;
    
    return CalculateApproxThetaGivenSpiralLength(spiral_length + spiral_length_r);
}

inline unsigned short Cramer::
ShiftCodeWordToCorrectQuadrant(double phase,
                               unsigned short codeword) const
{
    double theta = CalculateThetaGivenCW(codeword);
    
    double theta_mod2 = fmod(theta, 2.0 * PI);
    double diff_phase = phase - theta_mod2;
    theta += diff_phase;
    
    return round(CalculatCWGivenTheta(theta));
}

unsigned short Cramer::
CalculateNearestCodewordToVal(cmplxd val) const
{
    const double magnitude = abs(val);
    double phase = arg(val);
    phase = phase < 0 ? phase + (2 * PI) : phase;
   
    double temp_codeword = CalculatCWGivenMagnitude(magnitude);
    const unsigned short floor_cw = floor(temp_codeword);
    const unsigned short ceil_cw = ceil(temp_codeword);
    
    double floor_magnitude_diff = magnitude - CalculateMagnitudeGivenCW(floor_cw);
    double ceil_magnitude_diff =  CalculateMagnitudeGivenCW(ceil_cw) - magnitude;
    
    unsigned short codeword = floor_magnitude_diff < ceil_magnitude_diff ? floor_cw : ceil_cw;
    
    codeword = ShiftCodeWordToCorrectQuadrant(phase, codeword);
    
    return codeword;
}

unsigned short Cramer::
MapValToCodeword(cmplxd val)
{
    cmplxd amp_UT = UniformTransformMagnitudeAndAmp(val);
    double magnitude_UT = abs(amp_UT);
    
    if (magnitude_UT <= CalculateMagnitudeGivenC(r)) {
        avg_near_zero_val += val;
        ++num_zero_amps;
        return 0;
    }
    else if (magnitude_UT >= CalculateMagnitudeGivenC(R)) {
        k_largest_vals.push_back((cmplx)val);
        return num_codewords + 1;
    }
    else {
        unsigned short codeword = CalculateNearestCodewordToVal(amp_UT);
        
        if (codeword == 0) {
            avg_near_zero_val += val;
            ++num_zero_amps;
            return 0;
        }
        else if (codeword >= num_codewords + 1) {
            k_largest_vals.push_back((cmplx)val);
            return num_codewords + 1;
        }
        
        return codeword;
    }
}

cmplx* Cramer::
CramerCompress(const cmplx* state_vector)
{
    cmplx* compressed_vector = nullptr;
    if (posix_memalign((void**)&compressed_vector, 64, sizeof(unsigned short) * orig_vector_size) != 0)
        throw "Unable to allocate space for compressed vector";
    
    memset(compressed_vector, 0, sizeof(unsigned short) * orig_vector_size);
    
    size_t i = 0;
    
    vector<idx_size> cw_freq(num_codewords, 0);
    
    for (idx_size c_idx = 0; c_idx < orig_vector_size/4; ++c_idx, i+=4) {
        Packed4ShortArray temp = {0};
        temp[0] = MapValToCodeword(state_vector[i]); ++cw_freq[temp[0]];
        temp[1] = MapValToCodeword(state_vector[i + 1]); ++cw_freq[temp[1]];
        temp[2] = MapValToCodeword(state_vector[i + 2]); ++cw_freq[temp[2]];
        temp[3] = MapValToCodeword(state_vector[i + 3]); ++cw_freq[temp[3]];
       
        compressed_vector[c_idx] = *(cmplx*)temp;
    }
    
    PlotCWFrequency(cw_freq);
    
    avg_near_zero_val /= num_zero_amps;

    return compressed_vector;
}

cmplx* Cramer::
CramerDecompress(const cmplx* state_vector)
{
    cmplx* decompressed_vector = nullptr;
    if (posix_memalign((void**)&decompressed_vector, 64, sizeof(cmplx) * orig_vector_size) != 0)
        throw "Unable to allocate space for decompressed vector";
    
    memset(decompressed_vector, 0, sizeof(cmplx) * orig_vector_size);
    
    unsigned short * __restrict compressed_vector = (unsigned short *)state_vector;
    
    idx_size large_amps_idx = 0;
    
    for (idx_size i = 0; i < orig_vector_size; ++i) {
        if (compressed_vector[i] == 0)
            decompressed_vector[i] = avg_near_zero_val;
        else if (compressed_vector[i] != num_codewords + 1) {
            double magnitude = CalculateMagnitudeGivenCW(compressed_vector[i]);
            double theta = CalculateThetaGivenMagnitude(magnitude);
            decompressed_vector[i] = PTTransformMagnitudeAndAmp(cmplxd(magnitude * cos(theta), magnitude * sin(theta)));
        }
        else
            decompressed_vector[i] = k_largest_vals[large_amps_idx++];
    }
    
    return decompressed_vector;
}

void Cramer::
GetCodewordsForPlotting(vector<pair<float, float>>& codewords) const
{
    for (idx_size i = 0; i < num_codewords; ++i) {
        double s = spiral_length_r + (i * codewords_spacing);
        double theta = CalculateApproxThetaGivenSpiralLength(s);
        double magnitude = theta * factor_dist_bw_turns;
        codewords.push_back(make_pair(magnitude * cos(theta) , magnitude * sin(theta)));
    }
}

idx_size Cramer::
GetNumOfLargestVals() const
{
    return k_largest_vals.size();
}

idx_size Cramer::
GetCompressedVectorSize() const
{
    return compressed_vector_size;
}

double Cramer::
GetMinInnerRadius() const
{
    return CalculateMagnitudeGivenC(r);
}

double Cramer::
GetMaxOuterRadius() const
{
    return CalculateMagnitudeGivenC(R);
}

idx_size Cramer::
GetNumOfCodewords() const
{
    return num_codewords;
}

idx_size Cramer::
GetNumValsMappedToZero() const
{
    return num_zero_amps;
}

double Cramer::
GetFactorOfDistBetweenTurns() const
{
    return factor_dist_bw_turns;
}

double Cramer::
GetDistBetweenCodewords() const
{
    return codewords_spacing;
}
