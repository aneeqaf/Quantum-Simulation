
//  simulation.cpp
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 12/12/17.
//

#include "simulation.h"

unordered_map<string, array<cmplx, 5>> SequentialSimulation::benchmark = {};

SequentialSimulation::
SequentialSimulation(const Config& c): total_time(0), config(c) {}

void SequentialSimulation::
PopulateBenchmarkMap()
{
    benchmark["25_26"] = {cmplx(0.000104529,-0.000158536), cmplx(0.00011983,5.54614e-05), cmplx(0.000158796,6.78702e-05),
        cmplx(-1.13099e-05,-7.9362e-06), cmplx(4.02591e-05, -6.56695e-05)};
    benchmark["25_101"] = {cmplx(-0.000162581, -1.55533e-05), cmplx(-8.97293e-06, -1.42227e-05), cmplx(6.07801e-05, -4.70146e-05),
        cmplx(4.87014e-07, -0.000124961), cmplx(0.000202284, 0.000114519)};
    benchmark["30_26"] = {cmplx(-2.88494e-05, -3.6528e-05), cmplx(1.01401e-05, 6.81786e-06), cmplx(5.24095e-06, 4.47689e-05),
        cmplx(-6.54596e-06, -2.83312e-05), cmplx(1.06338e-05, 3.37055e-05)};
    benchmark["30_101"] = {cmplx(-9.25684e-06, 5.20232e-05), cmplx(-3.33498e-05, 2.11104e-05), cmplx(-4.62867e-05, -1.58411e-05),
        cmplx(-1.64932e-05, 3.05876e-05), cmplx(-1.56987e-05, -7.06633e-06)};
    benchmark["35_26"] = {cmplx(2.26241e-06, -2.0497e-06), cmplx(-2.16094e-06, -4.74918e-06), cmplx(2.07073e-06, -1.09369e-06),
        cmplx(-3.07386e-06, 2.53828e-06), cmplx(2.9415e-06, 4.65522e-06)};
}

void SequentialSimulation::
Simulate(GenericQuantumState& amp,
         Circuit& circuit)
{
    PopulateBenchmarkMap();
    idx_size size = circuit.GetTotalNumGates();
    int total_circuit_qubits = circuit.GetNumQubits(), current_cycle = 0;
    double XE_time = 0;
    
    if (circuit.google) {
        if (!circuit.ClockCycleEmpty())
            circuit.GroupAlternateCycles();
        circuit.GroupSimilarGates();
    }
    auto gates = circuit.GetGates();
    
    log << "Cycle \tRuntime \tMemory\t\tXEntropy\n";
    
    struct timeval start_p, end_p, cycle_start, xe_t_b, xe_t_e;
    gettimeofday(&start_p, NULL);
    for (idx_size i = 0; i < size; ++i) {
        
        if (amp.GetGlobalFactorPower() > 100) {
            clock_t begin = clock();
            ++amp.count_of_category.rescale;
            amp.Rescale();
            clock_t end = clock();
            amp.time_by_category.rescale +=  double(end - begin) / CLOCKS_PER_SEC;
        }
        
        Gate& current_gate = circuit.GetGateFromIndex(i);
        if(current_gate.ids.front() == Gate::Type::Control ||
           current_gate.ids.back() == Gate::Type::T) {
            
            struct timeval cycle_end;
            gettimeofday(&cycle_end, NULL);
            for (int c = 0; c < circuit.GetNumCycles(); ++c) {
                if (i < circuit.GateIndexForCycle(c)) {
                    current_cycle = c;
                    break;
                }
            }
            amp.data_per_cycles.cycles.push_back(current_cycle);
#ifdef CosineSimilarityDoubled
            gettimeofday(&xe_t_b, NULL);
            amp.PrintProbabilities(config.prob_outfile, current_cycle);
            gettimeofday(&xe_t_e, NULL);
            XE_time += ((xe_t_e.tv_sec  - xe_t_b.tv_sec) * 1000000u +
                        xe_t_e.tv_usec - xe_t_b.tv_usec) / 1.e6;
            
#endif
#ifdef XEDoubled
            gettimeofday(&xe_t_b, NULL);
            amp.PrintProbabilities(config.prob_outfile, current_cycle);
            gettimeofday(&xe_t_e, NULL);
            XE_time += ((xe_t_e.tv_sec  - xe_t_b.tv_sec) * 1000000u +
                        xe_t_e.tv_usec - xe_t_b.tv_usec) / 1.e6;
            
#endif
#ifdef FidelityDoubled
            gettimeofday(&xe_t_b, NULL);
            amp.PrintStateVector(config.amp_outfile, current_cycle);
            gettimeofday(&xe_t_e, NULL);
            XE_time += ((xe_t_e.tv_sec  - xe_t_b.tv_sec) * 1000000u +
                        xe_t_e.tv_usec - xe_t_b.tv_usec) / 1.e6;
            
#endif

             if (current_gate.ids.back() == Gate::Type::T ||
                current_gate.ids.back() == Gate::Type::Z) {
                 
                 double cross_entropy = 0;

                 if (config.verbose == 4) {
                     gettimeofday(&xe_t_b, NULL);
                     cross_entropy = amp.CalculateCrossEntropy(100);
                     gettimeofday(&xe_t_e, NULL);
                     XE_time += ((xe_t_e.tv_sec  - xe_t_b.tv_sec) * 1000000u +
                                 xe_t_e.tv_usec - xe_t_b.tv_usec) / 1.e6;
                 
                     log <<  setprecision(3) ;
                     log << current_cycle << "\t"
                     << ((cycle_end.tv_sec  - cycle_start.tv_sec) * 1000000u +
                        cycle_end.tv_usec - cycle_start.tv_usec) / 1.e6 << " s    \t";
                     double memory = amp.GetMemUsage();
                     if (memory >= 1e9) {
                         log << memory / 1e9 << " GB";
                     }
                     else if (memory >= 1e6) {
                         log << memory / 1e6 << " MB";
                     }
                     else if (memory >= 1e3) {
                         log << memory / 1e3 << " KB";
                     }
                     else
                         log << memory << " B";
                     
                     if (cross_entropy)
                         log << "\t\t" << cross_entropy ;
                     log << "\n";
                 }
                 
                 gettimeofday(&cycle_start, NULL);
                 idx_size prev_i = i;
                 idx_size T_bitmasks[2] = {0};
                 idx_size CZ_bitmasks[total_circuit_qubits];
                 amp.FormCZTGatesBitmask(CZ_bitmasks, T_bitmasks, i, gates, total_circuit_qubits);
                 amp.ApplyBlockOfDiagGates(CZ_bitmasks, T_bitmasks);
    
                 amp.count_of_category.CZ_T += i - prev_i;
                 if (config.sim_type == Config::SimType::FullState) {
                     amp.data_per_cycles.xCZ_H.push_back(0);
                     amp.data_per_cycles.xCZ_V.push_back(0);
                 }
                 amp.data_per_cycles.T_gates.push_back(
                                __builtin_popcountll(T_bitmasks[0]) + __builtin_popcountll(T_bitmasks[1]));
                 amp.data_per_cycles.CZ_gates.push_back(i - prev_i - amp.data_per_cycles.T_gates.back());
                 
                 i -= 1;
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
                     idx_size prev_i = i;

#ifdef ManualMerging
                      amp.ApplyMergedXYGate(gates[i], gates[i + 1] total_circuit_qubits);
                      i += 2;
#endif
#ifdef RT
                     idx_size X_bitmask = amp.FormXYGatesBitmask(i, gates, Gate::Type::X_1_2);
                     idx_size Y_bitmask = amp.FormXYGatesBitmask(i, gates, Gate::Type::Y_1_2);
                     amp.ApplyXYRecursiveTransform(X_bitmask, Y_bitmask, config.th);
#endif
                     amp.count_of_category.merged_XY1_2 += i - prev_i;
                     amp.data_per_cycles.XY_gates.push_back(i - prev_i);
                     --i;
             }
            else if(circuit.google && current_gate.ids.back() == Gate::Type::Hadamard) {
                gettimeofday(&cycle_start, NULL);
                amp.ApplyHGateOnAllAmps();
                amp.count_of_category.H = circuit.GetNumQubits();
                i += total_circuit_qubits - 1;
            }
            else {
                amp.ApplyNonCGate(current_gate.qubits[0],
                                  (Gate::Type)current_gate.ids.back(),  current_gate);
                amp.data_per_cycles.XY_gates.push_back(1);
                //TODO: Need to be able to count different types of non-control gates.
            }
        }
    }
    gettimeofday(&end_p, NULL);
    
    total_time = ((end_p.tv_sec  - start_p.tv_sec) * 1000000u +
             end_p.tv_usec - start_p.tv_usec) / 1.e6;

#ifdef Print
    amp.PrintStateVector();
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
        PrintReport(amp, circuit);
#endif
    
}

void SequentialSimulation::
PrintReport(GenericQuantumState& amp,
            const Circuit& circuit) const
{
    cout << "Rollright ver 1.2 - a quantum circuit simulator\n\n";

    char hostname[20] = {};
    gethostname(hostname, 20);
    cout << "Hostname : ";
    for(auto h : hostname) {
        cout << h;
    }
    cout << "\n";
    amp.Rescale();
    amp.ApplyGlobalICounter();
   
    if (config.verbose != Config::Verbose::NCCV && config.verbose != Config::Verbose::NCC) {
#ifdef __APPLE__
        cout << "CPU model name : "; flush(cout);
        system("sysctl -n machdep.cpu.brand_string");
        cout << "CPU cores : "; flush(cout);
        system("sysctl -n machdep.cpu.core_count");
        cout << "Hardware threads : "; flush(cout);
        system("sysctl -n machdep.cpu.thread_count");
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
        cout << "L3 " ; flush(cout);
        system("egrep cache /proc/cpuinfo | head -1");
    #endif
        cout << "CPU supports :"
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
             << ", avx2:" << __builtin_cpu_supports("avx2") << "\n\n";
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
        <<"\n\n";
    }
    
    if (config.google) {
        cout << "Circuit file : " + config.infile + "\n";
        cout << "Circuit type : Google\n";
    }
    
    cout << "Qubits : " << circuit.GetNumQubits() << "  ";
    cout << "Gates : " << circuit.GetTotalNumGates() << "  ";
    cout << "Cycles : " << circuit.GetNumCycles() << "\n\n";
    
    idx_size log_count = 0;
    cout << "Simulation type: ";
    if (config.sim_type == Config::SimType::FullState) {
        cout << "full state vector / lossless \n";
        cout << "xCZ gates simulated : exact\n";
    }
    else if (config.sim_type == Config::SimType::LosslessH) {
        cout << "sum of tensor products / losslessH \n";
        cout << "xCZ gates simulated : exact\n";
        cout << amp.log[log_count++];
    }
    else if (config.sim_type == Config::SimType::LosslessV) {
        cout << "sum of tensor products / losslessV \n";
        cout << "xCZ gates simulated : exact\n";
        cout << amp.log[log_count++];
    }
    else if (config.sim_type == Config::SimType::Approx1CutH) {
        cout << "tensor products / approx1CutH \n";
        cout << "xCZ gates simulated : ignored\n";
        cout << amp.log[log_count++];
    }
    else if (config.sim_type == Config::SimType::Approx1CutV) {
        cout << "tensor products / approx1CutV \n";
        cout << "xCZ gates simulated : ignored\n";
        cout << amp.log[log_count++];
    }
    else if (config.sim_type == Config::SimType::Approx2011) {
        cout << "tensor products / approx2011 \n";
        cout << "xCZ gates simulated : approx\n";
        cout << amp.log[log_count++];
    }
    else if (config.sim_type == Config::SimType::Approx2Cuts) {
        cout << "sum of tensor products / approx2Cuts\n";
        cout << "xCZ gates simulated : approx\n";
        cout << amp.log[log_count++];
        cout << amp.log[log_count++];
        amp.Normalize();
    }
    
    cout << "Size of complex : " << sizeof(cmplx) << " B\n";
    
    idx_size temp_amp_size = amp.GetFullStateVectorSize();
    
    {
        ostringstream ss (ostringstream::ate);
        ss << setprecision(3);
        double norm = amp.CalculateNormSquared();
        double avg_inacc = amp.CalculateAverageInaccuracy(norm);
#ifdef RT
        cout << "Recursion end-case(max) : " << config.th << " q\n";
#ifdef Parallel
        cout << "Number of threads : " << thread::hardware_concurrency() << "\n";
#else
        cout << "Number of threads : 1\n";
#endif
        cout << "Verbosity : " << config.verbose << "\n\n";
#endif
        double memory = amp.GetMemUsage();
        ss << "State representation size : ";

        if (memory >= 1e9) {
            ss << memory / 1e9 << " GB \n";
        }
        else if (memory >= 1e6) {
            ss << memory / 1e6 << " MB \n";
        }
        else if (memory >= 1e3) {
            ss << memory / 1e3 << " KB \n";
        }
        else
            ss << memory << " B \n";

        ss << "Norm ";
        if (config.sim_type != Config::SimType::FullState)
            ss << "(assuming orthogonal addends)";
        
        ss << " : " << sqrt(norm) << "\n";
        ss << "Mean entropy : " <<  amp.CalculateMeanEntropy() << " ";
        ss << "Cross entropy : " <<  amp.CalculateCrossEntropy(10) << "\n";
        ss << "Probabilities : " << amp.GetMinProb() << "(min), "
             << amp.GetMaxProb() << "(max), "
             << amp.GetAvgProb() << "(avg)\n";
        ss << "Log_2 (max / min) = " << log2(real(amp.GetMaxProb())/real(amp.GetMinProb())) << "\n" ;
        ss << "Avg inaccuracy per probability > " << avg_inacc << " ("<< (avg_inacc/amp.GetAvgProb()) * 100 << "%)\n\n";
        cout << ss.str();
    }
    
    
        string key = to_string(circuit.GetNumQubits()) + "_" + to_string(circuit.GetNumCycles());
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
                    if (imag(benchmark[key][0]) < 0) {
                        imag0[0] = ' ';
                        cout << " - " << imag0 << "i\n";
                    }
                    else
                        cout << " + " << imag0 << "i\n";
                    
                    cout << "amp[1/4]\t= " << real(benchmark[key][1]);
                    if (imag(benchmark[key][1]) < 0) {
                        imag1[0] = ' ';
                        cout << " - " << imag1 << "i\n";
                    }
                    else
                        cout << " + " << imag1 << "i\n";
                    
                    cout << "amp[1/2]\t= " << real(benchmark[key][2]);
                    if (imag(benchmark[key][2]) < 0) {
                        imag2[0] = ' ';
                        cout << " - " << imag2 << "i\n";
                    }
                    else
                        cout << " + " << imag2 << "i\n";
                    
                    cout << "amp[3/4]\t= " << real(benchmark[key][3]);
                    if (imag(benchmark[key][3]) < 0) {
                        imag3[0] = ' ';
                        cout << " - " << imag3 << "i\n";
                    }
                    else
                        cout << " + " << imag3 << "i\n";
                    
                    cout << "amp[-3] \t= "  << real(benchmark[key][4]);
                    if (imag(benchmark[key][4]) < 0) {
                        imag4[0] = ' ';
                        cout << " - " << imag4 << "i\n";
                    }
                    else
                        cout << " + " << imag4 << "i\n";
                    cout << "Incorrect results: \n";
                }
            }
        }
        else
            cout << "no data available\n";
    
    if (config.verbose != Config::Verbose::NCCV)
    {
        string imag0 = to_string(imag(amp[3])), imag1 = to_string(imag(amp[temp_amp_size/4])),
        imag2 = to_string(imag(amp[temp_amp_size/2])), imag3 = to_string(imag(amp[3 * temp_amp_size/4])),
        imag4 = to_string(imag(amp[temp_amp_size - 3]));
        
        cout << "amp[3]  \t= " << real(amp[3]);
        if (imag(amp[3]) < 0) {
            imag0[0] = ' ';
            cout << " - " << imag0 << "i\n";
        }
        else
            cout << " + " << imag0 << "i\n";
        cout << "amp[1/4]\t= " << real(amp[temp_amp_size/4]);
        if (imag(amp[temp_amp_size/4]) < 0) {
            imag1[0] = ' ';
            cout << " - " << imag1 << "i\n";
        }
        else
            cout << " + " << imag1 << "i\n";
        cout << "amp[1/2]\t= " << real(amp[temp_amp_size/2]);
        if (imag(amp[temp_amp_size/2]) < 0) {
            imag2[0] = ' ';
            cout << " - " << imag2 << "i\n";
        }
        else
            cout << " + " << imag2 << "i\n";
        cout << "amp[3/4]\t= " << real(amp[3 * temp_amp_size/4]);
        if (imag(amp[3 * temp_amp_size/4]) < 0) {
            imag3[0] = ' ';
            cout << " - " << imag3 << "i\n";
        }
        else
            cout << " + " << imag3 << "i\n";
        cout << "amp[-3] \t= " << real(amp[temp_amp_size - 3]);
        if (imag(amp[temp_amp_size - 3]) < 0) {
            imag4[0] = ' ';
            cout << " - " << imag4 << "i\n\n";
        }
        else
            cout << " + " << imag4 << "i\n\n";
    }

    {
        ostringstream ss (ostringstream::ate);
        ss << setprecision(3);
        ss << "Runtime (" << total_time << " s total) by category \n";
        
        string H_s = "     H (" + to_string(amp.count_of_category.H) +  ")";
        ss << H_s << setw(28 - H_s.size()) << right << ": " << amp.time_by_category.H
        << " s \t= " << (amp.time_by_category.H/total_time) * 100 << "%\n";

        if(amp.count_of_category.CZ_T) {
            string CZ_T_s = "     CZ & T (" +
            to_string(amp.count_of_category.CZ_T - amp.count_of_category.decomposed_CZ) + ")" ;
            ss << CZ_T_s << setw(28 - CZ_T_s.size()) << right << ": "
            << amp.time_by_category.CZ_T << " s \t= "
            << (amp.time_by_category.CZ_T/total_time) * 100 << "%\n";
        }
        
        if(amp.count_of_category.decomposed_CZ) {
            string CZ_s = "     Decomposed CZ (" + to_string(amp.count_of_category.decomposed_CZ) + ")" ;
            ss << CZ_s << setw(28 - CZ_s.size()) << right << ": "
            << amp.time_by_category.decomposed_CZ << " s \t= " << (amp.time_by_category.decomposed_CZ/total_time) * 100 << "%\n";
        }
        
        if (amp.count_of_category.X1_2 || amp.count_of_category.Y1_2) {
            string XY_s = "     Single X (" + to_string(amp.count_of_category.X1_2)
            + ") & Y (" + to_string(amp.count_of_category.Y1_2) + ")";
            ss << XY_s << setw(28 - XY_s.size()) << right << ": "
            << amp.time_by_category.X1_2 +  amp.time_by_category.Y1_2 << " s \t= "
            << ((amp.time_by_category.X1_2 +  amp.time_by_category.Y1_2)/total_time) * 100 << "%\n";
        }
        
        if (amp.count_of_category.merged_XY1_2) {
            string X_Y_s = "     Merged X & Y ("
            + to_string(amp.count_of_category.merged_XY1_2 - amp.count_of_category.Y1_2 - amp.count_of_category.X1_2) + ")";
            ss << X_Y_s << setw(28 - X_Y_s.size()) << right << ": " << amp.time_by_category.merged_XY1_2
            << " s \t= " << (amp.time_by_category.merged_XY1_2/total_time) * 100 << "%\n";
        }
        
        if (amp.count_of_category.rescale) {
            string RP_s = "     Rescaling passes (" + to_string(amp.count_of_category.rescale) + ")";
            ss <<  RP_s << setw(28 - RP_s.size()) << right << ": " << amp.time_by_category.rescale
            << " s \t= " << (amp.time_by_category.rescale/total_time) * 100 << "%\n";
        }
        
        if (amp.time_by_category.conversion) {
            string RP_s = "     Conversion ";
            ss <<  RP_s << setw(28 - RP_s.size()) << right << ": "
            << amp.time_by_category.conversion << " s \t= "
            << (amp.time_by_category.conversion/total_time) * 100 << "%\n";
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
            if (amp.data_per_cycles.memory[c] >= 1e9) {
                cout << amp.data_per_cycles.memory[c] / 1e9 << " GB \n";
            }
            else if (amp.data_per_cycles.memory[c] >= 1e6) {
                cout << amp.data_per_cycles.memory[c] / 1e6 << " MB \n";
            }
            else if (amp.data_per_cycles.memory[c] >= 1e3) {
                cout << amp.data_per_cycles.memory[c] / 1e3 << " KB \n";
            }
            else
                cout << amp.data_per_cycles.memory[c] << " B \n";
        }
    }
    
    cout << "\n¯\\_(ツ)_/¯ \n\n";
    
    ofstream file;
    file.open(config.misc_outfile + ".txt");
    for (idx_size i = 0; i < amp.data_per_cycles.cycles.size(); ++i) {
        file << amp.data_per_cycles.cycles[i] << ","
        << amp.data_per_cycles.CZ_gates[i] << ","
        << amp.data_per_cycles.T_gates[i] << ","
        << amp.data_per_cycles.XY_gates[i] ; //Fix when no XY gates 
        if (config.sim_type != Config::SimType::FullState)
            file << "," << amp.data_per_cycles.xCZ_H[i] << "," << amp.data_per_cycles.xCZ_V[i];
        file << "\n";
    }
    file.close();
}

void SequentialSimulation::
PrintReportToFile(GenericQuantumState& amp,
                  const Circuit& circuit) const
{
    ofstream file;
    file.open(config.report_outfile + ".txt");
    
    file << "Rollright ver 1.2 - a quantum circuit simulator\n\n";
    
    char hostname[20] = {};
    gethostname(hostname, 20);
    file << "Hostname : ";
    for(auto h : hostname) {
        file << h;
    }
    file << "\n";
    amp.Rescale();
    amp.ApplyGlobalICounter();
    
    if (config.verbose != Config::Verbose::NCCV && config.verbose != Config::Verbose::NCC) {
#ifdef __APPLE__
        file << "CPU model name : "; flush(file);
        system("sysctl -n machdep.cpu.brand_string");
        file << "CPU cores : "; flush(file);
        system("sysctl -n machdep.cpu.core_count");
        file << "Hardware threads : "; flush(file);
        system("sysctl -n machdep.cpu.thread_count");
        file << "L2 cache size : "; flush(file);
        system("sysctl -n hw.l2cachesize");
        file << "L3 cache size : "; flush(file);
        system("sysctl -n hw.l3cachesize");
#endif
#ifndef __APPLE__
        file << "CPU "; flush(file);
        system("egrep CPU /proc/cpuinfo | head -1");
        system("egrep cores /proc/cpuinfo | head -1");
        file << "Hardware threads : "; flush(file);
        system("egrep cores  /proc/cpuinfo | wc -l");
        file << "L3 " ; flush(file);
        system("egrep cache /proc/cpuinfo | head -1");
#endif
        file << "CPU supports :"
        << " popcnt:" << __builtin_cpu_supports("popcnt");
        
        if (__builtin_cpu_supports("sse4.2"))
            file << ", sse4.2:" << __builtin_cpu_supports("sse4.2");
        else if (__builtin_cpu_supports("sse4.1"))
            file << ", sse4.1:" << __builtin_cpu_supports("sse4.1");
        else if (__builtin_cpu_supports("sse4.1"))
            file << ", sse4.1:" << __builtin_cpu_supports("sse4.1");
        else if (__builtin_cpu_supports("ssse3"))
            file << ", ssse3:" << __builtin_cpu_supports("ssse3");
        else if (__builtin_cpu_supports("sse3"))
            file << ", sse3:" << __builtin_cpu_supports("sse3");
        else if (__builtin_cpu_supports("sse2"))
            file << ", sse2:" << __builtin_cpu_supports("sse2");
        else if (__builtin_cpu_supports("sse"))
            file << ", sse:" << __builtin_cpu_supports("sse");
        
        file << ", avx:" << __builtin_cpu_supports("avx")
        << ", avx2:" << __builtin_cpu_supports("avx2") << "\n\n";
        file << "Compiler : gcc " << __GNUC__  << "." << __GNUC_MINOR__ << "."
        <<  __GNUC_PATCHLEVEL__<< "\n";
        
        file << "Compiled on : " <<  __DATE__ << " " << __TIME__ << "\n";
        time_t t = time(0);
        struct tm * now = localtime (&t);
        file << "Executed on : "
        << (now->tm_mon + 1) << "/"
        <<  now->tm_mday << "/"
        << (now->tm_year + 1900) << " "
        <<  now->tm_hour << ":" << now->tm_min << ":"
        << std::setw(2) << std::setfill('0') << now->tm_sec
        <<"\n\n";
    }
    
    if (config.google) {
        file << "Circuit file : " + config.infile + "\n";
        file << "Circuit type : Google\n";
    }
    
    file << "Qubits : " << circuit.GetNumQubits() << "  ";
    file << "Gates : " << circuit.GetTotalNumGates() << "  ";
    file << "Cycles : " << circuit.GetNumCycles() << "\n\n";
    
    idx_size log_count = 0;
    file << "Simulation type: ";
    if (config.sim_type == Config::SimType::FullState) {
        file << "full state vector / lossless \n";
        file << "xCZ gates simulated : exact\n";
    }
    else if (config.sim_type == Config::SimType::LosslessH) {
        file << "sum of tensor products / losslessH \n";
        file << "xCZ gates simulated : exact\n";
        file << amp.log[log_count++];
    }
    else if (config.sim_type == Config::SimType::LosslessV) {
        file << "sum of tensor products / losslessV \n";
        file << "xCZ gates simulated : exact\n";
        file << amp.log[log_count++];
    }
    else if (config.sim_type == Config::SimType::Approx1CutH) {
        file << "tensor products / approx1CutH \n";
        file << "xCZ gates simulated : ignored\n";
        file << amp.log[log_count++];
    }
    else if (config.sim_type == Config::SimType::Approx1CutV) {
        file << "tensor products / approx1CutV \n";
        file << "xCZ gates simulated : ignored\n";
        file << amp.log[log_count++];
    }
    else if (config.sim_type == Config::SimType::Approx2011) {
        file << "tensor products / approx2011 \n";
        file << "xCZ gates simulated : approx\n";
        file << amp.log[log_count++];
    }
    else if (config.sim_type == Config::SimType::Approx2Cuts) {
        file << "sum of tensor products / approx2Cuts\n";
        file << "xCZ gates simulated : approx\n";
        file << amp.log[log_count++];
        file << amp.log[log_count++];
    }
    
    file << "Size of complex : " << sizeof(cmplx) << " B\n";
    
    idx_size temp_amp_size = amp.GetFullStateVectorSize();
    
    {
        ostringstream ss (ostringstream::ate);
        ss << setprecision(3);
        double norm = amp.CalculateNormSquared();
        double avg_inacc = amp.CalculateAverageInaccuracy(norm);
#ifdef RT
        file << "Recursion end-case(max) : " << config.th << " q\n";
#ifdef Parallel
        file << "Number of threads : " << thread::hardware_concurrency() << "\n";
#else
        file << "Number of threads : 1\n";
#endif
        file << "Verbosity : " << config.verbose << "\n\n";
#endif
        double memory = amp.GetMemUsage();
        ss << "State representation size : ";
        
        if (memory >= 1e9) {
            ss << memory / 1e9 << " GB \n";
        }
        else if (memory >= 1e6) {
            ss << memory / 1e6 << " MB \n";
        }
        else if (memory >= 1e3) {
            ss << memory / 1e3 << " KB \n";
        }
        else
            ss << memory << " B \n";
        
        ss << "Norm ";
        if (config.sim_type != Config::SimType::FullState)
            ss << "(assuming orthogonal addends)";
        
        ss << " : " << sqrt(norm) << "\n";
        ss << "Mean entropy : " <<  amp.CalculateMeanEntropy() << " ";
        ss << "Cross entropy : " <<  amp.CalculateCrossEntropy(10) << "\n";
        ss << "Probabilities : " << amp.GetMinProb() << "(min), "
        << amp.GetMaxProb() << "(max), "
        << amp.GetAvgProb() << "(avg)\n";
        ss << "Log_2 (max / min) = " << log2(real(amp.GetMaxProb())/real(amp.GetMinProb())) << "\n" ;
        ss << "Avg inaccuracy per probability > " << avg_inacc << " ("<< (avg_inacc/amp.GetAvgProb()) * 100 << "%)\n\n";
        file << ss.str();
    }
    
    
    string key = to_string(circuit.GetNumQubits()) + "_" + to_string(circuit.GetNumCycles());
    file << "Correctness check : ";
    
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
            file << "passed\n";
        }
        else {
            file << "failed\nCorrect results: \n";
            if (config.verbose != Config::Verbose::NCCV) {
                string imag0 = to_string(imag(benchmark[key][0])), imag1 = to_string(imag(benchmark[key][1])),
                imag2 = to_string(imag(benchmark[key][2])), imag3 = to_string(imag(benchmark[key][3])),
                imag4 = to_string(imag(benchmark[key][4]));
                
                file << "amp[3]  \t= "  << real(benchmark[key][0]) ;
                if (imag(benchmark[key][0]) < 0) {
                    imag0[0] = ' ';
                    file << " - " << imag0 << "i\n";
                }
                else
                    file << " + " << imag0 << "i\n";
                
                file << "amp[1/4]\t= " << real(benchmark[key][1]);
                if (imag(benchmark[key][1]) < 0) {
                    imag1[0] = ' ';
                    file << " - " << imag1 << "i\n";
                }
                else
                    file << " + " << imag1 << "i\n";
                
                file << "amp[1/2]\t= " << real(benchmark[key][2]);
                if (imag(benchmark[key][2]) < 0) {
                    imag2[0] = ' ';
                    file << " - " << imag2 << "i\n";
                }
                else
                    file << " + " << imag2 << "i\n";
                
                file << "amp[3/4]\t= " << real(benchmark[key][3]);
                if (imag(benchmark[key][3]) < 0) {
                    imag3[0] = ' ';
                    file << " - " << imag3 << "i\n";
                }
                else
                    file << " + " << imag3 << "i\n";
                
                file << "amp[-3] \t= "  << real(benchmark[key][4]);
                if (imag(benchmark[key][4]) < 0) {
                    imag4[0] = ' ';
                    file << " - " << imag4 << "i\n";
                }
                else
                    file << " + " << imag4 << "i\n";
                file << "Incorrect results: \n";
            }
        }
    }
    else
        file << "no data available\n";
    
    if (config.verbose != Config::Verbose::NCCV)
    {
        string imag0 = to_string(imag(amp[3])), imag1 = to_string(imag(amp[temp_amp_size/4])),
        imag2 = to_string(imag(amp[temp_amp_size/2])), imag3 = to_string(imag(amp[3 * temp_amp_size/4])),
        imag4 = to_string(imag(amp[temp_amp_size - 3]));
        
        file << "amp[3]  \t= " << real(amp[3]);
        if (imag(amp[3]) < 0) {
            imag0[0] = ' ';
            file << " - " << imag0 << "i\n";
        }
        else
            file << " + " << imag0 << "i\n";
        file << "amp[1/4]\t= " << real(amp[temp_amp_size/4]);
        if (imag(amp[temp_amp_size/4]) < 0) {
            imag1[0] = ' ';
            file << " - " << imag1 << "i\n";
        }
        else
            file << " + " << imag1 << "i\n";
        file << "amp[1/2]\t= " << real(amp[temp_amp_size/2]);
        if (imag(amp[temp_amp_size/2]) < 0) {
            imag2[0] = ' ';
            file << " - " << imag2 << "i\n";
        }
        else
            file << " + " << imag2 << "i\n";
        file << "amp[3/4]\t= " << real(amp[3 * temp_amp_size/4]);
        if (imag(amp[3 * temp_amp_size/4]) < 0) {
            imag3[0] = ' ';
            file << " - " << imag3 << "i\n";
        }
        else
            file << " + " << imag3 << "i\n";
        file << "amp[-3] \t= " << real(amp[temp_amp_size - 3]);
        if (imag(amp[temp_amp_size - 3]) < 0) {
            imag4[0] = ' ';
            file << " - " << imag4 << "i\n\n";
        }
        else
            file << " + " << imag4 << "i\n\n";
    }
    
    {
        ostringstream ss (ostringstream::ate);
        ss << setprecision(3);
        ss << "Runtime (" << total_time << " s total) by category \n";
        
        string H_s = "     H (" + to_string(amp.count_of_category.H) +  ")";
        ss << H_s << setw(28 - H_s.size()) << right << ": " << amp.time_by_category.H
        << " s \t= " << (amp.time_by_category.H/total_time) * 100 << "%\n";
        
        if(amp.count_of_category.CZ_T) {
            string CZ_T_s = "     CZ & T (" +
            to_string(amp.count_of_category.CZ_T - amp.count_of_category.decomposed_CZ) + ")" ;
            ss << CZ_T_s << setw(28 - CZ_T_s.size()) << right << ": "
            << amp.time_by_category.CZ_T << " s \t= "
            << (amp.time_by_category.CZ_T/total_time) * 100 << "%\n";
        }
        
        if(amp.count_of_category.decomposed_CZ) {
            string CZ_s = "     Decomposed CZ (" + to_string(amp.count_of_category.decomposed_CZ) + ")" ;
            ss << CZ_s << setw(28 - CZ_s.size()) << right << ": "
            << amp.time_by_category.decomposed_CZ << " s \t= " << (amp.time_by_category.decomposed_CZ/total_time) * 100 << "%\n";
        }
        
        if (amp.count_of_category.X1_2 || amp.count_of_category.Y1_2) {
            string XY_s = "     Single X (" + to_string(amp.count_of_category.X1_2)
            + ") & Y (" + to_string(amp.count_of_category.Y1_2) + ")";
            ss << XY_s << setw(28 - XY_s.size()) << right << ": "
            << amp.time_by_category.X1_2 +  amp.time_by_category.Y1_2 << " s \t= "
            << ((amp.time_by_category.X1_2 +  amp.time_by_category.Y1_2)/total_time) * 100 << "%\n";
        }
        
        if (amp.count_of_category.merged_XY1_2) {
            string X_Y_s = "     Merged X & Y ("
            + to_string(amp.count_of_category.merged_XY1_2 - amp.count_of_category.Y1_2 - amp.count_of_category.X1_2) + ")";
            ss << X_Y_s << setw(28 - X_Y_s.size()) << right << ": " << amp.time_by_category.merged_XY1_2
            << " s \t= " << (amp.time_by_category.merged_XY1_2/total_time) * 100 << "%\n";
        }
        
        if (amp.count_of_category.rescale) {
            string RP_s = "     Rescaling passes (" + to_string(amp.count_of_category.rescale) + ")";
            ss <<  RP_s << setw(28 - RP_s.size()) << right << ": " << amp.time_by_category.rescale
            << " s \t= " << (amp.time_by_category.rescale/total_time) * 100 << "%\n";
        }
        
        if (amp.time_by_category.conversion) {
            string RP_s = "     Conversion ";
            ss <<  RP_s << setw(28 - RP_s.size()) << right << ": "
            << amp.time_by_category.conversion << " s \t= "
            << (amp.time_by_category.conversion/total_time) * 100 << "%\n";
        }
        
        file << ss.str() << "\n";
    }
    
    if (config.verbose == Config::Verbose::Cycles) {
        file << log.str() << "\n";
        file << "Cycle\t xCZ\t Addends\t  Memory\n";
        for (idx_size c = 0; c < amp.data_per_cycles.memory.size(); ++c) {
            file << amp.data_per_cycles.cycles[c] << "\t "
            << amp.data_per_cycles.xCZ_H[c] << "," << amp.data_per_cycles.xCZ_V[c] << "\t "
            << amp.data_per_cycles.addends[c] << "\t  " ;
            if (amp.data_per_cycles.memory[c] >= 1e9) {
                file << amp.data_per_cycles.memory[c] / 1e9 << " GB \n";
            }
            else if (amp.data_per_cycles.memory[c] >= 1e6) {
                file << amp.data_per_cycles.memory[c] / 1e6 << " MB \n";
            }
            else if (amp.data_per_cycles.memory[c] >= 1e3) {
                file << amp.data_per_cycles.memory[c] / 1e3 << " KB \n";
            }
            else
                file << amp.data_per_cycles.memory[c] << " B \n";
        }
    }
    
    file << "\n¯\\_(ツ)_/¯ \n\n";
    
    ofstream misc_file;
    misc_file.open(config.misc_outfile + ".txt");
    for (idx_size i = 0; i < amp.data_per_cycles.cycles.size(); ++i) {
        misc_file << amp.data_per_cycles.cycles[i] << ","
        << amp.data_per_cycles.CZ_gates[i] << ","
        << amp.data_per_cycles.T_gates[i] << ","
        << amp.data_per_cycles.XY_gates[i] ;
        if (config.sim_type != Config::SimType::FullState)
            misc_file << "," << amp.data_per_cycles.xCZ_H[i] << "," << amp.data_per_cycles.xCZ_V[i];
        misc_file << "\n";
    }
    file.close();
    misc_file.close();
}


