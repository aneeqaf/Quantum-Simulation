//
//  state_sum_tensor.cpp
//  vector_state_sim
//
//  Created by Aneeqa Fatima on 1/18/18.
//  Copyright © 2018 Aneeqa Fatima. All rights reserved.
//

#include "state_sum_tensor.h"

SumOfTensorsProductsStateVector::
SumOfTensorsProductsStateVector(int qubits,
             SimType type): num_addends(1)
{
    sim_type = type;
    if (type == SimType::LosslessH || type == SimType::Approx1CutH)
        tensor_addends.push_back(new TensorProductStateVector(qubits, TensorProductStateVector::Cuts::Horizontal));
    else if (type == SimType::LosslessV || type == SimType::Approx1CutV)
        tensor_addends.push_back(new TensorProductStateVector(qubits, TensorProductStateVector::Cuts::Vertical));
    else {
        tensor_addends.push_back(new TensorProductStateVector(qubits, TensorProductStateVector::Cuts::Horizontal));
        tensor_addends.push_back(new TensorProductStateVector(qubits, TensorProductStateVector::Cuts::Vertical));
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
    vector<pair<int,idx_size>> qubits_gates_across;
    tensor_addends[0] -> FindCZGatesBetweenPartitions(qubits_gates_across, CZ_bitmasks);
    const int modified_num_q_B = tensor_addends[0] -> GetStateBNumQ() - 1;
    for (auto& g : qubits_gates_across) {
        while (g.second) {
            const int q = __builtin_ctzl(g.second);
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
    for (auto& t : tensor_addends)
        t -> ApplyXYRecursiveTransform(X_bitmask, Y_bitmask, th);
}

FullAmpStateVector* SumOfTensorsProductsStateVector::
ConvertSumOfTensorsToState()
{
    Rescale();
    ApplyGlobalICounter();
    int total_q = tensor_addends[0] -> GetStateANumQ() + tensor_addends[0] -> GetStateBNumQ();
    idx_size size = 1ull << total_q;
    cmplx* amp;
    posix_memalign((void**)&amp, 64, sizeof(cmplx) * size);
    memset(amp, 0, size * sizeof(amp));
    for (idx_size i = 0; i < size; ++i) {
        cmplx state_v = 0;
        for (idx_size j = 0; j < num_addends; ++j)
            state_v += (*tensor_addends[j])[i];
        
        amp[i] = state_v;
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
        max += t -> GetMinProb();
    return max;
}

double SumOfTensorsProductsStateVector::
GetAvgProb() const
{
    return 1.0/GetFullStateSize();
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
GetFullStateSize() const
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

double SumOfTensorsProductsStateVector::
CalculateNormOfAmp()
{
    double norm = 0;
    for (auto& t : tensor_addends)
        norm += t -> CalculateNormOfAmp();
    return norm;
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

