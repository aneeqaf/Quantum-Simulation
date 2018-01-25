//
//  state_tensor.cpp
//  vector_state_sim
//
//  Created by Aneeqa Fatima on 1/16/18.
//  Copyright © 2018 Aneeqa Fatima. All rights reserved.
//

#include "state_tensor.h"

idx_size TensorProductStateVector::
Project1QBitmask(const idx_size gate_bitmask,
                 const idx_size partition_bitmask,
                 const int num_qubits,
                 const bool zero_least_sig,
                 const bool leading_ones)
{
    const int modified_q = num_qubits - 1;
    idx_size projected_bitmask = 0, c = 0;
    if (!leading_ones) {
        if (!zero_least_sig) {
            for (int i = modified_q; i >= 0; --i) {
                if ((1ull << (modified_q - i)) & partition_bitmask) {
                    if ((1ull << (modified_q - i)) & gate_bitmask)
                        projected_bitmask |= 1ull << c;
                    ++c;
                }
            }
        }
        else {
            for (int i = 0; i < num_qubits; ++i) {
                if ((1ull << (modified_q - i)) & partition_bitmask) {
                    if ((1ull << i) & gate_bitmask)
                        projected_bitmask |= 1ull << c;
                    ++c;
                }
            }
        }
    }
    else
        return gate_bitmask & partition_bitmask;
    
    return projected_bitmask;
}

int TensorProductStateVector::
ProjectQubit(const int qubit_to_project,
             const idx_size partition_bitmask,
             const int num_qubits)
{
    const int modified_q = num_qubits - 1;
    int c = 0;
    for (int i = 0; i < num_qubits; ++i) {
        if ((1ull << (modified_q - i)) & partition_bitmask) {
            if (i == qubit_to_project)
                return c;
            ++c;
        }
    }
    assert(false);
    return -1;
}

bool TensorProductStateVector::
ProjectCZBitmask(idx_size* __restrict projected_bitmasks,
                 const idx_size partition_bitmask,
                 const idx_size* __restrict gate_bitmasks,
                 const int total_circuit_qubits)
{
    const int modified_q = total_circuit_qubits - 1;
    idx_size c = 0;
    bool bitmask_0 = true;
    for (int i = modified_q; i >= 0; --i) {
        if ((1ull << (modified_q - i)) & partition_bitmask) {
            if ((gate_bitmasks[modified_q - i] & partition_bitmask)) {
                projected_bitmasks[c] = Project1QBitmask(gate_bitmasks[modified_q - i]
                                                         & partition_bitmask, partition_bitmask, total_circuit_qubits,
                                                         false);
                bitmask_0 = false;
            }
            ++c;
        }
    }
    return !bitmask_0;
}

idx_size TensorProductStateVector::
ScatterGlobalIndex(const idx_size i,
                   const idx_size partition_bitmask,
                   const idx_size total_qubits)
{
    idx_size local_idx = 0, count = 0;
    
    for (idx_size j = 0; j < total_qubits; ++j) {
        if (((1ull << j) & partition_bitmask) && ((1ull << j) & i))
            local_idx |= (1ull << count++);
        else if (((1ull << j) & partition_bitmask) && !((1ull << j) & i))
            ++count;
    }
    return local_idx;
}

TensorProductStateVector::
TensorProductStateVector(int qubits,
                         Cuts cut):A_qubits_bitmask(0), B_qubits_bitmask(0), num_q_A(0), num_q_B(0)
{
    cut_type = cut;
    if (cut == Cuts::Horizontal)
       HorizontalCut(num_q_A, num_q_B, qubits);
    else
       VerticalCut(num_q_A, num_q_B, qubits);
    
    state_A = new FullAmpStateVector(num_q_A);
    state_B = new FullAmpStateVector(num_q_B);
}

TensorProductStateVector::
TensorProductStateVector(const TensorProductStateVector& rhs)
{
    state_A = new FullAmpStateVector(*(rhs.state_A));
    state_B = new FullAmpStateVector(*(rhs.state_B));
    A_qubits_bitmask = rhs.A_qubits_bitmask;
    B_qubits_bitmask = rhs.B_qubits_bitmask;
    cut_type = rhs.cut_type;
    num_q_A = rhs.num_q_A;
    num_q_B = rhs.num_q_B;
}

TensorProductStateVector::
~TensorProductStateVector()
{
    delete state_A;
    delete state_B;
}

void TensorProductStateVector::
HorizontalCut(int& num_qubits_A,
              int& num_qubits_B,
              const int total_qubits)
{
    num_qubits_A = ceil(total_qubits/2);
    const int modified_q = total_qubits - 1;
    
    for (int i = 0; i < num_qubits_A; ++i)
        A_qubits_bitmask |= 1ull << (modified_q - i);
    
    for (int i = num_qubits_A; i < total_qubits; ++i) {
        B_qubits_bitmask |= 1ull << (modified_q - i);
        ++num_qubits_B;
    }
}

int FindDivisor(int num)
{
    int div = 0;
    for (int i = 1; i <= floor(sqrt(num)); ++i){
        if (num % i == 0)
            div = i;
    }
    return div;
}

void TensorProductStateVector::
VerticalCut(int& num_qubits_A,
            int& num_qubits_B,
            const int total_qubits)
{
    const int x_axis_sz = FindDivisor(total_qubits), y_axis_sz = total_qubits/x_axis_sz,
    modified_q = total_qubits - 1, v_cut = ceil(x_axis_sz/2);
    
    for (int i = 0; i < y_axis_sz; ++i) {
        for (int j = 0; j < v_cut; ++j) {
            A_qubits_bitmask |= 1ull << (modified_q - ((i * x_axis_sz) + j));
            ++num_qubits_A;
        }
        for (int j = v_cut; j < x_axis_sz; ++j) {
            B_qubits_bitmask |= 1ull << (modified_q - ((i * x_axis_sz) + j));
            ++num_qubits_B;
        }
    }
}

void TensorProductStateVector::
FindCZGatesBetweenPartitions(vector<pair<int,idx_size>>& CZ_bitmasks,
                             const idx_size* __restrict gate_bitmasks)
{
    const int qubits_A = state_A -> GetNumQubits();
    const int modified_q = qubits_A + state_B -> GetNumQubits() - 1;
    int c = 0;
    for (int i = 0; c <  qubits_A; ++i) {
        if ((1ull << (modified_q - i)) & A_qubits_bitmask) {
            if ((gate_bitmasks[modified_q - i] & A_qubits_bitmask) != gate_bitmasks[modified_q - i]) {
                pair<int,idx_size> temp;
                temp.second = Project1QBitmask(gate_bitmasks[modified_q - i], B_qubits_bitmask, modified_q + 1,
                                               false);
                temp.first = c;
                CZ_bitmasks.push_back(temp);
            }
            ++c;
        }
    }
}

void TensorProductStateVector::
ApplyCZGateAcrossTensorFactors(const Gate::Type CZ_D_A,
                               const Gate::Type CZ_D_B,
                               const int qubit_A,
                               const int qubit_B)
{
    state_A -> ApplyCZDecompositions(qubit_A, CZ_D_A);
    state_B -> ApplyCZDecompositions(qubit_B, CZ_D_B);
}


void TensorProductStateVector::
ApplyBlockOfDiagGates(const idx_size* __restrict CZ_bitmasks,
                      const idx_size __restrict T_bitmasks[2])
{
    const int num_q_A = state_A -> GetNumQubits(), num_q_B = state_B -> GetNumQubits(),
    total_circuit_qubits = num_q_A + num_q_B;
    idx_size CZ_bitmasks_A[num_q_A];
    idx_size CZ_bitmasks_B[num_q_B];
    idx_size T_bitmasks_A[2] = {0};
    idx_size T_bitmasks_B[2] = {0};
    
    for (int i = 0; i < num_q_A; ++i)
        CZ_bitmasks_A[i] = 0;
        
    for (int i = 0; i < num_q_B; ++i)
        CZ_bitmasks_B[i] = 0;
    
    bool applyCZ_A = ProjectCZBitmask(CZ_bitmasks_A, A_qubits_bitmask, CZ_bitmasks, total_circuit_qubits);
    bool applyCZ_B = ProjectCZBitmask(CZ_bitmasks_B, B_qubits_bitmask, CZ_bitmasks, total_circuit_qubits);
    for (int i = 0; i < 2; ++i) {
        T_bitmasks_A[i] = Project1QBitmask(T_bitmasks[i], A_qubits_bitmask, total_circuit_qubits, false);
        T_bitmasks_B[i] = Project1QBitmask(T_bitmasks[i], B_qubits_bitmask, total_circuit_qubits, false);
    }
    
    if (applyCZ_A || T_bitmasks_A[0])
        state_A -> ApplyBlockOfDiagGates(CZ_bitmasks_A, T_bitmasks_A);
    if (applyCZ_B || T_bitmasks_B[0])
        state_B -> ApplyBlockOfDiagGates(CZ_bitmasks_B, T_bitmasks_B);
}

void TensorProductStateVector::
ApplyNonCGate(const int gate_qubit,
              const Gate::Type gate_type,
              const Gate& g)
{
    const int modified_q = state_A -> GetNumQubits() + state_B -> GetNumQubits() - 1;
    if (A_qubits_bitmask & (1ull << gate_qubit)) {
        const int projected_gate_q = ProjectQubit(modified_q - gate_qubit, A_qubits_bitmask, modified_q + 1);
        state_A -> ApplyNonCGate(projected_gate_q, gate_type, g);
    }
    else {
        const int projected_gate_q = ProjectQubit(modified_q - gate_qubit, B_qubits_bitmask, modified_q + 1);
        state_B -> ApplyNonCGate(projected_gate_q, gate_type, g);
    }
}

void TensorProductStateVector::
ApplyHGateOnAllAmps()
{
    state_A -> ApplyHGateOnAllAmps();
    state_B -> ApplyHGateOnAllAmps();
}

//TODO
void TensorProductStateVector::
ApplyCGate(const int num_controls,
           const vector<int>& gate_qubits,
           const Gate& g,
           const Gate::Type gate_type)
{
    
}

void TensorProductStateVector::
ApplyMergedXYGate(const Gate& gate1,
                  const Gate& gate2)
{
    if (((1ull << gate1.qubits[0]) & A_qubits_bitmask) && ((1ull << gate2.qubits[0]) & A_qubits_bitmask))
        state_A -> ApplyMergedXYGate(gate1, gate2);
    else if (((1ull << gate1.qubits[0]) & B_qubits_bitmask) && ((1ull << gate2.qubits[0]) & B_qubits_bitmask))
        state_B -> ApplyMergedXYGate(gate1, gate2);
    else {
        if ((1ull << gate1.qubits[0]) & A_qubits_bitmask)
            state_A -> ApplyNonCGate(gate1.qubits[0], (Gate::Type)gate1.ids.back());
        else
            state_B -> ApplyNonCGate(gate1.qubits[0], (Gate::Type)gate1.ids.back());

        if ((1ull << gate2.qubits[0]) & B_qubits_bitmask)
            state_A -> ApplyNonCGate(gate2.qubits[0], (Gate::Type)gate2.ids.back());
        else
            state_B -> ApplyNonCGate(gate2.qubits[0], (Gate::Type)gate2.ids.back());
    }
}

void TensorProductStateVector::
ApplyXYRecursiveTransform(idx_size X_bitmask,
                          idx_size Y_bitmask,
                          const int th)
{
    int total_circuit_qubits =  __builtin_popcountll(A_qubits_bitmask | B_qubits_bitmask);
    idx_size stateA_Xbitmask = Project1QBitmask(X_bitmask, A_qubits_bitmask, total_circuit_qubits, true);
    idx_size stateA_Ybitmask = Project1QBitmask(Y_bitmask, A_qubits_bitmask, total_circuit_qubits, true);
    idx_size stateB_Xbitmask = Project1QBitmask(X_bitmask, B_qubits_bitmask, total_circuit_qubits, true);
    idx_size stateB_Ybitmask = Project1QBitmask(Y_bitmask, B_qubits_bitmask, total_circuit_qubits, true);
    
    state_A -> ApplyXYRecursiveTransform(stateA_Xbitmask, stateA_Ybitmask, th);
    state_B -> ApplyXYRecursiveTransform(stateB_Xbitmask, stateB_Ybitmask, th);
}

cmplx TensorProductStateVector::
operator[](idx_size i) const
{
    const idx_size total_q = num_q_A + num_q_B;
    
    if (cut_type == Cuts::Horizontal) {
        idx_size a = i >> num_q_B;
        idx_size b = i & B_qubits_bitmask;
        return (*state_A)[a] * (*state_B)[b];
    }
    else {
        idx_size a = TensorProductStateVector::ScatterGlobalIndex(i, A_qubits_bitmask, total_q);
        idx_size b = TensorProductStateVector::ScatterGlobalIndex(i, B_qubits_bitmask, total_q);
        return (*state_A)[a] * (*state_B)[b];
    }
}

double TensorProductStateVector::
GetMinProb() const
{
    return state_A -> GetMinProb() * state_B -> GetMinProb();
}

double TensorProductStateVector::
GetMaxProb() const
{
    return state_A -> GetMaxProb() * state_B -> GetMaxProb();
}

double TensorProductStateVector::
GetAvgProb() const
{
    return 1.0/GetFullStateSize();
}

double TensorProductStateVector::
GetMemUsage() const
{
    return state_A -> GetMemUsage() + state_B -> GetMemUsage();
}

idx_size TensorProductStateVector::
GetSize() const
{
    return (idx_size)(state_A -> GetSize() + state_B -> GetSize());
}

idx_size TensorProductStateVector::
GetFullStateSize() const
{
    return (idx_size)(1ull << (num_q_A + num_q_B));
}

int TensorProductStateVector::
GetStateANumQ() const
{
    return num_q_A;
}

int TensorProductStateVector::
GetStateBNumQ() const
{
    return num_q_B;
}

double TensorProductStateVector::
CalculateNormOfAmp()
{
    return state_A -> CalculateNormOfAmp() * state_B -> CalculateNormOfAmp();
}

void TensorProductStateVector::
Rescale()
{
    state_A -> Rescale();
    state_B -> Rescale();
}

void TensorProductStateVector::
RescaleAndApplyGlobalICounter()
{
    state_A -> RescaleAndApplyGlobalICounter();
    state_B -> RescaleAndApplyGlobalICounter();
}

void TensorProductStateVector::
ApplyGlobalICounter()
{
    state_A -> ApplyGlobalICounter();
    state_B -> ApplyGlobalICounter();
}

idx_size TensorProductStateVector::
GetGlobalFactorPower() const
{
    return state_A -> GetGlobalFactorPower() > state_B -> GetGlobalFactorPower() ?
    state_A -> GetGlobalFactorPower() : state_B -> GetGlobalFactorPower();
}


void TensorProductStateVector::
PrintStateVector(const string& outfile) const
{
    
}

void TensorProductStateVector::
PrintStateVector() 
{
    for (idx_size i = 0; i < state_A -> GetSize(); ++i) {
        for (idx_size j = 0; j < state_B -> GetSize(); ++j) {
            auto state_v = (*state_A)[i] * (*state_B)[i];
           
            cout << real(state_v) ;
            
            if (imag(state_v) > 0) {
                cout << "+" << imag(state_v) << "i";
            }
            else if (imag(state_v) < 0) {
                cout << imag(state_v) << "i";
            }
            cout << "\n";
        }
    }
    cout << "\n";
}

void TensorProductStateVector::
PrintProbabilities(const string& out_file) const
{
    
}
