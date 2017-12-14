//
//  simulation.cpp
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 12/12/17.
//  Copyright © 2017 Aneeqa Fatima. All rights reserved.
//

#include "simulation.h"

sequentialSimulation::
sequentialSimulation(): g_begin(0), g_end(0),
                        num_rescaling(0), merged_X_Y(0), X(0), Y(0), CZ_T(0)
{
    gate_time.resize(5, 0);
}

sequentialSimulation::
sequentialSimulation(const sequentialSimulation& rhs)
{
    gate_time = rhs.gate_time;
    g_begin = rhs.g_begin;
    g_end = rhs.g_end;
    num_rescaling = rhs.num_rescaling;
    merged_X_Y = rhs.merged_X_Y;
    X = rhs.X;
    Y = rhs.Y;
    CZ_T = rhs.CZ_T;
}

void sequentialSimulation::
Simulate(const string &outfile,
         state& amp,
         circuit& circuit,
         const int sim_depth)
{
    idx_size size = circuit.GetTotalNumGates();
    int qubits = circuit.GetNumQubits();
    
    if (circuit.google) {
        if (!circuit.ClockCycleEmpty())
            circuit.GroupAlternateCycles();
        circuit.GroupSimilarGates();
    }
    
    int idx_end_cyle = circuit.GateIndexForCycle(sim_depth);
    auto gates = circuit.GetGates();
    
    clock_t begin = clock();
    for (idx_size i = 0; i < size; ++i) {
        
        g_begin = clock();
        if (i >= (idx_size)idx_end_cyle)
            break;
       
        const gate& current_gate = circuit.GetGateFromIndex(i);
        if(current_gate.ids.front() == gate::Gates::Control ||
           current_gate.ids.back() == gate::Gates::T) {
            
            if (amp.GetGlobalFactorPower() > 100)
                ++num_rescaling;
            
            if (current_gate.ids.back() == gate::Gates::T ||
                current_gate.ids.back() == gate::Gates::Z) {
                
                idx_size prev_i = i;
                amp.ApplyBlockOfDiagGates(gates, qubits, i);
    
                g_end = clock();
                gate_time[1] += double(g_end - g_begin)/ CLOCKS_PER_SEC;
                CZ_T += i - prev_i + 1;
                i -= 1;
                
            }
            else {
                amp.ApplyCGate(current_gate.num_controls, current_gate.qubits,
                              qubits, current_gate, (gate::Gates)current_gate.ids.back());
            }
        }
        else {
            if (circuit.google && i < circuit.GetTotalNumGates() - 1 &&
                (circuit.GetGateFromIndex(i + 1).ids.back() == gate::Gates::X_1_2 ||
                 circuit.GetGateFromIndex(i + 1).ids.back() == gate::Gates::Y_1_2)) {
                    amp.ApplyTwoMergedXYGate(gates, i, i + 1, qubits);
                    ++i;
                    g_end = clock();
                    gate_time[4] += double(g_end - g_begin)/ CLOCKS_PER_SEC;
                    merged_X_Y++;
                }
            else if(circuit.google && current_gate.ids.back() == gate::Gates::Hadamard) {
                amp.ApplyHGateOnAllAmps(qubits);
                i += qubits - 1;
                g_end = clock();
                gate_time[0] += double(g_end - g_begin)/ CLOCKS_PER_SEC;
            }
            else {
                amp.ApplyNonCGate(current_gate.qubits, qubits, current_gate,
                                  (gate::Gates)current_gate.ids.back());
                g_end = clock();
                
                if(current_gate.ids.back() == gate::Gates::X_1_2){
                    gate_time[2] += double(g_end - g_begin)/ CLOCKS_PER_SEC;
                    X++;
                    amp.IncrementGlobalFactorPower(2);
                }
                else if(current_gate.ids.back() == gate::Gates::Y_1_2) {
                    gate_time[3] += double(g_end - g_begin)/ CLOCKS_PER_SEC;
                    Y++;
                    amp.IncrementGlobalFactorPower(2);
                }
            }
        }
    }
    
    clock_t end = clock();
    
    if (qubits <= 16)
        amp.PrintStateVector();
    
    PrintReport(end , begin, amp, circuit, sim_depth);
}

sequentialSimulation& sequentialSimulation::
operator=(const sequentialSimulation& rhs)
{
    sequentialSimulation temp(rhs);
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

void sequentialSimulation::
PrintReport(const clock_t end,
            const clock_t begin,
            state& amp,
            const circuit& circuit,
            const int sim_depth) const
{
    char hostname[20] = {};
    gethostname(hostname, 20);
    cout << "Hostname : ";
    for(auto h : hostname) {
        cout << h;
    }
    cout << "\n";
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
    cout << "Cycles : " << sim_depth << "\n\n";
    
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
    cout << "Total runtime : " << total_time << "s\n";
    cout << "Norm : " << amp.CalculateNormOfAmp() << "\n";
    cout << "Probabilities : " << amp.GetMinProb() << "(min), "
         << amp.GetMaxProb() << "(max), "
         << amp.GetAvgProb() << "(avg)\n";
    cout << "Log_2 (max / min) = " << log2(real(amp.GetMaxProb())/real(amp.GetMinProb())) << "\n" ;
    cout << "Rescaling passes : " << num_rescaling << "\n\n";
    
    cout << "Runtimes by gate type\n";
    cout << "   H (" << circuit.GetNumQubits() << ") : " << gate_time[0]
    << "s = " << (gate_time[0]/total_time) * 100 << "%\n";

    cout << "   CZ & T (" << CZ_T << ") : " << gate_time[1]
    << "s = " << (gate_time[1]/total_time) * 100 << "%\n";
    
    cout << "   X (" << X << ") : " << gate_time[2] << "s = "
    << (gate_time[2]/total_time) * 100 << "%\n";
    
    cout << "   Y (" << Y << ") : " << gate_time[3]
    << "s = " << (gate_time[3]/total_time) * 100 << "%\n";
    
    cout << "   Merged X & Y (" << 2 * merged_X_Y << ") : " << gate_time[4]
    << "s = " << (gate_time[4]/total_time) * 100 << "%\n\n";
}

void sequentialSimulation::
PrintReport(const string &outfile,
            const clock_t end,
            const clock_t begin,
            state& amp,
            const circuit& circuit,
            const int sim_depth) const
{
    ofstream file;
    file.open("simulation_stats/" + outfile);
    
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
    file << "Cycles : " << sim_depth << "\n\n";
    
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
    file << "Total runtime : " << total_time << "s\n";
    file << "Norm : " << amp.CalculateNormOfAmp() << "\n";
    file << "Probabilities : " << amp.GetMinProb() << "(min), "
    << amp.GetMaxProb() << "(max), "
    << amp.GetAvgProb() << "(avg)\n";
    file << "Log_2 (max / min) = " << log2(real(amp.GetMaxProb())/real(amp.GetMinProb())) << "\n" ;
    file << "Rescaling passes : " << num_rescaling << "\n\n";
    
    file << "Runtimes by gate type\n";
    file << "   H (" << circuit.GetNumQubits() << ") : " << gate_time[0]
    << "s = " << (gate_time[0]/total_time) * 100 << "%\n";
    
    file << "   CZ & T (" << CZ_T << ") : " << gate_time[1]
    << "s = " << (gate_time[1]/total_time) * 100 << "%\n";
    
    file << "   X (" << X << ") : " << gate_time[2] << "s = "
    << (gate_time[2]/total_time) * 100 << "%\n";
    
    file << "   Y (" << Y << ") : " << gate_time[3]
    << "s = " << (gate_time[3]/total_time) * 100 << "%\n";
    
    file << "   Merged X & Y (" << 2 * merged_X_Y << ") : " << gate_time[4]
    << "s = " << (gate_time[4]/total_time) * 100 << "%\n\n";;
}


