//
//  circuit_gates.hpp
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 11/25/17.
//  Copyright © 2017 Aneeqa Fatima. All rights reserved.
//

#ifndef circuit_gates_h
#define circuit_gates_h

#include <stdio.h>
#include <vector>
#include <complex>
#include <utility>
#include <algorithm>
#include "matrix.h"

using namespace std;

const int NUM_GATES = 10;
const int NUM_BASIS_STATES = 2;

class gate {
public:
    enum Gates : int { Hadamard, X, Y, Z, Random, X_rotation, Y_rotation,
        Z_rotation, Phase, Control, Identity, T, Measurement, D_X12, D_Y12};
    
    vector<vector<cmplx>> rows;
    vector<short> qubits;
    vector<short> gate_identification;
    vector<float> theta;
    short num_controls;
    
    gate(vector<vector<cmplx>> g);
    gate();
    gate(const gate& rhs);
    gate& operator=(const gate& rhs);
};

extern gate create_hadamard();
extern gate create_X();
extern gate create_Y();
extern gate create_Z();
extern gate create_I();
extern gate random_gate();
extern gate create_T();
extern gate control_target(vector<int>& control_bits, vector<int>& targets,
                                  vector<gate>& target_gates);
extern gate create_phase_gate(double theta);
extern gate create_X_rotation(double theta);
extern gate create_Y_rotation(double theta);
extern gate create_Z_rotation(double theta);
extern gate create_gate(const cmplx& a, const cmplx& b,
                                   const cmplx& c, const cmplx& d);
    


#endif /* circuit_gates_h */
