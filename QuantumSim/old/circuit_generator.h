//
//  circuit_input _generator.h
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 10/18/17.
//  Copyright © 2017 Aneeqa Fatima. All rights reserved.
//

#ifndef circuit_generator_h
#define circuit_generator_h

#include <memory>
#include <iostream>
#include <string>
#include <vector>
#include <complex>
#include <ctime>
#include <algorithm>
#include <random>
#include <map>
#include "matrix.h"
#include "circuit_simulation.h"


using namespace std;

typedef gate (*func_t) (void);
typedef gate (*rot_f) (double);


/*
 This is a ciruit generator which provides functionality for creating most,
 if not all of the common gates.
 
 The CNOT gate is counted as 2 gates and the Toffolli is counted as 3 gates.
 This format is reflected in the way the data structures in the circuit class
 are populated.
 */
class circuit_generator {
public:
    
    static vector<string> quiddpro_func;
    
    static cmplx c;
    /*
     * 0:Hadamard
     * 1:X
     * 2:Y
     * 3:Z
     * 4:random
     */
    static vector<func_t> gates;
    
    /*
     * 0 : X rotation
     * 1 : Y rotation
     * 2 : Z rotation
     * 3 : Phase
     */
    static vector<rot_f> rot_gates;
    unique_ptr<circuit> q_circuit;
    vector<int> classical_bits;
    
    void create_rand_circuit(int qubits, int num_gates);
    
    circuit_generator();
    
};

#endif /* circuit_input__generator_h */
