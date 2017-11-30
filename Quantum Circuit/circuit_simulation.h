//
//  circuit_simulation.hpp
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 10/21/17.
//  Copyright © 2017 Aneeqa Fatima. All rights reserved.
//

#ifndef circuit_simulation_h
#define circuit_simulation_h

#include <stdio.h>
#include <stdlib.h>
#include <iomanip>
#include <unistd.h> 
#include <memory>
#include <iostream>
#include <fstream>
#include <vector>
#include <complex>
#include <map>
#include <iterator>
#include <utility>
#include <ctime>
#include <cassert>
#include <deque>
#include <algorithm>
#include <functional>
#include "matrix.h"
#include "circuit_gates.h"

using namespace std;

const float HADAMARD_CONST = 1/sqrt(2);
extern bool google;
extern float H_related_gates;

using index_size = size_t;

class state {
public:
    vector<cmplx> amp;
    vector<cmplx> apply_gate;
    vector<index_size> indices_for_ag;
    
    void operator*(const gate& matrix);
    
    cmplx measure_0(const short qubit);
    cmplx measure_1(const short qubit);
    void measure(const short qubit);
    vector<cmplx> tensor();
    
    state();
    state(const state& rhs);
    state& operator=(const state& rhs);
};

class circuit {
private:
    template<typename function>
    vector<index_size> FormBlockOfGates(int& num_X_gates,
                                        index_size gate_i,
                                        vector<index_size>& T_bit_mask,
                                        function comp);
    void ApplyBlockOfGates(const vector<index_size>& cbits,
                           index_size gate_i,
                           const vector<index_size>& T,
                           int X_gates);
    inline void FindRelevantAmp(vector<bool>& iterated,
                                index_size gate_i,
                                const int num_bits,
                                const index_size idx);
    inline void ApplyAnyCGate(const index_size c_bits,
                              const index_size gate_i);
    inline void ApplyNonControlGates(const index_size gate_i);
    inline void  ApplyCXGate(const index_size idx,
                             const index_size swap_index);
    inline void ApplyPhaseGate(const int i_count,
                               const index_size idx);
    inline void ApplyTGateKTimes(const index_size gate_c,
                                 const index_size idx);
    inline int ApplyHOnAllAmp(const index_size gate_i);
    inline void ApplyXXGate();
    inline void ApplyXYGate();
    inline void ApplyYYGate();
    inline void ApplyYXGate();
    void Merge2QXYGates(const index_size gate_i);
    void ApplyMergedXYGates(const short type,
                            const index_size gate_i);
    void ApplySingleTGate(const index_size gate_i);
    void ApplySingleCPhaseGate(const index_size gate_i);
    void GroupAlternateCycles();
    void GroupSimilarGates();

public:
    
    vector<gate> gates;
    vector<short> clock_cycles;
    state* circuit_state;
    short merged;
    short X;
    short Y;
    short CZ_T;
    short qubits;
    short num_cycles;
    short current_google_cycle;
    bool circuit_preprocessed;
    
    void Simulate(const string& outfile);
    void PrintStateVector(const string& outfile);
    void PrintStateVector();
    void PrintReport(const clock_t end,
                     const clock_t begin);
    void PrintReport(const string& outfile,
                     const clock_t end,
                     const clock_t begin);
    void PrintGatesAndCycles();
    void PrintGates();
    void PrintProbabilities(const string& out_file);
    
    circuit();
    circuit(const circuit& rhs);
    circuit& operator=(const circuit& rhs);
    ~circuit();
};


#endif /* circuit_simulation_h */
