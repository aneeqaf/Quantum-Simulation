//
//  simulation.hpp
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 12/12/17.
//  Copyright © 2017 Aneeqa Fatima. All rights reserved.
//

#ifndef simulation_h
#define simulation_h

#include <stdio.h>
#include <ctime>
#include "circuit.h"
#include "state.h"

using namespace std;

class sequentialSimulation {
private:
    
    vector<double> gate_time;
    clock_t g_begin;
    clock_t g_end;
    //1ull < (global_factor_power/2) * [[global_factor_power%2 == 1]] * sqrt(2)
    //global_factor_power represents the count of H, X_1_2, and Y_1_2 gates.
    int num_rescaling;
    int merged_X_Y;
    int X;
    int Y;
    int CZ_T;
    
   
public:
    void PrintReport(const clock_t end,
                     const clock_t begin,
                     state& amp,
                     const circuit& circuit,
                     const int sim_depth) const;
    void PrintReport(const string& outfile,
                     const clock_t end,
                     const clock_t begin,
                     state& amp,
                     const circuit& circuit,
                     const int sim_depth) const;
    
    void Simulate(const string& outfile,
                  state& amp,
                  circuit& circuit,
                  const int sim_depth = -1);
    
    sequentialSimulation();
    sequentialSimulation(const sequentialSimulation& rhs);
    sequentialSimulation& operator=(const sequentialSimulation& rhs);
};


#endif /* simulation_h */
