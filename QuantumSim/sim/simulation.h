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
#include "config.h"
#include "state_interface.h"
#include "state_autoconv.h"

using namespace std;

class SequentialSimulation {
private:
    
    static unordered_map<string, array<cmplx, 5>> benchmark;
    ostringstream log;
    double total_time, dfs_time, cz_path_time, XE_time, CZ_T_top_time, X_Y_top_time;
    idx_size curr_gate;
    Config config;
    //1ull < (global_factor_power/2) * [[global_factor_power%2 == 1]] * sqrt(2)
    //global_factor_power represents the count of H, X_1_2, and Y_1_2 gates.
   
    void PopulateBenchmarkMap();
   
public:
    void PrintSystemReport() const;
    void PrintSimSpecReport(const GenericQuantumState& amp,
                            const Circuit& circuit);
    void PrintSimReport(GenericQuantumState& amp,
                     const Circuit& circuit) const;
    void PrintReportToFile(GenericQuantumState& amp,
                           const Circuit& circuit) const;
    
    void Simulate(GenericQuantumState& amp,
                  Circuit& circuit);
    void Phase2Simulation(GenericQuantumState& amp,
                          Circuit& circuit,
                          const idx_size gate_i);
    void Phase1Simulation(GenericQuantumState& amp,
                          Circuit& circuit);
    bool SimulationLoop(GenericQuantumState& amp,
                        Circuit& circuit,
                        const idx_size gate_i = 0);
    
    void ReportingAfterSim(GenericQuantumState& amp,
                           Circuit& circuit);
    
    SequentialSimulation(const Config& c);
    SequentialSimulation(const SequentialSimulation& rhs) = delete;
    SequentialSimulation& operator=(const SequentialSimulation& rhs) = delete;
};


#endif /* simulation_h */
