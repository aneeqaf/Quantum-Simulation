
//  simulation.cpp
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 12/12/17.
//

#include "simulation.h"

unordered_map<string, array<cmplx, 5>> SequentialSimulation::benchmark = {};

SequentialSimulation::
SequentialSimulation(SimType st): filename({}),g_begin(0), g_end(0), rescale_time(0),
num_rescaling(0), merged_X_Y(0), X(0), Y(0), CZ_T(0), th(0), google(false), sim_type(st)
{
    gate_time.resize(5, 0);
}

SequentialSimulation::
SequentialSimulation(const string fname, bool g, SimType st): filename(fname),g_begin(0), g_end(0), rescale_time(0),
num_rescaling(0), merged_X_Y(0), X(0), Y(0), CZ_T(0), th(0), google(g), sim_type(st)
{
    gate_time.resize(5, 0);
}
SequentialSimulation::
SequentialSimulation(const SequentialSimulation& rhs)
{
    gate_time = rhs.gate_time;
    filename = rhs.filename;
    g_begin = rhs.g_begin;
    g_end = rhs.g_end;
    rescale_time = rhs.rescale_time;
    num_rescaling = rhs.num_rescaling;
    merged_X_Y = rhs.merged_X_Y;
    X = rhs.X;
    Y = rhs.Y;
    CZ_T = rhs.CZ_T;
    th = rhs.th;
    sim_type = rhs.sim_type;
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
    int total_circuit_qubits = circuit.GetNumQubits();
    th = threshold;
    
    if (circuit.google) {
        if (!circuit.ClockCycleEmpty())
            circuit.GroupAlternateCycles();
        circuit.GroupSimilarGates();
    }
    
    auto gates = circuit.GetGates();
    
    clock_t begin = clock();
    for (idx_size i = 0; i < size; ++i) {
        
        g_begin = clock();
        
        if (amp.GetGlobalFactorPower() > 100) {
            clock_t r_b = clock();
            amp.Rescale();
            ++num_rescaling;
            clock_t r_e = clock();
            rescale_time += double(r_e - r_b)/ CLOCKS_PER_SEC;
        }
        
        Gate& current_gate = circuit.GetGateFromIndex(i);
        if(current_gate.ids.front() == Gate::Type::Control ||
           current_gate.ids.back() == Gate::Type::T) {
            
             if (current_gate.ids.back() == Gate::Type::T ||
                current_gate.ids.back() == Gate::Type::Z) {
                
                 idx_size prev_i = i;
                 idx_size T_bitmasks[2] = {0};
                 idx_size CZ_bitmasks[total_circuit_qubits];
                 amp.FormCZTGatesBitmask(CZ_bitmasks, T_bitmasks, i, gates, total_circuit_qubits);
                 amp.ApplyBlockOfDiagGates(CZ_bitmasks, T_bitmasks);
    
                 g_end = clock();
                 gate_time[1] += double(g_end - g_begin)/ CLOCKS_PER_SEC;
                 CZ_T += i - prev_i;
                 i -= 1;
                
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
                     g_end = clock();
                     gate_time[4] += double(g_end - g_begin)/ CLOCKS_PER_SEC;
                     merged_X_Y += i - prev_i;
                     --i;
             }
            else if(circuit.google && current_gate.ids.back() == Gate::Type::Hadamard) {
                amp.ApplyHGateOnAllAmps();
                i += total_circuit_qubits - 1;
                g_end = clock();
                gate_time[0] += double(g_end - g_begin)/ CLOCKS_PER_SEC;
            }
            else {
                amp.ApplyNonCGate(current_gate.qubits[0],
                                  (Gate::Type)current_gate.ids.back(),  current_gate);
                g_end = clock();
                
                if(current_gate.ids.back() == Gate::Type::X_1_2){
                    gate_time[2] += double(g_end - g_begin)/ CLOCKS_PER_SEC;
                    X++;
                }
                else if(current_gate.ids.back() == Gate::Type::Y_1_2) {
                    gate_time[3] += double(g_end - g_begin)/ CLOCKS_PER_SEC;
                    Y++;
                }
            }
        }
    }
    
    clock_t end = clock();
    
#ifdef Print
    amp.PrintStateVector();
#endif
    
    PrintReport(amp, end , begin, circuit);
}

SequentialSimulation& SequentialSimulation::
operator=(const SequentialSimulation& rhs)
{
    SequentialSimulation temp(rhs);
    swap(gate_time, temp.gate_time);
    g_begin = temp.g_begin;
    g_end = temp.g_end;
    num_rescaling = temp.num_rescaling;
    merged_X_Y = temp.merged_X_Y;
    X = temp.X;
    Y = temp.Y;
    CZ_T = temp.CZ_T;
    return *this;
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
    else if (sim_type == SequentialSimulation::SimType::LosslessH)
        cout << "sum of tensor products / horiz cut / lossless \n";
    else if (sim_type == SequentialSimulation::SimType::LosslessV)
        cout << "sum of tensor products / vert cut / lossless \n";
    
    cout << "Number of threads : 1\n";
    cout << "Size of complex : " << sizeof(cmplx) << " B\n";
    
    idx_size temp_amp_size = amp.GetFullStateSize();
    
    {
        ostringstream ss (ostringstream::ate);
        ss << setprecision(3);
        double norm = amp.CalculateNormOfAmp();
        double avg_inacc = (1.0 - norm)/(double)temp_amp_size;
#ifdef RT
        cout << "Recursion end-case(max) : " << th << " q\n\n";
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

        ss << "Norm : " << norm << "\n";
        ss << "Probabilities : " << amp.GetMinProb() << "(min), "
             << amp.GetMaxProb() << "(max), "
             << amp.GetAvgProb() << "(avg)\n";
        ss << "Log_2 (max / min) = " << log2(real(amp.GetMaxProb())/real(amp.GetMinProb())) << "\n" ;
        ss << "Avg inaccuracy per probability > " << avg_inacc << " ("<< (avg_inacc/amp.GetAvgProb()) * 100 << "%)\n\n";
        cout << ss.str();
    }
    
    {
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
        else
            cout << "no data available\n";
    
    
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
        
        string H_s = "     H (" + to_string(circuit.GetNumQubits()) +  ")";
        ss << H_s << setw(28 - H_s.size()) << right << ": " << gate_time[0]
        << " s = " << (gate_time[0]/total_time) * 100 << "%\n";

        if(CZ_T) {
            string CZ_T_s = "     CZ & T (" + to_string(CZ_T) + ")" ;
            ss << CZ_T_s << setw(28 - CZ_T_s.size()) << right << ": " << gate_time[1]
            << " s = " << (gate_time[1]/total_time) * 100 << "%\n";
        }
        
        if (X) {
            string X_s = "     X (" + to_string(X) + ")";
            ss << X_s << setw(28 - X_s.size()) << right << ": " << gate_time[2] << " s = "
            << (gate_time[2]/total_time) * 100 << "%\n";
        }
        
        if (Y) {
            string Y_s = "     Y (" + to_string(Y) + ")";
            ss << Y_s << setw(28 - Y_s.size()) << right << ": " << gate_time[3]
            << " s = " << (gate_time[3]/total_time) * 100 << "%\n";
        }
        
        if (merged_X_Y) {
            string X_Y_s = "     X & Y (" + to_string(merged_X_Y) + ")";
            ss << X_Y_s << setw(28 - X_Y_s.size()) << right << ": " << gate_time[4]
            << " s = " << (gate_time[4]/total_time) * 100 << "%\n";
        }
        
        if (num_rescaling) {
            string RP_s = "     Rescaling passes (" + to_string(num_rescaling) + ")";
            ss <<  RP_s << setw(28 - RP_s.size()) << right << ": " << rescale_time << " s = "
            << (rescale_time/total_time) * 100 << "%\n\n";
        }
        
        cout << ss.str();
    }
    
    cout << "¯\\_(ツ)_/¯ \n\n";
    
}

void SequentialSimulation::
PrintReport(GenericQuantumState& amp,
            const string &outfile,
            const clock_t end,
            const clock_t begin,
            const Circuit& circuit) const
{
    ofstream file;
    file.open("output/simulation_reports/" + outfile);
    
    char hostname[20] = {};
    gethostname(hostname, 20);
    file << "Hostname : ";
    for(auto h : hostname) {
        file << h;
    }
    file << "\n";
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
    
    file << "Qubits : " << circuit.GetNumQubits() << "  ";
    file << "Gates : " << circuit.GetTotalNumGates() << "  ";
    file << "Cycles : " << circuit.GetNumCycles() << "\n\n";
    
    file << setprecision(3);
    double memory = amp.GetMemUsage();
    file << "State vector size: ";
    
    if (memory >= 1e9) {
        file << memory / 1e9 << " GB \n";
    }
    else if (memory >= 1e6) {
        file << memory / 1e6 << " MB \n";
    }
    else if (memory >= 1e3) {
        file << memory / 1e3 << " KB \n";
    }
    else
        file << memory << " B \n";
    
    double total_time = double(end - begin) / CLOCKS_PER_SEC;
    file << "Norm : " << amp.CalculateNormOfAmp() << "\n";
    file << "Probabilities : " << amp.GetMinProb() << "(min), "
    << amp.GetMaxProb() << "(max), "
    << amp.GetAvgProb() << "(avg)\n";
    file << "Log_2 (max / min) = " << log2(real(amp.GetMaxProb())/real(amp.GetMinProb())) << "\n\n" ;
    
    file << "Runtimes by gate type\n";
    file << "   H (" << circuit.GetNumQubits() << ") : " << gate_time[0]
    << "s = " << (gate_time[0]/total_time) * 100 << "%\n";
    
    file << "   CZ & T (" << CZ_T << ") : " << gate_time[1]
    << "s = " << (gate_time[1]/total_time) * 100 << "%\n";
    
    file << "   X (" << X << ") : " << gate_time[2] << "s = "
    << (gate_time[2]/total_time) * 100 << "%\n";
    
    file << "   Y (" << Y << ") : " << gate_time[3]
    << "s = " << (gate_time[3]/total_time) * 100 << "%\n";
    
    file << "   Merged X & Y (" << merged_X_Y << ") : " << gate_time[4]
    << "s = " << (gate_time[4]/total_time) * 100 << "%\n\n";
    
    file << "Total runtime : " << total_time << "s\n";
    file << "Rescaling time (" << num_rescaling << ") : " << rescale_time << "s\n\n";
}


