//
//  simulation.h
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 12/12/17.
//

#ifndef simulation_h
#define simulation_h

#include <chrono>
#include <ctime>
#include <fcntl.h>
#include <sys/mman.h>
#include <sstream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unordered_map>

#include "circuit.h"
#include "config.h"
#include "state_interface.h"
#include "state_sum_tensor.h"
#include "utility.h"

using namespace std;

inline string
ver_string(int a, int b, int c)
{
  ostringstream ss;
  ss << a << '.' << b << '.' << c;
  return ss.str();
}

class SequentialSimulation
{
private:
  static unordered_map<string, array<cmplx, 5>> benchmark;
  ostringstream log;
  idx_size curr_gate, num_layers;
  idx_size adjustment_factor;
  double memory_usage, total_time, branch_time, prefix_time, XE_time, CZ_T_top_time, mmap_time, cir_rearrangement_time;
  Config *config;
  vector<double> norms_CZ_paths;
  vector<cmplx> amps_of_interest;
  // 1ull < (global_factor_power/2) * [[global_factor_power%2 == 1]] * sqrt(2)
  // global_factor_power represents the count of H, X_1_2, and Y_1_2 gates.

  void AmplitudeSampler(GenericQuantumState &amp);
  void PopulateBenchmarkMap();
  void CopyFromCheckpoint(bool branch,
                          GenericQuantumState &amp,
                          const GenericQuantumState &copy_amp);
  void Checkpoint(bool branch,
                  GenericQuantumState &amp,
                  Circuit &circuit,
                  const idx_size gate_i = 0);
  void MainLoopForRanges(GenericQuantumState &amp,
                         Circuit &circuit,
                         const GenericQuantumState &copy_amp);
  void MainLoopForBranching(GenericQuantumState &amp,
                            Circuit &circuit,
                            const GenericQuantumState &copy_amp,
                            const idx_size gate_i = 0);
  void CheckpointWithRanges(GenericQuantumState &amp,
                            Circuit &circuit);
  void NoCheckpointWithRanges(GenericQuantumState &amp,
                              Circuit &circuit);

public:
  void SetThreshold(int th);
  void PrintSystemReport() const;
  void PrintSimSpecReport(const GenericQuantumState &amp,
                          const Circuit &circuit,
                          const pair<int, int> twoq_gates);
  void PrintSimReport(GenericQuantumState &amp,
                      const Circuit &circuit) const;
  void PrintReportToFile(GenericQuantumState &amp,
                         const Circuit &circuit) const;

  void Simulate(GenericQuantumState &amp,
                Circuit &circuit);
  void Phase2Simulation(GenericQuantumState &amp,
                        Circuit &circuit,
                        const idx_size gate_i);
  void Phase1Simulation(GenericQuantumState &amp,
                        Circuit &circuit,
                        idx_size gate_i,
                        int &remaining_cz_bits,
                        idx_size &cz_path,
                        const idx_size cz_path_len);
  bool SimulationLoop(GenericQuantumState &amp,
                      Circuit &circuit,
                      int &remaining_cz_bits,
                      idx_size &cz_path,
                      const idx_size cz_path_len,
                      const idx_size suffix_size,
                      const idx_size gate_i);
  void ReportingAfterSim(GenericQuantumState &amp,
                         Circuit &circuit);
  void PrintIdxsToFile() const;
  void WriteMmapToASCIIFile() const;
  void WriteAmpToASCIIFile(GenericQuantumState &amp) const;

  SequentialSimulation(Config *c);
  SequentialSimulation(const SequentialSimulation &rhs) = delete;
  SequentialSimulation &operator=(const SequentialSimulation &rhs) = delete;
};

#endif /* simulation_h */
