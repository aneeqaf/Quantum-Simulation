//
//  state_tensor.cpp
//  vector_state_sim
//
//  Created by Aneeqa Fatima on 1/16/18.
//  Copyright © 2018 Aneeqa Fatima. All rights reserved.
//

#include "state_tensor.h"

TensorPartitions::
TensorPartitions(int qubits, Cuts cut)
{
    int q_A = 0, q_B = 0 ;
    
    if (cut == Cuts::horizontal)
       HorizontalCut(q_A, q_B, qubits);
    else
       VerticalCut(q_A, q_B, qubits);
    
    state_A = new State(q_A);
    state_B = new State(q_B);
}

TensorPartitions::
TensorPartitions(const TensorPartitions& rhs)
{
    *state_A = *(rhs.state_A);
    *state_B = *(rhs.state_B);
    A_qubits_bitmask = rhs.A_qubits_bitmask;
    B_qubits_bitmask = rhs.B_qubits_bitmask;
}

TensorPartitions& TensorPartitions::
operator=(const TensorPartitions& rhs)
{
    TensorPartitions temp(rhs);
    swap(state_A, temp.state_A);
    swap(state_B, temp.state_B);
    A_qubits_bitmask = temp.A_qubits_bitmask;
    B_qubits_bitmask = temp.B_qubits_bitmask;
    return *this;
}

TensorPartitions::
~TensorPartitions()
{
    delete state_A;
    delete state_B;
}

void TensorPartitions::
HorizontalCut(int& num_qubits_A,
              int& num_qubits_B,
              const int total_qubits)
{
    num_qubits_A = total_qubits/2;
    
    for (int i = 0; i < num_qubits_A; ++i)
        A_qubits_bitmask |= 1ull << i;
    
    for (int i = num_qubits_A; i < total_qubits; ++i) {
        B_qubits_bitmask |= 1ull << i;
        ++num_qubits_B;
    }
}

void TensorPartitions::
VerticalCut(int& num_qubits_A,
            int& num_qubits_B,
            const int total_qubits)
{
    int x_axis = floor(sqrt(total_qubits));
    int y_axis = ceil(sqrt(total_qubits));
    
    int v_cut = ceil(sqrt(x_axis));
    
    for (int i = 0; i < y_axis; ++i) {
        for (int j = 0; j < v_cut; ++i) {
            A_qubits_bitmask |= 1ull << ((i * x_axis) + j);
        }
        for (int j = v_cut; j < x_axis; ++i) {
            B_qubits_bitmask |= 1ull << ((i * x_axis) + j);
        }
    }
}

void TensorPartitions::
PreprocessXYBlock(idx_size& gate_i,
                  vector<Gate>& stateA_XY_gates,
                  vector<Gate>& stateB_XY_gates,
                  const vector<Gate>& all_gates)
{
    for (; gate_i < all_gates.size(); ++gate_i) {
        if (A_qubits_bitmask & (1ull << all_gates[gate_i].qubits[0]))
            stateA_XY_gates.push_back(all_gates[gate_i]);
        else
            stateB_XY_gates.push_back(all_gates[gate_i]);
    }
}

void TensorPartitions::
PreprocessCZTBlock(idx_size& gate_i,
                   vector<Gate>& stateA_CZT_gates,
                   vector<Gate>& stateB_CZT_gates,
                   vector<Gate>& across_AB_CZ_gates,
                   const vector<Gate>& all_gates)
{
    for (; gate_i < all_gates.size() ; ++gate_i) {
        auto const gt = all_gates[gate_i];
        
        if ((gt.ids.front() == Gate::Type::Control && (A_qubits_bitmask & (1ull << gt.qubits[0]))
             && (A_qubits_bitmask & (1ull << gt.qubits[1]))) ||
            ((gt.ids[0] == Gate::Type::T)  &&  (A_qubits_bitmask & (1ull << gt.qubits[0]))))
                stateA_CZT_gates.push_back(gt);
        else if ((gt.ids.front() == Gate::Type::Control && (B_qubits_bitmask & (1ull << gt.qubits[0]))
                  && (B_qubits_bitmask & (1ull << gt.qubits[1]))) ||
                 ((gt.ids[0] == Gate::Type::T)  &&  (B_qubits_bitmask & (1ull << gt.qubits[0]))))
                stateB_CZT_gates.push_back(gt);
        else if (gt.ids.front() == Gate::Type::Control)
            across_AB_CZ_gates.push_back(gt);
        else break;
    }
}

void TensorPartitions::
ApplyBlockOfDiagGates(const idx_size* __restrict CZ_bitmasks,
                      const idx_size __restrict T_bitmasks[2],
                      const int total_circuit_qubits)
{
    
}

void TensorPartitions::
ApplyNonCGate(const int gate_qubit,
              const int total_circuit_qubits,
              const Gate::Type gate_type,
              const Gate& g)
{
    if (A_qubits_bitmask & (1ull << gate_qubit))
        state_A -> ApplyNonCGate(gate_qubit, total_circuit_qubits, gate_type, g);
    else
        state_B -> ApplyNonCGate(gate_qubit, total_circuit_qubits, gate_type, g);
}

void TensorPartitions::
ApplyHGateOnAllAmps()
{
    state_A -> ApplyHGateOnAllAmps();
    state_B -> ApplyHGateOnAllAmps();
}

//TODO
void TensorPartitions::
ApplyCGate(const int num_controls,
           const vector<int>& gate_qubits,
           const int total_circuit_qubits,
           const Gate& g,
           const Gate::Type gate_type)
{
    
}

void TensorPartitions::
ApplyMergedXYGate(const Gate& gate1,
                  const Gate& gate2,
                  const int total_circuit_qubits)
{
    
}

void TensorPartitions::
ApplyClusterOfXYHGates(idx_size& gate_i,
                       idx_size& odd_Xi,
                       idx_size& odd_Yi,
                       const vector<Gate>& all_gates,
                       const int total_circuit_qubits)
{
    
}

void TensorPartitions::
ApplyXYRecursiveTransform(idx_size X_bitmask,
                          idx_size Y_bitmask,
                          const int total_circuit_qubits,
                          const int th)
{
    
}

cmplx TensorPartitions::
operator[](idx_size i) const
{
    return 0;
}

double TensorPartitions::
GetMinProb() const
{
    return 0;
}

double TensorPartitions::
GetMaxProb() const
{
    return 0;
}

double TensorPartitions::
GetAvgProb() const
{
    return 0;
}

double TensorPartitions::
GetMemUsage() const
{
    return 0;
}

idx_size TensorPartitions::
GetAmpSize() const
{
    return 0;
}

idx_size TensorPartitions::
GetGlobalFactorPower() const
{
    return 0;
}

idx_size TensorPartitions::
GetGlobalIcounter() const
{
    return 0;
}

double TensorPartitions::
CalculateNormOfAmp()
{
    return 0;
}

const cmplx* const TensorPartitions::
GetAmp() const
{
    return 0;
}

void TensorPartitions::
IncrementGlobalFactorPower(int num)
{
    
}

void TensorPartitions::
ResetGlobalFactorPower()
{
    
}

void TensorPartitions::
Rescale()
{
    
}

void TensorPartitions::
ApplyGlobalICounter()
{
    
}

void TensorPartitions::
PrintStateVector(const string& outfile) const
{
    
}

void TensorPartitions::
PrintStateVector() const
{
    
}

void TensorPartitions::
PrintProbabilities(const string& out_file) const
{
    
}
