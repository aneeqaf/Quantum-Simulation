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
complex<double> circuit_generator::c = complex<double>(-0.99989190,0);


circuit_generator::circuit_generator(): q_circuit(new circuit()){
    
    gates.push_back(create_hadamard);
    gates.push_back(create_X);
    gates.push_back(create_Y);
    gates.push_back(create_Z);
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

shared_ptr<gate> circuit_generator::random_gate()
{
    srand(time(NULL));
    vector<shared_ptr<gate>> rotation_gates;
    
   double theta = ((double)rand())/double(RAND_MAX), theta1, theta2;
    rotation_gates.push_back(Z_rotation(theta));
    
    theta2 = (double)rand()/double(RAND_MAX);
    shared_ptr<gate> rand_gate = Z_rotation(theta2);
    rand_gate -> gate_identification.push_back(10);
    rand_gate -> theta.push_back(theta);
    
    for (int i = 0; i < 1; ++i) {
        theta1 = (double)rand()/double(RAND_MAX);
        int xy = rand() % 2;
        
        if (xy == 0) {
            rand_gate -> gate_identification.push_back(8);
            rand_gate -> theta.push_back(theta1);
            rotation_gates.push_back(X_rotation(theta1));
        }
        else {
            rand_gate -> gate_identification.push_back(9);
            rand_gate -> theta.push_back(theta1);
            rotation_gates.push_back(Y_rotation(theta1));
        }
    }
    
    rand_gate -> gate_identification.push_back(10);
    rand_gate -> theta.push_back(theta2);
    rotation_gates.push_back(rand_gate);
    
    for (int i = (int)(rotation_gates.size()-2); i >= 0; --i) {
        rand_gate -> rows = matrix_mult(rotation_gates[i] -> rows, rand_gate -> rows);
    }
    
    return rand_gate;
}

inline shared_ptr<gate> circuit_generator::control_target(vector<int>& control_bits,
                                                          vector<int>& targets,
                                                          vector<shared_ptr<gate>>& target_gates)
{
    //Need to sort because the tensor needs to be in order
    sort(control_bits.begin(), control_bits.end());
    
    //Looks up the target bit j in targets vector.
    auto t_lookup = [&targets](int j){
        for (int i = 0; i < targets.size(); ++i) {
            if(j == targets[i]) {
                return i;
            }
        }
        return -1;
    };
    
    vector<shared_ptr<gate>> gs;
    
    int size = (int)control_bits.size();
    for (int i = 0; i < size; ++i) {
        int k = 0;
        //In order to maintain order of bits.
        if (i > 0 && control_bits[i] != control_bits[i-1] + 1) {
            int val = control_bits[i-1] + 1;
            while(val != control_bits[i]) {
                if((k = t_lookup(val)) != -1) {
                    gs.push_back(target_gates[k]);
                    target_gates.erase(target_gates.begin()+k);
                }
                else {
                    gs.push_back(create_I());
                }
                val++;
            }
            gs.push_back(create_gate(c,complex<double>(0, 0),
                                     complex<double>(0, 0), complex<double>(1, 0)));
        }
        else {
            gs.push_back(create_gate(c,complex<double>(0, 0),
                                     complex<double>(0, 0), complex<double>(1, 0)));
        }
    }
    
    if (target_gates.size() > 0) {
        sort(target_gates.begin(), target_gates.end());
    
        size = (int)target_gates.size();
        for (int i = 0; i < size; ++i) {
            gs.push_back(target_gates[i]);
        }
    }
    
    shared_ptr<gate> c_gate = gs[0];
    for (int i = 1; i < gs.size(); ++i) {
        c_gate -> rows = tensor_c(c_gate -> rows, gs[i] -> rows, c);
    }
    
    for(auto& cg : c_gate -> rows) {
        for (auto& val : cg) {
            if(val == c) {
                val = 1;
            }
        }
    }
    
    return c_gate;
}

inline shared_ptr<gate> circuit_generator::X_rotation(double theta)
{
    return create_gate(complex<double>(cos(theta/2), 0),
                      complex<double>(0, -sin(theta/2)),
                      complex<double>(0, -sin(theta/2)),
                      complex<double>(cos(theta/2), 0));
}

inline shared_ptr<gate> circuit_generator::Y_rotation(double theta)
{
    return create_gate(complex<double>(cos(theta/2), 0),
                      complex<double>(sin(theta/2),0),
                      complex<double>(sin(theta/2),0),
                      complex<double>(cos(theta/2), 0));
}

inline shared_ptr<gate> circuit_generator::Z_rotation(double theta)
{
    return create_gate(exp(complex<double>(0,-theta/2)),
                      complex<double>(0, 0),
                      complex<double>(0, 0),
                      exp(complex<double>(0,theta/2)));
}

inline shared_ptr<gate> circuit_generator::phase_gate(double theta)
{
    return create_gate(complex<double>(1,0),
                      complex<double>(0, 0),
                      complex<double>(0, 0),
                      exp(complex<double>(0, theta * M_PI * 2)));

}

void circuit_generator::create_rand_circuit(int qubits, int num_gates)
{
    srand(time(NULL));
    if(qubits <= 0 || num_gates <= 0) {
        throw "Please request a valid circuit";
    }

    q_circuit -> qubits = qubits;
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
     
        #ifdef DEBUG
            test_bits.push_back(bits);
        #endif
        
        if (i > 0) {
            q_circuit -> state_vector = tensor_v_product(q_circuit -> state_vector, bits);
        }
        else {
            q_circuit -> state_vector = bits;
        }
    }
    
    #ifdef DEBUG
    for ( const auto& p : test_bits)
        cout << p[0] << " " << p[1] << "\n";
    #endif
    
    q_circuit -> gates_to_qubits.resize(num_gates);
    
    for (int i = 0 ; i < num_gates; ++i) {
        int q = rand() % qubits;
        int g = rand() % 12;
        
        if ( g < 5 || qubits == 1) {
            shared_ptr<gate> temp = gates[g]();
            if (g != 4) {
                temp -> gate_identification.push_back(g);
            }
            q_circuit -> gates.push_back(temp);
            q_circuit -> gates_to_qubits[i].push_back(q);
        }
        else if (g >= 5 && g < 8) {
            if ( g == 7 && qubits < 3) {
                g = 5;
            }
            shared_ptr<gate> temp;
            int cq = 0, cq1 = 0;
            if ((q <= qubits - 2 && g != 7) || q == 0) {
                cq = q + 1;
            }
            else {
                cq = q - 1;
            }
           // while ((cq = rand() % qubits) == q) {}
            vector<int> tempC(1, cq);
            
            if(g == 7){
                //while ((cq1 = rand() % qubits) == q && cq1 != cq) {}
                if ( cq < q) {
                    cq1 = q + 1;
                }
                else {
                    cq1 = cq + 1;
                }
                tempC.push_back(cq1);
            }
            
            vector<int> tempQ(1, q);
            vector<shared_ptr<gate>> tempG;
            
            if (g == 5){
                tempG.push_back(create_Z());
            }
            else {
                tempG.push_back(create_X());
            }
            
            temp = control_target(tempC, tempQ, tempG);
            if(g == 5) {
               temp -> gate_identification.push_back(5);
            }
            else if(g == 6) {
                temp -> gate_identification.push_back(6);
            }
            else {
               temp -> gate_identification.push_back(7);
            }
            
            q_circuit -> gates.push_back(temp);
            q_circuit -> gates_to_qubits[i].push_back(cq);
            
            if(g == 7) {
                q_circuit -> gates_to_qubits[i].push_back(cq1);
            }
            
            q_circuit -> gates_to_qubits[i].push_back(q);
        }
        else {
            double theta = (double)rand() / double(RAND_MAX);
            shared_ptr<gate> temp = rot_gates[g-8](theta);
            temp -> gate_identification.push_back(g);
            temp -> theta.push_back(theta);
            q_circuit -> gates.push_back(temp);
            q_circuit -> gates_to_qubits[i].push_back(q);
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
    int size_q = (int) q_circuit -> state_vector.size();
    
    if ( size_q == 0) {
        throw "Create circuit first by entering qubits";
    }
    ofstream file;
    file.open(input_file);
    
    file << q_circuit -> qubits << "\n";
    for (int i = 0; i < size_q; ++i) {
        
        file << q_circuit -> state_vector[i];
//        file << real(q_circuit -> state_vector[i]);
//
//        if (imag(q_circuit -> state_vector[i]) != 0) {
//            file << " + " << imag(q_circuit -> state_vector[i]) << " i";
//        }
        file << " " ;
    }
    
    file << "\n";
    
    int size_g = (int) q_circuit -> gates.size();
    
    if ( size_g == 0) {
        throw "Create circuit first by entering gates";
    }
    
    for (int i = 0; i < size_g; ++i) {
        int size_r = (int)q_circuit -> gates[i] -> rows.size();
        for(int j = 0; j < size_r; ++j) {
            int size_c = (int)q_circuit -> gates[i] -> rows[j].size();
            for (int k = 0; k < size_c; ++k) {
                file << real(q_circuit -> gates[i] -> rows[j][k]);
                
                if (imag(q_circuit -> gates[i] -> rows[j][k]) != 0) {
                    file << " + " << imag(q_circuit -> gates[i] -> rows[j][k]) << " i";
                }
                if (k != size_c-1) {
                    file << " , ";
                }
            }
            file << " & ";
        }
        file << "\n";
        file << "g ";
        for ( auto gate_num : q_circuit -> gates[i] -> gate_identification) {
            file << gate_num << " ";
        }
        file << "\nt ";
        for ( auto theta : q_circuit -> gates[i] -> theta) {
            file << theta << " ";
        }
        
        file << "\n";
        file << "q ";
        for (const auto& q : q_circuit -> gates_to_qubits[i]) {
            file << q << " ";
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
    
    file >> q_circuit -> qubits;
    getline(file, input);
    getline(file, input);
    
    complex<double> amp;
    double theta;
    int q, gt;
    char delim = '_';
    istringstream ss(input);
    while (ss >> amp) {
        q_circuit -> state_vector.push_back(amp);
    }
    
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
        
        char type;
        getline(file, input);
        istringstream gis(input);
        gis >> type;
        if (type == 'g') {
            while(gis >> gt) {
                g -> gate_identification.push_back(gt);
            }
        }
        getline(file, input);
        istringstream tis(input);
        tis >> type;
        if (type == 't') {
            while(gis >> theta) {
                g -> theta.push_back(theta);
            }
        }
        q_circuit -> gates.push_back(g);
        
        getline(file, input);
        istringstream qis(input);
        vector<int> qs;
        qis >> type;
        if (type == 'q') {
            while(qis >> q) {
                qs.push_back(q);
            }
            q_circuit -> gates_to_qubits.push_back(qs);
        }
    }
}

