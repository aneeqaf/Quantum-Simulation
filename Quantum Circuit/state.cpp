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
    amp_size = 1ull << qubits;
    amp = new cmplx[amp_size];
    memset(amp, 0, amp_size * sizeof(amp));
    amp[0] = 1;
}

State::
State(cmplx* a, idx_size size): amp_size(size), global_factor_power(0)
{
    amp = new cmplx[size];
    for (idx_size i = 0; i < size; ++i)
        amp[i] = a[i];
}

State::State(const State& rhs)
{
    amp = new cmplx[rhs.GetAmpSize()];
    for (idx_size i = 0; i < rhs.GetAmpSize(); ++i)
        amp[i] = rhs.amp[i];
    amp_size = rhs.GetAmpSize();
    global_factor_power = rhs.global_factor_power;
}

State& State::
operator=(const State& rhs)
{
    State temp(rhs);
    swap(amp, temp.amp);
    swap(amp_size, temp.amp_size);
    swap(global_factor_power, temp.global_factor_power);
    return *this;
}
State::
~State()
{
    delete [] amp;
    amp = nullptr;
}

void State::
ApplyBlockOfDiagGates(const vector<Gate>& block_gates,
                      const int total_q_cir,
                      idx_size& gate_i)
{
    valarray<idx_size> T_bitmask (2);
    valarray<idx_size> CZ_bitmask (total_q_cir);

    FormBlockOfCZTGates(block_gates, total_q_cir, gate_i, CZ_bitmask, T_bitmask);
    
    if (global_factor_power > 100)
        ApplyBlockOfGates(total_q_cir, CZ_bitmask, T_bitmask, amp, amp_size, ComputeRescalingFactor());
   
    else
        ApplyBlockOfGates(total_q_cir, CZ_bitmask, T_bitmask, amp, amp_size);
}

void State::
ApplyNonCGate(const vector<int>& gate_qubits,
              const int total_q_cir,
              const Gate& g,
              const Gate::Type gate_type)
{
    ApplyNonControl1QGates(gate_qubits[0], amp, amp_size, total_q_cir, g, gate_type);
}

void State::
ApplyHGateOnAllAmps(const int total_q_cir)
{
    for (idx_size i = 0; i < amp_size; ++i)
        amp[i] = cmplx(1,0);
    
    global_factor_power += total_q_cir;
}

void State::
ApplyCGate(const int num_controls,
           const vector<int>& gate_qubits,
           const int total_q_cir,
           const Gate& g,
           const Gate::Type gate_type)
{
    ApplyControlGate(num_controls, gate_qubits, amp, total_q_cir, g, gate_type);
}

void State::
ApplyTwoMergedXYGate(Gate& gate1,
                     Gate& gate2,
                     const int total_q_cir)
{
    Merge2QXY12Gates(gate1, gate2, total_q_cir, amp, amp_size);
    global_factor_power += 2;
}

void State::
ApplyClusterOfXYHGates(const vector<Gate>& block_gates,
                       const int total_q_cir,
                       idx_size& gate_i,
                       Gate::Type gate_type)
{
    vector<int> qubits_in_cluster =
            FormBlockOfXYHGates(block_gates, gate_i, gate_type);
    
    ApplyFWHT(amp, amp_size, qubits_in_cluster, total_q_cir, gate_type);
    
    global_factor_power += (2 * qubits_in_cluster.size());
}

double State::
GetMinProb() const
{
    double min = numeric_limits<double>::max();
    for (idx_size i = 0; i < amp_size; ++i)
        if (min > real(amp[i]))
            min = real(amp[i]);
    return min;
}

double State::
GetMaxProb() const
{
    double max = numeric_limits<double>::min();
    for (idx_size i = 0; i < amp_size; ++i)
        if (max < real(amp[i]))
            max = real(amp[i]);
    return max;
}

double State::
GetAvgProb() const
{
    return 1.0/(1ull << (idx_size)(log2(amp_size)));
}

double State::
GetMemUsage() const
{
    return sizeof(vector<cmplx>) + (sizeof(cmplx) * amp_size) ;
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
    idx_size qubits = log2(amp_size);
    
    for (idx_size i = 0; i < amp_size; ++i) {
        amp[i] /= pow(2,(global_factor_power/2));
        if (global_factor_power % 2 == 1)
            amp[i] /= sqrt(2);
        
        amp[i] *= conj(amp[i]);
        if (real(amp[i]) > (1.0/(1ull << qubits)))
            norm += real(amp[i]);
    }
    for (idx_size i = 0; i < amp_size; ++i)
        if (real(amp[i]) <= (1.0/(1ull << qubits)))
            norm += real(amp[i]);
    
    return norm;
}

const cmplx* const State::
GetAmp() const
{
    return amp;
}

idx_size State::
GetAmpSize() const
{
    return amp_size;
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
    
    for (idx_size i = 0; i < amp_size; ++i) {
        auto state_v = amp[i];
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
    for (idx_size i = 0; i < amp_size; ++i) {
        auto state_v = amp[i];
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
    
    for (idx_size i = 0; i < amp_size; ++i) {
        auto state_v = amp[i];
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
