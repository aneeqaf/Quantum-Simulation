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
#include <list>
#include <complex>
#include <algorithm>
#include "matrix.h"

using namespace std;

class gate {
public:
    vector<vector<complex<double>>> rows;
    
    gate(vector<vector<complex<double>>>& g): rows(g) {}
    gate(){}
};

class circuit {
public:
    vector<vector<complex<double>>> qubits;
    vector<complex<double>> state_vector;
    vector<vector<int>> gate_rec;
    vector<vector<shared_ptr<gate>>> gates;
};

typedef shared_ptr<gate> (*func_t) (void);
typedef shared_ptr<gate> (*rot_f) (int);

class circuit_generator {

public:
    static double c;
    
    /*
     * 0 : Hadamard
     * 1 : X
     * 2 : Y
     * 3 : Z
     * 4 : Control
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
    
    static inline void initialize_gates();
    
    void create_circuit(int qubits, int num_gates);
    
    void write_circuit_to_file(const string& input_file);
    circuit* read_input_file(const string& input_file);
    
    circuit_generator(){}
    
    circuit_generator(const circuit_generator& g) = delete;
    circuit_generator& operator=(const circuit_generator& g) = delete;
    
    circuit_generator(circuit_generator&&);
    circuit_generator& operator=(circuit_generator&&);
    
    
    ~circuit_generator(){}
};

#endif /* circuit_input__generator_h */
