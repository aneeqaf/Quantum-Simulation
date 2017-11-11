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
vector<string> circuit_generator::quiddpro_func;
cplx circuit_generator::c = cplx(-0.99989190,0);


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

inline shared_ptr<gate> circuit_generator::create_gate(const cplx& a, const cplx& b,
                                          const cplx& c, const cplx& d)
{
    vector<cplx> row2;
    vector<cplx> row1;
    vector<vector<cplx>> g;
    
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
    return create_gate(cplx(1/sqrt(2), 0),
                       cplx(1/sqrt(2), 0),
                       cplx(1/sqrt(2), 0),
                       cplx(-1/sqrt(2), 0));
}

inline shared_ptr<gate> circuit_generator::create_X()
{
    return create_gate(cplx(0, 0),
                       cplx(1, 0),
                       cplx(1, 0),
                       cplx(0, 0));
}

inline shared_ptr<gate> circuit_generator::create_Y()
{
    return create_gate(cplx(0, 0),
                       cplx(0, -1),
                       cplx(0, 1),
                       cplx(0, 0));
}

inline shared_ptr<gate> circuit_generator::create_Z()
{
    return create_gate(cplx(1, 0),
                       cplx(0, 0),
                       cplx(0, 0),
                       cplx(-1, 0));
}

inline shared_ptr<gate> circuit_generator::create_I()
{
    return create_gate(cplx(1, 0),
                       cplx(0, 0),
                       cplx(0, 0),
                       cplx(1, 0));
}

shared_ptr<gate> circuit_generator::random_gate()
{
    srand(time(NULL));
    vector<shared_ptr<gate>> rotation_gates;
    
   double theta = ((double)rand())/double(RAND_MAX), theta1, theta2;
    rotation_gates.push_back(Z_rotation(theta));
    
    theta2 = (double)rand()/double(RAND_MAX);
    shared_ptr<gate> rand_gate = Z_rotation(theta2);
    rand_gate -> gate_identification.push_back(7);
    rand_gate -> theta.push_back(theta);
    
    theta1 = (double)rand()/double(RAND_MAX);
    int xy = rand() % 2;
    
    if (xy == 0) {
        rand_gate -> gate_identification.push_back(5);
        rand_gate -> theta.push_back(theta1);
        rotation_gates.push_back(X_rotation(theta1));
    }
    else {
        rand_gate -> gate_identification.push_back(6);
        rand_gate -> theta.push_back(theta1);
        rotation_gates.push_back(Y_rotation(theta1));
    }
    
    rand_gate -> gate_identification.push_back(7);
    rand_gate -> theta.push_back(theta2);
    rotation_gates.push_back(rand_gate);
    
    for (int i = (int)(rotation_gates.size()-2); i >= 0; --i) {
        rand_gate -> rows = matrix_mult(rotation_gates[i] -> rows, rand_gate -> rows);
    }
    
    return rand_gate;
}

inline shared_ptr<gate> circuit_generator::X_rotation(double theta)
{
    return create_gate(cplx(cos(theta/2), 0),
                      cplx(0, -sin(theta/2)),
                      cplx(0, -sin(theta/2)),
                      cplx(cos(theta/2), 0));
}

inline shared_ptr<gate> circuit_generator::Y_rotation(double theta)
{
    return create_gate(cplx(cos(theta/2), 0),
                        cplx(-sin(theta/2),0),
                        cplx(sin(theta/2),0),
                        cplx(cos(theta/2), 0));
}

inline shared_ptr<gate> circuit_generator::Z_rotation(double theta)
{
    return create_gate(exp(cplx(0,-theta/2)),
                      cplx(0, 0),
                      cplx(0, 0),
                      exp(cplx(0,theta/2)));
}

inline shared_ptr<gate> circuit_generator::phase_gate(double theta)
{
    return create_gate(cplx(1,0),
                      cplx(0, 0),
                      cplx(0, 0),
                      cplx(0,1));
//exp(cplx(0, theta * M_PI * 2))
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
        vector<cplx> bits;
        
        if (q == 0) {
            bits.push_back(cplx(1,0));
            bits.push_back(cplx(0,0));
        }
        else {
            bits.push_back(cplx(0,0));
            bits.push_back(cplx(1,0));
        }
     
        test_bits.push_back(q);
        
        if (i > 0) {
            q_circuit -> state_vector = tensor_v_product(q_circuit -> state_vector, bits);
        }
        else {
            q_circuit -> state_vector = bits;
        }
    }
    
    q_circuit -> gates_to_qubits.resize(num_gates);
    
    for (int i = 0 ; i < num_gates; ++i) {
        int q = rand() % qubits;
        int g = rand() % 10;
        
        shared_ptr<gate> temp = shared_ptr<gate>(new gate());
        //change this to include any number of controls for many targets, but only one target gate.
        //Introduce a new gate that does that. If there are multiple target gates for the same control,
        //must be on a different line
        if (g == 9) {
            vector<int> num_c(rand() % (qubits-1) + 1, -1); //change 3 to qubits
            vector<int> num_t(1, -1); // change to rand() % qubits - num_c - 1
            q = -1;
            
            temp -> gate_identification.push_back(g);
            temp -> theta.push_back(0);
            temp -> num_controls = (int)num_c.size();
            g = rand() % 9;
            
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
            q_circuit -> gates_to_qubits[i] = num_c;
            q_circuit -> gates_to_qubits[i].insert(q_circuit -> gates_to_qubits[i].end(), num_t.begin(), num_t.end());
        }
        
        if ( g < 5 ) {
            if(g == 4) {
                shared_ptr<gate> rand_temp = gates[g]();
                temp -> gate_identification.insert(temp -> gate_identification.end(),
                                                   rand_temp -> gate_identification.begin(),
                                                   rand_temp -> gate_identification.end());
                temp -> theta.insert(temp -> theta.end(),
                                     rand_temp -> theta.begin(),
                                     rand_temp -> theta.end());
            }
            else {
                temp -> gate_identification.push_back(g);
            }
            temp -> rows = gates[g]() -> rows;
            q_circuit -> gates.push_back(temp);
            if (q != -1) {
                q_circuit -> gates_to_qubits[i].push_back(q);
            }
        }
        else if (g < 9) {
            double theta = (double)rand() / double(RAND_MAX);
            temp -> rows = rot_gates[g-5](theta) -> rows;
            temp -> gate_identification.push_back(g);
            if (q == -1) {
                temp -> theta.push_back(0);
            }
            temp -> theta.push_back(theta);
            q_circuit -> gates.push_back(temp);
            if (q != -1) {
                q_circuit -> gates_to_qubits[i].push_back(q);
            }
        }
    }
}

void circuit_generator::create_quiddpro_script(const string& input_file)
{
    ofstream file;
    file.open(input_file);
    
    file << "state = cb(\"" + to_string((int)real(test_bits[0]))<< "\");\n";;
    
    for (int q = 1; q < test_bits.size(); ++q) {
        file << "state = kron(state, cb(\"" + to_string(test_bits[q]) << "\"));\n";
    }
    
    for (int i = 0; i < q_circuit -> gates.size(); ++i) {
        if(i != 0) {
            file << ";\n";
        }
        int op_count = 0;
        bool control = false;
        for (int gt = 0; gt < q_circuit -> gates[i] -> gate_identification.size(); ++gt) {
            auto g = q_circuit -> gates[i] -> gate_identification[gt];
            if (g != 9 ) {
                file << "op" + to_string(op_count++)  + " = cu_gate(" + quiddpro_func[g] + "(";
            }
            
            if (g < 5 || g == 8) {
                file << "1), \"";
            }
            else if (g < 9) {
                file << to_string(q_circuit -> gates[i] -> theta[gt]) + ", 1), \"";
            }
            
            if (g != 9) {
                int q = 0;
                if (control) {
                    for (; q < q_circuit -> gates[i] -> num_controls; ++q) {
                        file << "c" + to_string(q_circuit -> gates_to_qubits[i][q] + 1);
                    }
                }
                for (; q < q_circuit -> gates_to_qubits[i].size(); ++q) {
                    file << "x" + to_string(q_circuit -> gates_to_qubits[i][q] + 1);
                }
                file << "\", " + to_string(q_circuit -> qubits) << ");\n";
            }
            else {
                control = true;
            }
        }
        
        if(op_count > 1) {
            file << "op = ";
            for (int n = 0; n < op_count; ++n) {
                if (n != 0) {
                    file << "*";
                }
                file << "op" + to_string(n);
            }
            file << ";\n";
            file << "state = op * state";
        }
        else {
            file << "state = op0 * state";
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
        file << "c " << q_circuit -> gates[i] -> num_controls << "\n";
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
    
    cplx amp;
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
        
        vector<cplx> row;
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
            
            cplx c1(re, im);
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
        
        getline(file, input);
        istringstream cis(input);
        cis >> type;
        if (type == 'c') {
            cis >> g -> num_controls;
        }
        q_circuit -> gates.push_back(g);
    }
}

