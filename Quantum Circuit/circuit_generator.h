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
#include <algorithm>
#include "matrix.h"
#include "circuit_simulation.h"

using namespace std;

typedef shared_ptr<gate> (*func_t) (void);
typedef shared_ptr<gate> (*rot_f) (int);

/*
 This is a ciruit generator which provides functionality for creating most,
 if not all of the common gates.
 
 The CNOT gate is counted as 2 gates and the Toffolli is counted as 3 gates.
 This format is reflected in the way the data structures in the circuit class
 are populated.
 */
class circuit_generator {

public:
    
    /*
     * 0 : Hadamard
     * 1 : X
     * 2 : Y
     * 3 : Z
     * 4 : CNot
     * 5 : random
     * 6 : toffolli
     */
    static vector<func_t> gates;
    
    /*
     * 0 : X rotation
     * 1 : Y rotation
     * 2 : Z rotation
     * 3 : Phase
     */
    static vector<rot_f> rot_gates;
    
    shared_ptr<circuit> q_circuit;
    
    static inline shared_ptr<gate> create_hadamard();
    static inline shared_ptr<gate> create_X();
    static inline shared_ptr<gate> create_Y();
    static inline shared_ptr<gate> create_Z();
    static inline shared_ptr<gate> create_I();
    static inline shared_ptr<gate> create_C();
    static shared_ptr<gate> random_gate();
    
    static inline shared_ptr<gate> phase_gate(int theta);
    static inline shared_ptr<gate> X_rotation(int theta);
    static inline shared_ptr<gate> Y_rotation(int theta);
    static inline shared_ptr<gate> Z_rotation(int theta);
    static inline shared_ptr<gate> create_gate(const complex<double>& a, const complex<double>& b,
                                        const complex<double>& c, const complex<double>& d);
    
    void create_rand_circuit(int qubits, int num_gates);
    void write_circuit_to_file(const string& input_file);
    void read_input_file(const string& input_file);
    void print_state();
    
    circuit_generator();
    
    ~circuit_generator(){}
};

#endif /* circuit_input__generator_h */
