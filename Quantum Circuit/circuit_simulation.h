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

using index_size = size_t;

class state {
public:
    vector<cmplx> amp;
    
    cmplx measure_0(const short qubit);
    cmplx measure_1(const short qubit);
    void measure(const short qubit);
    
    state();
    state(const state& rhs);
    state& operator=(const state& rhs);
};

class circuit {
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
    vector<double> gate_time;
    clock_t g_begin;
    clock_t g_end;
    bool google ;
    //1ull < (global_factor_power/2) * [[global_factor_power%2 == 1]] * sqrt(2)
    //global_factor_power represents the count of H, X_1_2, and Y_1_2 gates.
    index_size global_factor_power;
    
    template<typename function>
    index_size* FormBlockOfGates(const vector<gate>& gates,
                                index_size& gate_i,
                                vector<index_size>& T_bit_mask,
                                function comp);
    void ApplyBlockOfGates(const index_size* cbits,
                           const int qubits,
                           vector<cmplx>& amp,
                           const vector<index_size>& T);
    inline void FindRelevantAmp(vector<index_size>& indices_amp_gate,
                                index_size gate_i,
                                const int num_bits,
                                const index_size idx);
    inline void ApplyAnyCGate(const index_size c_bits,
                              const index_size gate_i);
    inline void ApplyNonControlGates(const index_size gate_i);
    inline void  ApplyCXGate(const index_size idx,
                             const index_size swap_index);
    inline cmplx ApplyPhaseGate(const index_size gate_c,
                               const cmplx& amp);
    inline cmplx ApplyTGateKTimes(const index_size gate_c,
                                  const cmplx& amp);
    inline int ApplyHOnAllAmp(const index_size gate_i);
    inline void ApplyXX12Gate(const index_size* const indices);
    inline void ApplyXY12Gate(const index_size* const indices,
                              const index_size size);
    inline void ApplyYY12Gate(const index_size* constindices);
    inline void ApplyYX12Gate(const index_size* const indices);
    void Merge2QXY12Gates(const index_size gate_i);
    void ApplyMergedXY12Gates(const short type,
                            const index_size gate_i);
    void ApplySingleTGate(const index_size gate_i);
    void ApplySingleCPhaseGate(const index_size gate_i);
    void ApplyGateOnGateSizeAmps(const index_size* const indices,
                                 const index_size size,
                                 const gate& q_gate);
    void GroupAlternateCycles();
    void GroupSimilarGates();
    float CalculateNormOfAmp();
    
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
