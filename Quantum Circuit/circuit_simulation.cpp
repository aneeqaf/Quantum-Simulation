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

circuit::circuit_simulation::~circuit_simulation()
{
    cir = nullptr;
}

void circuit::circuit_simulation::initialize_control(int& index, int i)
{
    int count = 0;
    int c_iters = pow(2, cir -> qubits - cir -> gates[i] -> num_controls);
    int transition = pow(2, abs(cir -> gates_to_qubits[i][0] - abs(cir -> qubits - 1)));
    int to_transition = 0;
    while(count < c_iters) {
        if ((index & c_bits) == c_bits) {
            ++count;
            amplitudes[index] = cir -> state_vector[index];
            ++index;
            ++to_transition;
        }
        else if (to_transition < transition) {
            ++to_transition;
            ++index;
        }
        else {
            to_transition = 0;
            index += pow(2, abs(cir -> gates_to_qubits[i][0] - abs(cir -> qubits - 1)));
        }
    }
}

void circuit::circuit_simulation::non_control_sim(vector<bool>& iterated, int i, int n, int num_bits)
{
    amplitudes[n] = cir -> state_vector[n];
    
    int index = 0;
    for (int j = 0 ; j < num_bits; ++j) {
        map<double, cplx> temp;
        for (const auto& a : amplitudes) {
            index = a.first + pow(2, abs(cir -> gates_to_qubits[i][j] - abs(cir -> qubits - 1)));
            temp[index] = cir -> state_vector[index];
            iterated[index] = true;
        }
        amplitudes.insert(temp.begin(), temp.end());
    }
    
    apply_gate_amp(i);
}
void circuit::circuit_simulation::control_Not_opt()
{
    vector<bool> iterated (amplitudes.size(), false);
    
    int count = 0;
    for (auto iter = amplitudes.begin(); iter != amplitudes.end(); ++iter) {
        if (iterated[count] == false) {
            auto iter_t = find_target(iterated ,iter, count);
            if(iter_t == amplitudes.end()) {
                throw "iter_t is null";
            };
            if(!(cir -> state_vector[iter -> first] == cplx(0,0)
               && cir -> state_vector[iter_t -> first] == cplx(0,0))) {
                swap(cir -> state_vector[iter -> first], cir -> state_vector[iter_t -> first]);
            }
        }
        ++count;
    }
}

void circuit::circuit_simulation::control_Z_opt(int i)
{
    for (auto c: amplitudes) {
       if ((c.first & t_bits) == t_bits) {
            cir -> state_vector[c.first] = (cplx)(-1) * c.second;
        }
    }
}

void circuit::circuit_simulation::control_rand_opt(int i)
{
    vector<bool> iterated (amplitudes.size(), false);
    
    int count = 0;
    for (auto iter = amplitudes.begin(); iter != amplitudes.end(); ++iter) {
        if (iterated[count] == false) {
            iterated[count] = true;
            auto iter_t = find_target(iterated, iter, count);
            if(iter_t == amplitudes.end()) {
                throw "iter_t is null";
            };
            apply_gate.push_back(iter -> second);
            apply_gate.push_back(iter_t -> second);
        
            apply_gate = matrix_v_mult(cir -> gates[i] -> rows, apply_gate);
        
            cir -> state_vector[iter -> first] = apply_gate[0];
            cir -> state_vector[iter_t -> first] = apply_gate[1];
        
            apply_gate.clear();
        }
        
        ++count;
    }
}

void circuit::circuit_simulation::apply_gate_amp(int i)
{
    for(const auto& a : amplitudes) {
        apply_gate.push_back(a.second);
    }
    
    //In the case of a diagonal gate only the gate application changes
    //Check if a gate is Identity, if so don't do anything
    apply_gate = matrix_v_mult(cir -> gates[i] -> rows, apply_gate);
    
    int c = 0;
    for(const auto& a : amplitudes) {
        cir -> state_vector[a.first] = apply_gate[c++];
    }
}

template <typename Iterator>
inline Iterator circuit::circuit_simulation::find_target(vector<bool>& iterated, Iterator iter, int c)
{
    auto iter_t = iter;
    ++iter_t;
    int count = c + 1;
    for (; iter_t != amplitudes.end(); ++iter_t) {
        if(iterated[count] == false) {
            if ((iter -> first & t_bits) != (iter_t -> first & t_bits)) {
                iterated[count] = true;
                return iter_t;
            }
        }
        ++count;
    }
    return iter_t;
}

circuit::circuit()
{
    state_vector = {};
    gates_to_qubits = {};
    gates = {};
}

circuit::circuit(const circuit& g)
{
    qubits = g.qubits;
    state_vector = g.state_vector;
    gates_to_qubits = g.gates_to_qubits;
    gates = g.gates;
}

circuit& circuit::operator=(const circuit& g)
{
    circuit temp(g);
    swap(qubits, temp.qubits);
    swap(gates, temp.gates);
    swap(gates_to_qubits, temp.gates_to_qubits);
    swap(state_vector, temp.state_vector);
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
        
        if(gates_to_qubits[i].size() > 1 && gates[i] -> gate_identification[0] != 9) {
            sort(gates_to_qubits[i].begin(), gates_to_qubits[i].end());
        }
        
        for(int n = 0 ; n < num_iters; ++n) {
            if (iterated[n] == false) {
                
                iterated[n] = true;
                
                circuit_simulation* sim = (new circuit_simulation(this));
                
                if(gates[i] -> gate_identification[0] == 9) {
                    
                    int index = n + pow(2, abs(gates_to_qubits[i][0] - abs(qubits - 1)));
                    
                    for (int j = 0 ; j < gates[i] -> num_controls; ++j) {
                        sim -> c_bits += pow(2, abs(gates_to_qubits[i][j] - abs(qubits - 1)));
                    }
                    
                    for (int j = gates[i] -> num_controls ; j < gates_to_qubits[i].size(); ++j) {
                        sim -> t_bits += pow(2, abs(gates_to_qubits[i][j] - abs(qubits - 1)));
                    }
                
                    sim -> initialize_control(index, i);
                   
                    if ( gates[i] -> gate_identification[1] == 1) {
                        sim -> control_Not_opt();
                    }
                    else if ( gates[i] -> gate_identification[1] == 3) {
                        sim -> control_Z_opt(i);
                    }
                    else {
                        sim -> control_rand_opt(i);
                    }
                    n = index + 1;
                }
                else {
                    sim -> non_control_sim(iterated, i, n, num_bits);
                }
                delete sim;
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



