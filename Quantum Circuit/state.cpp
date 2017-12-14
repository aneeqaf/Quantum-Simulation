//
//  state.cpp
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 12/13/17.
//  Copyright © 2017 Aneeqa Fatima. All rights reserved.
//

#include "state.h"

state::
state(int qubits): global_factor_power(0)
{
    amp.resize(1ull << qubits, 0);
    amp[0] = 1;
}

state::
state(valarray<cmplx>& a): amp(a), global_factor_power(0) {}

state::state(const state& rhs)
{
    amp = rhs.amp;
    global_factor_power = rhs.global_factor_power;
}

state& state::
operator=(const state& rhs)
{
    state temp(rhs);
    swap(amp, temp.amp);
    swap(global_factor_power, temp.global_factor_power);
    return *this;
}

void state::
ApplyBlockOfDiagGates(const vector<gate>& block_gates,
                      const int qubits,
                      idx_size& gate_i)
{
    valarray<idx_size> T_bitmask (2);
    valarray<idx_size> CZ_bitmask (qubits);

    FormBlockOfCZTGates(block_gates, qubits, gate_i, CZ_bitmask, T_bitmask);
    
    if (global_factor_power > 100)
        ApplyBlockOfGates(qubits, CZ_bitmask, T_bitmask, amp, ComputeRescalingFactor());
   
    else
        ApplyBlockOfGates(qubits, CZ_bitmask, T_bitmask, amp);
}

void state::
ApplyGateFWHT()
{
    
}

void state::
ApplyNonCGate(const vector<int>& gate_qubits,
              const int qubits,
              const gate& g,
              const gate::Gates gate_type)
{
    ApplyNonControl1QGates(gate_qubits, amp, qubits, g, gate_type);
}

void state::
ApplyHGateOnAllAmps(const int qubits)
{
    amp = cmplx(1, 0);
    global_factor_power += qubits;
}

void state::
ApplyCGate(const int num_controls,
           const vector<int>& gate_qubits,
           const int qubits,
           const gate& g,
           const gate::Gates gate_type)
{
    ApplyControlGate(num_controls, gate_qubits, amp, qubits, g, gate_type);
}

void state::
ApplyTwoMergedXYGate(vector<gate>& gates,
                     idx_size gate1,
                     idx_size gate2,
                     const int qubits)
{
    Merge2QXY12Gates(gates, gate1, gate2, qubits, amp);
    global_factor_power += 2;
}

double state::
GetMinProb() const
{
    double min = numeric_limits<double>::max();
    for (auto p : amp)
        if (min > real(p))
            min = real(p);
    return min;
}

double state::
GetMaxProb() const
{
    double max = numeric_limits<double>::min();
    for (auto p : amp)
        if (max < real(p))
            max = real(p);
    return max;
}

double state::
GetAvgProb() const
{
    return 1.0/(1ull << (idx_size)(log2(amp.size())));
}

double state::
GetMemUsage() const
{
    return sizeof(vector<cmplx>) + (sizeof(cmplx) * amp.size()) ;
}

idx_size state::
GetGlobalFactorPower() const
{
    return global_factor_power;
}

float state::
CalculateNormOfAmp()
{
    double norm = 0;
    cmplx prob(0, 0);
    
    amp /= pow(2,(global_factor_power/2));
    if (global_factor_power % 2 == 1)
        amp /= sqrt(2);
    
    auto conj_op = [](valarray<cmplx>::value_type v) {return conj(v);};
    amp *= amp.apply(conj_op);
    norm = real(amp.sum());
//    for (auto& prob : amp)
//        prob *= conj(prob);
//        if (real(prob) > (1.0/(1ull << qubits)))
//            norm += real(prob);
//
//    for (const auto& prob : amp)
//        if (real(prob) <= (1.0/(1ull << qubits)))
//            norm += real(prob);
    return norm;
}

const valarray<cmplx>& state::
GetAmp() const
{
    return amp;
}

void state::
IncrementGlobalFactorPower(int num)
{
    global_factor_power += num;
}

void state::
ResetGlobalFactorPower()
{
    global_factor_power = 0;
}

cmplx state::
ComputeRescalingFactor()
{
    cmplx rescaling_factor(0,0);
    rescaling_factor = cmplx(1.0)/cmplx(pow(2,(global_factor_power/2)));
    if ((global_factor_power % 2) == 1)
        rescaling_factor *= 1.0/sqrt(2.0);
    global_factor_power = 0;
    
    return rescaling_factor;
}

void state::
PrintProbabilities(const string &out_file) const
{
    static int count = 0;
    ofstream file;
    file.open(out_file + to_string(count) + ".txt");
    
    for (auto state_v : amp) {
        state_v /= pow(2,(global_factor_power/2));
        if (global_factor_power % 2 == 1)
            state_v /= sqrt(2);
        state_v *= conj(state_v); /// cmplx(pow(2, qubits));
        
        file << real(state_v) ;
        
        if (imag(state_v) > 0) {
            file << "+" << imag(state_v) << "i";
        }
        else if (imag(state_v) < 0) {
            file << imag(state_v) << "i";
        }
        file << "\n";
    }
    //file << "\n";
    ++count;
}

void state::
PrintStateVector() const
{
    for (auto state_v :amp) {
        state_v /= pow(2,(global_factor_power/2));
        if (global_factor_power % 2 == 1)
            state_v /= sqrt(2);
        
        cout << real(state_v) ;
        
        if (imag(state_v) > 0) {
            cout << "+" << imag(state_v) << "i";
        }
        else if (imag(state_v) < 0) {
            cout << imag(state_v) << "i";
        }
        cout << "\n";
    }
    cout << "\n\n";
}

void state::
PrintStateVector(const string& outfile) const
{
    static ofstream file;
    file.open(outfile + ".txt");
    
    for (auto state_v : amp) {
        state_v /= pow(2,(global_factor_power/2));
        if (global_factor_power % 2 == 1)
            state_v /= sqrt(2);
        
        file << real(state_v) ;
        
        if (imag(state_v) > 0) {
            file << "+" << imag(state_v) << "i";
        }
        else if (imag(state_v) < 0) {
            file << imag(state_v) << "i";
        }
        file << "\n";
    }
    file << "\n\n";
}
