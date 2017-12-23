//
//  gates.h
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 11/25/17.
//

#ifndef gates_h
#define gates_h

#include <algorithm>
#include <complex>
#include <stdio.h>
#include <vector>
#include <utility>

#include "matrix.h"

using namespace std;

const int NUM_GATES = 10;
const int NUM_BASIS_STATES = 2;

constexpr cmplx H[2][2] = {{{1, 0}, {1, 0}}, {{1, 0} , {-1, 0}}};
constexpr cmplx X[2][2] = {{{0, 0}, {1, 0}}, {{1, 0} , {0, 0}}};
constexpr cmplx Y[2][2] = {{{0, 0}, {0, -1}}, {{0, 1} , {0, 0}}};
constexpr cmplx Z[2][2] = {{{1, 0}, {0, 0}}, {{0, 0} , {-1, 0}}};
constexpr cmplx P[2][2] = {{{1, 0}, {0, 0}}, {{0, 0} , {0, 1}}};
constexpr cmplx I[2][2] = {{{1, 0}, {0, 0}}, {{0, 0} , {1, 0}}};
constexpr cmplx T[2][2] = {{{1, 0}, {0, 0}}, {{0, 0} , {0.707106781, 0.707106781}}};
constexpr cmplx X12[2][2] = {{{1, 1}, {1, -1}}, {{1, -1} , {1, 1}}};
constexpr cmplx Y12[2][2] = {{{1, 1}, {-1, -1}}, {{1, 1} , {1, 1}}};

struct Gate {
    enum Type : int { Hadamard, X, Y, Z, Random, X_rotation, Y_rotation,
        Z_rotation, Phase, Control, Identity, T, Measurement, X_1_2, Y_1_2};
    
    vector<vector<cmplx>> rows;
    vector<int> qubits;
    vector<int> ids;
    vector<float> theta;
    short num_controls;
    
    Gate(vector<vector<cmplx>> g);
    Gate();
    Gate(const Gate& rhs);
    Gate& operator=(const Gate& rhs);
};

inline Gate create_gate(const cmplx& a, const cmplx& b,
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
    
    return Gate(g);
}

inline Gate create_hadamard()
{
    Gate g =  create_gate(cmplx(1, 0),
                          cmplx(1, 0),
                          cmplx(1, 0),
                          cmplx(-1, 0));
    g.ids.push_back(Gate::Type::Hadamard);
    return g;
}

inline Gate create_X()
{
    Gate g =  create_gate(cmplx(0, 0),
                          cmplx(1, 0),
                          cmplx(1, 0),
                          cmplx(0, 0));
    g.ids.push_back(Gate::Type::X);
    return g;
}

inline Gate create_Y()
{
    Gate g =  create_gate(cmplx(0, 0),
                          cmplx(0, -1),
                          cmplx(0, 1),
                          cmplx(0, 0));
    g.ids.push_back(Gate::Type::Y);
    return g;
}

inline Gate create_X_1_2()
{
    Gate g =  create_gate(cmplx(1, 1),
                          cmplx(1, -1),
                          cmplx(1, -1),
                          cmplx(1, 1));
    g.ids.push_back(Gate::Type::X_1_2);
    return g;
}

inline Gate create_Y_1_2()
{
    Gate g =  create_gate(cmplx(1, 1),
                          cmplx(-1, -1),
                          cmplx(1, 1),
                          cmplx(1, 1));
    g.ids.push_back(Gate::Type::Y_1_2);
    return g;
}

inline Gate create_Z()
{
    Gate g = create_gate(cmplx(1, 0),
                         cmplx(0, 0),
                         cmplx(0, 0),
                         cmplx(-1, 0));
    g.ids.push_back(Gate::Type::Z);
    return g;
}

inline Gate create_I()
{
    Gate g = create_gate(cmplx(1, 0),
                         cmplx(0, 0),
                         cmplx(0, 0),
                         cmplx(1, 0));
    g.ids.push_back(Gate::Type::Identity);
    return g;
}


inline Gate create_X_rotation(double theta)
{
    Gate g = create_gate(cmplx(cos(theta*M_PI/2), 0),
                         cmplx(0, -sin(theta*M_PI/2)),
                         cmplx(0, -sin(theta*M_PI/2)),
                         cmplx(cos(theta*M_PI/2), 0));
    g.theta.push_back(theta);
    g.ids.push_back(Gate::Type::X_rotation);
    return g;
}

inline Gate create_Y_rotation(double theta)
{
    Gate g = create_gate(cmplx(cos(theta*M_PI/2), 0),
                         cmplx(-sin(theta*M_PI/2),0),
                         cmplx(sin(theta*M_PI/2),0),
                         cmplx(cos(theta*M_PI/2), 0));
    g.theta.push_back(theta);
    g.ids.push_back(Gate::Type::Y_rotation);
    return g;
}

inline Gate create_Z_rotation(double theta)
{
    Gate g = create_gate(exp(cmplx(0,-theta*M_PI/2)),
                         cmplx(0, 0),
                         cmplx(0, 0),
                         exp(cmplx(0,theta*M_PI/2)));
    g.theta.push_back(theta);
    g.ids.push_back(Gate::Type::Z_rotation);
    return g;
}

inline Gate create_phase_gate(double theta)
{
    Gate g = create_gate(cmplx(1,0),
                         cmplx(0, 0),
                         cmplx(0, 0),
                         cmplx(0,1));
    //exp(cmplx(0, theta * M_PI * 2))
    //    g -> theta.push_back(theta);
    g.ids.push_back(Gate::Type::Phase);
    return g;
}

inline Gate create_T()
{
    Gate g = create_gate(cmplx(1,0),
                         cmplx(0, 0),
                         cmplx(0, 0),
                         exp(cmplx(0,M_PI/4)));
    g.ids.push_back(Gate::Type::T);
    return g;
}

inline Gate random_gate()
{
    srand(time(NULL));
    vector<Gate> rotation_gates;
    
    double theta = ((double)rand())/double(RAND_MAX), theta1, theta2;
    rotation_gates.push_back(create_Z_rotation(theta));
    
    theta2 = (double)rand()/double(RAND_MAX);
    Gate rand_gate = create_Z_rotation(theta2);
    rand_gate.ids.push_back(Gate::Type::Z_rotation);
    rand_gate.theta.push_back(theta);
    
    theta1 = (double)rand()/double(RAND_MAX);
    int xy = rand() % NUM_BASIS_STATES;
    
    if (xy == 0) {
        rand_gate.ids.push_back(Gate::Type::X_rotation);
        rand_gate.theta.push_back(theta1);
        rotation_gates.push_back(create_X_rotation(theta1));
    }
    else {
        rand_gate.ids.push_back(Gate::Type::Y_rotation);
        rand_gate.theta.push_back(theta1);
        rotation_gates.push_back(create_Y_rotation(theta1));
    }
    
    rand_gate.ids.push_back(Gate::Type::Z_rotation);
    rand_gate.theta.push_back(theta2);
    rotation_gates.push_back(move(rand_gate));
    
    for (int i = (int)(rotation_gates.size()-2); i >= 0; --i) {
        rand_gate.rows = matrix_mult(rotation_gates[i].rows, rand_gate.rows);
    }
    
    return rand_gate;
}

#endif /* gates_h */
