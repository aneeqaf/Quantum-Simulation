//
//  state.cpp
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 12/13/17.
//  Copyright © 2017 Aneeqa Fatima. All rights reserved.
//

#include "state.h"

State::
State(int qubits): global_factor_power(0)
{
    amp.resize(1ull << qubits, 0);
    amp[0] = 1;
}

State::
State(valarray<cmplx>& a): amp(a), global_factor_power(0) {}

State::State(const State& rhs)
{
    amp = rhs.amp;
    global_factor_power = rhs.global_factor_power;
}

State& State::
operator=(const State& rhs)
{
    State temp(rhs);
    swap(amp, temp.amp);
    swap(global_factor_power, temp.global_factor_power);
    return *this;
}

void State::
ApplyBlockOfDiagGates(const vector<Gate>& block_gates,
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

void State::
ApplyGateFWHT()
{
    
}

void State::
ApplyNonCGate(const vector<int>& gate_qubits,
              const int qubits,
              const Gate& g,
              const Gate::type gate_type)
{
    ApplyNonControl1QGates(gate_qubits, amp, qubits, g, gate_type);
}

void State::
ApplyHGateOnAllAmps(const int qubits)
{
    amp = cmplx(1, 0);
    global_factor_power += qubits;
}

void State::
ApplyCGate(const int num_controls,
           const vector<int>& gate_qubits,
           const int qubits,
           const Gate& g,
           const Gate::type gate_type)
{
    ApplyControlGate(num_controls, gate_qubits, amp, qubits, g, gate_type);
}

void State::
ApplyTwoMergedXYGate(Gate& gate1,
                     Gate& gate2,
                     const int qubits)
{
    Merge2QXY12Gates(gate1, gate2, qubits, amp);
    global_factor_power += 2;
}

double State::
GetMinProb() const
{
    double min = numeric_limits<double>::max();
    for (auto p : amp)
        if (min > real(p))
            min = real(p);
    return min;
}

double State::
GetMaxProb() const
{
    double max = numeric_limits<double>::min();
    for (auto p : amp)
        if (max < real(p))
            max = real(p);
    return max;
}

double State::
GetAvgProb() const
{
    return 1.0/(1ull << (idx_size)(log2(amp.size())));
}

double State::
GetMemUsage() const
{
    return sizeof(vector<cmplx>) + (sizeof(cmplx) * amp.size()) ;
}

idx_size State::
GetGlobalFactorPower() const
{
    return global_factor_power;
}

float State::
CalculateNormOfAmp()
{
    double norm = 0;
    idx_size qubits = log2(amp.size());
    cmplx prob(0, 0);
    
    amp /= pow(2,(global_factor_power/2));
    if (global_factor_power % 2 == 1)
        amp /= sqrt(2);
    
    auto conj_op = [](valarray<cmplx>::value_type v) {return conj(v);};
    amp *= amp.apply(conj_op);
    for (const auto& prob : amp)
        if (real(prob) > (1.0/(1ull << qubits)))
            norm += real(prob);
    
    for (const auto& prob : amp)
        if (real(prob) <= (1.0/(1ull << qubits)))
            norm += real(prob);
    
    return norm;
}

const valarray<cmplx>& State::
GetAmp() const
{
    return amp;
}

void State::
IncrementGlobalFactorPower(int num)
{
    global_factor_power += num;
}

void State::
ResetGlobalFactorPower()
{
    global_factor_power = 0;
}

cmplx State::
ComputeRescalingFactor()
{
    cmplx rescaling_factor(0,0);
    rescaling_factor = cmplx(1.0)/cmplx(pow(2,(global_factor_power/2)));
    if ((global_factor_power % 2) == 1)
        rescaling_factor *= 1.0/sqrt(2.0);
    global_factor_power = 0;
    
    return rescaling_factor;
}

void State::
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

void State::
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

void State::
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
