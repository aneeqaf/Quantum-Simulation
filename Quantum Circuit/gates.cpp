//
//  circuit_gates.cpp
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 12/2/17.
//  Copyright © 2017 Aneeqa Fatima. All rights reserved.
//

#include <stdio.h>
#include "gates.h"

gate::gate(): rows({}), qubits({}), ids({}),
theta({}), num_controls(0){}

gate::gate(vector<vector<cmplx>> g): rows(g), qubits({}),
ids({}),theta({}), num_controls(0){}

gate::gate(const gate& rhs)
{
    rows = rhs.rows;
    qubits = rhs.qubits;
    ids = rhs.ids;
    theta = rhs.theta;
    num_controls = rhs.num_controls;
}

gate& gate::operator=(const gate& rhs)
{
    gate temp(rhs);
    swap(rows, temp.rows);
    swap(qubits, temp.qubits);
    swap(ids, temp.ids);
    swap(theta , temp.theta);
    swap(num_controls , temp.num_controls);
    return *this;
}


