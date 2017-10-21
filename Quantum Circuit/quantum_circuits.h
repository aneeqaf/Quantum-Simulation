//
//  quantum_circuits.h
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 10/14/17.
//  Copyright © 2017 Aneeqa Fatima. All rights reserved.
//

#ifndef quantum_circuits_h
#define quantum_circuits_h

#include <vector>
#include <stdio.h>
#include <cmath>
#include <string>
#include <math.h>
#include <stdlib.h>

using namespace std;


vector<vector<double>> Transform_Gate(vector<string>& gates) {
    
    vector<vector<double>> final_gate = Create_Gate(gates[0]);
    
    vector<vector<double>> gate;
    
    for (int i = 1; i < gates.size(); ++i) {
        
        gate = Create_Gate(gates[i]);
        final_gate = Tensor_Product(final_gate, gate);
    }
    
    for (int i = 0; i < final_gate.size(); ++i){
        for (int j = 0; j < final_gate.size(); ++j){
            if (final_gate[i][j] > 10 || final_gate[i][j] < -10)
                final_gate[i][j] = final_gate[i][j]/1000;
        }
    }
    
    return final_gate;
}

vector<double> Circuit(vector<vector<double>>& qubits, vector<vector<string>>& gates) {
    
    //Change the matrix of qubits into a vector
    vector<double> q = qubits[0];
    
    for (int i = 1; i < qubits.size(); ++i) {
        q = Tensor_Product(q, qubits[i]);
    }
    
    vector<vector<double>> final_gate = Transform_Gate(gates[gates.size()-1]);
    
    for (int i = (int)gates.size()-2; i >= 0; --i) {
        
        vector<vector<double>> gate = Transform_Gate(gates[i]);
        
        final_gate = Matrix_Multiplication(final_gate, gate);
    }
    
    vector<double> result = Matrix_Multiplication(final_gate, q);
    
    return result;
}


#endif /* quantum_circuits_h */
