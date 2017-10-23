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
    vector<vector<int>> qubit_gates;
    vector<shared_ptr<gate>> gates;
    
    void simulate();
    int test(circuit& result);
    
    circuit(const circuit& g) = delete;
    circuit& operator=(const circuit& g);
    
    circuit();
};

#endif /* circuit_simulation_h */
