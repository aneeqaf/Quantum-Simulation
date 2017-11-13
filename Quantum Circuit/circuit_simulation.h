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
#include "matrix.h"

using namespace std;

using cmplx = std::complex<float>;

class gate {
public:
    enum Gates : int { Hadamard, X, Y, Z, Random, X_rotation, Y_rotation,
        Z_rotation, Phase, Control, Identity, T, Measurement};
    /*
   * 0:Hadamard
   * 1:X
   * 2:Y
   * 3:Z
   * 4:random
   * 5:X_rotation
   * 6:Y_rotation
   * 7:Z_rotation
   * 8:phase
   * 9:control
   * 10:identity
   */
 
    vector<int> gate_identification;
    vector<double> theta;
    int num_controls;
    
    //Only fill the rows when reading from the input file
    vector<vector<cmplx>> rows;
    
    gate(vector<vector<cmplx>>& g): num_controls(0), rows(g) {}
    gate(): num_controls(0){}
};

class state {
public:
    vector<cmplx> state_vector;
    map<int, cmplx> amplitudes;
    vector<cmplx> apply_gate;
    
    map<int, cmplx>::iterator* iterator;
    map<int, cmplx>::iterator* iterator1;
    
    void operator*(shared_ptr<gate> matrix);
    
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
    
    vector<vector<short>> gates_to_qubits;
    vector<shared_ptr<gate>> gates;
    vector<int> clock_cycles;
    state* circuit_state;
    int qubits;
    bool google;
    
    void simulate(string& outfile);
    int test(circuit& result);
    void print_state();
    void print_probabilities(string& out_file);
    
    circuit();
    circuit(const circuit& rhs);
    circuit& operator=(const circuit& rhs);
    ~circuit();
};

class circuit::circuit_simulation {
public:
    circuit* cir;
    int t_bits;
    int c_bits;
    
    void initialize_control(int& index, int i);
    void control_Not_opt();
    void control_Z_opt(int i);
    void control_Phase_opt(int i);
    void control_rand_opt(int i);
    void non_control_sim(vector<bool>& iterated, int i, int n, int num_bits);
    template <typename Iterator> inline Iterator find_target(vector<bool>& iterated, Iterator iter, int count);
    
    circuit_simulation(circuit* c): cir(c), t_bits(0), c_bits(0) {};
    ~circuit_simulation();
};

#endif /* circuit_simulation_h */
