//
//  XYmicroBM.h
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 12/30/17.
//  Copyright © 2017 Aneeqa Fatima. All rights reserved.
//

#ifndef XYmicroBM_h
#define XYmicroBM_h

#include <array>
#include <complex>
#include <ctime>
#include <stdio.h>

#include "kernels1.h"

using namespace std;

using cmplx = complex<float>;
using idx_size = size_t;

void Apply2MergedGatesBM(cmplx* __restrict amp,
                         const idx_size amp_size,
                         const int total_circuit_qubits,
                         const int total_gates);

void Apply4MergedGatesBM(cmplx* __restrict amp,
                         const idx_size amp_size,
                         const int total_circuit_qubits,
                         const int total_gates);

#endif /* XYmicroBM_h */
