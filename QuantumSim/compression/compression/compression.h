//
//  compression.h
//  compression
//
//  Created by Aneeqa Fatima on 12/6/18.
//  Copyright © 2018 Aneeqa Fatima. All rights reserved.
//

#ifndef compression_h
#define compression_h

#include <algorithm>
#include <atomic>
#include <bitset>
#include <complex>
#include <cstring>
#include <cmath>
#include <complex>
#include <cassert>
#include <fstream>
#include <map>
#include <iostream>
#include <immintrin.h>
#include <queue>
#include <stdio.h>
#include <vector>
#include <utility>

#include "Cramer.h"

using namespace std;

using amp_idx_t = pair<size_t, complex<float>>;
using state_vector_t = vector<amp_idx_t>;

static inline double sciToDub(const string& str) {
    
    stringstream ss(str);
    double d = 0;
    ss >> d;
    
    if (ss.fail()) {
        string s = "Unable to format ";
        s += str;
        s += " as a number!";
        throw (s);
    }
    
    return (d);
}

complex<double> DotProduct(const complex<double>* vect_A,
                 const complex<double>* vect_B,
                 size_t state_vector_size);

double CalculateNorm(const complex<double>* amp,
                     size_t state_vector_size);


double CalculateFidelity(const complex<double>* original,
                         const complex<double>* processed,
                         size_t state_vector_size);

/*Returns:
 *   1) k largest amps from the state vector, where k = 1/16 x amp_size.
 *   2) Smallest amp from the k largest amp (this is to help in the calculation of the R).
 */
pair<amp_idx_t*, complex<float>> ExtractFractionsOfAmpsFromState(const complex<float>* state_vector,
                                                    size_t amp_size,
                                                    size_t k_largest);

void CompressDecompressStateVector(complex<float>* state_vector,
                                   size_t state_vector_size,
                                   Cramer& cramer);

#endif /* compression_h */
