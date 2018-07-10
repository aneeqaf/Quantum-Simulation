//
//  state.cpp
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 12/13/17.
//

#include "state.h"

using namespace std;

FullAmpStateVector::
FullAmpStateVector(const int qubits): max_prob(numeric_limits<double>::min()),
min_prob(numeric_limits<double>::max()), global_factor_power(0), global_i_counter(0),
num_qubits(qubits),zero_opt_mask(num_qubits), all_zeros(false)
{
    amp_size = 1ull << qubits;
    if (int err = posix_memalign((void**)&amp, 64, sizeof(cmplx) * amp_size) != 0) {
        idx_size memory = sizeof(cmplx) * amp_size;
        cerr << "Memory requirement exceeds availiable memory for aligned storage. Requested ";
        if (memory >= (1 << 30)) {
            cerr << memory / (1 << 30) << " GiB \n";
        }
        else if (memory >= (1 << 20)) {
            cerr << memory / (1 << 20) << " MiB \n";
        }
        else if (memory >= (1 << 10)) {
            cerr << memory / (1 << 10) << " KiB \n";
        }
        else
            cerr << memory << " B \n";
        free(amp);
        exit(err);
    }
    memset(amp, 0, amp_size * sizeof(amp));
    amp[0] = 1;
}

FullAmpStateVector::
FullAmpStateVector(cmplx* a,
                   const idx_size size): max_prob(numeric_limits<double>::min()),
min_prob(numeric_limits<double>::max()), amp_size(size), global_factor_power(0), global_i_counter(0),
num_qubits(__builtin_log2l(size)), zero_opt_mask(num_qubits), all_zeros(false)
{
    if (int err = posix_memalign((void**)&amp, 64, sizeof(cmplx) * amp_size) != 0) {
        idx_size memory = sizeof(cmplx) * amp_size;
        cerr << "Memory requirement exceeds availiable memory for aligned storage. Requested ";
        if (memory >= (1 << 30)) {
            cerr << memory / (1 << 30) << " GiB \n";
        }
        else if (memory >= (1 << 20)) {
            cerr << memory / (1 << 20) << " MiB \n";
        }
        else if (memory >= (1 << 10)) {
            cerr << memory / (1 << 10) << " KiB \n";
        }
        else
            cerr << memory << " B \n";
        free(amp);
        exit(err);
    }
    for (idx_size i = 0; i < size; ++i)
        amp[i] = a[i];
}

FullAmpStateVector::
FullAmpStateVector(const FullAmpStateVector& rhs):
zero_opt_mask(rhs.zero_opt_mask), min_prob(rhs.min_prob), max_prob(rhs.max_prob),
amp_size(rhs.amp_size), global_factor_power(rhs.global_factor_power),
global_i_counter(rhs.global_i_counter), num_qubits(rhs.num_qubits), all_zeros(rhs.all_zeros)
{
   if (int err = posix_memalign((void**)&amp, 64, sizeof(cmplx) * amp_size) != 0) {
        idx_size memory = sizeof(cmplx) * amp_size;
        cerr << "Memory requirement exceeds availiable memory for aligned storage. Requested ";
        if (memory >= (1 << 30)) {
            cerr << memory / (1 << 30) << " GiB \n";
        }
        else if (memory >= (1 << 20)) {
            cerr << memory / (1 << 20) << " MiB \n";
        }
        else if (memory >= (1 << 10)) {
            cerr << memory / (1 << 10) << " KiB \n";
        }
        else
            cerr << memory << " B \n";
        free(amp);
        exit(err);
    }
    
    idx_size size = 2 * rhs.GetSize();
    
    float* __restrict rhs_t_amp = (float*)__builtin_assume_aligned(rhs.amp, 64);
    float* __restrict t_amp = (float*)__builtin_assume_aligned(amp, 64);
    
    #pragma omp parallel for num_threads(num_threads)
    for (idx_size i = 0; i < size; i+=8) {
        const __m256 temp_amp = _mm256_load_ps (&rhs_t_amp[i]);
        _mm256_store_ps(&t_amp[i], temp_amp);
    }
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
                      const bitset<128>  T_bitmasks[2],
                      const bitset<128>& H_bitmask,
                      const bool last_cycle)
{
    Time time, time1;
    time.StartTime();
    
    if (all_zeros)
        return -1;
    
//    if (last_cycle)
//        for (int i = num_qubits - 1; i >= 0; --i)
//            UnsetZeroPatternAtQubit(num_qubits - 1 - i);
//
    int th = ceil((float)num_qubits/2.0);
    idx_size H_bitmask_64 = H_bitmask.to_ulong();
    idx_size hiH_bitmask = last_cycle ? (H_bitmask_64 & ((1ull << (num_qubits - th)) - 1)) : 0;
    idx_size loH_bitmask = last_cycle ? H_bitmask_64 & ~((1ull << (num_qubits - th)) - 1) : 0;
    idx_size CZ_bitmasks_64[num_qubits];
    idx_size T_bitmasks_64[2] = {T_bitmasks[0].to_ulong(), T_bitmasks[1].to_ulong()};
    bool CZ_app = false;
    for (int i = 0; i < num_qubits; ++i) {
        CZ_bitmasks_64[i] = CZ_bitmasks[i].to_ulong();
        if (CZ_bitmasks_64[i]) CZ_app = true;
    }
    time_by_category.low_q_XY_CZT += time.GetElapsedTime();
    int single_H = 0;
    
    if (zero_opt_mask.GetNonZeroBitmask()) {
        for (int i = num_qubits - 1; i >= 0; --i)
            if (loH_bitmask & (1ull << i))
                UnsetZeroPatternAtQubit(num_qubits - 1 - i);
    }
    
    if (last_cycle) {
        global_factor_power +=  __builtin_popcountll(loH_bitmask) +  __builtin_popcountll(hiH_bitmask);
        time1.StartTime();
        if (__builtin_popcountll(loH_bitmask) % 2) {
            idx_size odd_H_bit = 1ull << __builtin_ctzl(loH_bitmask);
            hiH_bitmask |= odd_H_bit;
            loH_bitmask ^= odd_H_bit;
        }
        time_by_category.last_H += time1.GetElapsedTime();
    }
    
    time.StartTime();
    if (CZ_app || T_bitmasks_64[0]) {
        if (num_qubits > 4)
            ApplyBlockOfCZTGatesAVXParallel(amp, num_qubits, CZ_bitmasks_64, T_bitmasks_64,
                                            loH_bitmask >> (num_qubits - th), num_threads, zero_opt_mask);
        else
            ApplyBlockOfCZTGates(amp, num_qubits, CZ_bitmasks_64, T_bitmasks_64); //fix for H
    }
    else {
        hiH_bitmask |= loH_bitmask;
        loH_bitmask = 0;
    }
    
    time_by_category.low_q_XY_CZT += time.GetElapsedTime();
    
    for (int i = num_qubits - 1; i >= 0; --i)
        if (hiH_bitmask & (1ull << i))
            UnsetZeroPatternAtQubit(num_qubits - 1 - i);
    
    if (last_cycle && hiH_bitmask) {
        time1.StartTime();
        
        if (__builtin_popcountll(hiH_bitmask) % 2 != 0) {
            int q = __builtin_ctzl(hiH_bitmask);
            Apply1QXYHGates(amp, q, num_qubits, Gate::Type::Hadamard, num_threads);
            hiH_bitmask ^= 1ull << q;
            ++single_H;
        }
        
        ApplyHGatesRecursively(amp, num_qubits, num_threads, hiH_bitmask);
        time_by_category.last_H += time1.GetElapsedTime();
        
        if (book_keep) {
            count_of_category.H_merged_lo += __builtin_popcountll(loH_bitmask);
            count_of_category.last_H += __builtin_popcountll(hiH_bitmask) + single_H;
        }
    }
    
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
    all_zeros = ApplyxCZGateAVX(amp, num_threads, num_qubits, xCZ_bitmasks, zero_opt_mask);

    for (int q = 0; q < num_qubits; ++q)
        if (xCZ_bitmasks[1] & (1ull << q))
            SetEvenZeroPatternAtQubit(q);
    for (int q = 0; q < num_qubits; ++q)
        if (xCZ_bitmasks[2] & (1ull << q))
            SetOddZeroPatternAtQubit(q);
}

void FullAmpStateVector::
ApplyHGateOnAllAmps(bool not_cycle_0)
{
    Time time;
    time.StartTime();
    
    if (!not_cycle_0) {
        float* __restrict t_amp = (float*)__builtin_assume_aligned(amp, 64);
        constexpr __m256 re_ones = {1, 0, 1, 0, 1, 0 , 1, 0};
        
        #pragma omp parallel for num_threads(num_threads)
        for (idx_size i = 0; i < amp_size; i += 4) {
            __m256 t = _mm256_load_ps(t_amp + (2 * i));
            t = _mm256_or_ps(t, re_ones);
            _mm256_store_ps(t_amp + (2 * i), t);
        }
        if (book_keep)
            count_of_category.initial_H += num_qubits;
        time_by_category.initial_H += time.GetElapsedTime();
    }
    else {
        idx_size H_bm = (1ull << num_qubits) - 1;
        if (num_qubits % 2 != 0) {
            Apply1QXYHGates(amp, 0, num_qubits, Gate::Type::Hadamard, num_threads);
            H_bm ^= 1;
        }
        ApplyHGatesRecursively(amp, num_qubits, num_threads, H_bm);
        if (book_keep)
            count_of_category.last_H += num_qubits;
        time_by_category.last_H += time.GetElapsedTime();
    }
    
    global_factor_power += num_qubits;
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

// Transfer odd bit/gate in high qubit bitmask to low qubit bitmask.
// Modifies to threshold to reflect the transfer.
void FullAmpStateVector::
TransferOddBitsFromHiQubitsBM(int& th,
                              idx_size& hi_q_X_bitmask,
                              idx_size& hi_q_Y_bitmask,
                              idx_size& lo_q_X_bitmask,
                              idx_size& lo_q_Y_bitmask,
                              int& num_hi_X_bits,
                              int& num_hi_Y_bits,
                              const idx_size X_bitmask,
                              const idx_size Y_bitmask)
{
    num_hi_X_bits = __builtin_popcountll(hi_q_X_bitmask);
    num_hi_Y_bits = __builtin_popcountll(hi_q_Y_bitmask);
    
    if ((num_hi_X_bits + num_hi_Y_bits) % 2 == 1) {
        const int most_sig_q_X = hi_q_X_bitmask ? 63 - __builtin_clzl(hi_q_X_bitmask) : kRT;
        const int most_sig_q_Y = hi_q_Y_bitmask ? 63 - __builtin_clzl(hi_q_Y_bitmask) : kRT;
        if (most_sig_q_X > most_sig_q_Y) {
            th = most_sig_q_X;
            hi_q_X_bitmask ^= 1ull << most_sig_q_X;
            --num_hi_X_bits;
        }
        else {
            th = most_sig_q_Y;
            hi_q_Y_bitmask ^= 1ull << most_sig_q_Y;
            --num_hi_Y_bits;
        }
    }
    
    lo_q_X_bitmask = X_bitmask & ~((1ull << th) - 1);
    lo_q_Y_bitmask = Y_bitmask & ~((1ull << th) - 1);
}

void FullAmpStateVector::
TransferOddBitsFromLowQubitsBM(int& th,
                              idx_size& hi_q_X_bitmask,
                              idx_size& hi_q_Y_bitmask,
                              idx_size& lo_q_X_bitmask,
                              idx_size& lo_q_Y_bitmask,
                              int& num_lo_X_bits,
                              int& num_lo_Y_bits,
                              const idx_size X_bitmask,
                              const idx_size Y_bitmask)
{
    num_lo_X_bits = __builtin_popcountll(lo_q_X_bitmask);
    num_lo_Y_bits = __builtin_popcountll(lo_q_Y_bitmask);
    
    if ((num_lo_X_bits + num_lo_Y_bits) % 2 == 1) {
        const int least_sig_q_X = lo_q_X_bitmask ? __builtin_ctzl(lo_q_X_bitmask) : kRT;
        const int least_sig_q_Y = lo_q_Y_bitmask ? __builtin_ctzl(lo_q_Y_bitmask) : kRT;
        if (least_sig_q_X < least_sig_q_Y) {
            th = least_sig_q_X + 1;
            lo_q_X_bitmask ^= 1ull << least_sig_q_X;
            --num_lo_X_bits;
        }
        else {
            th = least_sig_q_Y + 1;
            lo_q_Y_bitmask ^= 1ull << least_sig_q_Y;
            --num_lo_Y_bits;
        }
    }
    
    hi_q_X_bitmask = X_bitmask & ((1ull << th) - 1);
    hi_q_Y_bitmask = Y_bitmask & ((1ull << th) - 1);
}

void FullAmpStateVector::
ApplyOddGates(idx_size& X_bitmask,
              idx_size& Y_bitmask,
              int& num_X_bits,
              int& num_Y_bits)
{
    Time time;
    time.StartTime();

    const int X_q = X_bitmask ? __builtin_ctzl(X_bitmask) : kRT;
    const int Y_q = Y_bitmask ? __builtin_ctzl(Y_bitmask) : kRT;
    
    //        const int X_q = X_bitmask_64 ? 63 - __builtin_clzl(X_bitmask_64) : 1000;
    //        const int Y_q = Y_bitmask_64 ? 63 - __builtin_clzl(Y_bitmask_64): 1000;
    
    if (!(X_q == 1000 && Y_q == 1000)) {
        if (X_q < Y_q) {
            Apply1QXYHGates(amp, X_q, num_qubits, Gate::Type::X_1_2, num_threads);
            X_bitmask ^= 1ull << X_q;
            --num_X_bits;
            global_factor_power += 2;
            time_by_category.X1_2 += time.GetElapsedTime();
            if (book_keep)
                ++count_of_category.X1_2;
        }
        else {
            Apply1QXYHGates(amp, Y_q, num_qubits, Gate::Type::Y_1_2, num_threads);
            Y_bitmask ^= 1ull << Y_q;
            --num_Y_bits;
            global_factor_power += 2;
            time_by_category.Y1_2 += time.GetElapsedTime();
            if (book_keep)
                ++count_of_category.Y1_2;
        }
    }
}

pair<int, int> FullAmpStateVector::
GetMostSigOddBit(idx_size& X_bitmask,
                 idx_size& Y_bitmask,
                 int& num_X_bits,
                 int& num_Y_bits)
{
    if ((num_X_bits + num_Y_bits) % 2 == 1) {
        const int X_q = X_bitmask ? __builtin_ctzl(X_bitmask) : kRT;
        const int Y_q = Y_bitmask ? __builtin_ctzl(Y_bitmask) : kRT;
        if (X_q < Y_q) {
            X_bitmask ^= 1ull << X_q;
            --num_X_bits;
            return pair<int, int>(X_q, 0);
        }
        else {
            Y_bitmask ^= 1ull << Y_q;
            --num_Y_bits;
              return pair<int, int>(Y_q, 1);
        }
    }
    return pair<int, int> (-1, -1);
}

pair<int, int> FullAmpStateVector::
GetLeasttSigOddBit(idx_size& X_bitmask,
                   idx_size& Y_bitmask,
                   int& num_X_bits,
                   int& num_Y_bits)
{
    if ((num_X_bits + num_Y_bits) % 2 == 1) {
        const int X_q = X_bitmask ? 63 - __builtin_clzl(X_bitmask) : -kRT;
        const int Y_q = Y_bitmask ? 63 - __builtin_clzl(Y_bitmask) : -kRT;
        if (X_q > Y_q) {
            X_bitmask ^= 1ull << X_q;
            --num_X_bits;
            return pair<int, int>(X_q, 0);
        }
        else {
            Y_bitmask ^= 1ull << Y_q;
            --num_Y_bits;
            return pair<int, int>(Y_q, 1);
        }
    }
    return pair<int, int> (-1, -1);
}

void FullAmpStateVector::
ApplyXYRecursiveTransform(bitset<128> X_bitmask,
                          bitset<128> Y_bitmask,
                          int th)
{
    Time time;
    idx_size X_bitmask_64 = X_bitmask.to_ulong(), Y_bitmask_64 = Y_bitmask.to_ulong();
    
    idx_size hiq_X_bitmask = X_bitmask_64 & ((1ull << th) - 1);
    idx_size hiq_Y_bitmask = Y_bitmask_64 & ((1ull << th) - 1);
    idx_size loq_X_bitmask = 0, loq_Y_bitmask = 0;
    int num_bits_hi_X = 0, num_bits_hi_Y = 0;
    
    TransferOddBitsFromHiQubitsBM(th, hiq_X_bitmask,  hiq_Y_bitmask, loq_X_bitmask, loq_Y_bitmask,
                                  num_bits_hi_X, num_bits_hi_Y, X_bitmask_64, Y_bitmask_64);
    
    int num_lo_X_bits = __builtin_popcountll(loq_X_bitmask), num_lo_Y_bits = __builtin_popcountll(loq_Y_bitmask);
   
    assert((num_lo_X_bits + num_bits_hi_X + num_bits_hi_Y + num_lo_Y_bits) ==
           ( __builtin_popcountll(X_bitmask_64) + __builtin_popcountll(Y_bitmask_64)));
    
    if ((num_lo_X_bits + num_lo_Y_bits) % 2 == 1)
        ApplyOddGates(loq_X_bitmask, loq_Y_bitmask, num_lo_X_bits, num_lo_Y_bits);
    
    global_factor_power += num_lo_X_bits + num_bits_hi_X + num_bits_hi_Y + num_lo_Y_bits;
    
    time.StartTime();
    if (X_bitmask_64 || Y_bitmask_64) {
        //Process low qubits first,
        global_i_counter += XYFastTransformLowQ(amp, loq_X_bitmask, loq_Y_bitmask,
                                                num_qubits, num_threads);
        global_i_counter += XYFastTransform(amp, hiq_X_bitmask, hiq_Y_bitmask,
                                            num_qubits, num_threads, zero_opt_mask, th);
    }
    time_by_category.merged_XY1_2 += time.GetElapsedTime();
}

int FullAmpStateVector::
ApplyLoXYHAndCZTInSamePass(string& cz_bits,
                           idx_size prefix_size,
                           const bitset<128>& X_bitmask,
                           const bitset<128>& Y_bitmask,
                           const bitset<128>& H_bitmask,
                           const bitset<128>* __restrict CZ_bitmasks,
                           const bitset<128> T_bitmasks[2],
                           int th,
                           bool last_cycle)
{    
    Time time;
    time.StartTime();
    
//    int z_c = 0;
//    for (int i = 0; i < amp_size; ++i) {
//        if (amp[i] == cmplx(0,0))
//            ++z_c;
//    }
//    cout << "\nzero_count before: " << z_c << endl;
    
    
    if (all_zeros) {
//        cout << "All zeros : " << zero_opt_mask.print() << endl;
        return -1;
    }

    idx_size CZ_bitmasks_64[num_qubits];
    idx_size T_bitmasks_64[2] = {T_bitmasks[0].to_ulong(), T_bitmasks[1].to_ulong()};
    idx_size X_bitmask_64 = X_bitmask.to_ulong(), Y_bitmask_64 = Y_bitmask.to_ulong(),
    H_bitmask_64 = H_bitmask.to_ulong();
    idx_size hiq_X_bitmask = X_bitmask_64 & ((1ull << th) - 1);
    idx_size hiq_Y_bitmask = Y_bitmask_64 & ((1ull << th) - 1);
    idx_size loq_X_bitmask = X_bitmask_64 & ~((1ull << th) - 1);
    idx_size loq_Y_bitmask = Y_bitmask_64 & ~((1ull << th) - 1);
    idx_size hiq_H_bitmask = last_cycle ? H_bitmask_64 & ((1ull << th) - 1): 0;
    idx_size loq_H_bitmask = last_cycle ? H_bitmask_64 & ~((1ull << th) - 1) : 0;
    int num_lo_X_bits = __builtin_popcountll(loq_X_bitmask);
    int num_lo_Y_bits = __builtin_popcountll(loq_Y_bitmask);
    int single_H = 0;
    
    if (zero_opt_mask.GetNonZeroBitmask()) {
//        cout << "Before : " << zero_opt_mask.print() << endl;
        for (int i = num_qubits - 1; i >= 0; --i)
            if (X_bitmask[i] || Y_bitmask[i] || (loq_H_bitmask & (1ull << i)))
                UnsetZeroPatternAtQubit(num_qubits - 1 - i);
//        cout << "After : " << zero_opt_mask.print() << endl;
    }

    
    for (int i = 0; i < num_qubits; ++i)
        CZ_bitmasks_64[i] = CZ_bitmasks[i].to_ulong();
    
    pair<int, int> odd_bit_low_XY = GetMostSigOddBit(loq_X_bitmask, loq_Y_bitmask,
                                                     num_lo_X_bits, num_lo_Y_bits);
    if (last_cycle) {
        global_factor_power += __builtin_popcountll(H_bitmask_64);

        idx_size odd_bit_low = odd_bit_low_XY.first < 0 ? 0 : 1ull << odd_bit_low_XY.first;
        loq_H_bitmask ^= odd_bit_low;
        hiq_H_bitmask |= odd_bit_low;
        
        if (__builtin_popcountll(loq_H_bitmask) % 2 != 0) {
            idx_size odd_H_bit = 1ull << __builtin_ctzl(loq_H_bitmask);
            hiq_H_bitmask |= odd_H_bit;
            loq_H_bitmask ^= odd_H_bit;
        }
    }
    
    global_i_counter += ApplyBlockOfCZTAndLowQXYHGatesAVX(amp, num_qubits, CZ_bitmasks_64,
                                                         T_bitmasks_64, loq_X_bitmask >> th,
                                                         loq_Y_bitmask >> th, loq_H_bitmask >> th,
                                                          num_threads,th, zero_opt_mask);
    
    int num_hi_X_bits = __builtin_popcountll(hiq_X_bitmask), num_hi_Y_bits = __builtin_popcountll(hiq_Y_bitmask);
    if (odd_bit_low_XY.second == 0) {
        hiq_X_bitmask |= 1ull << odd_bit_low_XY.first;
        ++num_hi_X_bits;
    }
    else if (odd_bit_low_XY.second == 1) {
        hiq_Y_bitmask |= 1ull << odd_bit_low_XY.first;
        ++num_hi_Y_bits;
    }
    
    time_by_category.low_q_XY_CZT += time.GetElapsedTime();
    
    if ((num_hi_X_bits + num_hi_Y_bits) % 2 == 1)
        ApplyOddGates(hiq_X_bitmask, hiq_Y_bitmask, num_hi_X_bits, num_hi_Y_bits);
    
    time.StartTime();
    if (hiq_X_bitmask || hiq_Y_bitmask) {
        if (book_keep && last_cycle)
            count_of_category.H_merged_hi += __builtin_popcountll(hiq_H_bitmask & (hiq_X_bitmask | hiq_Y_bitmask));

        global_i_counter += XYHFastTransformHighQ(amp, hiq_X_bitmask, hiq_Y_bitmask, hiq_H_bitmask,
                                                 num_qubits, num_threads, zero_opt_mask,
                                                 false);
        
        hiq_H_bitmask ^= (hiq_H_bitmask & (hiq_X_bitmask | hiq_Y_bitmask));
    }
    time_by_category.high_q_XY1_2 += time.GetElapsedTime();
    
    if (zero_opt_mask.GetNonZeroBitmask()) {
        for (int i = num_qubits - 1; i >= 0; --i)
            if (hiq_H_bitmask & (1ull << i))
                UnsetZeroPatternAtQubit(num_qubits - 1 - i);
    }
    
    if (last_cycle && hiq_H_bitmask) {
        time.StartTime();
        if (__builtin_popcountll(hiq_H_bitmask) % 2 != 0) {
            int q = __builtin_ctzl(hiq_H_bitmask);
            Apply1QXYHGates(amp, q, num_qubits, Gate::Type::Hadamard, num_threads);
            hiq_H_bitmask ^= 1ull << q;
            ++single_H;
        }
        ApplyHGatesRecursively(amp, num_qubits, num_threads, hiq_H_bitmask);
        time_by_category.last_H += time.GetElapsedTime();
    }

    global_factor_power += num_lo_X_bits + num_hi_X_bits + num_hi_Y_bits + num_lo_Y_bits;
    
    if (book_keep) {
        count_of_category.low_q_XY1_2 += num_lo_X_bits + num_lo_Y_bits;
        count_of_category.high_q_XY1_2 += num_hi_Y_bits + num_hi_X_bits;
        
        if (last_cycle) {
            count_of_category.H_merged_lo += __builtin_popcountll(loq_H_bitmask);
            count_of_category.last_H += __builtin_popcountll(hiq_H_bitmask) + single_H;
        }
    }
//
//    int z_c_1 = 0;
//    for (int i = 0; i < amp_size; ++i) {
//        if (amp[i] == cmplx(0,0))
//            ++z_c_1;
//    }
//    cout << "zero_count after : " << z_c_1 << endl;
    
    return -1;
}

void FullAmpStateVector::
CopyState(const FullAmpStateVector& rhs)
{
    global_factor_power = rhs.global_factor_power;
    global_i_counter = rhs.global_i_counter;
    zero_opt_mask = rhs.zero_opt_mask;
    
    idx_size size = 2 * rhs.GetSize();
    
    float* __restrict rhs_t_amp = (float*)__builtin_assume_aligned(rhs.amp, 64);
    float* __restrict t_amp = (float*)__builtin_assume_aligned(amp, 64);
    
#pragma omp parallel for num_threads(num_threads)
    for (idx_size i = 0; i < size; i+=8) {
        const __m256 temp_amp = _mm256_load_ps (&rhs_t_amp[i]);
        _mm256_store_ps(&t_amp[i], temp_amp);
    }
}

void FullAmpStateVector::
CopyMemberVars(const FullAmpStateVector& rhs)
{
    max_prob = rhs.max_prob;
    min_prob = rhs. min_prob;
    amp_size = rhs.amp_size;
    num_qubits = rhs.num_qubits;
    global_factor_power = rhs.global_factor_power;
    global_i_counter = rhs.global_i_counter;
    zero_opt_mask = rhs.zero_opt_mask;
}

cmplx FullAmpStateVector::
operator[](bitset<128> i)
{
    const float rescaling_factor = (global_factor_power % 2) ?
    1.0/(pow(2,(global_factor_power/2)) * sqrt(2.0)): 1.0/pow(2,(global_factor_power/2));
    cmplx a = amp[i.to_ulong()] * cmplx(pow(ki, global_i_counter));
    a *= rescaling_factor;
    
    return a;
}

cmplx FullAmpStateVector::
GetGlobalAmpAtInterestingIdx(idx_size i)
{
    return (*this)[i];
}

const cmplx* const FullAmpStateVector::
GetAmpVector() const
{
    return amp;
}

double FullAmpStateVector::
GetMinProb()
{
     for (idx_size i = 0; i < amp_size; ++i) {
        float t = norm(amp[i]);

       if (min_prob > t)
            min_prob = t;
    }
    return min_prob;
}

double FullAmpStateVector::
GetMaxProb()
{
    for (idx_size i = 0; i < amp_size; ++i) {
        float t = norm(amp[i]);
        
        if (max_prob < t)
            max_prob = t;
    }
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

int FullAmpStateVector::
GetNumQInBlock(idx_size block) const
{
    return num_qubits;
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

ZeroOptMask FullAmpStateVector::
GetZeroOptMask() const
{
    return zero_opt_mask;
}

double FullAmpStateVector::
CalculateNormSquared()
{
    Time norm_time;
    norm_time.StartTime();

    float rescaling_factor = 1.0/pow(2,(global_factor_power/2));
    if ((global_factor_power % 2) == 1)
        rescaling_factor *= 1.0/sqrt(2.0);

    double hi_sum = 0, lo_sum = 0;

    for (idx_size i = 0; i < amp_size; i += 4) {
        
        float t = 0;
        for (int j = 0; j < 4; ++j)
         t += norm(amp[i + j] * rescaling_factor);

        if ((t * (1ull << num_qubits)) > 1.0)
            hi_sum += t;
        else
            lo_sum += t;
    }
    
    
//    const float rescaling_factor = (global_factor_power % 2) ? 1.0/(pow(2,(global_factor_power/2)) * sqrt(2.0))
//    : 1.0/pow(2,(global_factor_power/2));
//
//    const __m256 rescaling = {rescaling_factor, rescaling_factor, rescaling_factor, rescaling_factor,
//        rescaling_factor, rescaling_factor , rescaling_factor, rescaling_factor};
//
//    float* __restrict t_amp = (float*)__builtin_assume_aligned(amp, 64);
//
//    double hi_sum = 0, lo_sum = 0;
//
//    #pragma omp parallel for num_threads(num_threads) reduction(+:hi_sum, lo_sum)
//    for (idx_size i = 0; i < amp_size; i += 4) {
//        __m256 t = _mm256_load_ps(t_amp + (2 * i));
////        __m256 t_1 = _mm256_load_ps(t_amp + (2 * (i + 4)));
//        t = _mm256_mul_ps(t, rescaling);
//        t = _mm256_mul_ps(t, t);
////        t_1 = _mm256_mul_ps(t_1, rescaling);
////        t_1 = _mm256_mul_ps(t_1, t_1);
////        __m256 t_r = _mm256_hadd_ps(t_0, t_1);
//
//        __m256 t1 = _mm256_hadd_ps(t,t);
//        __m256 t2 = _mm256_hadd_ps(t1,t1);
//        __m128 t3 = _mm256_extractf128_ps(t2,1);
//        __m128 t4 = _mm_add_ss(_mm256_castps256_ps128(t2),t3);
//        float t_hs = _mm_cvtss_f32(t4);
//
//        if ((t_hs * (i + 3)) > hi_sum || (!hi_sum && (t_hs * (i + 3)) > 1/(1ull << GetNumQubits())))
//            hi_sum += t_hs;
//        else
//            lo_sum += t_hs;
//    }
//
    time_by_category.norm += norm_time.GetElapsedTime();

    return hi_sum + lo_sum;
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

double FullAmpStateVector::
CountZeroAmpPercentage() const
{
    idx_size zero_count = 0;
    
    for (idx_size i = 0; i < amp_size; ++i) {
        if (amp[i] == cmplx(0,0))
            ++zero_count;
    }
    
    return double(zero_count)/double(amp_size) * 100;
}

idx_size FullAmpStateVector::
CountZerosInBlock(int block)
{
    //    cout << zero_opt_mask.print() << endl;
    idx_size z_c = 0;
//    #pragma omp parallel for num_threads(num_threads) reduction(+:z_c)
    for (int i = 0; i < amp_size; ++i) {
        if (amp[i] == cmplx(0,0))
            ++z_c;
    }
    return z_c;
}

bool FullAmpStateVector::
AreAllAmpsZeros() const
{
    return all_zeros;
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
    all_zeros = false;
    
    zero_opt_mask.reset();
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

void FullAmpStateVector::
SetOddZeroPatternAtQubit(int qubit)
{
    zero_opt_mask.SetOddBit(qubit);
}

void FullAmpStateVector::
SetEvenZeroPatternAtQubit(int qubit)
{
    zero_opt_mask.SetEvenBit(qubit);
}

void FullAmpStateVector::
UnsetZeroPatternAtQubit(int qubit)
{
    zero_opt_mask.SetNonZeroBit(qubit);
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

void FullAmpStateVector::
WriteAmpToDisk(const string& filename)
{
    ofstream file;
    file.open (filename, ios::out | ios::binary);
    file.seekp(0);
    if (!file.write((char*)amp, sizeof(cmplx) * amp_size)) {
        cerr << "Error in writing to file\n";
        string cmd = "rm -rf " + filename.substr(filename.find_last_of('/'));
        system(cmd.c_str());
        exit(1);
    }
    file.close();
}

void FullAmpStateVector::
ReadFromDisk(const string& filename)
{
    ifstream file;
    file.open (filename, ios::in | ios::binary);
    if (!file.read((char*)amp, sizeof(cmplx) * amp_size)) {
        cerr << "Error in reading from file\n";
        string cmd = "rm -rf " + filename.substr(filename.find_last_of('/'));
        system(cmd.c_str());
        exit(1);
    }
    file.close();
}

void FullAmpStateVector::
SetMemberVariables(const GenericQuantumState& rhs)
{
    const FullAmpStateVector& amp = (const FullAmpStateVector&)rhs;
    max_prob = amp.max_prob;
    min_prob = amp. min_prob;
    amp_size = amp.amp_size;
    num_qubits = amp.num_qubits;
    global_factor_power = amp.global_factor_power;
    global_i_counter = amp.global_i_counter;
    zero_opt_mask = amp.zero_opt_mask;
}

