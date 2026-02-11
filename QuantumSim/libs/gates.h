//
//  gates.h
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 11/25/17.
//

#ifndef gates_h
#define gates_h

#include <algorithm>
#include <boost/functional/hash.hpp>
#include <cassert>
#include <complex>
#include <deque>
#include <functional>
#include <memory>
#include <stdio.h>
#include <utility>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "matrix.h"

using namespace std;

constexpr idx_size NUM_GATES = 21;
constexpr idx_size NUM_BASIS_STATES = 2;

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
constexpr cmplx CZ[4][4] = {{cmplx(1, 0), cmplx(0, 0), cmplx(0, 0), cmplx(0, 0)},
    {cmplx(0, 0), cmplx(1, 0), cmplx(0, 0), cmplx(0, 0)},
    {cmplx(0, 0), cmplx(0, 0), cmplx(1, 0), cmplx(0, 0)},
    {cmplx(0, 0), cmplx(0, 0), cmplx(0, 0), cmplx(-1, 0)}};

struct Gate {
public:
    enum Type : size_t {h, x, y, z, rx, ry, rz, ph, cz, i, t, m, x_1_2, y_1_2,
        cz_d1, cz_d2, cz_d3, cz_d4, cz_d5, cz_d6, cz_d7, none
    };

private:
    vector<idx_size> qubits;
    vector<float> thetas;
    Type type;
    short num_controls;
    bool diagonal;
    
    static unordered_map<Gate::Type, vector<vector<cmplx>>> constant_gate_matrices;
    static unordered_map<Gate::Type, vector<vector<cmplx>> (*)(const vector<float>&)> phase_gate_matrices;
    
    static vector<vector<cmplx>> GetGateMatrix(Type gate_type, const vector<float>& args);
    
    static const char * gate_enum_type_strings[NUM_GATES];

public:
    Gate(Gate::Type type,
         idx_size num_controls,
         bool is_diag,
         const vector<idx_size>& qubits,
         const vector<float>& thetas);
    
    Gate(const Gate& rhs)
        : qubits(rhs.qubits), thetas(rhs.thetas), type(rhs.type),  num_controls(rhs.num_controls), diagonal(rhs.diagonal){}
    Gate(Gate&& rhs);
    Gate& operator=(const Gate& rhs);
    
    Type GetType() const { return type; }
    const char* TypeToString() const { return gate_enum_type_strings[type]; };
    const vector<vector<cmplx>> GetMatrix() const { return GetGateMatrix(type, thetas); }
    const vector<idx_size>& GetQubits() const { return qubits; }
    const vector<float>& GetTheta() const { return thetas; }
    bool IsDiagonal() const { return diagonal; }
    idx_size GetNumControls() const { return num_controls; }
};

struct Hash_1q_Gate {
    idx_size qubit;
    float theta;
    Gate::Type type;
    
    bool operator==(const Hash_1q_Gate& g) const {
        return (qubit == g.qubit
              && theta == g.theta
              && type == g.type);
    }
    
    size_t operator()(const Hash_1q_Gate& g) const
    {
        size_t seed = 0;
        boost::hash_combine(seed, g.qubit);
        boost::hash_combine(seed, g.theta);
        boost::hash_combine(seed, g.type);
        
        return seed;
    }
};

// Args start with qubits and then phases in order
typedef Gate (*gate_generator_ptr) (const vector<float>&);

inline vector<vector<cmplx>> create_X_rotation_matrix(const vector<float>& args)
{
    return {{cmplx(cos(args[0]/2), 0), cmplx(0, -sin(args[0]/2))},
        {cmplx(0, -sin(args[0]/2)), cmplx(cos(args[0]/2), 0)}};
}

inline vector<vector<cmplx>> create_Y_rotation_matrix(const vector<float>& args)
{
    return {{cmplx(cos(args[0]/2), 0), cmplx(-sin(args[0]/2), 0)},
        {cmplx(sin(args[0]/2), 0), cmplx(cos(args[0]/2), 0)}};
}

inline vector<vector<cmplx>> create_Z_rotation_matrix(const vector<float>& args)
{
    return {{exp(cmplx(0, -args[0]/2)), cmplx(0, 0)}, {cmplx(0, 0), exp(cmplx(0, args[0]/2))}};
}

inline vector<vector<cmplx>> create_Ph_matrix(const vector<float>& args)
{
    return {{cmplx(1, 0), cmplx(0, 0)}, {cmplx(0, 0), exp(cmplx(0, args[1]))}};
}

inline Gate create_Hadamard(const vector<float>& args)
{
    return Gate(Gate::Type::h,
               0,
               false,
               {static_cast<size_t>(args[0])},
               {});
}

inline Gate create_X(const vector<float>& args)
{
    return Gate(Gate::Type::x,
               0,
               false,
               {static_cast<size_t>(args[0])},
               {});
}

inline Gate create_Y(const vector<float>& args)
{
    return Gate(Gate::Type::y,
               0,
               false,
               {static_cast<size_t>(args[0])},
               {});
}

inline Gate create_X_1_2(const vector<float>& args)
{
    return Gate(Gate::Type::x_1_2,
               0,
               false,
               {static_cast<size_t>(args[0])},
               {});
}

inline Gate create_Y_1_2(const vector<float>& args)
{
    return Gate(Gate::Type::y_1_2,
                0,
                false,
                {static_cast<size_t>(args[0])},
                {});
}

inline Gate create_Z(const vector<float>& args)
{
    return Gate(Gate::Type::z,
                0,
                true,
                {static_cast<size_t>(args[0])},
                {});
}

inline Gate create_I(const vector<float>& args)
{
    return Gate(Gate::Type::i,
                0,
                true,
                {static_cast<size_t>(args[0])},
                {});
}

inline Gate create_X_rotation(const vector<float>& args)
{
    return Gate(Gate::Type::rx,
               0,
               false,
               {static_cast<size_t>(args[0])},
               {args[1]});
}

inline Gate create_Y_rotation(const vector<float>& args)
{
    return Gate(Gate::Type::ry,
                0,
                false,
                {static_cast<size_t>(args[0])},
                {args[1]});
}

inline Gate create_Z_rotation(const vector<float>& args)
{
    return Gate(Gate::Type::rz,
                0,
                true,
                {static_cast<size_t>(args[0])},
                {args[1]});
}

inline Gate create_Ph(const vector<float>& args)
{
    return Gate(Gate::Type::ph,
                0,
                true,
                {static_cast<size_t>(args[0])},
                {args[1]});
}

inline Gate create_T(const vector<float>& args)
{
    return Gate(Gate::Type::t,
                0,
                true,
                {static_cast<size_t>(args[0])},
                {});
}

inline Gate create_CZ(const vector<float>& args)
{
    return Gate(Gate::Type::cz,
                1,
                true,
                {static_cast<size_t>(args[1]), static_cast<size_t>(args[0])},
                {});
}

#endif /* gates_h */
