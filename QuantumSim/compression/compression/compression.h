//
//  compression.h
//  compression
//
//  Created by Aneeqa Fatima on 12/6/18.
//  Copyright © 2018 Aneeqa Fatima. All rights reserved.
//

#ifndef compression_h
#define compression_h

#include "Cramer.h"

using namespace std;

using amp_idx_t = pair<size_t, cmplx>;
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

cmplxd DotProduct(const cmplxd* vect_A,
                 const cmplxd* vect_B,
                 idx_size state_vector_size);

double CalculateNorm(const cmplxd* amp,
                     idx_size state_vector_size);


double CalculateFidelity(const cmplxd* original,
                         const cmplxd* processed,
                         idx_size state_vector_size);

cmplx FindAmpWithMaxMagnitude(const cmplx* state_vector,
                              idx_size amp_size);

/*Returns:
 *   1) k largest amps from the state vector, where k = 1/16 x amp_size.
 *   2) Smallest amp from the k largest amp (this is to help in the calculation of the R).
 */
pair<amp_idx_t*, cmplx> ExtractFractionsOfAmpsFromState(const cmplx* state_vector,
                                                    idx_size amp_size,
                                                    idx_size k_largest);

void PlotUniformSpiralAndAmpDensity(const string& filename,
                                    const cmplx* state_vector,
                                    idx_size state_vector_size,
                                    const Cramer& cramer);

void ApplyUniformTransformToStateVector(cmplx* state_vector,
                                        idx_size state_vector_size);

void ApplyPTTransformToStateVector(cmplx* state_vector,
                                   idx_size state_vector_size);

void CompressDecompressStateVector(cmplx* state_vector,
                                   idx_size state_vector_size,
                                   Cramer& cramer);

#endif /* compression_h */
