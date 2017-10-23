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

vector<func_t> circuit_generator::gates{};
vector<rot_f> circuit_generator::rot_gates{};


circuit_generator::circuit_generator(): q_circuit(new circuit()){
    
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
    return create_I();
}

shared_ptr<gate> circuit_generator::random_gate()
{
    srand(time_t(NULL));
    vector<vector<complex<double>>> rand_gate;
    shared_ptr<gate> rotation_gate;
    
    for (int i = 0; i < 2; ++i) {
        vector<complex<double>> row;
        for (int j = 0; j < 2; ++j) {
            row.push_back(complex<double>(rand() % 10, rand() % 10));
        }
        rand_gate.push_back(row);
    }
    
    for (int i = 0; i < 2; ++i) {
        int theta = rand() % 361;
        int xyz = rand() % 3;
        
        if (xyz == 0) {
            rotation_gate = X_rotation(theta);
            rand_gate = matrix_mult(rand_gate, rotation_gate -> rows);
        }
        else if (xyz == 1) {
            rotation_gate = Y_rotation(theta);
            rand_gate  = matrix_mult(rand_gate , rotation_gate -> rows);
        }
        else {
            rotation_gate = Z_rotation(theta);
            rand_gate = matrix_mult(rand_gate , rotation_gate -> rows);
        }
    }
    
    return shared_ptr<gate>(new gate(rand_gate));
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

void circuit_generator::create_rand_circuit(int qubits, int num_gates)
{
    srand(time_t(NULL));
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
    
    q_circuit -> qubit_gates.resize(qubits);
    
    for (int i = 0 ; i < num_gates; ++i) {
        int q = rand() % qubits;
        int g = rand() % 11;
        
        if ( g < 6) {
            q_circuit -> gates.push_back(gates[g]());
            q_circuit -> qubit_gates[q].push_back(i);
            
            if (g == 4) {
                int old_q = q;
                while ((q = rand() % qubits) != old_q) {}
                q_circuit -> gates.push_back(gates[1]());
                q_circuit -> qubit_gates[q].push_back(++i);
                num_gates++;
            }
        }
        else if ( g == 6 && qubits > 2) {
            q_circuit -> gates.push_back(gates[4]());
            q_circuit -> qubit_gates[q].push_back(i);
            
            int old_q = q;
            while ((q = rand() % qubits) != old_q) {}
            q_circuit -> gates.push_back(gates[4]());
            q_circuit -> qubit_gates[old_q].push_back(++i);
            num_gates++;
            
            int old_q1 = q;
            while ((q = rand() % qubits) != old_q && q != old_q1) {}
            q_circuit -> gates.push_back(gates[1]());
            q_circuit -> qubit_gates[old_q1].push_back(++i);
            num_gates++;
        }
        else if (g == 6) {
            q_circuit -> gates.push_back(gates[5]());
            q_circuit -> qubit_gates[q].push_back(i);
        }
        else {
            int theta = rand() % 361;
            q_circuit -> gates.push_back(rot_gates[g-7](theta));
            q_circuit -> qubit_gates[q].push_back(i);
        }
    }
}

/*
 $: separates all the different qubits
 &: separates the rows in a gate
 ,: separates columns within a row in a gate
 
 e.g output
 
 0 1 $ 1 0 $
 1 , 1 + 9 i & 0 , 2 &
 1
 
 The above example represents this gate: [1  1 + 9i
                                          0  2     ]
 */
void circuit_generator::write_circuit_to_file(const string& input_file)
{
    int size_q = (int) q_circuit -> qubits.size();
    
    if ( size_q == 0) {
        throw "Create circuit first by entering qubits";
    }
    ofstream file;
    file.open(input_file);
    
    for (int i = 0; i < size_q; ++i) {
        
        file << real(q_circuit -> qubits[i][0]);
        
        file << " " << real(q_circuit -> qubits[i][1]);
        
        file << " $ ";
    }
    
    file << "\n";
    
    int size_g = (int) q_circuit -> gates.size();
    
    if ( size_q == 0) {
        throw "Create circuit first by entering gates";
    }
    
    for (int i = 0; i < size_g; ++i) {
        for (int j = 0; j < 2; ++j) {
            file << real(q_circuit -> gates[i] -> rows[j][0]);
            
            if (imag(q_circuit -> gates[i] -> rows[j][0]) != 0) {
                file << " + " << imag(q_circuit -> gates[i] -> rows[j][0]) << " i";
            }
            file << " , " << real(q_circuit -> gates[i] -> rows[j][1]);
            
            if (imag(q_circuit -> gates[i] -> rows[j][1]) != 0) {
                file << " + " << imag(q_circuit -> gates[i] -> rows[j][1]) <<" i" ;
            }
            file << " & ";
        }
        file << "\n";
        
        for (int k = 0; k < size_q; ++k) {
            for (const auto& g : q_circuit -> qubit_gates[k]) {
                if ( g == i )
                {
                    file << k << " ";
                }
            }
        }
        file << "\n";
    }
    file.close();
}

void circuit_generator::read_input_file(const string& input_file)
{
    ifstream file;
    file.open(input_file);
    string input;
    
    getline(file, input);
    

    int q1, q2;
    char delim = '_';
    istringstream ss(input);
    while (ss >> q1 >> q2) {
        
        ss >> delim;
        
        vector<complex<double>> bits;
        
        bits.push_back(complex<double>(q1,0));
        bits.push_back(complex<double>(q2,0));
        q_circuit -> qubits.push_back(bits);
    }
    
    int gate_num = 0;
    string null;
    q_circuit -> qubit_gates.resize(q_circuit -> qubits.size());
    while(getline(file, input)) {
        istringstream iss(input);
        
        double re, im;
        char sign = '_' , i;
        delim = '_';
        shared_ptr<gate> g (new gate());
        
        vector<complex<double>> row;
        while(iss >> re) {
            
            if(delim == '&' || sign == '&') {
                g -> rows.push_back(row);
                row.clear();
            }
            
            iss >> sign;
            if (sign != '+') {
                im = 0;
            }
            else {
                iss >> im >> i;
                iss >> delim;
            }
            
            complex<double> c1(re, im);
            row.push_back(c1);
        }
        
        if(delim == '&' || sign == '&') {
            g -> rows.push_back(row);
        }
        
        q_circuit -> gates.push_back(g);
        file >> q1;
        q_circuit -> qubit_gates[q1].push_back(gate_num);
        gate_num++;
        getline(file, null);
    }
}

void circuit_generator::print_state() {
    
    for (const auto& state_v : q_circuit -> state_vector) {
        cout << real(state_v) ;
        
        if (imag(state_v) != 0) {
            cout << "+" << imag(state_v) << "i";
        }
        cout << " ";
    }
    cout << "\n";
}

