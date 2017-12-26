//
//  state.cpp
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 12/13/17.
//

#include "state.h"

using namespace std;

State::
State(int qubits): global_factor_power(0)
{
    amp_size = 1ull << qubits;
    
#ifndef __APPLE__
    amp = static_cast<cmplx*>(aligned_alloc(64, sizeof(cmplx) * amp_size));
#else
    amp = new cmplx[amp_size];
#endif
//    amp = new cmplx[amp_size];
    memset(amp, 0, amp_size * sizeof(amp));
    amp[0] = 1;
}

State::
State(cmplx* a, idx_size size): amp_size(size), global_factor_power(0)
{
#ifndef __APPLE__
    amp = static_cast<cmplx*>(aligned_alloc(64, sizeof(cmplx) * size));
#else
    amp = new cmplx[size];
#endif
//    amp = new cmplx[size];
    for (idx_size i = 0; i < size; ++i)
        amp[i] = a[i];
}

State::State(const State& rhs)
{
#ifndef __APPLE__
    amp = static_cast<cmplx*>(aligned_alloc(64, sizeof(cmplx) * rhs.GetAmpSize()));
#else
    amp = new cmplx[rhs.GetAmpSize()];
#endif
//     amp = new cmplx[rhs.GetAmpSize()];
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
ApplyBlockOfDiagGates(const vector<Gate>& cluster,
                      const int total_circuit_qubits,
                      idx_size& gate_i)
{
    valarray<idx_size> T_bitmask (2);
    valarray<idx_size> CZ_bitmask (total_circuit_qubits);

    FormBlockOfCZTGates(gate_i, CZ_bitmask, T_bitmask, cluster, total_circuit_qubits);
    
    if (global_factor_power > 100)
        Rescale();
   
    ApplyBlockOfCZTGates(amp, amp_size, total_circuit_qubits, CZ_bitmask, T_bitmask);
}

void State::
ApplyNonCGate(const vector<int>& gate_qubits,
              const int total_circuit_qubits,
              const Gate& g,
              const Gate::Type gate_type)
{
    ApplyNonControl1QGates(amp, amp_size,  gate_qubits[0], total_circuit_qubits, g, gate_type);
}

void State::
ApplyHGateOnAllAmps(const int total_circuit_qubits)
{
    for (idx_size i = 0; i < amp_size; ++i)
        amp[i] = cmplx(1,0);
    
    global_factor_power += total_circuit_qubits;
}

void State::
ApplyCGate(const int num_controls,
           const vector<int>& gate_qubits,
           const int total_circuit_qubits,
           const Gate& g,
           const Gate::Type gate_type)
{
    ApplyControlGate(amp, num_controls, gate_qubits, total_circuit_qubits, g, gate_type);
}

void State::
ApplyMergedXYGate(const vector<Gate>& all_gates,
                  idx_size& gate_i,
                  const int total_circuit_qubits)
{
    vector<Gate> cluster;
    FormBlockOfXYHGates(cluster, gate_i, all_gates);
    idx_size num_gates = cluster.size();
    
    if (num_gates % 2 == 1) {
        --gate_i;
        --num_gates;
        cluster.pop_back();
    }
    
    if (num_gates == 2)
        Merge2XY12Gates(cluster[0], cluster[1], amp, amp_size, total_circuit_qubits);
    else if (num_gates == 4)
        Merge4XY12Gates(cluster, amp, amp_size, total_circuit_qubits);
    
    global_factor_power += num_gates;
}

void State::
ApplyClusterOfXYHGates(const vector<Gate>& all_gates,
                       const int total_circuit_qubits,
                       idx_size& gate_i,
                       Gate::Type gate_type)
{
    vector<int> qubits_in_cluster =
            FormBlockOfXYHGates(gate_i, gate_type, all_gates);
    
    ApplyFWHT(amp, amp_size, qubits_in_cluster, total_circuit_qubits, gate_type);
    
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

void State::
Rescale()
{
    cmplx rescaling_factor(0,0);
    rescaling_factor = cmplx(1.0)/cmplx(pow(2,(global_factor_power/2)));
    if ((global_factor_power % 2) == 1)
        rescaling_factor *= 1.0/sqrt(2.0);
    global_factor_power = 0;
    
    for (idx_size i = 0; i < amp_size; ++i)
        amp[i] *= rescaling_factor;
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
