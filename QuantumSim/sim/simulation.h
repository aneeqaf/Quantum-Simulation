//
//  simulation.h
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 12/12/17.
//

#ifndef simulation_h
#define simulation_h

#include <ctime>
#include <sstream>
#include <stdio.h>
#include <unordered_map>

#include "circuit.h"
#include "state_interface.h"

using namespace std;

class SequentialSimulation {
public:
    enum SimType : int {LosslessH, LosslessV, Approx1CutH, Approx1CutV, Approx2Cuts, FullState};
    enum Verbose : int {None, NCCV, NCC, Default, Cycles};
private:
    
    static unordered_map<string, array<cmplx, 5>> benchmark;
    vector<double> gate_time;
    ostringstream log;
    string filename;
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
    int cut_sizes;
    int conv_cycle;
    bool google;
    SimType sim_type;
    Verbose verbose;
    
    void PopulateBenchmarkMap();
   
public:

    void PrintReport(GenericQuantumState& amp,
                     const clock_t end,
                     const clock_t begin,
                     const Circuit& circuit) const;
    void PrintReport(GenericQuantumState& amp,
                     const string& outfile,
                     const clock_t end,
                     const clock_t begin,
                     const Circuit& circuit) const;
    
    void Simulate(const string& outfile,
                  GenericQuantumState& amp,
                  Circuit& circuit,
                  const int th = 0);
    
    SequentialSimulation(const SimType st,
                         const int cut_sizes = 0,
                         const Verbose v = Default);
    SequentialSimulation(const string filename,
                         const bool g,
                         const SimType st,
                         const int cut_sizes = 0,
                         const Verbose v = Default);
    SequentialSimulation(const SequentialSimulation& rhs);
    SequentialSimulation& operator=(const SequentialSimulation& rhs);
};

#endif /* simulation_h */
