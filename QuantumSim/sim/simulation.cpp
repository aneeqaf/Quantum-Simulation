
//  simulation.cpp
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 12/12/17.
//

#include "simulation.h"

unordered_map<string, array<cmplx, 5>> SequentialSimulation::benchmark = {};



SequentialSimulation::
SequentialSimulation(const SimType st,
                     const int cut,
                     const Verbose v):
filename({}),g_begin(0), g_end(0), th(0), cut_sizes(cut), conv_cycle(0),
google(false), sim_type(st), verbose(v) {}

SequentialSimulation::
SequentialSimulation(const string fname,
                     const bool g,
                     const SimType st,
                     const int cut,
                     const Verbose v):
filename(fname) ,g_begin(0), g_end(0), th(0), cut_sizes(cut), conv_cycle(0),
google(g), sim_type(st), verbose(v) {}

SequentialSimulation::
SequentialSimulation(const SequentialSimulation& rhs)
{
    filename = rhs.filename;
    g_begin = rhs.g_begin;
    g_end = rhs.g_end;
    th = rhs.th;
    sim_type = rhs.sim_type;
}

SequentialSimulation& SequentialSimulation::
operator=(const SequentialSimulation& rhs)
{
    filename = rhs.filename;
    g_begin = rhs.g_begin;
    g_end = rhs.g_end;
    th = rhs.th;
    sim_type = rhs.sim_type;
    return *this;
}


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
Simulate(const string &outfile,
         GenericQuantumState& amp,
         Circuit& circuit,
         const int threshold)
{
    PopulateBenchmarkMap();
    idx_size size = circuit.GetTotalNumGates();
    int total_circuit_qubits = circuit.GetNumQubits(), current_cycle = 0;
    th = threshold;
    
    if (circuit.google) {
        if (!circuit.ClockCycleEmpty())
            circuit.GroupAlternateCycles();
        circuit.GroupSimilarGates();
    }
    
    auto gates = circuit.GetGates();
    log << "Cycle \tRuntime \tMemory\t\tXEntropy\n";
    clock_t begin = clock(), cycle_start = clock();
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
            
            current_cycle += 2;
             if (current_gate.ids.back() == Gate::Type::T ||
                current_gate.ids.back() == Gate::Type::Z) {
                
                 double cross_entropy = 0;
#ifdef CrossEntropy
                 if (verbose == 4) 
                    cross_entropy = amp.CalculateCrossEntropy(1000);
#endif
                 clock_t cycle_end = clock();
                 
                 log <<  setprecision(3) ;
                 log << current_cycle << "\t"
                 << double(cycle_end - cycle_start)/ CLOCKS_PER_SEC << " s    \t";
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
                 
                 idx_size prev_i = i;
                 idx_size T_bitmasks[2] = {0};
                 idx_size CZ_bitmasks[total_circuit_qubits];
                 amp.FormCZTGatesBitmask(CZ_bitmasks, T_bitmasks, i, gates, total_circuit_qubits);
                 amp.ApplyBlockOfDiagGates(CZ_bitmasks, T_bitmasks);
    
                 amp.count_of_category.CZ_T += i - prev_i;
                 i -= 1;
                 
                 if (amp.GetSize() == amp.GetFullStateVectorSize())
                     conv_cycle = current_cycle;
                 
                cycle_start = clock();
            }
            else 
                amp.ApplyCGate(current_gate.num_controls, current_gate.qubits,
                               current_gate, (Gate::Type)current_gate.ids.back());
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
                     amp.ApplyXYRecursiveTransform(X_bitmask, Y_bitmask, th);
#endif
                     amp.count_of_category.merged_XY1_2 += i - prev_i;
                     --i;
             }
            else if(circuit.google && current_gate.ids.back() == Gate::Type::Hadamard) {
                cycle_start = clock();
                amp.ApplyHGateOnAllAmps();
                amp.count_of_category.H = circuit.GetNumQubits();
                i += total_circuit_qubits - 1;
            }
            else {
                amp.ApplyNonCGate(current_gate.qubits[0],
                                  (Gate::Type)current_gate.ids.back(),  current_gate);
                //TODO: Need to be able to count different types of non-control gates.
            }
        }
    }
    
    clock_t end = clock();
    
#ifdef Print
    amp.PrintStateVector();
#endif
    
    if (verbose)
        PrintReport(amp, end , begin, circuit);
}

void SequentialSimulation::
PrintReport(GenericQuantumState& amp,
            const clock_t end,
            const clock_t begin,
            const Circuit& circuit) const
{
    char hostname[20] = {};
    gethostname(hostname, 20);
    cout << "Hostname : ";
    for(auto h : hostname) {
        cout << h;
    }
    cout << "\n";
    amp.Rescale();
    amp.ApplyGlobalICounter();
    vector<string> class_log = amp.GetClassDataLog();
   
    if (verbose != Verbose::NCCV && verbose != Verbose::NCC) {
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
    
    if (google) {
        cout << "Circuit file : " + filename + "\n";
        cout << "Circuit type : Google\n";
    }
    
    cout << "Qubits : " << circuit.GetNumQubits() << "  ";
    cout << "Gates : " << circuit.GetTotalNumGates() << "  ";
    cout << "Cycles : " << circuit.GetNumCycles() << "\n\n";
    
    cout << "Simulation type: ";
    if (sim_type == SequentialSimulation::SimType::FullState)
        cout << "full state vector / lossless \n";
    else if (sim_type == SequentialSimulation::SimType::LosslessH) {
        cout << "sum of tensor products / lossless \n";
        cout << class_log[0];
    }
    else if (sim_type == SequentialSimulation::SimType::LosslessV) {
        cout << "sum of tensor products / lossless \n";
        cout << class_log[0];
    }
    
    cout << "Size of complex : " << sizeof(cmplx) << " B\n";
    
    idx_size temp_amp_size = amp.GetFullStateVectorSize();
    
    {
        ostringstream ss (ostringstream::ate);
        ss << setprecision(3);
        double norm = amp.CalculateNormSquared();
        double avg_inacc = amp.CalculateAverageInaccuracy(norm);
#ifdef RT
        cout << "Recursion end-case(max) : " << th << " q\n";
        cout << "Number of threads : 1\n";
        cout << "Verbosity : " << verbose << "\n\n";
#endif
        double memory = amp.GetMemUsage();
        ss << "State vector size : ";

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
        if (sim_type != SequentialSimulation::SimType::FullState)
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
                if (verbose != Verbose::NCCV) {
                    string imag0 = to_string(imag(benchmark[key][0])), imag1 = to_string(imag(benchmark[key][1])),
                    imag2 = to_string(imag(benchmark[key][2])), imag3 = to_string(imag(benchmark[key][3])),
                    imag4 = to_string(imag(benchmark[key][4]));
                    
                    cout << "amp[3] b\t= " << real(benchmark[key][0]) ;
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
                    cout << "i\namp[1/2]\t= " << real(benchmark[key][2]);
                    if (imag(benchmark[key][2]) < 0) {
                        imag2[0] = ' ';
                        cout << " - " << imag2 << "i\n";
                    }
                    else
                        cout << " + " << imag2 << "i\n";
                    cout << "i\namp[3/4]\t= " << real(benchmark[key][3]);
                    if (imag(benchmark[key][3]) < 0) {
                        imag3[0] = ' ';
                        cout << " - " << imag3 << "i\n";
                    }
                    else
                        cout << " + " << imag3 << "i\n";
                    cout << "i\namp[-3] \t= "  << real(benchmark[key][4]);
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
    
    if (verbose != Verbose::NCCV)
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
        double total_time = double(end - begin) / CLOCKS_PER_SEC;
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
    
    if (verbose == Verbose::Cycles) {
        cout << log.str() << "\n";
        for (idx_size s = 1; s < class_log.size(); ++s)
            cout << setprecision(3) << class_log[s];
    }
    
    cout << "\n¯\\_(ツ)_/¯ \n\n";
    
}

void SequentialSimulation::
PrintReport(GenericQuantumState& amp,
            const string &outfile,
            const clock_t end,
            const clock_t begin,
            const Circuit& circuit) const
{
    
}


