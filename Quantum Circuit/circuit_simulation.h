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
#include <utility>
#include "matrix.h"

using namespace std;

class gate {
public:
    /*
   * 0:Hadamard
   * 1:X
   * 2:Y
   * 3:Z
   * 4:random
   * 5:Control Z
   * 6:CNOT
   * 7:toffoli
   * 8:X_rotation
   * 9:Y_rotation
   * 10:Z_rotation
   * 11:phase
   */
 
    vector<int> gate_identification;
    vector<double> theta;
    
    //Only fill the rows when reading from the input file
    vector<vector<complex<double>>> rows;
    
    gate(int identity): gate_identification(identity) {}
    gate(int identity , vector<vector<complex<double>>>& g):
                gate_identification(identity), rows(g) {}
    gate(vector<vector<complex<double>>>& g): rows(g) {}
    gate(){}
};

class circuit {
public:
    int qubits;
    vector<complex<double>> state_vector;
    vector<vector<int>> gates_to_qubits;
    vector<shared_ptr<gate>> gates;
    
    void simulate();
    int test(circuit& result);
    void print_state();
    
    circuit(const circuit& g) = delete;
    circuit& operator=(const circuit& g);
    
    circuit();
};

#endif /* circuit_simulation_h */
