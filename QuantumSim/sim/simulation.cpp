
//  simulation.cpp
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 12/12/17.
//

#include "simulation.h"

unordered_map<string, array<cmplx, 5>> SequentialSimulation::benchmark = {};

SequentialSimulation::
SequentialSimulation(const Config& c): total_time(0), dfs_time(0), phase1_time(0), XE_time(0), config(c){}

void SequentialSimulation::
PopulateBenchmarkMap()
{
    benchmark["inst_5_5_100_5_26"] = {cmplx(-1.131e-05,-7.93622e-06), cmplx(8.75183e-05,5.33763e-05), cmplx(1.07944e-05,4.87672e-05),
        cmplx(0.000104529,-0.000158536), cmplx(5.64697e-05,-0.000154015)};
    benchmark["inst_5_5_100_5_101"] = {cmplx(4.87169e-07,-0.000124961), cmplx(-3.05764e-05,-0.000204519), cmplx(-6.19013e-05,-8.43196e-05),
        cmplx(-0.00016258,-1.55531e-05), cmplx(-0.000111381,0.000135595)};
    benchmark["inst_6_5_100_5_26"] = {cmplx(-6.54596e-06,-2.83312e-05), cmplx(9.81011e-06,-1.3834e-05), cmplx(-1.328e-06,-5.20411e-05),
        cmplx(-2.88494e-05,-3.6528e-05), cmplx(2.02035e-05,-1.49718e-05)};
    benchmark["inst_6_5_100_5_101"] = {cmplx(-1.64932e-05,3.05875e-05), cmplx(-2.23461e-05,-2.09741e-05), cmplx(4.19768e-06,-3.64171e-05),
        cmplx(-9.25683e-06,5.20232e-05), cmplx(4.8687e-06,-1.32963e-05)};
    benchmark["inst_8_4_100_0_26"] = {cmplx(1.26873e-05,1.5801e-05), cmplx(-1.3945e-05,5.97119e-06), cmplx(-4.64556e-06,7.54662e-06),
        cmplx(-4.70546e-06,-3.98253e-06), cmplx(2.23744e-05,2.45329e-06)};
    benchmark["inst_8_4_100_0_31"] = {cmplx(-1.12582e-05,6.73938e-06), cmplx(5.84793e-06,-6.60197e-06), cmplx(-5.53118e-06,1.13912e-05),
        cmplx(1.60148e-05,-1.06845e-06), cmplx(1.77797e-05,9.72003e-06)};
    benchmark["inst_7_5_100_5_26"] = {cmplx(-3.07386e-06,2.53827e-06), cmplx(-6.51143e-06,-2.31236e-06), cmplx(-1.1414e-06,-7.2411e-06),
        cmplx(2.26241e-06,-2.0497e-06), cmplx(4.57272e-06,-8.47909e-07)};
    
//    benchmark["25_26"] = {cmplx(0.000104529,-0.000158536), cmplx(0.00011983,5.54614e-05), cmplx(0.000158796,6.78702e-05),
//        cmplx(-1.13099e-05,-7.9362e-06), cmplx(4.02591e-05, -6.56695e-05)};
//    benchmark["25_101"] = {cmplx(-0.000162581, -1.55533e-05), cmplx(-8.97293e-06, -1.42227e-05), cmplx(6.07801e-05, -4.70146e-05),
//        cmplx(4.87014e-07, -0.000124961), cmplx(0.000202284, 0.000114519)};
//    benchmark["30_26"] = {cmplx(-2.88494e-05, -3.6528e-05), cmplx(1.01401e-05, 6.81786e-06), cmplx(5.24095e-06, 4.47689e-05),
//        cmplx(-6.54596e-06, -2.83312e-05), cmplx(1.06338e-05, 3.37055e-05)};
//    benchmark["30_101"] = {cmplx(-9.25684e-06, 5.20232e-05), cmplx(-3.33498e-05, 2.11104e-05), cmplx(-4.62867e-05, -1.58411e-05),
//        cmplx(-1.64932e-05, 3.05876e-05), cmplx(-1.56987e-05, -7.06633e-06)};
}

void SequentialSimulation::
Simulate(GenericQuantumState& amp,
         Circuit& circuit)
{
    PopulateBenchmarkMap();
    if (circuit.google) {
        if (!circuit.ClockCycleEmpty())
            circuit.GroupAlternateCycles();
        circuit.GroupSimilarGates();
    }
    
    int xCZ_gate_count = 0;
    if (config.sim_type != Config::SimType::FullState) {
        bitset<128> a_qubits_bitmask = 0, b_qubits_bitmask = 0;
        int num_q_a = 0, num_q_b = 0;
        if (config.sim_type == Config::SimType::LosslessH || config.sim_type == Config::SimType::Approx1CutH)
            HorizontalCut(a_qubits_bitmask, b_qubits_bitmask, num_q_a, num_q_b, circuit.GetNumQubits(), config.hcut);
        else
            VerticalCut(a_qubits_bitmask, b_qubits_bitmask, num_q_a, num_q_b, circuit.GetNumQubits(), config.vcut);
        
        xCZ_gate_count = circuit.MovexCZGates(a_qubits_bitmask, b_qubits_bitmask);
    }
    
    if (config.verbose)
        PrintSimSpecReport(amp, circuit, xCZ_gate_count);
    
    log << "Cycle \tRuntime \tMemory\t\tXEntropy\n";
    
    Time time;
    time.StartTime();
    string cz_path = "-";
    if (config.cz_num_bits) {
        amp.amps_of_interest.resize(config.indices.size(), 0);
       
        config.cz_path <<= config.czp_append_len;
        idx_size cz_paths_ex = 1ull << config.czp_append_len;
        idx_size total_bits = config.cz_num_bits + config.czp_append_len;
        for (idx_size cz_p = 0; cz_p < cz_paths_ex; ++cz_p) {
            cz_path = bitset<128>(config.cz_path + cz_p).to_string();
            cz_path = cz_path.substr(cz_path.size() - total_bits);

            amp.ResetAmpVector();
            Phase1Simulation(amp, circuit, cz_path);

            auto& idx = config.indices;
            if (config.dfs_length == 0)
                for(idx_size i = 0; i < idx.size(); ++i)
                    amp.amps_of_interest[i] += amp[idx[i]];
            
        }
        if (config.print_amp)
            for (idx_size i = 0; i < config.indices.size(); ++i)
                *(*config.mmap_obj)[i] += amp.amps_of_interest[i];
    }
    else
        Phase1Simulation(amp, circuit, cz_path);

    cout << endl;
    total_time += time.GetElapsedTime() - XE_time;
    
    ReportingAfterSim(amp, circuit);
}

bool SequentialSimulation::
SimulationLoop(GenericQuantumState &amp,
               Circuit &circuit,
               string& cz_path,
               const idx_size prefix_size,
               const idx_size gate_i)
{
    idx_size size = circuit.GetTotalNumGates();
    int total_circuit_qubits = circuit.GetNumQubits(), current_cycle = 0;
    auto gates = circuit.GetGates();
    bool terminate = false;
    
    Time time, cycle_time;
    idx_size i = gate_i;
    for (; i < size; ++i) {
        
        if (amp.GetGlobalFactorPower() > 100) {
            Time rescale_time;
            rescale_time.StartTime();
            
            if (config.curr_mode != Config::SimMode::Phase2)
                ++amp.count_of_category.rescale;
            amp.Rescale();
            
            amp.time_by_category.rescale += rescale_time.GetElapsedTime();
        }
        
        curr_gate = i;
        Gate& current_gate = circuit.GetGateFromIndex(i);
        if(current_gate.ids.front() == Gate::Type::Control ||
           current_gate.ids.back() == Gate::Type::T) {

            double cycle_elapsed_t = time.GetElapsedTime();
            for (int c = 0; c < (int)circuit.GetNumCycles(); ++c) {
                if (i < (idx_size)circuit.GateIndexForCycle(c)) {
                    current_cycle = c;
                    break;
                }
            }
            if (config.curr_mode != Config::SimMode::Phase2)
                amp.data_per_cycles.cycles.push_back(current_cycle);
            
            if (config.curr_mode != Config::SimMode::Phase2) {
#ifdef CosineSimilarityDoubled
                Time misc_time;
                misc_time.StartTime();
                amp.PrintProbabilities(config.prob_outfile, current_cycle);
                XE_time += misc_time.GetElapsedTime();
                
#endif
#ifdef XEDoubled
                Time misc_time;
                misc_time.StartTime();
                amp.PrintProbabilities(config.prob_outfile, current_cycle);
                XE_time += misc_time.GetElapsedTime();
                            xe_t_e.tv_usec - xe_t_b.tv_usec) / 1.e6;
                
#endif
#ifdef FidelityDoubled
                Time misc_time;
                misc_time.StartTime();
                amp.PrintStateVector(config.amp_outfile, current_cycle);
               XE_time += misc_time.GetElapsedTime();
                
#endif
            }
            if (current_gate.ids.back() == Gate::Type::T ||
                current_gate.ids.back() == Gate::Type::Z) {
                
                Time CZT_time;
                CZT_time.StartTime();
                
                double cross_entropy = 0;
                
                if (config.verbose == 4 && config.curr_mode == Config::SimMode::Phase1) {
                    Time misc_time;
                    misc_time.StartTime();
                    cross_entropy = amp.CalculateCrossEntropy(10);
                    XE_time += misc_time.GetElapsedTime();
                    
                    log <<  setprecision(3) ;
                    log << current_cycle << "\t" << cycle_elapsed_t << " s    \t";
                    double memory = amp.GetMemUsage();
                    if (memory >= (1 << 30)) {
                        log << memory / (1 << 30) << " GiB";
                    }
                    else if (memory >= (1 << 20)) {
                        log << memory / (1 << 20) << " MiB";
                    }
                    else if (memory >= (1 << 10)) {
                        log << memory / (1 << 10) << " KiB";
                    }
                    else
                        log << memory << " B";
                    
                    if (cross_entropy)
                        log << "\t\t" << cross_entropy ;
                    log << "\n";
                }
                
                cycle_time.StartTime();
                idx_size prev_i = i;
                bitset<128> T_bitmasks[2] = {0};
                bitset<128> CZ_bitmasks[total_circuit_qubits];
                amp.FormCZTGatesBitmask(CZ_bitmasks, T_bitmasks, i, gates, total_circuit_qubits);
                int last_xCZ_idx = amp.ApplyBlockOfDiagGates(cz_path, prefix_size, CZ_bitmasks, T_bitmasks);
                terminate = last_xCZ_idx != -1 ? true : false;
                
                if (config.curr_mode != Config::SimMode::Phase2) {
                    if (config.sim_type == Config::SimType::FullState) {
                        amp.data_per_cycles.xCZ_H.push_back(0);
                        amp.data_per_cycles.xCZ_V.push_back(0);
                    }
                    
                    if (!terminate) {
                        amp.count_of_category.CZ_T += i - prev_i - amp.count_of_category.xCZ_not_applied;
                        amp.data_per_cycles.T_gates.push_back(T_bitmasks[0].count() + T_bitmasks[1].count());
                        amp.data_per_cycles.CZ_gates.push_back(i - prev_i -
                                                               amp.data_per_cycles.T_gates.back() -
                                                               amp.count_of_category.xCZ_not_applied);
                    }
                    else if (last_xCZ_idx) {
                        amp.count_of_category.CZ_T += last_xCZ_idx;
                        amp.data_per_cycles.CZ_gates.push_back(last_xCZ_idx); //not accurate
                    }
                }
                
                if (terminate) {
                    curr_gate = prev_i + last_xCZ_idx;
                    return terminate;
                }
                
                i -= 1;
                
                CZ_T_top_time += CZT_time.GetElapsedTime();
        }
            else {
                amp.ApplyCGate(current_gate.num_controls, current_gate.qubits,
                               current_gate, (Gate::Type)current_gate.ids.back());
            }
        }
        else {
            if (circuit.google && i < circuit.GetTotalNumGates() - 1 &&
                (current_gate.ids.back() == Gate::Type::X_1_2 ||
                 current_gate.ids.back() == Gate::Type::Y_1_2) &&
                (circuit.GetGateFromIndex(i + 1).ids.back() == Gate::Type::Y_1_2 ||
                 circuit.GetGateFromIndex(i + 1).ids.back() == Gate::Type::X_1_2)) {
                    
                    Time XY_time;
                    XY_time.StartTime();
                    
                    idx_size prev_i = i;
                    
                    bitset<128> X_bitmask = amp.FormXYGatesBitmask(i, gates, Gate::Type::X_1_2);
                    bitset<128> Y_bitmask = amp.FormXYGatesBitmask(i, gates, Gate::Type::Y_1_2);
                    amp.ApplyXYRecursiveTransform(X_bitmask, Y_bitmask, config.th);
                    
                    if (config.curr_mode != Config::SimMode::Phase2) {
                        amp.count_of_category.merged_XY1_2 += i - prev_i;
                        amp.data_per_cycles.XY_gates.push_back(i - prev_i);
                    }
                    --i;

                    X_Y_top_time += XY_time.GetElapsedTime();
                }
            else if(circuit.google && current_gate.ids.back() == Gate::Type::Hadamard) {
                
                cycle_time.StartTime();
                amp.ApplyHGateOnAllAmps();
                if (config.curr_mode != Config::SimMode::Phase2)
                    amp.count_of_category.H += circuit.GetNumQubits();
                i += total_circuit_qubits - 1;
            }
            else {
                Time single_xy_time;
                single_xy_time.StartTime();
                amp.ApplyNonCGate(current_gate.qubits[0],
                                  (Gate::Type)current_gate.ids.back(),  current_gate);
                double xy_single_elapsed_t = single_xy_time.GetElapsedTime();
                
                if (config.curr_mode != Config::SimMode::Phase2)
                    amp.data_per_cycles.XY_gates.push_back(1);
                if (current_gate.ids.back() == Gate::Type::X_1_2) {
                    ++amp.count_of_category.X1_2;
                    amp.time_by_category.X1_2 += xy_single_elapsed_t;
                }
                if (current_gate.ids.back() == Gate::Type::Y_1_2) {
                    ++amp.count_of_category.Y1_2;
                    amp.time_by_category.Y1_2 += xy_single_elapsed_t;
                }
                ++amp.count_of_category.merged_XY1_2;
                //TODO: Need to be able to count different types of non-control gates.
            }
        }
    }
    
    curr_gate = circuit.GetTotalNumGates();
    return terminate;
}

void SequentialSimulation::
Phase1Simulation(GenericQuantumState& amp,
                 Circuit& circuit,
                 string& cz_path)
{
    static int exec = 0; ++exec;
    
    Time cz_path_time;
    cz_path_time.StartTime();
    bool terminate = SimulationLoop(amp, circuit, cz_path, config.dfs_length);
    phase1_time += cz_path_time.GetElapsedTime();
    
    if (terminate && config.dfs_length != 0)
        Phase2Simulation(amp, circuit, curr_gate);
    else if (exec == 1) {
        if (terminate && config.dfs_length == 0 && cz_path == "" && cz_path != "-") {
            cout << "Simulated " + to_string(curr_gate) + " gates, including "
            + to_string(amp.count_of_category.decomposed_CZ) + " xCZ gates. ";
            
            if (circuit.GetTotalNumGates() != curr_gate) {
                int current_cycle = 0;
                for (int c = 0; c < (int)circuit.GetNumCycles(); ++c) {
                    if (curr_gate < (idx_size)circuit.GateIndexForCycle(c)) {
                        current_cycle = c;
                        break;
                    }
                }
                cout << "CZpath exhausted early.\n";// Simulated " << current_cycle << " out of " << config.depth << " cycles\n";
            }
            else {
                cout << "No xCZ gates left\n";
                cout << "Truncated CZ path : " << cz_path.size() << "\n";
            }
        }
        else if (!terminate && cz_path == "") {
            cout << "Simulated " + to_string(curr_gate) + " gates, including "
            + to_string(amp.count_of_category.decomposed_CZ) + " xCZ gates. No xCZ gates left.\n";
            if (config.dfs_length != 0)
                cout << "Truncated DFS length : " << config.dfs_length << "\n";
        }
        else if (!terminate && cz_path != "-") {
            cout << "Truncated CZ path : " << cz_path.size() << "\n";
            if (config.dfs_length != 0)
                cout << "Truncated DFS length : " << config.dfs_length << "\n";
        }
    }
}

void SequentialSimulation::
Phase2Simulation(GenericQuantumState& amp,
                 Circuit& circuit,
                 const idx_size gate_i)
{
    static int exec = 0; ++exec;
    
    Time phase2_time;
    phase2_time.StartTime();
    
//    bool terminate = false;
    amp.RescaleAndApplyGlobalICounter();
    
    Time copy_time;
    string cz_path = "-";
    idx_size num_CZ_paths = 1ull << config.dfs_length;
    for (idx_size i = 0; i < num_CZ_paths; ++i) {
        curr_gate = gate_i;
        cz_path = bitset<1000>(i).to_string();
        cz_path = cz_path.substr(cz_path.size() - config.dfs_length);
        
        copy_time.StartTime();
        SumOfTensorsProductsStateVector temp_amp ((SumOfTensorsProductsStateVector&)amp);
        amp.time_by_category.copying += copy_time.GetElapsedTime();
        ++amp.count_of_category.copying;
        
        SimulationLoop(temp_amp, circuit, cz_path, 0, gate_i);
                
        auto& idx = config.indices;
        for(idx_size i = 0; i < idx.size(); ++i)
            amp.amps_of_interest[i] += temp_amp[idx[i]];
        
        config.curr_mode = Config::SimMode::Phase2;
        amp.sim_mode = Config::SimMode::Phase2;
    }
    
    dfs_time += phase2_time.GetElapsedTime();
    
    config.curr_mode = Config::SimMode::Phase1;
    amp.sim_mode = Config::SimMode::Phase1;
    if (exec == 1) {
        if (!cz_path.size()) {
            if (circuit.GetTotalNumGates() != curr_gate)
                cout << "DFS path exhausted early";
            else
                cout << "No xCZ gates left\n";
        }
        else
            cout << "Truncated DFS length : " << cz_path.size() << "\n";
    }
}

void SequentialSimulation::
ReportingAfterSim(GenericQuantumState& amp,
                  Circuit& circuit)
{
#ifdef Print
//    amp.PrintStateVector();
#endif
#ifdef CosineSimilarity
    amp.PrintProbabilities(config.prob_outfile, circuit.GetNumCycles() - 1);
#endif
#ifdef PorterThomas
    amp.PrintProbabilities(config.prob_outfile, circuit.GetNumCycles() - 1);
#endif
#ifdef ReportToFile
    if (config.verbose)
        PrintReportToFile(amp, circuit);
#else
    if (config.verbose)
        PrintSimReport(amp, circuit);
#endif
    
    if (config.print_amp) {
      if (config.cz_num_bits) {
        if (!config.ascii)
            config.mmap_obj -> WriteToDisk();
        else
            WriteMmapToASCIIFile();
      }
     else
        WriteAmpToASCIIFile(amp);
    }
    
}

void SequentialSimulation::
 WriteMmapToASCIIFile() const
{
    string amp_outfile ="output/amp_vectors/" + config.infile + "_" + to_string(config.depth)
    + "_" + to_string(config.cz_num_bits + config.czp_append_len) + "_" + to_string(config.num_threads) +
    config.amp_outfile.substr(config.amp_outfile.find_last_of("/")) + "_ascii.amps";
    ofstream amp_out;
    amp_out.open(amp_outfile);
    
    auto& idx_print = config.indices;
    
    for (idx_size i = 0; i < idx_print.size() - 5; ++i) {
        amp_out << real(*(*config.mmap_obj)[i]);
        if (imag(*(*config.mmap_obj)[i]) < 0) amp_out << imag(*(*config.mmap_obj)[i]) << "j";
        else amp_out << "+" << imag(*(*config.mmap_obj)[i]) << "j";
        amp_out << "\n";
    }
    amp_out.close();
}

void SequentialSimulation::
WriteAmpToASCIIFile(const GenericQuantumState& amp) const
{
    string dir = "output/amp_vectors/" + config.infile + "_" + to_string(config.depth)
    + "_" + to_string(config.cz_num_bits + config.czp_append_len) + "_" + to_string(config.num_threads);
    string command = "mkdir -p " + dir;
    system(command.c_str());
    auto time = to_string(clock());
    string amp_outfile = dir + config.amp_outfile.substr(config.amp_outfile.find_last_of("/"))  + ".amps";
    ofstream amp_out;
    amp_out.open(amp_outfile);
    
    auto& idx_print = config.indices;
    
    for (idx_size i = 5; i < idx_print.size(); ++i) {
        amp_out << real(amp[idx_print[i]]);
        if (imag(amp[idx_print[i]]) < 0) amp_out << imag(amp[idx_print[i]]) << "j";
        else amp_out << "+" << imag(amp[idx_print[i]]) << "j";
        amp_out << "\n";
    }
    amp_out.close();
}

//void SequentialSimulation::
//AddToExternallyStoredAmps(const GenericQuantumState& amp) const
//{
//    auto does_file_exist = [](const char *fileName)
//    {
//        std::ifstream infile(fileName);
//        return infile.good();
//    };
//
//    string amp_outfile = config.amp_outfile + "/output_" + to_string(config.indices.size() - 5) + ".amps";
//    if (does_file_exist (amp_outfile.c_str())) {
//        idx_size size = config.indices.size() * sizeof(cmplx);
//
//        int fd = open(amp_outfile.c_str(), O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
//        cmplx *map = (cmplx*)mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
//        if (map == MAP_FAILED)
//        {
//            close(fd);
//            perror("Error mmapping the file");
//            exit(EXIT_FAILURE);
//        }
//
//        auto& idx_print = config.indices;
//        for (idx_size i = 5; i < idx_print.size(); ++i)
//            map[i] += amp[idx_print[i]];
//
//        // Write it now to disk
//        if (msync(map, size, MS_SYNC) == -1)
//        {
//            perror("Could not sync the file to disk");
//        }
//
//    }
//    else {
//
//        string command = "mkdir -p " + config.amp_outfile;
//        system(command.c_str());
//        string idx_outfile = config.amp_outfile + "/output_" + to_string(config.indices.size() - 5) + ".idx";
//        ofstream idx_out;
//
//        if (config.print_idx)
//            idx_out.open(idx_outfile);
//
//        auto& idx_print = config.indices;
//        cmplx amps_w[idx_print.size() - 5];
//        if (config.print_idx)
//            for (idx_size i = 5; i < idx_print.size(); ++i)
//                idx_out << idx_print[i].to_ullong() << "\n";
//
//        FILE *amp_file;
//        amp_file = fopen(amp_outfile.c_str(), "wb");  // w for write, b for binary
//        fwrite(amps_w, sizeof(amps_w), 1, amp_file);
//
//        if (config.print_idx)
//            idx_out.close();
//    }
//}

void SequentialSimulation::
PrintSystemReport() const
{
    cout << "\n(C) 2017, 2018  Regents of the University of Michigan\n";
    cout << "Rollright ver 1.7 - a quantum circuit simulator\n";
    cout << "Igor L. Markov and Aneeqa Fatima\n\n";
    
    char hostname[30] = {};
    gethostname(hostname, 30);
    cout << "Hostname : " << string(hostname) << "\n";
    
    
    if (config.verbose != Config::Verbose::NCCV && config.verbose != Config::Verbose::NCC) {
#ifdef __APPLE__
        cout << "CPU model name : "; flush(cout);
        system("sysctl -n machdep.cpu.brand_string");
        cout << "CPU cores : "; flush(cout);
        system("sysctl -n machdep.cpu.core_count");
        cout << "Hardware threads : "; flush(cout);
        system("sysctl -n machdep.cpu.thread_count");
#ifdef Parallel
        flush(cout);
        cout << "Max threads per process : " << config.num_threads << "\n";
#endif
        cout << "L2 cache size : "; flush(cout);
        system("sysctl -n hw.l2cachesize");
        cout << "L3 cache size : "; flush(cout);
        system("sysctl -n hw.l3cachesize");
#endif
#ifndef __APPLE__
        cout << "CPU "; flush(cout);
        system("egrep CPU /proc/cpuinfo | head -1");
        system("egrep cores /proc/cpuinfo | head -1");
        cout << "Hardware threads : "; flush(cout);
        system("egrep cores  /proc/cpuinfo | wc -l");
        #ifdef Parallel
        cout << "Max threads per process : " << config.num_threads << "\n";
        flush(cout);
        #endif
        system("grep \"MemTotal\" /proc/meminfo");
        cout << "L3 " ; flush(cout);
        system("egrep cache /proc/cpuinfo | head -1");
        system("grep \"Hugepagesize:\" /proc/meminfo");
        system("grep \"HugePages_Total\" /proc/meminfo");
        system("grep \"HugePages_Free\" /proc/meminfo");
#endif
        cout << "CPU instructions width :"
        << " popcnt:" << __builtin_cpu_supports("popcnt");
        
        if (__builtin_cpu_supports("sse4.2"))
            cout << ", sse4.2:" << __builtin_cpu_supports("sse4.2");
        else if (__builtin_cpu_supports("sse4.1"))
            cout << ", sse4.1:" << __builtin_cpu_supports("sse4.1");
        else if (__builtin_cpu_supports("sse4.1"))
            cout << ", sse4.1:" << __builtin_cpu_supports("sse4.1");
        else if (__builtin_cpu_supports("ssse3"))
            cout << ", ssse3:" << __builtin_cpu_supports("ssse3");
        else if (__builtin_cpu_supports("sse3"))
            cout << ", sse3:" << __builtin_cpu_supports("sse3");
        else if (__builtin_cpu_supports("sse2"))
            cout << ", sse2:" << __builtin_cpu_supports("sse2");
        else if (__builtin_cpu_supports("sse"))
            cout << ", sse:" << __builtin_cpu_supports("sse");
        
        cout << ", avx:" << __builtin_cpu_supports("avx")
        << ", avx2:" << __builtin_cpu_supports("avx2") << "\n";
        cout << "Using instructions : " << "AVX-2, popcnt\n\n";
        
        cout << "Compiler : gcc " << __GNUC__  << "." << __GNUC_MINOR__ << "."
        <<  __GNUC_PATCHLEVEL__<< "\n";
        
        cout << "Compiled on : " <<  __DATE__ << " " << __TIME__ << "\n";
        time_t t = time(0);
        struct tm * now = localtime (&t);
        cout << "Executed on : "
        << (now->tm_mon + 1) << "/"
        <<  now->tm_mday << "/"
        << (now->tm_year + 1900) << " "
        <<  now->tm_hour << ":" << now->tm_min << ":"
        << std::setw(2) << std::setfill('0') << now->tm_sec
        <<"\n";
        cout << "Size of complex : " << sizeof(cmplx) << " B\n";
        cout << "Verbosity : " << config.verbose << "\n\n";
    }
}

void SequentialSimulation::
PrintSimSpecReport(const GenericQuantumState& amp,
                   const Circuit& circuit,
                   const int xCZ_gates)
{
    if (config.google) {
        cout << "Circuit file : " + config.infile + "\n";
        cout << "Circuit type : Google\n";
    }
    
    cout << "Qubits : " << circuit.GetNumQubits() << "  ";
    cout << "Gates : " << circuit.GetTotalNumGates() << "  ";
    cout << "Cycles : " << circuit.GetNumCycles() << "\n\n";
    
    idx_size log_count = 0;
    cout << "Simulation type : ";
    if (config.sim_type == Config::SimType::FullState) {
        cout << "full state-vector  \n";
        cout << "Recursion end-case(max) : " << config.th << " q\n";
    }
    else if (config.sim_type == Config::SimType::LosslessH) {
        cout << "sum of tensor products / single cut\n";
        cout << amp.log[log_count++] << " (" << xCZ_gates <<" xCZ)\n";
        cout << "Recursion end-case(max) : " << config.th << " q\n";
        cout << "Simulating xCZ gates : exactly\n";
        
    }
    else if (config.sim_type == Config::SimType::LosslessV) {
        cout << "sum of tensor products / single cut\n";
        cout << amp.log[log_count++] << " (" << xCZ_gates <<" xCZ)\n";
        cout << "Recursion end-case(max) : " << config.th << " q\n";
        cout << "Simulating xCZ gates : exactly\n";
    }
    else if (config.sim_type == Config::SimType::Approx1CutH) {
        cout << "tensor products / approx single cut\n";
        cout << amp.log[log_count++];
        cout << "Recursion end-case(max) : " << config.th << " q\n";
        cout << "Simulating xCZ gates : ignored\n";
    }
    else if (config.sim_type == Config::SimType::Approx1CutV) {
        cout << "tensor products / approx single cut\n";
        cout << amp.log[log_count++];
        cout << "Recursion end-case(max) : " << config.th << " q\n";
        cout << "Simulating xCZ gates : ignored\n";
    }
    else if (config.sim_type == Config::SimType::Approx2011) {
        cout << "tensor products / approx2011 \n";
        cout << amp.log[log_count++];
        cout << "Recursion end-case(max) : " << config.th << " q\n";
        cout << "Simulating xCZ gates : approx\n";
    }
    else if (config.sim_type == Config::SimType::Approx_i11i) {
        cout << "tensor products / approx-i11i \n";
        cout << amp.log[log_count++];
        cout << "Recursion end-case(max) : " << config.th << " q\n";
        cout << "Simulating xCZ gates : approx\n";
    }
    else if (config.sim_type == Config::SimType::ApproxOWT) {
        cout << "sum of tensor products / approx 2 cuts \n";
        cout << amp.log[log_count++];
        cout << amp.log[log_count++];
        cout << "Recursion end-case(max) : " << config.th << " q\n";
        cout << "Simulating xCZ gates : approx\n";
       
    }
    else if (config.sim_type == Config::SimType::Approx2011OWT) {
        cout << "sum of tensor products / approx 2 cuts (2011) \n";
        cout << amp.log[log_count++];
        cout << amp.log[log_count++];
        cout << "Recursion end-case(max) : " << config.th << " q\n";
        cout << "Simulating xCZ gates : approx\n";
        
    }
    else if (config.sim_type == Config::SimType::Approx_i11iOWT) {
        cout << "sum of tensor products / approx 2 cuts (-i11i) \n";
        cout << amp.log[log_count++];
        cout << amp.log[log_count++];
        cout << "Recursion end-case(max) : " << config.th << " q\n";
        cout << "Simulating xCZ gates : approx\n";
        
    }
    
    if (config.cz_num_bits) {
        cout << "xCZ path breakdown : " << config.cz_num_bits << "p" ;
        if (config.czp_append_len)
            cout << " + " << config.czp_append_len << "r";
        if (config.dfs_length)
            cout << " + " << config.dfs_length << "b";
        cout << "\n";
        if (config.czp_append_len) {
            cout << "xCZ paths: ";
            
            const int total_bits = config.cz_num_bits + config.czp_append_len + config.dfs_length;
            const idx_size czb_v = config.cz_path << (config.czp_append_len + config.dfs_length);
            const idx_size cze_v = czb_v + (1ull << (config.czp_append_len + config.dfs_length)) - 1;
            const string czb = bitset<128>(czb_v).to_string();
            const string cze = bitset<128>(cze_v).to_string();
            cout << "[" << czb.substr(czb.size() - total_bits) << " (" << czb_v << "), "
            << cze.substr(cze.size() - total_bits) << " (" << cze_v << "))";
        }
        else {
            const string czp = bitset<128>(config.cz_path).to_string();
            cout << "CZ path : " << czp.substr(czp.size() - config.cz_num_bits)
            << " (" << config.cz_num_bits << ")";
        }
        
        cout << "\n";
        config.verbose = Config::Verbose::NCC;
    }
//    else
//       cout << "None\n";
}

void SequentialSimulation::
PrintSimReport(GenericQuantumState& amp,
               const Circuit& circuit) const
{
//    if (amp.GetGlobalFactorPower())
        amp.RescaleAndApplyGlobalICounter();
//    cout << "\n";
    
    if (config.sim_type == Config::SimType::ApproxOWT || config.sim_type == Config::SimType::Approx2011OWT
        || config.sim_type == Config::SimType::Approx_i11iOWT)
        amp.Normalize();
    
    idx_size temp_amp_size = amp.GetFullStateVectorSize();
    
    {
        ostringstream ss (ostringstream::ate);
        ss << setprecision(3);
        double norm = amp.CalculateNormSquared();
        double avg_inacc = amp.CalculateAverageInaccuracy(norm);

        double memory = amp.GetMemUsage();
        ss << "State representation size : ";

        if (memory >= (1 << 30)) {
            ss << memory / (1 << 30) << " GiB \n";
        }
        else if (memory >= (1 << 20)) {
            ss << memory / (1 << 20) << " MiB \n";
        }
        else if (memory >= (1 << 10)) {
            ss << memory / (1 << 10) << " KiB \n";
        }
        else
            ss << memory << " B \n";
        
        if (config.verbose >= Config::Verbose::Default) {
            ss << "Norm";
            if (amp.GetNumAddends() > 1)
                ss << "(assuming orthogonal addends)";
            ss << " : " << sqrt(norm) << "\n";
            
            double min = amp.GetMinProb();
            if (sqrt(norm) > 0.9) {
                ss << "Mean entropy : " <<  amp.CalculateMeanEntropy() << " ";
                ss << "Cross entropy : " <<  amp.CalculateCrossEntropy(sampling_factor) << "\n";
            }
            ss << "Probabilities : " << amp.GetMinProb() << "(min), "
                 << amp.GetMaxProb() << "(max), "
                 << amp.GetAvgProb() << "(avg)\n";
            
            if (min)
               ss << "Log_2 (max / min) = " << log2(real(amp.GetMaxProb())/real(amp.GetMinProb())) << "\n" ;
            if (norm >= 0.9)
                ss << "Avg inaccuracy per probability > " << avg_inacc << " ("<< (avg_inacc/amp.GetAvgProb()) * 100 << "%)\n";
            
            idx_size zero_amps = amp.CountZeroAmp();
            if (zero_amps)
                ss << "Stored zero amplitudes : " << (double(zero_amps)/double(temp_amp_size)) * 100 << "%\n";
        }
        ss << "\n";
        cout << ss.str();
    }
    
    if (!config.cz_num_bits) {
        
        string key = config.infile + "_" + to_string(circuit.GetNumCycles());
        cout << "Correctness check : ";
    
        if (benchmark.count(key)) {
            if (real(amp[3]) - real(benchmark[key][0]) < 1e-9
                && imag(amp[3]) - imag(benchmark[key][0]) < 1e-9
                && real(amp[temp_amp_size/4]) - real(benchmark[key][1]) < 1e-9
                && imag(amp[temp_amp_size/4]) - imag(benchmark[key][1]) < 1e-9
                && real(amp[temp_amp_size/2]) - real(benchmark[key][2]) < 1e-9
                && imag(amp[temp_amp_size/2]) - imag(benchmark[key][2]) < 1e-9
                && real(amp[3 * temp_amp_size/4]) -  real(benchmark[key][3]) < 1e-9
                && imag(amp[3 * temp_amp_size/4]) -  imag(benchmark[key][3]) < 1e-9
                && real(amp[temp_amp_size - 3]) - real(benchmark[key][4]) < 1e-9
                && imag(amp[temp_amp_size - 3]) - imag(benchmark[key][4]) < 1e-9)
            {
                cout << "passed\n";
            }
            else {
                cout << "failed\nCorrect results: \n";
                if (config.verbose != Config::Verbose::NCCV) {
                    string imag0 = to_string(imag(benchmark[key][0])), imag1 = to_string(imag(benchmark[key][1])),
                    imag2 = to_string(imag(benchmark[key][2])), imag3 = to_string(imag(benchmark[key][3])),
                    imag4 = to_string(imag(benchmark[key][4]));
                    
                    cout << "amp[3]  \t= "  << real(benchmark[key][0]) ;
                    if (imag(benchmark[key][0]) < 0)
                        cout << imag(benchmark[key][0]) << "j\n";
                    else
                        cout << "+" << imag(benchmark[key][0]) << "j\n";
                    
                    cout << "amp[1/4]\t= " << real(benchmark[key][1]);
                    if (imag(benchmark[key][1]) < 0)
                        cout << imag(benchmark[key][1]) << "j\n";
                    else
                        cout << "+" << imag(benchmark[key][1]) << "j\n";
                    
                    cout << "amp[1/2]\t= " << real(benchmark[key][2]);
                    if (imag(benchmark[key][2]) < 0)
                        cout << imag(benchmark[key][2]) << "j\n";
                    else
                        cout << "+" << imag(benchmark[key][2]) << "j\n";
                    
                    cout << "amp[3/4]\t= " << real(benchmark[key][3]);
                    if (imag(benchmark[key][3]) < 0)
                        cout << imag(benchmark[key][3]) << "j\n";
                    else
                        cout << "+" << imag(benchmark[key][3]) << "j\n";
                    
                    cout << "amp[-3] \t= "  << real(benchmark[key][4]);
                    if (imag(benchmark[key][4]) < 0)
                        cout << imag(benchmark[key][4]) << "j\n";
                    else
                        cout << "+" << imag(benchmark[key][4]) << "j\n";
                    
                    cout << "Incorrect results: \n";
                }
            }
        }
        else
            cout << "no data available\n";
    }
    
    if (config.verbose != Config::Verbose::NCCV)
    {
        if (!amp.amps_of_interest.empty()) {
            auto amp3 = amp.amps_of_interest[0];
            cout << "amp[3]  \t= " << real(amp3);
            if (imag(amp3) < 0)
                cout << imag(amp3) << "j\n";
            else
                cout << "+" << imag(amp3) << "j\n";
            
            auto amp14 = amp.amps_of_interest[1];
            cout << "amp[1/4]\t= " << real(amp14);
            if (imag(amp14) < 0)
                cout << imag(amp14) << "j\n";
            else
                cout << "+" << imag(amp14) << "j\n";
            
            auto amp12 = amp.amps_of_interest[2];
            cout << "amp[1/2]\t= " << real(amp12);
            if (imag(amp12) < 0)
                cout << imag(amp12) << "j\n";
            else
                cout << "+" << imag(amp12) << "j\n";
            
            auto amp34 = amp.amps_of_interest[3];
            cout << "amp[3/4]\t= " << real(amp34);
            if (imag(amp34) < 0)
                cout << imag(amp34) << "j\n";
            else
                cout << "+" << imag(amp34) << "j\n";
            
            auto amp_3 = amp.amps_of_interest[4];
            cout << "amp[-3] \t= " << real(amp_3);
            if (imag(amp_3) < 0)
                cout << imag(amp_3) << "j\n";
            else
                cout << "+" << imag(amp_3) << "j\n";
            cout << "\n";
        }
        else {
            cout << "amp[3]  \t= " << real(amp[3]);
            if (imag(amp[3]) < 0)
                cout << imag(amp[3]) << "j\n";
            else
                cout << "+" << imag(amp[3]) << "j\n";
            
            cout << "amp[1/4]\t= " << real(amp[temp_amp_size/4]);
            if (imag(amp[temp_amp_size/4]) < 0)
                cout << imag(amp[temp_amp_size/4]) << "j\n";
            else
                cout << "+" << imag(amp[temp_amp_size/4]) << "j\n";
            
            cout << "amp[1/2]\t= " << real(amp[temp_amp_size/2]);
            if (imag(amp[temp_amp_size/2]) < 0)
                cout << imag(amp[temp_amp_size/2]) << "j\n";
            else
                cout << "+" << imag(amp[temp_amp_size/2]) << "j\n";
            
            cout << "amp[3/4]\t= " << real(amp[3 * temp_amp_size/4]);
            if (imag(amp[3 * temp_amp_size/4]) < 0)
                cout << imag(amp[3 * temp_amp_size/4]) << "j\n";
            else
                cout << "+" << imag(amp[3 * temp_amp_size/4]) << "j\n";
            
            cout << "amp[-3] \t= " << real(amp[temp_amp_size - 3]);
            if (imag(amp[temp_amp_size - 3]) < 0)
                cout << imag(amp[temp_amp_size - 3]) << "j\n";
            else
                cout << "+" << imag(amp[temp_amp_size - 3]) << "j\n";
            cout << "\n";
        }
    }

    {
        int factor = 1ull << config.czp_append_len;
        ostringstream ss (ostringstream::ate);
        ss << setprecision(3);
        ss << "Runtime (" << total_time << " s total) by category \n";
        
        string H_s = "\tH (" + to_string(amp.count_of_category.H/factor) +  ")";
        ss << H_s << setw(30 - H_s.size()) << right << ": " << amp.time_by_category.H
        << " s\t\t= " << (amp.time_by_category.H/total_time) * 100 << "%\n";

        if(amp.count_of_category.CZ_T - amp.count_of_category.decomposed_CZ) {
            string CZ_T_s = "\tCZ & T (" +
            to_string((amp.count_of_category.CZ_T - amp.count_of_category.decomposed_CZ)/factor) + ")" ;
            ss << CZ_T_s << setw(30 - CZ_T_s.size()) << right << ": "
            << amp.time_by_category.CZ_T << " s\t\t= "
            << (amp.time_by_category.CZ_T/(total_time)) * 100 << "%\n";
        }
        
        if(amp.count_of_category.decomposed_CZ) {
            string CZ_s = "\txCZ (" + to_string(amp.count_of_category.decomposed_CZ/factor) + ")" ;
            ss << CZ_s << setw(30 - CZ_s.size()) << right << ": "
            << amp.time_by_category.decomposed_CZ << " s\t\t= "
            << (amp.time_by_category.decomposed_CZ/(total_time)) * 100 << "%\n";
        }
        if (amp.count_of_category.X1_2 || amp.count_of_category.Y1_2) {
            string XY_s = "\tSingle X (" + to_string(amp.count_of_category.X1_2/factor)
            + ") & Y (" + to_string(amp.count_of_category.Y1_2/factor) + ")";
            ss << XY_s << setw(30 - XY_s.size()) << right << ": "
            << (amp.time_by_category.X1_2 +  amp.time_by_category.Y1_2) << " s\t\t= "
            << ((amp.time_by_category.X1_2 +  amp.time_by_category.Y1_2)/(total_time)) * 100 << "%\n";
        }
        
        if (amp.count_of_category.merged_XY1_2
            && (amp.count_of_category.merged_XY1_2 - amp.count_of_category.Y1_2 - amp.count_of_category.X1_2)) {
            string X_Y_s = "\tMerged X & Y ("
            + to_string((amp.count_of_category.merged_XY1_2 -
                         amp.count_of_category.Y1_2 - amp.count_of_category.X1_2)/factor) + ")";
            ss << X_Y_s << setw(30 - X_Y_s.size()) << right << ": " << amp.time_by_category.merged_XY1_2
            << " s\t\t= " << (amp.time_by_category.merged_XY1_2/(total_time)) * 100 << "%\n";
        }
        if (amp.count_of_category.rescale) {
            string RP_s = "\tRescaling passes (" + to_string(amp.count_of_category.rescale/factor) + ")";
            ss <<  RP_s << setw(30 - RP_s.size()) << right << ": " << (amp.time_by_category.rescale)
            << " s\t\t= " << (amp.time_by_category.rescale/(total_time)) * 100 << "%\n";
        }
        
        if (amp.time_by_category.conversion) {
            string RP_s = "\tConversion ";
            ss <<  RP_s << setw(30 - RP_s.size()) << right << ": "
            << amp.time_by_category.conversion<< " s\t\t= "
            << (amp.time_by_category.conversion/(total_time)) * 100 << "%\n";
        }
        
        if (amp.time_by_category.copying) {
            string RP_s = "\tCopying (" + to_string(amp.count_of_category.copying/factor) + ")";
            ss <<  RP_s << setw(30 - RP_s.size()) << right << ": "
            << amp.time_by_category.copying << " s\t\t= "
            << (amp.time_by_category.copying/(total_time)) * 100 << "%\n";
        }
        
        double sum_percen = ((amp.time_by_category.H/total_time) * 100) + ((amp.time_by_category.CZ_T/total_time) * 100)
        + ((amp.time_by_category.decomposed_CZ/total_time) * 100)
        + (((amp.time_by_category.X1_2 +  amp.time_by_category.Y1_2)/total_time) * 100)
        + ((amp.time_by_category.merged_XY1_2/total_time) * 100) + ((amp.time_by_category.rescale/total_time) * 100)
        + ((amp.time_by_category.conversion/total_time) * 100) + ((amp.time_by_category.copying/total_time) * 100);
        
        ss << "\t\t\t\t\t\t\t  ----\n";
        ss << "\tTotal \t\t\t\t\t\t   " << sum_percen << "%\n";
        
        ss << "\nAverage time per gate : " << total_time/(factor * circuit.GetTotalNumGates()) << " s\n";
        
        if (config.cz_num_bits) {
            ss << "Simulation runtime breakdown : \n\tPhase 1 : " << phase1_time << " s = "
             << (phase1_time/(total_time)) * 100 << "%\n";
        }
        if (config.dfs_length != 0) {
            ss << "\tPhase 2 : " << dfs_time << " s = "
            << (dfs_time/(total_time)) * 100 << "%\n";
        }
        cout << ss.str() << "\n";
    }
    
    if (config.verbose == Config::Verbose::Cycles) {
        cout << log.str() << "\n";
        cout << "Cycle\t xCZ\t Addends\t  Memory\n";
        for (idx_size c = 0; c < amp.data_per_cycles.memory.size(); ++c) {
            cout << amp.data_per_cycles.cycles[c] << "\t "
            << amp.data_per_cycles.xCZ_H[c] << "," << amp.data_per_cycles.xCZ_V[c] << "\t "
            << amp.data_per_cycles.addends[c] << "\t  " ;
            if (amp.data_per_cycles.memory[c] >= (1 << 30)) {
                cout << amp.data_per_cycles.memory[c] / (1 << 30) << " GiB \n";
            }
            else if (amp.data_per_cycles.memory[c] >= (1 << 20)) {
                cout << amp.data_per_cycles.memory[c] / (1 << 20) << " MiB \n";
            }
            else if (amp.data_per_cycles.memory[c] >= (1 << 10)) {
                cout << amp.data_per_cycles.memory[c] / (1 << 10) << " KiB \n";
            }
            else
                cout << amp.data_per_cycles.memory[c] << " B \n";
        }
    }
    
    cout << "¯\\_(ツ)_/¯ \n\n";
}

void SequentialSimulation::
PrintReportToFile(GenericQuantumState& amp,
                  const Circuit& circuit) const
{
}

