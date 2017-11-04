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
    state_vector = {};
    gates_to_qubits = {};
    gates = {};
}

circuit& circuit::operator=(const circuit& g)
{
    this -> gates = g.gates;
    this -> gates_to_qubits = g.gates_to_qubits;
    this -> state_vector = g.state_vector;
    return *this;
}

// TO DO: apply optimizations for diagonal gates, control Z, toffolli, and control X. 
void circuit::simulate()
{
    int size = (int)gates_to_qubits.size();
    for (int i = 0; i < size; ++i) {
        int num_bits = (int)gates_to_qubits[i].size();
        if (num_bits == 0) {
            throw "Please enter non-zero number of bits for each gate";
        }
        
        int num_iters = (int)state_vector.size();//pow(2, num_bits);
        vector<bool> iterated(num_iters, false);
        
        if(gates_to_qubits[i].size() > 1) {
            sort(gates_to_qubits[i].begin(), gates_to_qubits[i].end());
        }
        
        for(int n = 0 ; n < num_iters; ++n) {
            map<int, complex<double>> amplitudes;
            vector<complex<double>> apply_gate;
            
            if (iterated[n] == false) {
                iterated[n] = true;
                amplitudes[n] = state_vector[n];
                
                int index = 0;
                for (int j = 0 ; j < num_bits; ++j) {
                    map<double, complex<double>> temp;
                    for (const auto& a : amplitudes) {
                        index = a.first + pow(2, abs(gates_to_qubits[i][j] - abs(qubits - 1)));
                        temp[index] = state_vector[index];
                        iterated[index] = true;
                    }
                    amplitudes.insert(temp.begin(), temp.end());
                }
                
                for(const auto& a : amplitudes) {
                    apply_gate.push_back(a.second);
                }
                
                apply_gate = matrix_v_mult(gates[i] -> rows, apply_gate);
                
                int c = 0;
                for(const auto& a : amplitudes) {
                    state_vector[a.first] = apply_gate[c++];
                }
            }
        }
    }
}

int circuit::test(circuit& result)
{
    for (int i = 0; i < gates.size(); ++i) {
        gates[i] -> rows = matrix_inv(gates[i] -> rows);
    }
    
    //reverse the order of the gates and call simulate;
    //save the initial state vector in test.
    return 0;
}

void circuit::print_state() {
    
    for (const auto& state_v : state_vector) {
        cout << real(state_v) ;
        
        if (imag(state_v) > 0) {
            cout << "+" << imag(state_v) << "i";
        }
        else if (imag(state_v) < 0) {
            cout << imag(state_v) << "i";
        }
        cout << " ";
    }
    cout << "\n";
}


