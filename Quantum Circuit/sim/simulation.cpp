
//  simulation.cpp
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 12/12/17.
//

#include "simulation.h"

SequentialSimulation::
SequentialSimulation(): g_begin(0), g_end(0), rescale_time(0),
                        num_rescaling(0), merged_X_Y(0), X(0), Y(0), CZ_T(0)
{
    gate_time.resize(5, 0);
}

SequentialSimulation::
SequentialSimulation(const SequentialSimulation& rhs)
{
    gate_time = rhs.gate_time;
    g_begin = rhs.g_begin;
    g_end = rhs.g_end;
    rescale_time = rhs.rescale_time;
    num_rescaling = rhs.num_rescaling;
    merged_X_Y = rhs.merged_X_Y;
    X = rhs.X;
    Y = rhs.Y;
    CZ_T = rhs.CZ_T;
}

void SequentialSimulation::
Simulate(const string &outfile,
         State& amp,
         Circuit& circuit)
{
    idx_size size = circuit.GetTotalNumGates();
    int total_circuit_qubits = circuit.GetNumQubits();
    
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
                amp.ApplyBlockOfDiagGates(gates, total_circuit_qubits, i);
    
                g_end = clock();
                gate_time[1] += double(g_end - g_begin)/ CLOCKS_PER_SEC;
                CZ_T += i - prev_i;
                i -= 1;
                
            }
            else 
                amp.ApplyCGate(current_gate.num_controls, current_gate.qubits,
                              total_circuit_qubits, current_gate, (Gate::Type)current_gate.ids.back());
        }
        else {
            bool applied = false;
#ifdef Clustering
             if (circuit.google && i < circuit.GetTotalNumGates() - 1 &&
                 (current_gate.ids.back() == Gate::Type::X_1_2 ||
                  current_gate.ids.back() == Gate::Type::Y_1_2) &&
                 (circuit.GetGateFromIndex(i + 1).ids.back() == current_gate.ids.back())) {
                 applied = true;
                 idx_size prev_i = i;
                 amp.ApplyClusterOfXYHGates(gates, total_circuit_qubits, i, (Gate::Type)current_gate.ids.back());
                 g_end = clock();
                 gate_time[4] += double(g_end - g_begin)/ CLOCKS_PER_SEC;
                 merged_X_Y += i - prev_i;
                 --i;
             }
#endif
//#ifdef ManualMerging
           if (circuit.google && i < circuit.GetTotalNumGates() - 1 &&
               (circuit.GetGateFromIndex(i + 1).ids.back() == Gate::Type::Y_1_2 ||
                circuit.GetGateFromIndex(i + 1).ids.back() == Gate::Type::X_1_2)) {
               applied = true;
               idx_size prev_i = i;
               amp.ApplyMergedXYGate(gates, i, total_circuit_qubits);
               g_end = clock();
               gate_time[4] += double(g_end - g_begin)/ CLOCKS_PER_SEC;
               merged_X_Y += i - prev_i;;
               --i;
           }
//#endif
           if(circuit.google && current_gate.ids.back() == Gate::Type::Hadamard) {
                amp.ApplyHGateOnAllAmps(total_circuit_qubits);
                i += total_circuit_qubits - 1;
                g_end = clock();
                gate_time[0] += double(g_end - g_begin)/ CLOCKS_PER_SEC;
            }
            else if (!applied) {
                amp.ApplyNonCGate(current_gate.qubits, total_circuit_qubits, current_gate,
                                  (Gate::Type)current_gate.ids.back());
                g_end = clock();
                
                if(current_gate.ids.back() == Gate::Type::X_1_2){
                    gate_time[2] += double(g_end - g_begin)/ CLOCKS_PER_SEC;
                    X++;
                    amp.IncrementGlobalFactorPower(2);
                }
                else if(current_gate.ids.back() == Gate::Type::Y_1_2) {
                    gate_time[3] += double(g_end - g_begin)/ CLOCKS_PER_SEC;
                    Y++;
                    amp.IncrementGlobalFactorPower(2);
                }
            }
        }
    }
    
    clock_t end = clock();
    
//    amp.PrintStateVector();
    
    PrintReport(end , begin, amp, circuit);
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
PrintReport(const clock_t end,
            const clock_t begin,
            State& amp,
            const Circuit& circuit) const
{
    char hostname[20] = {};
    gethostname(hostname, 20);
    cout << "Hostname : ";
    for(auto h : hostname) {
        cout << h;
    }
    cout << "\n";
#ifdef CPUname
    cout << "CPU name : ";
    if (__builtin_cpu_is("amd")) {
        cout << "AMD ";
        if (__builtin_cpu_is("amdfam10h"))
            cout << "Family 10h\n";
        else if (__builtin_cpu_is("barcelona"))
            cout << "Family 10h Barcelona\n";
        else if (__builtin_cpu_is("shanghai"))
            cout << "Family 10h Shanghai\n";
        else if (__builtin_cpu_is("istanbul"))
            cout << "Family 10h Istanbul\n";
        else if (__builtin_cpu_is("btver1"))
            cout << "Family 14h\n";
        else if (__builtin_cpu_is("amdfam15h"))
            cout << "Family 15h\n";
        else if (__builtin_cpu_is("bdver1"))
            cout << "Family 15h Bulldozer version 1\n";
        else if (__builtin_cpu_is("bdver2"))
            cout << "Family 15h Bulldozer version 2\n";
        else if (__builtin_cpu_is("bdver3"))
            cout << "Family 15h Bulldozer version 3\n";
        else if (__builtin_cpu_is("btver2"))
            cout << "Family 16h\n";
        else
            cout << "\n";
    }
    else if (__builtin_cpu_is("intel")) {
        cout << "Intel ";
        
        if (__builtin_cpu_is("corei7"))
            cout << "Core i7\n";
        else if (__builtin_cpu_is("atom"))
            cout << "Atom\n";
        else if (__builtin_cpu_is("core2"))
            cout << "Core 2\n";
        else if (__builtin_cpu_is("nehalem"))
            cout << "Core i7 Nehalem\n";
        else if (__builtin_cpu_is("westmere"))
            cout << "Core i7 Westmere\n";
        else if (__builtin_cpu_is("sandybridge"))
            cout << "Core i7 Sandy Bridge\n";
        else
            cout << "\n";
    }
    else
        cout << "Unrecognized\n";
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
         << ", avx2:" << __builtin_cpu_supports("avx2") << "\n";
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
    
    cout << "Qubits : " << circuit.GetNumQubits() << "  ";
    cout << "Gates : " << circuit.GetTotalNumGates() << "  ";
    cout << "Cycles : " << circuit.GetNumCycles() << "\n\n";
    
    cout << setprecision(3);
    double memory = amp.GetMemUsage();
    cout << "State vector size: ";
    
    if (memory >= 1e9) {
        cout << memory / 1e9 << " GB \n";
    }
    else if (memory >= 1e6) {
        cout << memory / 1e6 << " MB \n";
    }
    else if (memory >= 1e3) {
        cout << memory / 1e3 << " KB \n";
    }
    else
        cout << memory << " B \n";
    
    double total_time = double(end - begin) / CLOCKS_PER_SEC;
    cout << "Norm : " << amp.CalculateNormOfAmp() << "\n";
    cout << "Probabilities : " << amp.GetMinProb() << "(min), "
         << amp.GetMaxProb() << "(max), "
         << amp.GetAvgProb() << "(avg)\n";
    cout << "Log_2 (max / min) = " << log2(real(amp.GetMaxProb())/real(amp.GetMinProb())) << "\n\n" ;
    
    cout << "Total runtime : " << total_time << "s\n";
    cout << "Runtimes by gate type\n";
    cout << "   H (" << circuit.GetNumQubits() << ") : " << gate_time[0]
    << "s = " << (gate_time[0]/total_time) * 100 << "%\n";

    cout << "   CZ & T (" << CZ_T << ") : " << gate_time[1]
    << "s = " << (gate_time[1]/total_time) * 100 << "%\n";
    
    cout << "   X (" << X << ") : " << gate_time[2] << "s = "
    << (gate_time[2]/total_time) * 100 << "%\n";
    
    cout << "   Y (" << Y << ") : " << gate_time[3]
    << "s = " << (gate_time[3]/total_time) * 100 << "%\n";
    
    cout << "   Merged X & Y (" << merged_X_Y << ") : " << gate_time[4]
    << "s = " << (gate_time[4]/total_time) * 100 << "%\n";
    cout << "Rescaling passes (" << num_rescaling << ") : " << rescale_time << "s = "
    << (rescale_time/total_time) * 100 << "%\n\n";
}

void SequentialSimulation::
PrintReport(const string &outfile,
            const clock_t end,
            const clock_t begin,
            State& amp,
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


