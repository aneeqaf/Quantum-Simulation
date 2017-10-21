//
//  main.cpp
//  Quantum Circuits
//
//  Created by Aneeqa Fatima on 10/13/17.
//

/*
 * Input File Format:
 * <Number of qubits in the systems>
 * <value of qubit 1> <value of qubit 2> ...
 * <gate to be applied to the below qubits>
 * <qubit number> <qubit number>
 * <gate to be applied to the below qubits>
 * <qubit number> <qubit number>
 *  .... so on. Rest of file follows same format.
 */

#include <sstream>
#include <fstream>
#include <iostream>
#include "quantum_circuits.h"

using namespace std;

int main(int argc, char *argv[]) {
    
#ifdef __APPLE__
    if (getenv("STDIN")) {
        if (!freopen(getenv("STDIN"), "r", stdin)) {
            cerr << "Unable to redirect stdin to " << getenv("STDIN") << endl;
            exit (1);
        }
    }
#endif
    
    ostringstream os;
    
    int qubit_system = 0;
    cin >> qubit_system;
    
    string gate = "", qubits;
    int qubit_num;
    vector<vector<string>> gates_inorder;
    vector<vector<double>> matrix_of_qubits;
    
    if (qubit_system > 0) {
        
        try {
            string vals;
            string null;
            getline(cin, null);
            getline(cin,vals);
            istringstream iss(vals);
            for (size_t j = 0; j < qubit_system; ++j) {
                int a = 0;
                try {
                    iss >> a;
                }
                catch(char* e){}
                vector<double> row;
                if(a == 0) {
                    row.push_back(1);
                    row.push_back(0);
                }
                else {
                    row.push_back(0);
                    row.push_back(1);
                }
                matrix_of_qubits.push_back(row);
            }
            
            while (getline(cin,gate)) {
                vector<string> gates (qubit_system, "identity");
                string g;
                getline(cin, qubits);
                istringstream qiss(qubits);
                istringstream giss(gate);
                while ( qiss >> qubit_num && giss >> g) {
                    gates[qubit_num-1] = g;
                }
                
                gates_inorder.push_back(gates);
            }
            
            vector<double> result = Circuit(matrix_of_qubits, gates_inorder);
            cout << "result : ";
            for (int i = 0; i < result.size(); ++i)
                cout << result[i] << " ";
            
        } catch (char* error) {
            cerr << "incorrect file format\n";
            cerr << "\
            * Input File Format:/n\
            * <Number of qubits in the systems>/n\
            * <value of qubit 1> <value of qubit 2> .../n\
            * <gate to be applied to the below qubits>/n\
            * <qubit number> <qubit number>/n\
            * <gate to be applied to the below qubits>/n\
            * <qubit number> <qubit number>/n\
            .... so on. Rest of file follows same format./n";
            exit(1);
        }
    }
    return 0;
    
}

