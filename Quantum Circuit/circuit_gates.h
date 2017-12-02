//
//  circuit_gates.hpp
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 11/25/17.
//  Copyright © 2017 Aneeqa Fatima. All rights reserved.
//

#ifndef circuit_gates_h
#define circuit_gates_h

#include <stdio.h>
#include <vector>
#include <complex>
#include <utility>
#include <algorithm>
#include "matrix.h"

using namespace std;

const int NUM_GATES = 10;
const int NUM_BASIS_STATES = 2;

class gate {
public:
    enum Gates : int { Hadamard, X, Y, Z, Random, X_rotation, Y_rotation,
        Z_rotation, Phase, Control, Identity, T, Measurement, X_1_2, Y_1_2};
    
    vector<vector<cmplx>> rows;
    vector<short> qubits;
    vector<short> gate_identification;
    vector<float> theta;
    short num_controls;
    
    gate(vector<vector<cmplx>> g);
    gate();
    gate(const gate& rhs);
    gate& operator=(const gate& rhs);
};

inline gate create_gate(const cmplx& a, const cmplx& b,
                 const cmplx& c, const cmplx& d)
{
    vector<cmplx> row2;
    vector<cmplx> row1;
    vector<vector<cmplx>> g;
    
    row1.push_back(a);
    row1.push_back(b);
    row2.push_back(c);
    row2.push_back(d);
    g.push_back(row1);
    g.push_back(row2);
    
    return gate(g);
}

inline gate create_hadamard()
{
    gate g =  create_gate(cmplx(1, 0),
                          cmplx(1, 0),
                          cmplx(1, 0),
                          cmplx(-1, 0));
    g.gate_identification.push_back(gate::Gates::Hadamard);
    return g;
}

inline gate create_X()
{
    gate g =  create_gate(cmplx(0, 0),
                          cmplx(1, 0),
                          cmplx(1, 0),
                          cmplx(0, 0));
    g.gate_identification.push_back(gate::Gates::X);
    return g;
}

inline gate create_Y()
{
    gate g =  create_gate(cmplx(0, 0),
                          cmplx(0, -1),
                          cmplx(0, 1),
                          cmplx(0, 0));
    g.gate_identification.push_back(gate::Gates::Y);
    return g;
}

inline gate create_X_1_2()
{
    gate g =  create_gate(cmplx(1, 1),
                          cmplx(1, -1),
                          cmplx(1, -1),
                          cmplx(1, 1));
    g.gate_identification.push_back(gate::Gates::X_1_2);
    return g;
}

inline gate create_Y_1_2()
{
    gate g =  create_gate(cmplx(1, 1),
                          cmplx(-1, -1),
                          cmplx(1, 1),
                          cmplx(1, 1));
    g.gate_identification.push_back(gate::Gates::Y_1_2);
    return g;
}

inline gate create_Z()
{
    gate g = create_gate(cmplx(1, 0),
                         cmplx(0, 0),
                         cmplx(0, 0),
                         cmplx(-1, 0));
    g.gate_identification.push_back(gate::Gates::Z);
    return g;
}

inline gate create_I()
{
    gate g = create_gate(cmplx(1, 0),
                         cmplx(0, 0),
                         cmplx(0, 0),
                         cmplx(1, 0));
    g.gate_identification.push_back(gate::Gates::Identity);
    return g;
}


inline gate create_X_rotation(double theta)
{
    gate g = create_gate(cmplx(cos(theta*M_PI/2), 0),
                         cmplx(0, -sin(theta*M_PI/2)),
                         cmplx(0, -sin(theta*M_PI/2)),
                         cmplx(cos(theta*M_PI/2), 0));
    g.theta.push_back(theta);
    g.gate_identification.push_back(gate::Gates::X_rotation);
    return g;
}

inline gate create_Y_rotation(double theta)
{
    gate g = create_gate(cmplx(cos(theta*M_PI/2), 0),
                         cmplx(-sin(theta*M_PI/2),0),
                         cmplx(sin(theta*M_PI/2),0),
                         cmplx(cos(theta*M_PI/2), 0));
    g.theta.push_back(theta);
    g.gate_identification.push_back(gate::Gates::Y_rotation);
    return g;
}

inline gate create_Z_rotation(double theta)
{
    gate g = create_gate(exp(cmplx(0,-theta*M_PI/2)),
                         cmplx(0, 0),
                         cmplx(0, 0),
                         exp(cmplx(0,theta*M_PI/2)));
    g.theta.push_back(theta);
    g.gate_identification.push_back(gate::Gates::Z_rotation);
    return g;
}

inline gate create_phase_gate(double theta)
{
    gate g = create_gate(cmplx(1,0),
                         cmplx(0, 0),
                         cmplx(0, 0),
                         cmplx(0,1));
    //exp(cmplx(0, theta * M_PI * 2))
    //    g -> theta.push_back(theta);
    g.gate_identification.push_back(gate::Gates::Phase);
    return g;
}

inline gate create_T()
{
    gate g = create_gate(cmplx(1,0),
                         cmplx(0, 0),
                         cmplx(0, 0),
                         exp(cmplx(0,M_PI/4)));
    g.gate_identification.push_back(gate::Gates::T);
    return g;
}

inline gate random_gate()
{
    srand(time(NULL));
    vector<gate> rotation_gates;
    
    double theta = ((double)rand())/double(RAND_MAX), theta1, theta2;
    rotation_gates.push_back(create_Z_rotation(theta));
    
    theta2 = (double)rand()/double(RAND_MAX);
    gate rand_gate = create_Z_rotation(theta2);
    rand_gate.gate_identification.push_back(gate::Gates::Z_rotation);
    rand_gate.theta.push_back(theta);
    
    theta1 = (double)rand()/double(RAND_MAX);
    int xy = rand() % NUM_BASIS_STATES;
    
    if (xy == 0) {
        rand_gate.gate_identification.push_back(gate::Gates::X_rotation);
        rand_gate.theta.push_back(theta1);
        rotation_gates.push_back(create_X_rotation(theta1));
    }
    else {
        rand_gate.gate_identification.push_back(gate::Gates::Y_rotation);
        rand_gate.theta.push_back(theta1);
        rotation_gates.push_back(create_Y_rotation(theta1));
    }
    
    rand_gate.gate_identification.push_back(gate::Gates::Z_rotation);
    rand_gate.theta.push_back(theta2);
    rotation_gates.push_back(move(rand_gate));
    
    for (int i = (int)(rotation_gates.size()-2); i >= 0; --i) {
        rand_gate.rows = matrix_mult(rotation_gates[i].rows, rand_gate.rows);
    }
    
    return rand_gate;
}

#endif /* circuit_gates_h */
