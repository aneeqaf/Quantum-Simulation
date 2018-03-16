//
//  circuit.h
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 12/12/17.
//

#ifndef circuit_h
#define circuit_h

#include <fstream>
#include <iostream>
#include <string>
#include <stdio.h>
#include <valarray>
#include <vector>

#include "gates.h"
#include "state_interface.h"

using namespace std;

using idx_size = size_t;

class Circuit {
private:
    static vector<string> quiddpro_func;
    vector<Gate> gates;
    vector<idx_size> clock_cycles;
    vector<int> classical_bits;
    int qubits;
    
    void PrintGatesAndCycles() const;
    void PrintGates() const;
 
public:
    bool google ;
    
    void GroupAlternateCycles();
    void GroupSimilarGates();
    int MovexCZGates(bitset<128>& a_qubits_bitmask,
                      bitset<128>& b_qubits_bitmask);
    void ReadGoogleCircuitFile(const string& input_file,
                               const int depth);
    void ReadCustomInputFiles(const string& input_file,
                              cmplx*& amp,
                              idx_size& amp_size);
    void CreateGoogleCircuit(int qubits,
                             int clock_cycles);
    void WriteGeneratedCircuitFile(const string& out_file,
                                   const idx_size size_q);
    void CreateQuiddProScript(const string& out_file);
    
    int GetNumQubits() const;
    idx_size GetTotalNumGates() const;
    bool ClockCycleEmpty() const;
    idx_size GetNumCycles() const;
    int GateIndexForCycle(int cycle_num) const;
    Gate& GetGateFromIndex(idx_size i);
    const vector<Gate>& GetGates() const;
    
    Circuit();
    Circuit(const Circuit& rhs);
    Circuit& operator=(const Circuit& rhs);
};

#endif /* circuit_h */
