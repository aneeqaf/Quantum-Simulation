//
//  simulation.h
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 12/12/17.
//

#ifndef simulation_h
#define simulation_h

#include <ctime>
#include <stdio.h>

#include "circuit.h"
#include "state.h"

using namespace std;

class SequentialSimulation {
private:
    
    vector<double> gate_time;
    clock_t g_begin;
    clock_t g_end;
    double rescale_time;
    //1ull < (global_factor_power/2) * [[global_factor_power%2 == 1]] * sqrt(2)
    //global_factor_power represents the count of H, X_1_2, and Y_1_2 gates.
    int num_rescaling;
    int merged_X_Y;
    int X;
    int Y;
    int CZ_T;
    int th;
    
   
public:
    void PrintReport(const clock_t end,
                     const clock_t begin,
                     State& amp,
                     const Circuit& circuit) const;
    void PrintReport(const string& outfile,
                     const clock_t end,
                     const clock_t begin,
                     State& amp,
                     const Circuit& circuit) const;
    
    void Simulate(const string& outfile,
                  State& amp,
                  Circuit& circuit,
                  const int th = 0);
    
    SequentialSimulation();
    SequentialSimulation(const SequentialSimulation& rhs);
    SequentialSimulation& operator=(const SequentialSimulation& rhs);
};


#endif /* simulation_h */
