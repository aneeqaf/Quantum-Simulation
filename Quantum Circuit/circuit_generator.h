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

using cmplx = std::complex<float>;
typedef shared_ptr<gate> (*func_t) (void);
typedef shared_ptr<gate> (*rot_f) (double);
const int NUM_GATES = 10;
const int NUM_BASIS_STATES = 2;

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
    shared_ptr<circuit> q_circuit;
    vector<int> classical_bits;
    
    static inline shared_ptr<gate> create_hadamard();
    static inline shared_ptr<gate> create_X();
    static inline shared_ptr<gate> create_Y();
    static inline shared_ptr<gate> create_Z();
    static inline shared_ptr<gate> create_I();
    static shared_ptr<gate> random_gate();
    static inline shared_ptr<gate> create_T();
    
    static inline shared_ptr<gate> control_target(vector<int>& control_bits, vector<int>& targets,
                                                  vector<shared_ptr<gate>>& target_gates);
    
    static inline shared_ptr<gate> create_phase_gate(double theta);
    static inline shared_ptr<gate> create_X_rotation(double theta);
    static inline shared_ptr<gate> create_Y_rotation(double theta);
    static inline shared_ptr<gate> create_Z_rotation(double theta);
    static inline shared_ptr<gate> create_gate(const cmplx& a, const cmplx& b,
                                        const cmplx& c, const cmplx& d);
    
    void create_rand_circuit(int qubits, int num_gates);
    void create_google_rand_circuit(int qubits, int num_gates);
    void write_circuit_to_file(const string& input_file);
    void create_quiddpro_script(const string& input_file);
    void read_input_file(const string& input_file);
    
    circuit_generator();
    
    ~circuit_generator(){}
};

#endif /* circuit_input__generator_h */
