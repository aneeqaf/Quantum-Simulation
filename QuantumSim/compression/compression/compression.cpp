//
//  compression.cpp
//  compression
//
//  Created by Aneeqa Fatima on 12/6/18.
//  Copyright © 2018 Aneeqa Fatima. All rights reserved.
//

#include "compression.h"

complex<double> DotProduct(const complex<double>* vect_A,
                 const complex<double>* vect_B,
                 size_t state_vector_size)
{
    
    complex<double> product = 0;
    
    // Loop for calculate cot product
    for (int i = 0; i < state_vector_size; i++)
        product +=  conj(vect_A[i]) * vect_B[i];
    return product;
}

double CalculateNorm(const complex<double>* amp,
                     size_t state_vector_size)
{
    double norm_ = 0;
    
    for (size_t i = 0; i < state_vector_size; ++i)
        norm_ += norm(amp[i]);
    
    return sqrt(norm_);
}

double CalculateFidelity(const complex<double>* original,
                         const complex<double>* processed,
                         size_t state_vector_size)
{
    //temp
    state_vector_size /= (1ull << 12);
    double norm_product = (CalculateNorm(original, state_vector_size) * CalculateNorm(processed, state_vector_size));
    complex<double> dotp_exact_approx = DotProduct(original, processed, state_vector_size);
    return pow(norm(complex<double>(dotp_exact_approx.real()/norm_product, dotp_exact_approx.imag()/norm_product)), 2);
}

pair<amp_idx_t*, complex<float>> ExtractFractionsOfAmpsFromState(const complex<float>* state_vector,
                                                    size_t amp_size,
                                                    size_t kth_fraction_of_amps)
{
    const size_t fraction_starting_amps = amp_size/(kth_fraction_of_amps << 4);
    const size_t fraction_amps = amp_size/kth_fraction_of_amps;
    
    auto comp_amp = [](const amp_idx_t first, const amp_idx_t second) {
        return norm(first.second) > norm(second.second);
    };
    
    auto comp_idx = [](const amp_idx_t first, const amp_idx_t second) {
        return first.first < second.first;
    };
    
    vector<amp_idx_t> heap_amps;
    for (size_t i = 0; i < fraction_starting_amps; ++i)
        heap_amps.push_back(make_pair(i, state_vector[i]));
    
    priority_queue<amp_idx_t, state_vector_t, decltype(comp_amp)> pq_k_largest_amps (heap_amps.begin(), heap_amps.end(), comp_amp);
    heap_amps.clear();
    
    for (size_t i = fraction_starting_amps; i < amp_size; ++i) {
        if (norm(state_vector[i]) > norm(pq_k_largest_amps.top().second)) {
            if (pq_k_largest_amps.size() == fraction_amps)
                pq_k_largest_amps.pop();
            
            pq_k_largest_amps.push(make_pair(i, state_vector[i]));
        }
    }
    
    amp_idx_t* k_amps = nullptr;
    
    if (posix_memalign((void**)&k_amps, 64, sizeof(amp_idx_t) * fraction_amps) != 0)
        throw "Unable to allocate";
    memset(k_amps, 0, fraction_amps * sizeof(amp_idx_t));
    complex<float> minimum = pq_k_largest_amps.top().second;
    
    for (size_t x = 0; x < fraction_amps; ++x) {
        k_amps[x] = pq_k_largest_amps.top();
        
//        if (x == fraction_amps/2)
//            minimum = k_amps[x].second;
        
        pq_k_largest_amps.pop();
    }
    sort(k_amps, k_amps + fraction_amps, comp_idx);
//    for (idx_size x = 0; x < fraction_amps; ++x) {
//        k_amps[x] = temp_k_amps[x].second;
////        cout << k_amps[x] << " ";
//    }
////    cout << endl;
    
    float amp_real = minimum.real(), amp_imag = minimum.imag();
    if (amp_real < 0)
        amp_real *= -1;
    if (amp_imag < 0)
        amp_imag *= -1;
    
    return make_pair(k_amps, complex<float>(amp_real, amp_imag));
}



void CompressDecompressStateVector(complex<float>* state_vector,
                                   size_t state_vector_size,
                                   Cramer& cramer)
{
    vector<complex<double>> copy_state_vector(state_vector_size);
    for (size_t i = 0; i < state_vector_size; ++i)
        copy_state_vector[i] = state_vector[i];
    
    
    complex<float>* comp_vector = cramer.CramerCompress(nullptr, state_vector);
    free(state_vector);
    state_vector = nullptr;

    state_vector = cramer.CramerDecompress(nullptr, comp_vector);
    
    vector<complex<double>> new_state_vector(state_vector_size);
    for (size_t i = 0; i < state_vector_size; ++i)
        new_state_vector[i] = state_vector[i];
    
    double fidelity = CalculateFidelity(copy_state_vector.data(), new_state_vector.data(), state_vector_size);

    cout << "Results after compression-decompression of " << log2(state_vector_size) << "q\n";
    cout << "r = " << cramer.GetMinInnerRadius() << ", R = " << cramer.GetMaxOuterRadius() << endl;
    cout << "Calculated Lambda : " << cramer.GetLog2Lambda() << endl;
    cout << "Calculated K : " << cramer.GetKForGammaDist() << endl;
    cout << "Fraction of amps mapped to zero : " << cramer.GetNumValsMappedToZero() << "/" << state_vector_size
    << " (" << (double)cramer.GetNumValsMappedToZero()/(double)state_vector_size << ")" << endl;
    cout << "Number of codewords : " << cramer.GetNumOfCW() << endl;
    cout << "Fidelity of Compression : " << fidelity << endl;
    cout << "Compression ratio : " << (double)state_vector_size /(double)cramer.GetCompressedVectorSize() << endl;
}
