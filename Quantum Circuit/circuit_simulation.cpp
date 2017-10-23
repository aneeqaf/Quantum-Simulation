//
//  circuit_simulation.cpp
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 10/21/17.
//  Copyright © 2017 Aneeqa Fatima. All rights reserved.
//

#include <stdio.h>

#include "circuit_simulation.h"

using namespace std;

circuit::circuit()
{
    qubits={};
    state_vector = {};
    qubit_gates = {};
    gates = {};
}

circuit& circuit::operator=(const circuit& g)
{
    this -> gates = g.gates;
    this -> qubit_gates = g.qubit_gates;
    this -> qubits = g.qubits;
    this -> state_vector = g.state_vector;
    return *this;
}
    
void circuit::simulate()
{
    int qubits_size = (int)qubits.size();
    
    for (int i = 0; i < qubits_size; ++i) {
        int q_gates_size = (int) qubit_gates[i].size();
        
        if (q_gates_size > 0) {
            vector<vector<complex<double>>> g =  gates[qubit_gates[i][0]] -> rows;
            for (int j = q_gates_size - 1; j > 0; --j) {
                g = matrix_mult( gates[ qubit_gates[i][j-1]] -> rows, g);
            }
        
            qubits[i] = matrix_v_mult(g,  qubits[i]);
        }
    }
    
     state_vector =  qubits[0];
    
    for (int i = 1; i < qubits_size; ++i) {
         state_vector = tensor_v_product( state_vector, qubits[i]);
    }
}

int circuit::test(circuit& result)
{
    
    return 0;
}


