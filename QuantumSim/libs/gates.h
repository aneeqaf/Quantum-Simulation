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
#include <deque>
#include <stdio.h>
#include <utility>
#include <vector>

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
constexpr cmplx CZ_D1[2][2] = {{{1, 0}, {0, 0}}, {{0, 0} , {-1, 0}}};
constexpr cmplx CZ_D2[2][2] = {{{0, 0}, {0, 0}}, {{0, 0} , {1, 0}}};
constexpr cmplx CZ_D3[2][2] = {{{1, 0}, {0, 0}}, {{0, 0} , {1, 0}}};
constexpr cmplx CZ_D4[2][2] = {{{1, 0}, {0, 0}}, {{0, 0} , {0, 0}}};
constexpr cmplx CZ_D5[2][2] = {{{2, 0}, {0, 0}}, {{0, 0} , {0, 0}}};
constexpr cmplx CZ_D6[2][2] = {{{0, -1}, {0, 0}}, {{0, 0} , {1, 0}}};
constexpr cmplx CZ_D7[2][2] = {{{1, 0}, {0, 0}}, {{0, 0} , {0, 1}}};

struct Gate {
    enum Type : size_t { Hadamard, X, Y, Z, Random, X_rotation, Y_rotation,
        Z_rotation, Phase, Control, Identity, T, Measurement, X_1_2, Y_1_2,
        CZ_D1, CZ_D2, CZ_D3, CZ_D4, CZ_D5, CZ_D6, CZ_D7
    };
    
    vector<vector<cmplx>> rows;
    vector<size_t> qubits;
    deque<size_t> ids;
    vector<float> theta;
    short num_controls;
    
    Gate(vector<vector<cmplx>> g);
    Gate();
    Gate(const Gate& rhs);
    Gate& operator=(const Gate& rhs);
};

// Args start with qubits and then phases in order
typedef Gate (*gate_generator_ptr) (const vector<float>&);

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

inline Gate create_Hadamard(const vector<float>& args)
{
    Gate g =  create_gate(cmplx(1, 0),
                          cmplx(1, 0),
                          cmplx(1, 0),
                          cmplx(-1, 0));
    g.ids.push_back(Gate::Type::Hadamard);
    g.qubits.push_back(static_cast<size_t>(args[0]));
    return g;
}

inline Gate create_X(const vector<float>& args)
{
    Gate g =  create_gate(cmplx(0, 0),
                          cmplx(1, 0),
                          cmplx(1, 0),
                          cmplx(0, 0));
    g.ids.push_back(Gate::Type::X);
    g.qubits.push_back(static_cast<size_t>(args[0]));
    return g;
}

inline Gate create_Y(const vector<float>& args)
{
    Gate g =  create_gate(cmplx(0, 0),
                          cmplx(0, -1),
                          cmplx(0, 1),
                          cmplx(0, 0));
    g.ids.push_back(Gate::Type::Y);
    g.qubits.push_back(static_cast<size_t>(args[0]));
    return g;
}

inline Gate create_X_1_2(const vector<float>& args)
{
    Gate g =  create_gate(cmplx(1, 1),
                          cmplx(1, -1),
                          cmplx(1, -1),
                          cmplx(1, 1));
    g.ids.push_back(Gate::Type::X_1_2);
    g.qubits.push_back(static_cast<size_t>(args[0]));
    return g;
}

inline Gate create_Y_1_2(const vector<float>& args)
{
    Gate g =  create_gate(cmplx(1, 1),
                          cmplx(-1, -1),
                          cmplx(1, 1),
                          cmplx(1, 1));
    g.ids.push_back(Gate::Type::Y_1_2);
    g.qubits.push_back(static_cast<size_t>(args[0]));
    return g;
}

inline Gate create_Z(const vector<float>& args)
{
    Gate g = create_gate(cmplx(1, 0),
                         cmplx(0, 0),
                         cmplx(0, 0),
                         cmplx(-1, 0));
    g.ids.push_back(Gate::Type::Z);
    g.qubits.push_back(static_cast<size_t>(args[0]));
    return g;
}

inline Gate create_I(const vector<float>& args)
{
    Gate g = create_gate(cmplx(1, 0),
                         cmplx(0, 0),
                         cmplx(0, 0),
                         cmplx(1, 0));
    g.ids.push_back(Gate::Type::Identity);
    g.qubits.push_back(static_cast<size_t>(args[0]));
    return g;
}


inline Gate create_X_rotation(const vector<float>& args)
{
    Gate g = create_gate(cmplx(cos(args[1]/2), 0),
                         cmplx(0, -sin(args[1]/2)),
                         cmplx(0, -sin(args[1]/2)),
                         cmplx(cos(args[1]/2), 0));
    g.theta.push_back(args[1]);
    g.ids.push_back(Gate::Type::X_rotation);
    g.qubits.push_back(static_cast<size_t>(args[0]));
    return g;
}

inline Gate create_Y_rotation(const vector<float>& args)
{
    Gate g = create_gate(cmplx(cos(args[1]/2), 0),
                         cmplx(-sin(args[1]/2),0),
                         cmplx(sin(args[1]/2),0),
                         cmplx(cos(args[1]/2), 0));
    g.theta.push_back(args[1]);
    g.ids.push_back(Gate::Type::Y_rotation);
    g.qubits.push_back(static_cast<size_t>(args[0]));
    return g;
}

inline Gate create_Z_rotation(const vector<float>& args)
{
    Gate g = create_gate(exp(cmplx(0,-args[1]/2)),
                         cmplx(0, 0),
                         cmplx(0, 0),
                         exp(cmplx(0,args[1]/2)));
    g.theta.push_back(args[1]);
    g.ids.push_back(Gate::Type::Z_rotation);
    g.qubits.push_back(static_cast<size_t>(args[0]));
    return g;
}

inline Gate create_Ph(const vector<float>& args)
{
    Gate g = create_gate(cmplx(1,0),
                         cmplx(0, 0),
                         cmplx(0, 0),
                         exp(cmplx(0,args[1])));
    //exp(cmplx(0, theta * M_PI * 2))
    g.theta.push_back(args[1]);
    g.ids.push_back(Gate::Type::Phase);
    g.qubits.push_back(static_cast<size_t>(args[0]));
    return g;
}

inline Gate create_T(const vector<float>& args)
{
    Gate g = create_gate(cmplx(1,0),
                         cmplx(0, 0),
                         cmplx(0, 0),
                         exp(cmplx(0,M_PI/4)));
    g.ids.push_back(Gate::Type::T);
    g.qubits.push_back(static_cast<size_t>(args[0]));
    return g;
}

inline Gate create_CZ(const vector<float>& args)
{
    Gate g = create_Z({args[1]});
    g.ids.push_front(Gate::Type::Control);
    g.num_controls = 1;
    g.qubits.push_back(static_cast<size_t>(args[0]));
    
    return g;
}


[[deprecated]]
inline Gate random_gate(size_t q)
{
    srand(time(NULL));
    vector<Gate> rotation_gates;
    
    float theta = ((double)rand())/double(RAND_MAX),
    theta1 = (double)rand()/double(RAND_MAX),
    theta2 = (double)rand()/double(RAND_MAX);
    rotation_gates.push_back(create_Z_rotation({static_cast<float>(q), theta2}));
    
    Gate rand_gate = create_Z_rotation({static_cast<float>(q), theta2});
    rand_gate.ids.push_back(Gate::Type::Z_rotation);
    rand_gate.theta.push_back(theta);
    
    int xy = rand() % NUM_BASIS_STATES;
    
    if (xy == 0) {
        rand_gate.ids.push_back(Gate::Type::X_rotation);
        rand_gate.theta.push_back(theta1);
        rotation_gates.push_back(create_X_rotation({static_cast<float>(q), theta2}));
    }
    else {
        rand_gate.ids.push_back(Gate::Type::Y_rotation);
        rand_gate.theta.push_back(theta1);
        rotation_gates.push_back(create_Y_rotation({static_cast<float>(q), theta2}));
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
