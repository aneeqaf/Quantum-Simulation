//
//  circuit_generator_impl.cpp
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 10/19/17.
//  Copyright © 2017 Aneeqa Fatima. All rights reserved.
//

#include <stdio.h>
#include <cmath>
#include <math.h>
#include <fstream>
#include "circuit_generator.h"
#include "circuit_gates.h"

using namespace std;

vector<func_t> circuit_generator::gates{};
vector<rot_f> circuit_generator::rot_gates{};
vector<string> circuit_generator::quiddpro_func;
cmplx circuit_generator::c = cmplx(-0.99989190,0);


circuit_generator::circuit_generator(): q_circuit(new circuit()){
    
    gates.push_back(create_hadamard);
    gates.push_back(create_X);
    gates.push_back(create_Y);
    gates.push_back(create_Z);
    gates.push_back(random_gate);
    
    rot_gates.push_back(create_X_rotation);
    rot_gates.push_back(create_Y_rotation);
    rot_gates.push_back(create_Z_rotation);
    rot_gates.push_back(create_phase_gate);
//     gates.push_back(create_T);
    
    quiddpro_func.push_back("hadamard");
    quiddpro_func.push_back("sigma_x");
    quiddpro_func.push_back("sigma_y");
    quiddpro_func.push_back("sigma_z");
    quiddpro_func.push_back("");
    quiddpro_func.push_back("rx");
    quiddpro_func.push_back("ry");
    quiddpro_func.push_back("rz");
    quiddpro_func.push_back("phase");
    
}

//TO DO:qubits to gate vector
void circuit_generator::create_rand_circuit(int qubits, int num_gates)
{
    srand(time(NULL));
    if(qubits <= 0 || num_gates <= 0) {
        throw "Please request a valid circuit";
    }

    
//    q_circuit -> qubits = qubits;
//    for (int i = 0; i < qubits; ++i) {
//        int q = rand() % 2;
//        valarray<cmplx> bits(2);
//
//        if (q == 0) {
//            bits[0] = cmplx(1,0);
//            bits[1] = cmplx(0,0);
//        }
//        else {
//            bits[0] = cmplx(0,0);
//            bits[1] = cmplx(1,0);
//        }
//
//        classical_bits.push_back(q);
//
//        if (i > 0) {
////            q_circuit -> circuit_state -> amp = tensor_v_product(q_circuit -> circuit_state -> amp, bits);
//        }
//        else {
//            q_circuit -> circuit_state -> amp = bits;
//        }
//    }
    
    for (int i = 0 ; i < num_gates; ++i) {
        int q = rand() % qubits;
        gate::Gates g = static_cast<gate::Gates>(rand() % NUM_GATES);
        
        gate temp = gate();
        
        if (g == gate::Gates::Control) {
            vector<int> num_c(rand() % (qubits-1) + 1, -1); //change 3 to qubits
            vector<int> num_t(1, -1); // change to rand() % qubits - num_c - 1
            q = -1;
            
            temp.gate_identification.insert(temp.gate_identification.begin(), g);
            temp.theta.insert(temp.theta.begin(), 0);
            temp.num_controls = (int)num_c.size();
            g = static_cast<gate::Gates>(rand() % gate::Gates::Control);
            
            auto check_reoccurance = [&num_c, &num_t](int i) {
                for(auto n : num_c) {
                    if(n == i) {
                        return false;
                    }
                }
                for(auto n : num_t) {
                    if(n == i) {
                        return false;
                    }
                }
                return true;
            };
            
            auto rand_bit_gen = [&check_reoccurance, &qubits](vector<int>& to_gen) {
                int rbg = rand() % qubits;
                for(auto& gen : to_gen) {
                    while(!check_reoccurance(rbg)) {
                        rbg = rand() % qubits;
                    }
                    gen = rbg;
                }
            };
            
            rand_bit_gen(num_c);
            rand_bit_gen(num_t);
            
            sort(num_c.begin(), num_c.end());
            sort(num_t.begin(), num_t.end());
            temp.qubits = num_c;
            temp.qubits.insert(temp.qubits.end(),
                                num_t.begin(), num_t.end());
        }
        
        if ( g <= gate::Gates::Random) {
            gate n_temp = gates[g]();
            if (q == -1) {
                temp.rows = n_temp.rows;
                temp.gate_identification.push_back(n_temp.gate_identification[0]);
            }
            else {
                temp = move(n_temp);
                temp.qubits.push_back(q);
            }
            q_circuit -> gates.push_back(move(temp));
        }
        else if (g < gate::Gates::Control) {
            double theta = (double)rand() / double(RAND_MAX);
            gate n_temp = rot_gates[g - (gate::Gates::X_rotation)](theta);
            if (q == -1) {
                temp.rows = n_temp.rows;
                temp.gate_identification.push_back(n_temp.gate_identification[0]);
                temp.theta.push_back(n_temp.theta[0]);
            }
            else {
                temp = move(n_temp);
                temp.qubits.push_back(q);
            }
            q_circuit -> gates.push_back(move(temp));
        }
    }
}


