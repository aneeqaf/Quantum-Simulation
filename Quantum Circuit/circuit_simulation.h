//
//  circuit_simulation.hpp
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 10/21/17.
//  Copyright © 2017 Aneeqa Fatima. All rights reserved.
//

#ifndef circuit_simulation_h
#define circuit_simulation_h

#include <stdio.h>
#include <memory>
#include <iostream>
#include <fstream>
#include <vector>
#include <complex>
#include <map>
#include <iterator>
#include <utility>
#include <ctime>
#include <cassert>
#include <deque>
#include <algorithm>
#include <functional>
#include "matrix.h"

using namespace std;

const float HADAMARD_CONST = 1/sqrt(2);
extern bool google;
extern float H_related_gates;

using index_size = size_t;

class gate {
public:
    enum Gates : int { Hadamard, X, Y, Z, Random, X_rotation, Y_rotation,
        Z_rotation, Phase, Control, Identity, T, Measurement};
 
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

class state {
public:
    vector<cmplx> state_vector;
    map<index_size, cmplx> amp_apply_gate;
    vector<cmplx> apply_gate;
    vector<index_size> indices_for_ag;
    
    void operator*(const gate& matrix);
    
    cmplx measure_0(short qubit);
    cmplx measure_1(short qubit);
    void measure(short qubit);
    vector<cmplx> tensor();
    
    state();
    state(const state& rhs);
    state& operator=(const state& rhs);
};

class circuit {
private:
   class circuit_simulation;
public:
    
    vector<gate> gates;
    vector<vector<int>> qubit_to_gates;
    vector<short> clock_cycles;
    state* circuit_state;
    short qubits;
    bool circuit_preprocessed;
    
    void circuit_preprocessing();
    void simulate(string outfile);
    int test(circuit& result);
    void print_state(string outfile);
    void print_state();
    void print_probabilities(string& out_file);
    
    circuit();
    circuit(const circuit& rhs);
    circuit& operator=(const circuit& rhs);
    ~circuit();
};

class circuit::circuit_simulation {
public:
    const circuit& cir;
    int gate_i;
    
    void initialize_control(index_size& index, index_size c_bits);
    inline void control_rand_sim(index_size c_bits);
    inline void rand_sim(vector<bool>& iterated,int g_i, int num_bits, index_size index);
    inline int CX_opt();
    inline int CZ_opt();
    inline int Phase_opt();
    inline int T_opt();
    inline int H_google_opt();
    inline void non_control_sim();
    
    template <typename Iterator> inline Iterator find_control_target(vector<bool>& iterated,
                                                             Iterator iter, int c, index_size t_bits);
    template<typename blockType, typename function>
    void block_gate_opt(vector<blockType>& cbits, function& gate_specific_opt);
    vector<index_size> initialize_gate_set(index_size& index, gate::Gates I);
    
    circuit_simulation(const circuit& c, int gate_i);
};

#endif /* circuit_simulation_h */
