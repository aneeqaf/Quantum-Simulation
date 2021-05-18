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
public:
    enum Type : size_t { Hadamard, X, Y, Z, Random, X_rotation, Y_rotation,
        Z_rotation, Phase, ControlZ, Identity, T, Measurement, X_1_2, Y_1_2,
        CZ_D1, CZ_D2, CZ_D3, CZ_D4, CZ_D5, CZ_D6, CZ_D7
    };
    
private:
    vector<vector<cmplx>> matrix;
    vector<idx_size> qubits;
    vector<float> theta;
    Type type;
    short num_controls;
    bool diagonal;
    
public:
    Gate(vector<vector<cmplx>> matrix,
         Gate::Type type,
         idx_size num_controls,
         bool is_diag,
         const vector<idx_size>& qubits,
         const vector<float>& thetas): matrix(matrix), qubits(qubits), theta(thetas), type(type), num_controls(num_controls), diagonal(is_diag){}
    
    Gate(const Gate& rhs)
    : matrix(rhs.matrix), qubits(rhs.qubits), theta(rhs.theta), type(rhs.type),  num_controls(rhs.num_controls), diagonal(rhs.diagonal){}
    
    Gate& operator=(const Gate& rhs)
    {
        Gate temp(rhs);
        swap(matrix, temp.matrix);
        swap(qubits, temp.qubits);
        swap(type, temp.type);
        swap(theta , temp.theta);
        swap(num_controls , temp.num_controls);
        swap(diagonal, temp.diagonal);
        return *this;
    }
    
    Type GetType() const { return type; }
    const vector<vector<cmplx>>& GetMatrix() const { return matrix; }
    const vector<idx_size>& GetQubits() const { return qubits; }
    const vector<float>& GetTheta() const { return theta; }
    bool IsDiagonal() const { return diagonal; }
    idx_size GetNumControls() const { return num_controls; }
};

// Args start with qubits and then phases in order
typedef Gate (*gate_generator_ptr) (const vector<float>&);

inline Gate create_Hadamard(const vector<float>& args)
{
    return Gate({{cmplx(1, 0), cmplx(1, 0)},
                 {cmplx(1, 0), cmplx(-1, 0)}},
               Gate::Type::Hadamard,
               0,
               false,
               {static_cast<size_t>(args[0])},
               {});
}

inline Gate create_X(const vector<float>& args)
{
    return Gate({{cmplx(0, 0), cmplx(1, 0)},
                 {cmplx(1, 0), cmplx(0, 0)}},
               Gate::Type::X,
               0,
               false,
               {static_cast<size_t>(args[0])},
               {});
}

inline Gate create_Y(const vector<float>& args)
{
    return Gate({{cmplx(0, 0), cmplx(0, -1)},
                {cmplx(0, 1), cmplx(0, 0)}},
               Gate::Type::Y,
               0,
               false,
               {static_cast<size_t>(args[0])},
               {});
}

inline Gate create_X_1_2(const vector<float>& args)
{
    return Gate({{cmplx(1, 1), cmplx(1, -1)},
                 {cmplx(1, -1), cmplx(1, 1)}},
               Gate::Type::X_1_2,
               0,
               false,
               {static_cast<size_t>(args[0])},
               {});
}

inline Gate create_Y_1_2(const vector<float>& args)
{
    return Gate({{cmplx(1, 1), cmplx(-1, -1)},
                 {cmplx(1, 1), cmplx(1, 1)}},
                Gate::Type::Y_1_2,
                0,
                false,
                {static_cast<size_t>(args[0])},
                {});
}

inline Gate create_Z(const vector<float>& args)
{
    return Gate({{cmplx(1, 0), cmplx(0, 0)},
                 {cmplx(0, 0), cmplx(-1, 0)}},
                Gate::Type::Z,
                0,
                true,
                {static_cast<size_t>(args[0])},
                {});
}

inline Gate create_I(const vector<float>& args)
{
    return Gate({{cmplx(1, 0), cmplx(0, 0)},
                 {cmplx(0, 0), cmplx(1, 0)}},
                Gate::Type::Identity,
                0,
                true,
                {static_cast<size_t>(args[0])},
                {});
}


inline Gate create_X_rotation(const vector<float>& args)
{
    return Gate({{cmplx(cos(args[1]/2), 0), cmplx(0, -sin(args[1]/2))},
                   {cmplx(0, -sin(args[1]/2)), cmplx(cos(args[1]/2), 0)}},
                  Gate::Type::X_rotation,
                  0,
                  false,
                  {static_cast<size_t>(args[0])},
                  {args[1]});
}

inline Gate create_Y_rotation(const vector<float>& args)
{
    return Gate({{cmplx(cos(args[1]/2), 0), cmplx(-sin(args[1]/2), 0)},
                 {cmplx(sin(args[1]/2), 0), cmplx(cos(args[1]/2), 0)}},
                Gate::Type::Y_rotation,
                0,
                false,
                {static_cast<size_t>(args[0])},
                {args[1]});
}

inline Gate create_Z_rotation(const vector<float>& args)
{
    return Gate({{exp(cmplx(0, -args[1]/2)), cmplx(0, 0)},
                  {cmplx(0, 0), exp(cmplx(0, args[1]/2))}},
                Gate::Type::Z_rotation,
                0,
                true,
                {static_cast<size_t>(args[0])},
                {args[1]});
}

inline Gate create_Ph(const vector<float>& args)
{
    return Gate({{cmplx(1, 0), cmplx(0, 0)},
                  {cmplx(0, 0), exp(cmplx(0, args[1]))}},
                Gate::Type::Phase,
                0,
                true,
                {static_cast<size_t>(args[0])},
                {args[1]});
}

inline Gate create_T(const vector<float>& args)
{
    return Gate({{cmplx(1, 0), cmplx(0, 0)},
                  {cmplx(0, 0), exp(cmplx(0, M_PI/4))}},
                Gate::Type::T,
                0,
                true,
                {static_cast<size_t>(args[0])},
                {});
}

inline Gate create_CZ(const vector<float>& args)
{
    return Gate({{cmplx(1, 0), cmplx(0, 0), cmplx(0, 0), cmplx(0, 0)},
                    {cmplx(0, 0), cmplx(1, 0), cmplx(0, 0), cmplx(0, 0)},
                    {cmplx(0, 0), cmplx(0, 0), cmplx(1, 0), cmplx(0, 0)},
                    {cmplx(0, 0), cmplx(0, 0), cmplx(0, 0), cmplx(-1, 0)}},
                Gate::Type::ControlZ,
                1,
                true,
                {static_cast<size_t>(args[1]), static_cast<size_t>(args[0])},
                {});
}

#endif /* gates_h */
