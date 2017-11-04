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
#include <random>
#include "matrix.h"
#include "circuit_simulation.h"

using namespace std;

typedef shared_ptr<gate> (*func_t) (void);
typedef shared_ptr<gate> (*rot_f) (double);

/*
 This is a ciruit generator which provides functionality for creating most,
 if not all of the common gates.
 
 The CNOT gate is counted as 2 gates and the Toffolli is counted as 3 gates.
 This format is reflected in the way the data structures in the circuit class
 are populated.
 */
class circuit_generator {

public:
    
    static complex<double> c;
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
    vector<vector<complex<double>>> test_bits;
    
    shared_ptr<circuit> q_circuit;
    
    static inline shared_ptr<gate> create_hadamard();
    static inline shared_ptr<gate> create_X();
    static inline shared_ptr<gate> create_Y();
    static inline shared_ptr<gate> create_Z();
    static inline shared_ptr<gate> create_I();
    static shared_ptr<gate> random_gate();
    
    static inline shared_ptr<gate> control_target(vector<int>& control_bits, vector<int>& targets,
                                                  vector<shared_ptr<gate>>& target_gates);
    
    static inline shared_ptr<gate> phase_gate(double theta);
    static inline shared_ptr<gate> X_rotation(double theta);
    static inline shared_ptr<gate> Y_rotation(double theta);
    static inline shared_ptr<gate> Z_rotation(double theta);
    static inline shared_ptr<gate> create_gate(const complex<double>& a, const complex<double>& b,
                                        const complex<double>& c, const complex<double>& d);
    
    void create_rand_circuit(int qubits, int num_gates);
    void write_circuit_to_file(const string& input_file);
    void read_input_file(const string& input_file);
    
    circuit_generator();
    
    ~circuit_generator(){}
};

#endif /* circuit_input__generator_h */
