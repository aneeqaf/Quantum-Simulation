//
//  state.cpp
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 12/13/17.
//

#include "state.h"

using namespace std;

FullAmpStateVector::
FullAmpStateVector(const int qubits): max_prob(numeric_limits<double>::min()), min_prob(numeric_limits<double>::max()),
global_factor_power(0), global_i_counter(0), num_qubits(qubits)
{
    amp_size = 1ull << qubits;
    posix_memalign((void**)&amp, 64, sizeof(cmplx) * amp_size);
    memset(amp, 0, amp_size * sizeof(amp));
    amp[0] = 1;
}

FullAmpStateVector::
FullAmpStateVector(cmplx* a,
                   const idx_size size): max_prob(numeric_limits<double>::min()),
min_prob(numeric_limits<double>::max()), amp_size(size), global_factor_power(0), global_i_counter(0),
num_qubits(__builtin_log2l(size))
{
    posix_memalign((void**)&amp, 64, sizeof(cmplx) * amp_size);
    for (idx_size i = 0; i < size; ++i)
        amp[i] = a[i];
}

FullAmpStateVector::
FullAmpStateVector(const FullAmpStateVector& rhs)
{
    amp_size = rhs.amp_size;
    posix_memalign((void**)&amp, 64, sizeof(cmplx) * amp_size);
    idx_size size = 2 * rhs.GetSize();
    
    float* __restrict rhs_t_amp = (float*)__builtin_assume_aligned(rhs.amp, 64);
    float* __restrict t_amp = (float*)__builtin_assume_aligned(amp, 64);
    
    #pragma omp parallel for num_threads(num_threads)
    for (idx_size i = 0; i < size; i+=8) {
        const __m256 temp_amp = _mm256_load_ps (&rhs_t_amp[i]);
        _mm256_store_ps(&t_amp[i], temp_amp);
    }

    global_factor_power = rhs.global_factor_power;
    global_i_counter = rhs.global_i_counter;
    min_prob = rhs.min_prob;
    max_prob = rhs.max_prob;
    num_qubits = rhs.num_qubits;
}

FullAmpStateVector::
~FullAmpStateVector()
{
    free(amp);
    amp = nullptr;
}

bitset<128> FullAmpStateVector::
FormBitmask(const vector<int>& qubits)
{
    bitset<128> qubits_bitmask = 0;
    for (idx_size i = 0; i < qubits.size(); ++i)
        qubits_bitmask |= qubits[i];
    
    return qubits_bitmask;
}

int FullAmpStateVector::
ApplyBlockOfDiagGates(string& cz_bits,
                      idx_size prefix_size,
                      const bitset<128>* __restrict CZ_bitmasks,
                      const bitset<128>  T_bitmasks[2])
{
    Time time;
    time.StartTime();
    
    idx_size CZ_bitmasks_64[num_qubits];
    idx_size T_bitmasks_64[2] = {T_bitmasks[0].to_ulong(), T_bitmasks[1].to_ulong()};
    for (int i = 0; i < num_qubits; ++i)
        CZ_bitmasks_64[i] = CZ_bitmasks[i].to_ulong();
    
    if (num_qubits >= 4) {
#ifdef Parallel
        ApplyBlockOfCZTGatesAVXParallel(amp, num_qubits, CZ_bitmasks_64, T_bitmasks_64, num_threads);
#else
        ApplyBlockOfCZTGatesAVXSeq(amp, num_qubits, CZ_bitmasks_64, T_bitmasks_64);
#endif
    }
    else
        ApplyBlockOfCZTGates(amp, num_qubits, CZ_bitmasks_64, T_bitmasks_64);
    
    time_by_category.CZ_T += time.GetElapsedTime();
    
    return -1;
}

void FullAmpStateVector::
ApplyNonCGate(const int gate_qubit,
              const Gate::Type gate_type,
              const Gate& g)
{
    ApplyNonControl1QGates(amp, gate_qubit, num_qubits, gate_type, g);
    if (gate_type == Gate::Type::X_1_2 || gate_type == Gate::Type::Y_1_2)
        global_factor_power += 2;
}

void FullAmpStateVector::
ApplyCZDecompositions(const int gate_qubit,
                      const Gate::Type gate_type)
{
    if (gate_type != Gate::Type::CZ_D3)
        ApplyCZDecomposition(amp, num_qubits, gate_qubit, gate_type);
    if (gate_type == Gate::Type::CZ_D5)
        ++global_factor_power;
}

void FullAmpStateVector::
ApplyCZDecompositionDist(const idx_size* __restrict xCZ_bitmasks)
{
    /*0 : Z; 1 : 01; 2 : 10 */
    ApplyxCZGateAVX(amp, num_threads, num_qubits, xCZ_bitmasks);
}

void FullAmpStateVector::
ApplyHGateOnAllAmps()
{
    Time time;
    time.StartTime();
    
    float* __restrict t_amp = (float*)__builtin_assume_aligned(amp, 64);
    constexpr __m256 re_ones = {1, 0, 1, 0, 1, 0 , 1, 0};
    
    #pragma omp parallel for num_threads(num_threads)
    for (idx_size i = 0; i < amp_size; i += 4) {
        __m256 t = _mm256_load_ps(t_amp + (2 * i));
        t = _mm256_or_ps(t, re_ones);
        _mm256_store_ps(t_amp + (2 * i), t);
    }
    
    global_factor_power += num_qubits;
    
    time_by_category.H += time.GetElapsedTime();
}

void FullAmpStateVector::
ApplyCGate(const int num_controls,
           const vector<int>& gate_qubits,
           const Gate& g,
           const Gate::Type gate_type)
{
    ApplyControlGate(amp, num_controls, gate_qubits, num_qubits, g, gate_type);
}

void FullAmpStateVector::
ApplyMergedXYGate(const Gate& gate1,
                  const Gate& gate2)
{
    Time time;
    time.StartTime();
    
    Apply2MergedXY12Gates(gate1, gate2, amp, num_qubits);
    
    global_factor_power += 2;
    
    if (gate1.ids.back() == Gate::Type::Y_1_2 && gate2.ids.back() == Gate::Type::Y_1_2)
        ++global_i_counter;
    
    time_by_category.merged_XY1_2 += time.GetElapsedTime();
}


void FullAmpStateVector::
ApplyClusterOfXYHGates(idx_size& gate_i,
                       idx_size& odd_Xi,
                       idx_size& odd_Yi,
                       const vector<Gate>& all_gates)
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
        const bitset<128> clus1_q_bitmask = FormBitmask(qubits_in_cluster1);
        ApplyFWHT(amp, clus1_q_bitmask.to_ulong(), num_qubits, Gate::Type::X_1_2);
        global_factor_power += qubits_in_cluster1.size();
    }
    if (qubits_in_cluster2.size()) {
        const bitset<128> clus2_q_bitmask = FormBitmask(qubits_in_cluster2);
        ApplyFWHT(amp, clus2_q_bitmask.to_ulong(), num_qubits, Gate::Type::Y_1_2);
        global_factor_power += qubits_in_cluster2.size();
        global_i_counter += qubits_in_cluster2.size()/2;
    }
    
    if (odd_Xi && odd_Yi) {
        global_factor_power += 2;
        Apply2MergedXY12Gates(all_gates[odd_Xi], all_gates[odd_Yi], amp, num_qubits);
    }
}

void FullAmpStateVector::
ApplyXYRecursiveTransform(bitset<128> X_bitmask,
                          bitset<128> Y_bitmask,
                          const int th)
{
    Time time;
    idx_size X_bitmask_64 = X_bitmask.to_ulong(), Y_bitmask_64 = Y_bitmask.to_ulong();
    
    idx_size num_Xgates = __builtin_popcountll(X_bitmask_64),
    num_Ygates = __builtin_popcountll(Y_bitmask_64);
    if ((num_Xgates + num_Ygates) % 2 == 1) {
        time.StartTime();
        
        const int X_q = X_bitmask_64 ? __builtin_ctzl(X_bitmask_64) : 1000;
        const int Y_q = Y_bitmask_64 ? __builtin_ctzl(Y_bitmask_64) : 1000;

//        const int X_q = X_bitmask_64 ? 63 - __builtin_clzl(X_bitmask_64) : 1000;
//        const int Y_q = Y_bitmask_64 ? 63 - __builtin_clzl(Y_bitmask_64): 1000;
        
        if (!(X_q == 1000 && Y_q == 1000)) {
            if (X_q < Y_q) {
                Apply1QXYGates(amp, X_q, num_qubits, Gate::Type::X_1_2, num_threads);
                X_bitmask_64 ^= 1ull << X_q;
                global_factor_power += 2;
                --num_Xgates;
                time_by_category.X1_2 += time.GetElapsedTime();
                if (sim_mode != Config::SimMode::Phase2)
                    ++count_of_category.X1_2;
            }
            else {
                Apply1QXYGates(amp, Y_q, num_qubits, Gate::Type::Y_1_2, num_threads);
                Y_bitmask_64 ^= 1ull << Y_q;
                global_factor_power += 2;
                --num_Ygates;
                time_by_category.Y1_2 += time.GetElapsedTime();
                if (sim_mode != Config::SimMode::Phase2)
                    ++count_of_category.Y1_2;
            }
        }
    } 
    
    time.StartTime();
    
//    if (X_bitmask_64 || Y_bitmask_64)
//        global_i_counter += ApplyHighQXYGates(amp, X_bitmask_64, Y_bitmask_64, num_qubits);

    if (X_bitmask_64 || Y_bitmask_64)
        global_i_counter += XYFastTransform(amp, X_bitmask_64, Y_bitmask_64, num_qubits, num_threads, th);

    if (num_Xgates)
        global_factor_power += num_Xgates;
    if (num_Ygates)
        global_factor_power += num_Ygates;
    
    time_by_category.merged_XY1_2 += time.GetElapsedTime();
}

cmplx FullAmpStateVector::
operator[](bitset<128> i) const
{
    const float rescaling_factor = (global_factor_power % 2) ? 1.0/(pow(2,(global_factor_power/2)) * sqrt(2.0))
    : 1.0/pow(2,(global_factor_power/2));
    cmplx a = amp[i.to_ulong()] * cmplx(pow(ki, global_i_counter));
    a *= rescaling_factor;
    
    return a;
}

const cmplx* const FullAmpStateVector::
GetAmpVector() const
{
    return amp;
}

double FullAmpStateVector::
GetMinProb() const
{
    return min_prob;
}

double FullAmpStateVector::
GetMaxProb() const
{
    return max_prob;
}

double FullAmpStateVector::
GetAvgProb() const
{
    return 1.0/amp_size;
}

double FullAmpStateVector::
GetMemUsage() const
{
    return sizeof(cmplx) * amp_size;
}


double FullAmpStateVector::
CalculateNormSquared()
{
    float rescaling_factor = 1.0/pow(2,(global_factor_power/2));
    if ((global_factor_power % 2) == 1)
        rescaling_factor *= 1.0/sqrt(2.0);
    
    double norm_sq = 0;
    
    for (idx_size i = 0; i < amp_size; ++i) {
        float t = norm(amp[i] * rescaling_factor);
        
        if (t > (1.0/(1ull << num_qubits)))
            norm_sq += t;
        
        if (min_prob > t)
            min_prob = t;
        
        if (max_prob < t)
            max_prob = t;
    }
    
    for (idx_size i = 0; i < amp_size; ++i) {
        float t = norm(amp[i] * rescaling_factor);
    
        if (t <= (1.0/(1ull << num_qubits)))
            norm_sq += t;
    }
    
    return norm_sq;
}

double FullAmpStateVector::
CalculateAverageInaccuracy(double norm) const
{
    return abs(1.0 - norm) /(double)amp_size;
}

double FullAmpStateVector::
CalculateMeanEntropy() const
{
    float rescaling_factor = 1.0/pow(2,(global_factor_power/2));
    if ((global_factor_power % 2) == 1)
        rescaling_factor *= 1.0/sqrt(2.0);
    
    double entropy = 0.0;
    idx_size num_ranges = amp_size / sampling_factor;
    for (idx_size i = 0; i < num_ranges; ++i) {
        idx_size idx = (i * sampling_factor) + (rand() % sampling_factor);
        if (real(amp[idx]) > 1e-50 || imag(amp[idx]) > 1e-50)
             entropy += norm(amp[idx] * rescaling_factor) * log2l(norm(amp[idx] * rescaling_factor));
    }
    
    return -entropy * sampling_factor;
}

double FullAmpStateVector::
CalculateCrossEntropy(int range) const
{
    float rescaling_factor = 1.0/pow(2,(global_factor_power/2));
    if ((global_factor_power % 2) == 1)
        rescaling_factor *= 1.0/sqrt(2.0);
    
    double xe = 0.0;
    idx_size num_ranges = amp_size / range;
    for (idx_size i = 0; i < num_ranges; ++i) {
        idx_size idx = (i * range) + (rand() % range);
         if (real(amp[idx]) > 1e-20 || imag(amp[idx]) > 1e-20)
            xe += log2l(norm(amp[idx] * rescaling_factor)) ;
    }
    
    return -xe / num_ranges;
}

void FullAmpStateVector::
ResetAmpVector()
{
    float* __restrict t_amp = (float*)__builtin_assume_aligned(amp, 64);
    
    idx_size size = 2 * amp_size;
    #pragma omp parallel for num_threads(num_threads)
    for (idx_size i = 0; i < size; i+=8)
        _mm256_store_ps(&t_amp[i], kzeros);
    
    global_factor_power = 0;
    global_i_counter = 0;
}

void FullAmpStateVector::
Normalize()
{
    double norm = sqrt(CalculateNormSquared());
    if (norm != 0)
        for (idx_size i = 0; i < amp_size; ++i)
            amp[i] /= norm;
}

void FullAmpStateVector::
IncrementGlobalFactorPower()
{
    ++global_factor_power;
}

void FullAmpStateVector::
IncrementGlobalICounter()
{
    ++global_i_counter;
}

idx_size FullAmpStateVector::
GetSize() const
{
    return amp_size;
}

idx_size FullAmpStateVector::
GetFullStateVectorSize() const
{
    return amp_size;
}

idx_size FullAmpStateVector::
GetGlobalFactorPower() const
{
    return global_factor_power;
}

idx_size FullAmpStateVector::
GetGlobalICounter() const
{
    return global_i_counter;
}


int FullAmpStateVector::
GetNumQubits() const
{
    return num_qubits;
}

void FullAmpStateVector::
Rescale()
{
    const float rescaling_factor = (global_factor_power % 2) ? 1.0/(pow(2,(global_factor_power/2)) * sqrt(2.0))
                            : 1.0/pow(2,(global_factor_power/2));

    global_factor_power = 0;
//    for (idx_size i = 0; i < amp_size; ++i)
//        amp[i] *= rescaling_factor;
    
    float* __restrict t_amp = (float*)__builtin_assume_aligned(amp, 64);
    const __m256 rescaling = {rescaling_factor, rescaling_factor, rescaling_factor, rescaling_factor,
        rescaling_factor, rescaling_factor , rescaling_factor, rescaling_factor};
    
    #pragma omp parallel for num_threads(num_threads)
    for (idx_size i = 0; i < amp_size; i += 4) {
        __m256 t = _mm256_load_ps(t_amp + (2 * i));
        t = _mm256_mul_ps(t, rescaling);
        _mm256_store_ps(t_amp + (2 * i), t);
    }
}

void FullAmpStateVector::
RescaleAndApplyGlobalICounter()
{
    float rescaling_factor = 1.0/pow(2,(global_factor_power/2));
    if ((global_factor_power % 2) == 1)
        rescaling_factor *= 1.0/sqrt(2.0);
    global_factor_power = 0;

    const auto i_multiplier = cmplx(pow(ki, global_i_counter));
    global_i_counter = 0;
    
//    for (idx_size i = 0; i < amp_size; ++i)
//        amp[i] *= rescaling_factor * i_multiplier;
    
    float* __restrict t_amp = (float*)__builtin_assume_aligned(amp, 64);
    const __m256 rescaling = {rescaling_factor, rescaling_factor, rescaling_factor, rescaling_factor,
        rescaling_factor, rescaling_factor , rescaling_factor, rescaling_factor};

    #pragma omp parallel for num_threads(num_threads)
    for (idx_size i = 0; i < amp_size; i += 4) {
        amp[i] *= i_multiplier; amp[i + 1] *= i_multiplier; amp[i + 2] *= i_multiplier;
        amp[i + 3] *= i_multiplier;
        __m256 t = _mm256_load_ps(t_amp + (2 * i));
        t = _mm256_mul_ps(t, rescaling);
        _mm256_store_ps(t_amp + (2 * i), t);
    }

}

void FullAmpStateVector::
ApplyGlobalICounter()
{
    const auto multiplier = pow(ki, global_i_counter);
    for (idx_size i = 0; i < amp_size; ++i)
        amp[i] *= multiplier;
    global_i_counter = 0;
}

idx_size FullAmpStateVector::
CountZeroAmp() const
{
    idx_size zero_count = 0;
    
    for (idx_size i = 0; i < amp_size; ++i) {
        if (amp[i] == cmplx(0,0))
            ++zero_count;
    }
    
    return zero_count;
}

void FullAmpStateVector::
PrintProbabilities(const string &out_file,
                   const int cycle_num)
{
    ofstream file;
    file.open(out_file + "_" + to_string(cycle_num) + ".txt");
    
    RescaleAndApplyGlobalICounter();
    double norm_f = sqrt(CalculateNormSquared());
    
    srand(6);
    idx_size off = 0;
    for (idx_size i = 0; i + off < amp_size; i += off) {
        float prob = (norm(amp[i])/norm_f) * amp_size;
        file << prob << "\n";
        
        off = 1 + rand() % sampling_factor;
    }
}

void FullAmpStateVector::
PrintStateVector() 
{
    RescaleAndApplyGlobalICounter();
    
    for (idx_size i = 0; i < amp_size; ++i) {
        auto a = amp[i];
        cout << real(a) ;

        if (imag(a) > 0)
            cout << "+" << imag(a) << "j";
        else if (imag(a) < 0)
            cout << imag(a) << "j";
        cout << "\n";
    }
     cout << "\n\n";
}

void FullAmpStateVector::
PrintStateVector(const string& outfile,
                 const int cycle_num)
{
    ofstream file;
    file.open(outfile + "_" + to_string(cycle_num) + ".txt");
    
    RescaleAndApplyGlobalICounter();
    
    srand(6);
    idx_size off = 0;
    for (idx_size i = 0; i + off < amp_size; i += off) {
        auto a = amp[i];
        file << real(a) ;
        
        if (imag(a) > 0)
            file << "+" << imag(a) << "j";
        else if (imag(a) < 0)
            file << imag(a) << "j";
        file << "\n";
        
        off = 1 + rand() % sampling_factor;
    }
}
