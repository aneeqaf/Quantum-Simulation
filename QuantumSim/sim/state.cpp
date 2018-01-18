//
//  state.cpp
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 12/13/17.
//

#include "state.h"

using namespace std;

State::
State(int qubits): max_prob(numeric_limits<double>::min()), min_prob(numeric_limits<double>::max()),
global_factor_power(0), global_i_counter(0)
{
    amp_size = 1ull << qubits;
    posix_memalign((void**)&amp, 64, sizeof(cmplx) * amp_size);
    memset(amp, 0, amp_size * sizeof(amp));
    amp[0] = 1;
}

State::
State(cmplx* a, idx_size size): max_prob(numeric_limits<double>::min()), min_prob(numeric_limits<double>::max()),
amp_size(size), global_factor_power(0), global_i_counter(0)
{
    posix_memalign((void**)&amp, 64, sizeof(cmplx) * amp_size);
    for (idx_size i = 0; i < size; ++i)
        amp[i] = a[i];
}

State::State(const State& rhs)
{
    amp_size = rhs.GetAmpSize();
    posix_memalign((void**)&amp, 64, sizeof(cmplx) * amp_size);
    for (idx_size i = 0; i < rhs.GetAmpSize(); ++i)
        amp[i] = rhs.amp[i];
    global_factor_power = rhs.global_factor_power;
    global_i_counter = rhs.global_i_counter;
    min_prob = rhs.min_prob;
    max_prob = rhs.max_prob;
}

State& State::
operator=(const State& rhs)
{
    State temp(rhs);
    swap(amp, temp.amp);
    swap(amp_size, temp.amp_size);
    swap(global_factor_power, temp.global_factor_power);
    swap(global_i_counter, temp.global_i_counter);
    swap(min_prob, temp.min_prob);
    swap(max_prob, temp.max_prob);
    return *this;
}
State::
~State()
{
    delete [] amp;
    amp = nullptr;
}

idx_size State::
FormBitmask(const vector<int>& qubits)
{
    idx_size qubits_bitmask = 0;
    for (idx_size i = 0; i < qubits.size(); ++i)
        qubits_bitmask |= qubits[i];
    
    return qubits_bitmask;
}

void State::
ApplyBlockOfDiagGates(const idx_size* __restrict CZ_bitmasks,
                      const idx_size __restrict T_bitmasks[2],
                      const int total_circuit_qubits)
{
   ApplyBlockOfCZTGatesAVX(amp, total_circuit_qubits, CZ_bitmasks, T_bitmasks);
}

void State::
ApplyNonCGate(const int gate_qubit,
              const int total_circuit_qubits,
              const Gate::Type gate_type,
              const Gate& g)
{
    ApplyNonControl1QGates(amp, gate_qubit, total_circuit_qubits, gate_type, g);
}

void State::
ApplyHGateOnAllAmps()
{
//    fill_n(amp, amp_size, 1);
    const int total_circuit_qubits = log2(amp_size);
    
    float* __restrict t_amp = (float*)__builtin_assume_aligned(amp, 64);
    constexpr __m256 re_ones = {1, 0, 1, 0, 1, 0 , 1, 0};
    for (idx_size i = 0; i < amp_size; i += 4) {
        __m256 t = _mm256_load_ps(t_amp + (2 * i));
        t = _mm256_or_ps(t, re_ones);
        _mm256_store_ps(t_amp + (2 * i), t);
    }
    
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
ApplyMergedXYGate(const Gate& gate1,
                  const Gate& gate2,
                  const int total_circuit_qubits)
{
    Apply2MergedXY12Gates(gate1, gate2, amp, total_circuit_qubits);
    
    global_factor_power += 2;
    
    if (gate1.ids.back() == Gate::Type::Y_1_2 && gate2.ids.back() == Gate::Type::Y_1_2)
        ++global_i_counter;
    
}

void State::
ApplyClusterOfXYHGates(idx_size& gate_i,
                       idx_size& odd_Xi,
                       idx_size& odd_Yi,
                       const vector<Gate>& all_gates,
                       const int total_circuit_qubits)
{
    vector<int> qubits_in_cluster1 , qubits_in_cluster2;
    
    if ((Gate::Type)all_gates[gate_i].ids.back() == Gate::Type::X_1_2)
        qubits_in_cluster1 = FormBlockOfXYHGates(gate_i, Gate::Type::X_1_2, all_gates);
    
    if (qubits_in_cluster1.size() % 2 == 1) {
        qubits_in_cluster1.pop_back();
        odd_Xi = gate_i - 1;
    }
    
    if ((Gate::Type)all_gates[gate_i].ids.back() == Gate::Type::Y_1_2)
        qubits_in_cluster2 = FormBlockOfXYHGates(gate_i, Gate::Type::Y_1_2, all_gates);
    
    if (qubits_in_cluster2.size() % 2 == 1) {
        qubits_in_cluster2.pop_back();
        odd_Yi = gate_i - 1;
    }
    
    if (qubits_in_cluster1.size()) {
        const idx_size clus1_q_bitmask = FormBitmask(qubits_in_cluster1);
        ApplyFWHT(amp, clus1_q_bitmask, total_circuit_qubits, Gate::Type::X_1_2);
        global_factor_power += qubits_in_cluster1.size();
    }
    if (qubits_in_cluster2.size()) {
        const idx_size clus2_q_bitmask = FormBitmask(qubits_in_cluster2);
        ApplyFWHT(amp, clus2_q_bitmask, total_circuit_qubits, Gate::Type::Y_1_2);
        global_factor_power += qubits_in_cluster2.size();
        global_i_counter += qubits_in_cluster2.size()/2;
    }
    
    if (odd_Xi && odd_Yi) {
        global_factor_power += 2;
        Apply2MergedXY12Gates(all_gates[odd_Xi], all_gates[odd_Yi], amp, total_circuit_qubits);
    }
}

void State::
ApplyXYRecursiveTransform(idx_size X_bitmask,
                          idx_size Y_bitmask,
                          const int total_circuit_qubits,
                          const int th)
{
    idx_size num_Xgates = __builtin_popcountll(X_bitmask), num_Ygates = __builtin_popcountll(Y_bitmask);
    if ((num_Xgates + num_Ygates) % 2 == 1) {
        const int X_q = X_bitmask ? __builtin_ctzl(X_bitmask) : 1000;
        const int Y_q = Y_bitmask ? __builtin_ctzl(Y_bitmask) : 1000;
        
        if (X_q < Y_q) {
            ApplyNonControl1QGates(amp, X_q, total_circuit_qubits, Gate::Type::X_1_2);
            X_bitmask ^= 1ull << X_q;
            global_factor_power += 2;
            --num_Xgates;
        }
        else {
            ApplyNonControl1QGates(amp, Y_q, total_circuit_qubits, Gate::Type::Y_1_2);
            Y_bitmask ^= 1ull << Y_q;
            global_factor_power += 2;
            --num_Ygates;
        }
    }
    
    if (X_bitmask || Y_bitmask)
      global_i_counter += XYRecursiveTransform(amp, X_bitmask, Y_bitmask, total_circuit_qubits, th);
    
    if (num_Xgates)
        global_factor_power += num_Xgates;
    if (num_Ygates)
        global_factor_power += num_Ygates;
}

cmplx State::
operator[](idx_size i) const
{
    return amp[i];
}

double State::
GetMinProb() const
{
    return min_prob;
}

double State::
GetMaxProb() const
{
    return max_prob;
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

idx_size State::
GetGlobalIcounter() const
{
    return global_i_counter;
}

double State::
CalculateNormOfAmp()
{
    double norm = 0;
    idx_size qubits = log2(amp_size);
    
    for (idx_size i = 0; i < amp_size; ++i) {
        cmplx t = amp[i];
        t /= pow(2,(global_factor_power/2));
        if (global_factor_power % 2 == 1)
            t /= sqrt(2);
        
        t *= conj(t);
        if (real(t) > (1.0/(1ull << qubits)))
            norm += real(t);
        
        if (min_prob > real(t))
            min_prob = real(t);
        
        if (max_prob < real(t))
            max_prob = real(t);
    }
    
    for (idx_size i = 0; i < amp_size; ++i) {
        auto t = amp[i];
        t /= pow(2,(global_factor_power/2));
        if (global_factor_power % 2 == 1)
            t /= sqrt(2);
        
        t *= conj(t);
        if (real(t) <= (1.0/(1ull << qubits)))
            norm += real(t);
    }
    
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
    float rescaling_factor = 1.0/pow(2,(global_factor_power/2));
    if ((global_factor_power % 2) == 1)
        rescaling_factor *= 1.0/sqrt(2.0);
    global_factor_power = 0;
    
//    for (idx_size i = 0; i < amp_size; ++i)
//        amp[i] *= rescaling_factor;
    
    float* __restrict t_amp = (float*)__builtin_assume_aligned(amp, 64);
    const __m256 rescaling = {rescaling_factor, rescaling_factor, rescaling_factor, rescaling_factor,
        rescaling_factor, rescaling_factor , rescaling_factor, rescaling_factor};
    for (idx_size i = 0; i < amp_size; i += 4) {
        __m256 t = _mm256_load_ps(t_amp + (2 * i));
        t = _mm256_mul_ps(t, rescaling);
        _mm256_store_ps(t_amp + (2 * i), t);
    }
    
}

void State::
ApplyGlobalICounter()
{
    for (idx_size i = 0; i < amp_size; ++i)
        amp[i] *= pow(ki, global_i_counter);
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
        state_v *= pow(ki, global_i_counter);
        
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
