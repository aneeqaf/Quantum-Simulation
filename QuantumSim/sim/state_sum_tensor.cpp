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
                                const SimType type,
                                const int cut_size): num_addends(1)
{
    sim_type = type;
    if (type == SimType::LosslessH || type == SimType::Approx1CutH)
        tensor_addends.push_back(new TensorProductStateVector(qubits,
                                                              TensorProductStateVector::Cuts::Horizontal,
                                                              cut_size));
    else if (type == SimType::LosslessV || type == SimType::Approx1CutV)
        tensor_addends.push_back(new TensorProductStateVector(qubits,
                                                              TensorProductStateVector::Cuts::Vertical,
                                                              cut_size));
    else {
        tensor_addends.push_back(new TensorProductStateVector(qubits,
                                                              TensorProductStateVector::Cuts::Horizontal,
                                                              cut_size));
        tensor_addends.push_back(new TensorProductStateVector(qubits,
                                                              TensorProductStateVector::Cuts::Vertical,
                                                              cut_size));
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
void SumOfTensorsProductsStateVector::
ApplyBlockOfDiagGates(const idx_size* __restrict CZ_bitmasks,
                      const idx_size __restrict T_bitmasks[2])
{
    for (auto& t : tensor_addends)
        t -> ApplyBlockOfDiagGates(CZ_bitmasks, T_bitmasks);
    
    //int represents qubit in block A and idx_size represents bitmask of qubits in block B
    //of tensor product.
    clock_t begin = clock();
    vector<pair<int,idx_size>> qubits_gates_across;
    tensor_addends[0] -> FindCZGatesBetweenPartitions(qubits_gates_across, CZ_bitmasks);
    const int modified_num_q_B = tensor_addends[0] -> GetStateBNumQ() - 1;
    const idx_size prev_CZ_count = count_of_category.decomposed_CZ;
    for (auto& g : qubits_gates_across) {
        while (g.second) {
            const int q = __builtin_ctzl(g.second);
            ++count_of_category.decomposed_CZ;
            for (idx_size i = 0; i < num_addends; ++i) {
                    TensorProductStateVector* new_t = new TensorProductStateVector(*tensor_addends[i]);
                    tensor_addends[i] -> ApplyCZGateAcrossTensorFactors(Gate::Type::CZ_D1, Gate::Type::CZ_D2,
                                                                        g.first, modified_num_q_B - q);
                    new_t -> ApplyCZGateAcrossTensorFactors(Gate::Type::CZ_D3, Gate::Type::CZ_D4,
                                                            g.first, modified_num_q_B - q);
                    tensor_addends.push_back(new_t);
            }
            g.second ^= 1ull << q;
            num_addends = tensor_addends.size();
        }
    }
    clock_t end = clock();
    time_by_category.decomposed_CZ +=  double(end - begin) / CLOCKS_PER_SEC;
    string data = to_string(count_of_category.decomposed_CZ - prev_CZ_count) + "\t";
    log.push_back(data);
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
ApplyXYRecursiveTransform(idx_size X_bitmask,
                             idx_size Y_bitmask,
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
    const idx_size size = 1ull << total_q, A_size = 1ull << num_q_A, B_size = 1ull << num_q_B,
    B_qubits_bitmask = tensor_addends[0] -> GetStateBBitmask();
    
    cmplx* amp;
    posix_memalign((void**)&amp, 64, sizeof(cmplx) * size);
    memset(amp, 0, size * sizeof(amp));
    
    for (idx_size j = 0; j < num_addends; ++j) {
        auto& state_A = *(tensor_addends[j] -> state_a);
        auto& state_B = *(tensor_addends[j] -> state_b);
        for (idx_size a = 0; a < A_size; ++a) {
            const cmplx t_a = state_A[a];
            for (idx_size b = 0; b < B_size; ++b) {
                idx_size i = (a << num_q_B) | (b & B_qubits_bitmask);
                amp[i] += t_a * state_B[b];
            }
        }
    }
    FullAmpStateVector* full_state = new FullAmpStateVector(amp, size);
    return full_state;
}

cmplx SumOfTensorsProductsStateVector::
operator[](idx_size i) const
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
    return (idx_size)(1ull << (tensor_addends[0] -> GetStateANumQ() + tensor_addends[0] -> GetStateBNumQ()));
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
    const idx_size a_size = 1ull << tensor_addends[0] -> GetStateANumQ(),
    b_size = 1ull << tensor_addends[0] -> GetStateBNumQ();
    long double entropy = 0.0, num_ranges_a = a_size / 100, num_ranges_b = b_size / 10;
    
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

    return -entropy;
}

double SumOfTensorsProductsStateVector::
CalculateCrossEntropy(int range) const
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

void SumOfTensorsProductsStateVector::
PrintStateVector(const string& outfile) const
{
    
}

void SumOfTensorsProductsStateVector::
PrintStateVector() 
{
    FullAmpStateVector* f_st = ConvertSumOfTensorsToState();
    f_st -> PrintStateVector();
    delete f_st;
}

void SumOfTensorsProductsStateVector::
PrintProbabilities(const string& out_file) const
{
    
}

