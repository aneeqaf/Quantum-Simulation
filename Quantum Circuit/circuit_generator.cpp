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

using namespace std;

inline void circuit_generator::initialize_gates()
{
    c = 0.9919029010;
    
    gates.push_back(create_hadamard);
    gates.push_back(create_X);
    gates.push_back(create_Y);
    gates.push_back(create_Z);
    gates.push_back(create_C);
    gates.push_back(random_gate);
    
    rot_gates.push_back(X_rotation);
    rot_gates.push_back(Y_rotation);
    rot_gates.push_back(Z_rotation);
    rot_gates.push_back(phase_gate);
}

inline shared_ptr<gate> circuit_generator::create_gate(const complex<double>& a, const complex<double>& b,
                                          const complex<double>& c, const complex<double>& d)
{
    vector<complex<double>> row2;
    vector<complex<double>> row1;
    vector<vector<complex<double>>> g;
    
    row1.push_back(a);
    row1.push_back(b);
    row2.push_back(c);
    row2.push_back(d);
    g.push_back(row1);
    g.push_back(row2);
    
    return shared_ptr<gate>(new gate(g));
}

inline shared_ptr<gate> circuit_generator::create_hadamard()
{
    return create_gate(complex<double>(1/sqrt(2), 0),
                       complex<double>(1/sqrt(2), 0),
                       complex<double>(1/sqrt(2), 0),
                       complex<double>(-1/sqrt(2), 0));
}

inline shared_ptr<gate> circuit_generator::create_X()
{
    return create_gate(complex<double>(0, 0),
                       complex<double>(1, 0),
                       complex<double>(1, 0),
                       complex<double>(0, 0));
}

inline shared_ptr<gate> circuit_generator::create_Y()
{
    return create_gate(complex<double>(0, 0),
                       complex<double>(0, -1),
                       complex<double>(0, 1),
                       complex<double>(0, 0));
}

inline shared_ptr<gate> circuit_generator::create_Z()
{
    return create_gate(complex<double>(1, 0),
                       complex<double>(0, 0),
                       complex<double>(0, 0),
                       complex<double>(-1, 0));
}

inline shared_ptr<gate> circuit_generator::create_I()
{
    return create_gate(complex<double>(1, 0),
                       complex<double>(0, 0),
                       complex<double>(0, 0),
                       complex<double>(1, 0));
}

inline shared_ptr<gate> circuit_generator::create_C()
{
    return create_gate(complex<double>(c, 0),
                       complex<double>(0, 0),
                       complex<double>(0, 0),
                       complex<double>(1, 0));
}

shared_ptr<gate> circuit_generator::random_gate()
{
    vector<vector<complex<double>>> rand_matrix;
    shared_ptr<gate> rand_gate(new gate(rand_matrix));
    shared_ptr<gate> rotation_gate;
    
    for (int i = 0; i < 2; ++i) {
        vector<complex<double>> row;
        for (int j = 0; j < 2; ++j) {
            row.push_back(complex<double>(rand() % 10, rand() % 10));
        }
        rand_matrix.push_back(row);
    }
    
    for (int i = 0; i < 2; ++i) {
        int theta = rand() % 361;
        int xyz = rand() % 3;
        
        if (xyz == 0) {
            rotation_gate = X_rotation(theta);
            rand_gate -> rows = matrix_mult(rand_gate -> rows, rotation_gate -> rows);
        }
        else if (xyz == 1) {
            rotation_gate = Y_rotation(theta);
            rand_gate -> rows = matrix_mult(rand_gate -> rows, rotation_gate -> rows);
        }
        else {
            rotation_gate = Z_rotation(theta);
            rand_gate -> rows = matrix_mult(rand_gate -> rows, rotation_gate -> rows);
        }
    }
    
    return shared_ptr<gate>(new gate(rand_matrix));
}

inline shared_ptr<gate> circuit_generator::X_rotation(int theta)
{
    return create_gate(complex<double>(cos(theta/2), 0),
                      complex<double>(0, -sin(theta/2)),
                      complex<double>(0, -sin(theta/2)),
                      complex<double>(cos(theta/2), 0));
}

inline shared_ptr<gate> circuit_generator::Y_rotation(int theta)
{
    return create_gate(complex<double>(cos(theta/2), 0),
                      complex<double>(sin(theta/2),0),
                      complex<double>(sin(theta/2),0),
                      complex<double>(cos(theta/2), 0));
}

inline shared_ptr<gate> circuit_generator::Z_rotation(int theta)
{
    return create_gate(exp(complex<double>(0,-theta/2)),
                      complex<double>(0, 0),
                      complex<double>(0, 0),
                      exp(complex<double>(0,theta/2)));
}

inline shared_ptr<gate> circuit_generator::phase_gate(int theta)
{
    return create_gate(complex<double>(1,0),
                      complex<double>(0, 0),
                      complex<double>(0, 0),
                      exp(complex<double>(0, theta * M_PI * 2)));

}

void circuit_generator::create_circuit(int qubits, int num_gates)
{
    if(qubits <= 0 || num_gates <= 0) {
        throw "Please request a valid circuit";
    }

    for (int i = 0; i < qubits; ++i) {
        int q = rand() % 2;
        vector<complex<double>> bits;
        
        if (q == 0) {
            bits.push_back(complex<double>(1,0));
            bits.push_back(complex<double>(0,0));
        }
        else {
            bits.push_back(complex<double>(0,0));
            bits.push_back(complex<double>(1,0));
        }
        
        q_circuit -> qubits.push_back(bits);
    }
    
    q_circuit -> gates.resize(qubits, vector<shared_ptr<gate>>(num_gates, nullptr));
    q_circuit -> gate_rec.resize(num_gates);
    
    for (int i = 0 ; i < num_gates; ++i) {
        int q = rand() % qubits;
        int g = rand() % 11;
        
        if ( g < 6) {
            q_circuit -> gates[q][i] = gates[g]();
            q_circuit -> gate_rec[i].push_back(q);
            
            if (g == 5) {
                int old_q = q;
                while ((q = rand() % qubits) != old_q) {
                    q_circuit -> gates[q][i] = gates[1]();
                }
                q_circuit -> gate_rec[i].push_back(q);
            }
        }
        else if ( g == 6)
        {
            q_circuit -> gates[q][i] = gates[5]();
            
            int old_q = q;
            while ((q = rand() % qubits) != old_q) {
                q_circuit -> gates[q][i] = gates[5]();
            }
            
            int old_q1 = q;
            while ((q = rand() % qubits) != old_q && q != old_q1) {
                q_circuit -> gates[q][i] = gates[1]();
            }
            q_circuit -> gate_rec[i].push_back(q);
            q_circuit -> gate_rec[i].push_back(old_q);
            q_circuit -> gate_rec[i].push_back(old_q1);
        }
        else {
            int theta = rand() % 361;
            q_circuit -> gates[q][i] = rot_gates[g-7](theta);
            q_circuit -> gate_rec[i].push_back(q);
        }
    }
}

void circuit_generator::write_circuit_to_file(const string& input_file)
{
    int size = (int) q_circuit -> qubits.size();
    
    if ( size == 0) {
        throw "Create circuit first";
    }
    ofstream file;
    file.open(input_file);
    
    for (int i = 0; i < size; ++i) {
        
        file << real(q_circuit -> qubits[i][0]);
        
        if (imag(q_circuit -> qubits[i][0]) != 0) {
            file << "+" << imag(q_circuit -> qubits[i][0]) << " ";
        }
        file << real(q_circuit -> qubits[i][1]);
        
        if (imag(q_circuit -> qubits[i][1]) != 0) {
            file << "+" << imag(q_circuit -> qubits[i][1]) ;
        }
        file << " $ ";
    }
    
    file << "\n";
    
    size = (int) q_circuit -> gates.size();
    
    for (int i = 0; i < size; ++i) {
        for (const auto q : q_circuit -> gate_rec[i]){
            for (int j = 0; j < 2; ++j) {
                file << real(q_circuit -> gates[q][i] -> rows[j][0]);
                
                if (imag(q_circuit -> gates[q][i] -> rows[j][0]) != 0) {
                    file << "+" << imag(q_circuit -> gates[q][i] -> rows[j][0]);
                }
                file << " " << real(q_circuit -> gates[q][i] -> rows[j][1]);
                
                if (imag(q_circuit -> gates[q][i] -> rows[j][1]) != 0) {
                    file << "+" << imag(q_circuit -> gates[q][i] -> rows[j][1]) ;
                }
                
                file << " & ";
            }
            file << "\n";
        }
        
        file << "q ";
        for (const auto q : q_circuit -> gate_rec[i]) {
            file << q << " ";
        }
        file << "\n";
    }
    file.close();
}

circuit* circuit_generator::read_input_file(const string& input_file)
{
    
}
