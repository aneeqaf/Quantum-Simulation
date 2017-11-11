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
#include <vector>
#include <complex>
#include <map>
#include <iterator>
#include <utility>
#include "matrix.h"

using namespace std;

using cplx = std::complex<float>;

class gate {
public:
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
    vector<vector<cplx>> rows;
    
    gate(vector<vector<cplx>>& g): num_controls(0), rows(g) {}
    gate(): num_controls(0){}
};

class circuit {
private:
    class circuit_simulation;
public:
    int qubits;
    vector<cplx> state_vector;
    vector<vector<int>> gates_to_qubits;
    vector<shared_ptr<gate>> gates;
    
    void simulate();
    int test(circuit& result);
    void print_state();
    
    circuit();
    circuit(const circuit& rhs);
    circuit& operator=(const circuit& rhs);
};

class circuit::circuit_simulation {
public:
    map<int, cplx> amplitudes;
    vector<cplx> apply_gate;
    circuit* cir;
    int t_bits;
    int c_bits;
    
    void initialize_control(int& index, int i);
    void control_Not_opt();
    void control_Z_opt(int i);
    void control_rand_opt(int i);
    void apply_gate_amp(int i);
    void non_control_sim(vector<bool>& iterated, int i, int n, int num_bits);
    template <typename Iterator> inline Iterator find_target(vector<bool>& iterated, Iterator iter, int count);
    
    circuit_simulation(circuit* c): cir(c), t_bits(0), c_bits(0) {};
    ~circuit_simulation();
};

#endif /* circuit_simulation_h */
