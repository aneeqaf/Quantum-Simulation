//
//  gates.cpp
//  QuantumSimProj
//
//  Created by Aneeqa Fatima on 6/6/21.
//  Copyright © 2021 Aneeqa Fatima. All rights reserved.
//

#include <stdio.h>
#include "gates.h"

unordered_map<Gate::Type, vector<vector<cmplx>>> Gate::constant_gate_matrices;
unordered_map<Gate::Type, vector<vector<cmplx>> (*)(const vector<float>&)> Gate::phase_gate_matrices;

Gate::
Gate(Gate::Type type,
     idx_size num_controls,
     bool is_diag,
     const vector<idx_size>& qubits,
     const vector<float>& thetas): qubits(qubits), thetas(thetas), type(type), num_controls(num_controls), diagonal(is_diag)
{
    Gate::constant_gate_matrices[Gate::Type::Hadamard] = {{cmplx(1, 0), cmplx(1, 0)}, {cmplx(1, 0), cmplx(-1, 0)}};
    Gate::constant_gate_matrices[Gate::Type::X] = {{cmplx(0, 0), cmplx(1, 0)}, {cmplx(1, 0), cmplx(0, 0)}};
    Gate::constant_gate_matrices[Gate::Type::Y] = {{cmplx(0, 0), cmplx(0, -1)}, {cmplx(0, 1), cmplx(0, 0)}};
    Gate::constant_gate_matrices[Gate::Type::Z] = {{cmplx(1, 0), cmplx(0, 0)}, {cmplx(0, 0), cmplx(-1, 0)}};
    Gate::constant_gate_matrices[Gate::Type::X_1_2] = {{cmplx(1, 1), cmplx(1, -1)}, {cmplx(1, -1), cmplx(1, 1)}};
    Gate::constant_gate_matrices[Gate::Type::Y_1_2] = {{cmplx(1, 1), cmplx(-1, -1)}, {cmplx(1, 1), cmplx(1, 1)}};
    Gate::constant_gate_matrices[Gate::Type::T] = {{cmplx(1, 0), cmplx(0, 0)}, {cmplx(0, 0), exp(cmplx(0, M_PI/4))}};
    Gate::constant_gate_matrices[Gate::Type::Identity] = {{{1, 0}, {0, 0}}, {{0, 0} , {1, 0}}};
    Gate::constant_gate_matrices[Gate::Type::ControlZ] = {{cmplx(1, 0), cmplx(0, 0), cmplx(0, 0), cmplx(0, 0)},
                                                    {cmplx(0, 0), cmplx(1, 0), cmplx(0, 0), cmplx(0, 0)},
                                                    {cmplx(0, 0), cmplx(0, 0), cmplx(1, 0), cmplx(0, 0)},
                                                    {cmplx(0, 0), cmplx(0, 0), cmplx(0, 0), cmplx(-1, 0)}};
    
    Gate::phase_gate_matrices[Gate::Type::X_rotation] = create_X_rotation_matrix;
    Gate::phase_gate_matrices[Gate::Type::Y_rotation] = create_X_rotation_matrix;
    Gate::phase_gate_matrices[Gate::Type::Z_rotation] = create_X_rotation_matrix;
    Gate::phase_gate_matrices[Gate::Type::Phase] = create_X_rotation_matrix;
}

Gate& Gate::
operator=(const Gate& rhs)
{
    Gate temp(rhs);
    swap(qubits, temp.qubits);
    swap(thetas, temp.thetas);
    swap(type, temp.type);
    swap(num_controls , temp.num_controls);
    swap(diagonal, temp.diagonal);
    return *this;
}

Gate::Gate(Gate&& rhs)
{
    qubits = move(rhs.qubits);
    thetas = move(rhs.thetas);
    type = rhs.type;
    num_controls = rhs.num_controls;
    diagonal = rhs.diagonal;
}

vector<vector<cmplx>> Gate::
GetGateMatrix(Type gate_type, const vector<float>& args)
{
    if (constant_gate_matrices.count(gate_type) == 0)
        return phase_gate_matrices[gate_type](args);
    
    return constant_gate_matrices[gate_type];
}
