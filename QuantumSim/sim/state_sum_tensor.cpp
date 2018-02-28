//
//  state_sum_tensor.cpp
//  vector_state_sim
//
//  Created by Aneeqa Fatima on 1/18/18.
//  Copyright © 2018 Aneeqa Fatima. All rights reserved.
//

#include "state_sum_tensor.h"

SumOfTensorsProductsStateVector::
SumOfTensorsProductsStateVector(const int qubits,
                                const Config::SimType type,
                                const int hcut,
                                const int vcut): num_addends(1)
{
    sim_type = type;
    
    if (type == Config::SimType::LosslessH || type == Config::SimType::Approx1CutH)
        tensor_addends.push_back(new TensorProductStateVector(qubits,
                                                              TensorProductStateVector::Cuts::Horizontal,
                                                              hcut,
                                                              vcut,
                                                              sim_type));
    else if (type == Config::SimType::LosslessV || type == Config::SimType::Approx1CutV)
        tensor_addends.push_back(new TensorProductStateVector(qubits,
                                                              TensorProductStateVector::Cuts::Vertical,
                                                              hcut,
                                                              vcut,
                                                              sim_type));
    else if (sim_type == Config::SimType::Approx2011OWT || sim_type == Config::SimType::Approx_i11iOWT) {
        tensor_addends.push_back(new TensorProductStateVector(qubits,
                                                              TensorProductStateVector::Cuts::Horizontal,
                                                              hcut,
                                                              vcut,
                                                              sim_type));
        tensor_addends.push_back(new TensorProductStateVector(qubits,
                                                              TensorProductStateVector::Cuts::Vertical,
                                                              hcut,
                                                              vcut,
                                                              sim_type));
        tensor_addends[0] -> state_a -> IncrementGlobalFactorPower();
        tensor_addends[1] -> state_a -> IncrementGlobalFactorPower();
        ++num_addends;
    }
    else {
        tensor_addends.push_back(new TensorProductStateVector(qubits,
                                                              TensorProductStateVector::Cuts::Horizontal,
                                                              hcut,
                                                              vcut,
                                                              Config::SimType::LosslessH));
        tensor_addends.push_back(new TensorProductStateVector(qubits,
                                                              TensorProductStateVector::Cuts::Vertical,
                                                              hcut,
                                                              vcut,
                                                              Config::SimType::LosslessV));
        tensor_addends[0] -> state_a -> IncrementGlobalFactorPower();
        tensor_addends[1] -> state_a -> IncrementGlobalFactorPower();
        ++num_addends;
    }
}

SumOfTensorsProductsStateVector::
~SumOfTensorsProductsStateVector()
{
    for (auto& t : tensor_addends) {
        delete t;
        t = nullptr;
    }
}

//TODO
bool SumOfTensorsProductsStateVector::
ApplyBlockOfDiagGates(string& cz_bits,
                      const bitset<128>* __restrict CZ_bitmasks,
                      const bitset<128> T_bitmasks[2])
{
    bool terminate = false;
    for (auto& t : tensor_addends)
        t -> ApplyBlockOfDiagGates(cz_bits, CZ_bitmasks, T_bitmasks);
    
    if (sim_type == Config::SimType::LosslessH || sim_type == Config::SimType::LosslessV)
        terminate = ApplyXCZGatesExact(cz_bits, CZ_bitmasks);
    else if (sim_type == Config::SimType::ApproxOWT || sim_type == Config::SimType::Approx_i11iOWT ||
             sim_type == Config::SimType::Approx2011OWT ){
        data_per_cycles.xCZ_H.push_back(tensor_addends[0] -> CountXCZGates(CZ_bitmasks));
        data_per_cycles.xCZ_V.push_back(tensor_addends[1] -> CountXCZGates(CZ_bitmasks));
    }
    
    data_per_cycles.memory.push_back(GetMemUsage());
    data_per_cycles.addends.push_back(GetNumAddends());
    
    return terminate;
}

inline bool SumOfTensorsProductsStateVector::
ApplyXCZGatesExact(string& cz_bits,
                   const bitset<128>* __restrict CZ_bitmasks)
{
    //int represents qubit in block A and idx_size represents bitmask of qubits in block B
    //of tensor product.
    clock_t begin = clock();
    vector<pair<int,bitset<128>>> qubits_gates_across;
    tensor_addends[0] -> FindCZGatesBetweenPartitions(qubits_gates_across, CZ_bitmasks);
    const int modified_num_q_B = tensor_addends[0] -> GetStateBNumQ() - 1;
    const ul prev_CZ_count = count_of_category.decomposed_CZ;
    bool terminate = false;
    
    for (auto& g : qubits_gates_across) {
        if (cz_bits == "") {
            terminate = true;
            break;
        }
        while (g.second != 0) {
            int first_half = __builtin_ctzl(g.second.to_ulong());
            int second_half = __builtin_ctzl((g.second >> 64).to_ulong());
            const int q = first_half ? first_half : second_half ? 64 + second_half : 0;
            ++count_of_category.decomposed_CZ;
            for (idx_size i = 0; i < num_addends; ++i) {
                if (cz_bits == "*" || cz_bits == "") {
                    TensorProductStateVector* new_t = new TensorProductStateVector(*tensor_addends[i]);
                    tensor_addends[i] -> ApplyCZGateAcrossTensorFactors(Gate::Type::CZ_D1, Gate::Type::CZ_D2,
                                                                        g.first, modified_num_q_B - q);
                    new_t -> ApplyCZGateAcrossTensorFactors(Gate::Type::CZ_D3, Gate::Type::CZ_D4,
                                                            g.first, modified_num_q_B - q);
                    tensor_addends.push_back(new_t);
                }
                else {
                    if (cz_bits[cz_bits.size() - 1] == '0')
                        tensor_addends[i] -> ApplyCZGateAcrossTensorFactors(Gate::Type::CZ_D1, Gate::Type::CZ_D2,
                                                                            g.first, modified_num_q_B - q);
                    else
                        tensor_addends[i] -> ApplyCZGateAcrossTensorFactors(Gate::Type::CZ_D3, Gate::Type::CZ_D4,
                                                                             g.first, modified_num_q_B - q);
                }
            }
            if (cz_bits != "" && cz_bits != "*")
                cz_bits.pop_back();
            
            g.second[q] = 0;
            num_addends = tensor_addends.size();
        }
        if (terminate)
            break;
    }
    clock_t end = clock();
    time_by_category.decomposed_CZ +=  double(end - begin) / CLOCKS_PER_SEC;
    
    if (sim_type == Config::SimType::LosslessH) {
        data_per_cycles.xCZ_H.push_back(count_of_category.decomposed_CZ - prev_CZ_count);
        data_per_cycles.xCZ_V.push_back(0);
    }
    else if (sim_type == Config::SimType::LosslessV) {
        data_per_cycles.xCZ_V.push_back(count_of_category.decomposed_CZ - prev_CZ_count);
        data_per_cycles.xCZ_H.push_back(0);
    }
    count_of_category.xCZ_not_applied += tensor_addends[0] -> CountXCZGates(CZ_bitmasks)
    - count_of_category.decomposed_CZ - prev_CZ_count;
    return terminate;
}

void SumOfTensorsProductsStateVector::
ApplyNonCGate(const int gate_qubit,
              const Gate::Type gate_type,
              const Gate& g)
{
    const int modified_q = tensor_addends[0] -> GetStateANumQ() + tensor_addends[0] -> GetStateBNumQ() - 1;
    for (auto& t : tensor_addends)
        t -> ApplyNonCGate(modified_q - gate_qubit, gate_type, g);
}

void SumOfTensorsProductsStateVector::
ApplyHGateOnAllAmps()
{
    for (auto& t : tensor_addends)
        t -> ApplyHGateOnAllAmps();
}

//TODO
void SumOfTensorsProductsStateVector::
ApplyCGate(const int num_controls,
           const vector<int>& gate_qubits,
           const Gate& g,
           const Gate::Type gate_type)
{
    
}

void SumOfTensorsProductsStateVector::
ApplyMergedXYGate(const Gate& gate1,
                  const Gate& gate2)
{
    for (auto& t : tensor_addends)
        t -> ApplyMergedXYGate(gate1, gate2);
}

void SumOfTensorsProductsStateVector::
ApplyXYRecursiveTransform(bitset<128> X_bitmask,
                         bitset<128> Y_bitmask,
                         const int th)
{
    idx_size prev_X_count = count_of_category.X1_2, prev_Y_count = count_of_category.Y1_2;
    for (auto& t : tensor_addends)
        t -> ApplyXYRecursiveTransform(X_bitmask, Y_bitmask, th);
    
    if (count_of_category.X1_2 - prev_X_count)
        count_of_category.X1_2 = count_of_category.X1_2 - (count_of_category.X1_2 - prev_X_count - 1);
    if (count_of_category.Y1_2 - prev_Y_count)
        count_of_category.Y1_2 = count_of_category.Y1_2 - (count_of_category.Y1_2 - prev_Y_count - 1);
}

FullAmpStateVector* SumOfTensorsProductsStateVector::
ConvertSumOfTensorsToStateAVX()
{
    RescaleAndApplyGlobalICounter();
    const int num_q_b = tensor_addends[0] -> GetStateBNumQ(), num_q_a = tensor_addends[0] -> GetStateANumQ(),
    total_q = num_q_a  + num_q_b;
    const idx_size size = 1ull << total_q, a_size = 2 * (1ull << num_q_a), b_size = 2 * (1ull << num_q_b);
    
    cmplx* amp;
    posix_memalign((void**)&amp, 64, sizeof(cmplx) * size);
    memset(amp, 0, size * sizeof(amp));
   
    float* __restrict result = (float*)__builtin_assume_aligned(amp, 64);
    for (idx_size n = 0; n < num_addends; ++n) {
        float* __restrict amp_a = (float*)__builtin_assume_aligned(tensor_addends[n] -> state_a -> GetAmpVector(), 64);
        float* __restrict amp_b = (float*)__builtin_assume_aligned(tensor_addends[n] -> state_b -> GetAmpVector(), 64);
        
        idx_size i = 0;
        for (idx_size a = 0; a + 7 < a_size; a += 8) {
            
            __m256 t_amp_a_re[4] , t_amp_a_im[4];
            
            for (int j = 0; j < 8; j += 2) {
                auto t_a_re = amp_a[a + j], t_a_im = amp_a[a + j + 1];
                const __m256 t0 = {t_a_re, t_a_re, t_a_re, t_a_re,
                    t_a_re, t_a_re, t_a_re, t_a_re};
                const __m256 t1 = {t_a_im, t_a_im, t_a_im, t_a_im,
                    t_a_im, t_a_im, t_a_im, t_a_im};
                t_amp_a_re[j/2] = t0;
                t_amp_a_im[j/2] = t1;
            }
            
            for (idx_size b = 0; b + 15 < b_size; b += 16) {
                
                const __m256 t_amp_b0 = _mm256_load_ps (&amp_b[b]);
                const __m256 t_amp_b1 = _mm256_load_ps (&amp_b[b + 8]);
                //The order of real and imag amps is 2 floats from 1st group of 4, 2 floats from 2nd group of 4, and so on.
                __m256 t_amp_b_re = _mm256_shuffle_ps(t_amp_b0, t_amp_b1, 0b10001000);
                __m256 t_amp_b_im = _mm256_shuffle_ps(t_amp_b0, t_amp_b1, 0b11011101);
                
                for (int j = 0; j < 4; ++j) {
                    const __m256 res_0 = _mm256_load_ps (&result[b + ((i + j) * b_size)]);
                    const __m256 res_1 = _mm256_load_ps (&result[b + ((i + j) * b_size) + 8]);
                    
                    __m256 res_re = _mm256_shuffle_ps(res_0, res_1, 0b10001000);
                    __m256 res_im = _mm256_shuffle_ps(res_0, res_1, 0b11011101);
                    const __m256 tim_amps = _mm256_fmadd_ps(t_amp_a_re[j], t_amp_b_im,
                                                            _mm256_mul_ps(t_amp_a_im[j], t_amp_b_re));
                    const __m256 tre_amps = _mm256_fmsub_ps(t_amp_a_re[j], t_amp_b_re,
                                                            _mm256_mul_ps(t_amp_a_im[j], t_amp_b_im));
                    res_re = _mm256_add_ps(res_re, tre_amps);
                    res_im = _mm256_add_ps(res_im, tim_amps);
                    
                    __m256 first_set = _mm256_shuffle_ps(res_re, res_im, 0b01000100);
                    __m256 second_set = _mm256_shuffle_ps(res_re, res_im, 0b11101110);
                    first_set = _mm256_permute_ps(first_set, 0b11011000);
                    second_set = _mm256_permute_ps(second_set, 0b11011000);
                    
                    _mm256_store_ps(&result[b + ((i + j) * b_size)], first_set);
                    _mm256_store_ps(&result[b + ((i + j) * b_size) + 8], second_set);
                }
            }
            i += 4;
        }
        
    }
    
    FullAmpStateVector* full_state = new FullAmpStateVector(amp, size);
    return full_state;
}

FullAmpStateVector* SumOfTensorsProductsStateVector::
ConvertSumOfTensorsToState()
{
    Rescale();
    ApplyGlobalICounter();
    const int num_q_B = tensor_addends[0] -> GetStateBNumQ(), num_q_A = tensor_addends[0] -> GetStateANumQ(),
    total_q = num_q_A  + num_q_B;
    const idx_size size = 1ull << total_q, A_size = 1ull << num_q_A, B_size = 1ull << num_q_B;
    const bitset<128> B_qubits_bitmask = tensor_addends[0] -> GetStateBBitmask();
    
    cmplx* amp;
    posix_memalign((void**)&amp, 64, sizeof(cmplx) * size);
    memset(amp, 0, size * sizeof(amp));
    
    if (sim_type == Config::SimType::LosslessV) {
        for (idx_size i = 0; i < size; ++i) {
            for (idx_size n = 0; n < num_addends; ++n)
                amp[i] += (*tensor_addends[n])[i];
        }
    }
    else {
        for (idx_size j = 0; j < num_addends; ++j) {
            auto& state_A = *(tensor_addends[j] -> state_a);
            auto& state_B = *(tensor_addends[j] -> state_b);
            for (idx_size a = 0; a < A_size; ++a) {
                const cmplx t_a = state_A[a];
                for (idx_size b = 0; b < B_size; ++b) {
                    bitset<128> temp_b = b;
                    bitset<128> temp_a = a << num_q_B;
                    bitset<128> i = temp_a | (temp_b & B_qubits_bitmask);
                    amp[i.to_ulong()] += t_a * state_B[b];
                }
            }
        }
    }
    FullAmpStateVector* full_state = new FullAmpStateVector(amp, size);
    return full_state;
}

cmplx SumOfTensorsProductsStateVector::
operator[](bitset<128> i) const
{
    cmplx val = 0;
    for (auto& t : tensor_addends)
        val += (*t)[i];
    return val;
}

double SumOfTensorsProductsStateVector::
GetMinProb() const
{
    double min = 0;
    for (auto& t : tensor_addends)
        min += t -> GetMinProb();
    return min;
}

double SumOfTensorsProductsStateVector::
GetMaxProb() const
{
    double max = 0;
    for (auto& t : tensor_addends)
        max += t -> GetMaxProb();
    return max;
}

double SumOfTensorsProductsStateVector::
GetAvgProb() const
{
    return 1.0/GetFullStateVectorSize();
}

double SumOfTensorsProductsStateVector::
GetMemUsage() const
{
    double mem = 0;
    for (auto& t : tensor_addends)
        mem += t -> GetMemUsage();
    return mem;
}

idx_size  SumOfTensorsProductsStateVector::
GetNumAddends() const
{
    return num_addends;
}

idx_size SumOfTensorsProductsStateVector::
GetSize() const
{
    idx_size size = 0;
    for (auto& t : tensor_addends)
        size += t -> GetSize();
    return size;
}

idx_size SumOfTensorsProductsStateVector::
GetFullStateVectorSize() const
{
    return tensor_addends[0] -> GetFullStateVectorSize();
}

idx_size SumOfTensorsProductsStateVector::
GetGlobalFactorPower() const
{
    idx_size max = 0;
    for (auto& t : tensor_addends)
        if (t -> GetGlobalFactorPower() > max)
            max = t -> GetGlobalFactorPower();
    return max;
}

int SumOfTensorsProductsStateVector::
GetStateANumQ() const
{
    return tensor_addends[0] -> GetStateANumQ();
}

int SumOfTensorsProductsStateVector::
GetStateBNumQ() const
{
  return tensor_addends[0] -> GetStateBNumQ();
}

Config::SimType SumOfTensorsProductsStateVector::
GetSimType() const
{
    return sim_type;
}

double SumOfTensorsProductsStateVector::
CalculateNormSquared()
{
    double norm = 0;
    for (auto& t : tensor_addends)
        norm += t -> CalculateNormSquared();
    return norm;
}

double SumOfTensorsProductsStateVector::
CalculateAverageInaccuracy(double norm) const
{
    return abs(1.0 - norm)/ (GetFullStateVectorSize());
}

double SumOfTensorsProductsStateVector::
CalculateMeanEntropy() const
{
    if (sim_type == Config::SimType::ApproxOWT || sim_type == Config::SimType::Approx2011OWT)
        return CalculateMeanEntropy2Cuts();
    else
        return CalculateMeanEntropyHCuts();
}

double SumOfTensorsProductsStateVector::
CalculateCrossEntropy(int range) const
{
    if (sim_type == Config::SimType::ApproxOWT || sim_type == Config::SimType::Approx2011OWT)
        return CalculateCrossEntropy2Cuts(range);
    else
        return CalculateCrossEntropyHCuts(range);
}

double SumOfTensorsProductsStateVector::
CalculateMeanEntropyHCuts() const
{
    const idx_size a_size = 1ull << tensor_addends[0] -> GetStateANumQ(),
    b_size = 1ull << tensor_addends[0] -> GetStateBNumQ();
    long double entropy = 0.0;
    const idx_size num_ranges_a = a_size / 100, num_ranges_b = b_size / 10;
    
    for (idx_size n = 0; n < num_addends; ++n)
        tensor_addends[n] -> Rescale();
    
    for (idx_size a = 0; a < num_ranges_a; ++a) {
        idx_size idx_a = (a * 100) + (rand() % 100);
        for (idx_size b = 0; b < num_ranges_b; ++b) {
            idx_size idx_b = (b * 10) + (rand() % 10);
            cmplx ampl = 0;
            for (idx_size n = 0; n < num_addends; ++n) {
                auto& state_v_a = *(tensor_addends[n] -> state_a),
                state_v_b = *(tensor_addends[n] -> state_b);
                ampl += state_v_a[idx_a] * state_v_b[idx_b];
            }
            if ((real(ampl) > 1e-20 || imag(ampl) > 1e-20))
                entropy += norm(ampl) * log2l(norm(ampl));
        }
    }
    
    return -entropy * 1000;
}

double SumOfTensorsProductsStateVector::
CalculateCrossEntropyHCuts(int range) const
{
    const idx_size a_size = 1ull << tensor_addends[0] -> GetStateANumQ(),
    b_size = 1ull << tensor_addends[0] -> GetStateBNumQ();
    double xe = 0.0, num_ranges_a = a_size / range, num_ranges_b = b_size / 10;
    
    for (idx_size n = 0; n < num_addends; ++n)
        tensor_addends[n] -> Rescale();
    
    for (idx_size a = 0; a < num_ranges_a; ++a) {
        idx_size idx_a = (a * range) + (rand() % range);
        for (idx_size b = 0; b < num_ranges_b; ++b) {
            idx_size idx_b = (b * 10) + (rand() % 10);
            cmplx ampl = 0;
            for (idx_size n = 0; n < num_addends; ++n) {
                auto& state_v_a = *(tensor_addends[n] -> state_a), state_v_b = *(tensor_addends[n] -> state_b);
                ampl += state_v_a[idx_a] * state_v_b[idx_b];
            }
            if ((real(ampl) > 1e-20 || imag(ampl) > 1e-20))
                xe += log2l(norm(ampl));
        }
    }
    return -xe / (num_ranges_a * num_ranges_b);
}

double SumOfTensorsProductsStateVector::
CalculateMeanEntropy2Cuts() const
{
    for (idx_size n = 0; n < num_addends; ++n)
        tensor_addends[n] -> Rescale();
    
    const idx_size amp_size = GetFullStateVectorSize();
    double entropy = 0.0;
    
    const auto& t0 = *tensor_addends[0], t1 = *tensor_addends[1];
    idx_size range = sampling_factor , num_ranges = amp_size / range;
    for (idx_size i = 0; i < num_ranges; ++i) {
        idx_size idx = (i * range) + (rand() % range);
        cmplx ampl =  t0[idx] + t1[idx];
        
        if ((real(ampl) > 1e-20 || imag(ampl) > 1e-20))
             entropy += norm(ampl) * log2l(norm(ampl));
    }
    
    return -entropy * range;
}

double SumOfTensorsProductsStateVector::
CalculateCrossEntropy2Cuts(int range) const
{
    for (idx_size n = 0; n < num_addends; ++n)
        tensor_addends[n] -> Rescale();
    
    const idx_size amp_size = GetFullStateVectorSize();
    long double xe = 0.0;
    
    srand(6);
    const auto& t0 = *tensor_addends[0], t1 = *tensor_addends[1];
    idx_size num_ranges = amp_size / range;
    for (idx_size i = 0; i < num_ranges; ++i) {
        idx_size idx = (i * range) + (rand() % range);
        cmplx ampl =  t0[idx] + t1[idx];
        
        if ((real(ampl) > 1e-20 || imag(ampl) > 1e-20))
            xe += log2l(norm(ampl));
    }
    
    return -xe / num_ranges;
}

void SumOfTensorsProductsStateVector::
Normalize()
{
    for (auto& t : tensor_addends)
        t -> Normalize();
}

void SumOfTensorsProductsStateVector::
Rescale()
{
    for (auto& t : tensor_addends)
        t -> Rescale();
}

void SumOfTensorsProductsStateVector::
RescaleAndApplyGlobalICounter()
{
    for (auto& t : tensor_addends)
        t -> RescaleAndApplyGlobalICounter();
}

void SumOfTensorsProductsStateVector::
ApplyGlobalICounter()
{
    for (auto& t : tensor_addends)
        t -> ApplyGlobalICounter();
}

idx_size SumOfTensorsProductsStateVector::
CountZeroAmp() const
{
    // const idx_size total_size = 1ull << (tensor_addends[0] -> GetStateANumQ() + tensor_addends[0] -> GetStateBNumQ());
    // idx_size zero_count = 0;
    
//    for (idx_size i = 0; i < total_size; ++i) {
//        if ((*this)[i] == cmplx(0,0))
//            ++zero_count;
//    }

    if (num_addends == 1)
        return tensor_addends[0] -> CountZeroAmp();
    
   return 0;
}

void SumOfTensorsProductsStateVector::
PrintStateVector(const string& outfile,
                 const int cycle_num) 
{
    ofstream file;
    file.open(outfile + "_" + to_string(cycle_num) + ".txt");
    
    RescaleAndApplyGlobalICounter();
    
    if (sim_type == Config::SimType::ApproxOWT || sim_type == Config::SimType::Approx2011OWT ||
        sim_type == Config::SimType::Approx_i11iOWT) {
        srand(6);
        const auto& t0 = *tensor_addends[0], t1 = *tensor_addends[1];
        
        idx_size off = 0, amp_size = GetFullStateVectorSize();
        for (idx_size i = 0; i + off < amp_size; i += off) {
            cmplx amp =  (t0[i] + t1[i]);
            
            file << real(amp) ;
            
            if (imag(amp) > 0)
                file << "+" << imag(amp) << "j";
            else if (imag(amp) < 0)
                file << imag(amp) << "j";
            file << "\n";
            
            off = 1 + rand() % sampling_factor;
        }
    }
    else {
        
    }
}

void SumOfTensorsProductsStateVector::
PrintStateVector() 
{
    RescaleAndApplyGlobalICounter();
    
    idx_size amp_size = GetFullStateVectorSize();
    for (idx_size i = 0; i < amp_size; ++i) {
        cmplx amp =  0;
        for (idx_size n = 0; n < num_addends; ++n)
            amp += (*tensor_addends[n])[i];
        
        cout << real(amp) ;
        
        if (imag(amp) > 0)
            cout << "+" << imag(amp) << "j";
        else if (imag(amp) < 0)
            cout << imag(amp) << "j";
        cout << "\n";
    }
     cout << "\n\n";
}

void SumOfTensorsProductsStateVector::
PrintProbabilities(const string& out_file,
                   const int cycle_num)
{
    ofstream file;
    file.open(out_file + "_" + to_string(cycle_num) + ".txt");
    
    RescaleAndApplyGlobalICounter();
    double norm_f = sqrt(CalculateNormSquared());
    
    if (sim_type == Config::SimType::ApproxOWT || sim_type == Config::SimType::Approx2011OWT ||
        sim_type == Config::SimType::Approx_i11iOWT) {
        srand(6);
        const auto& t0 = *tensor_addends[0], t1 = *tensor_addends[1];
        
        idx_size off = 0, amp_size = GetFullStateVectorSize();
        for (idx_size i = 0; i + off < amp_size; i += off) {
            float prob =  (norm(t0[i] + t1[i])/norm_f) * (amp_size/2);
            
            file << prob << "\n";
            
            off = 1 + rand() % sampling_factor;
        }
    }
    else {
        
    }
}

