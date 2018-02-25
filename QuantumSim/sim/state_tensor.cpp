//
//  state_tensor.cpp
//  vector_state_sim
//
//  Created by Aneeqa Fatima on 1/16/18.
//  Copyright © 2018 Aneeqa Fatima. All rights reserved.
//

#include "state_tensor.h"

idx_size
Project1QBitmask(const bitset<128> gate_bitmask,
                 const bitset<128> partition_bitmask,
                 const int num_qubits,
                 const bool zero_least_sig,
                 const bool leading_ones)
{
    const int modified_q = num_qubits - 1;
    idx_size projected_bitmask = 0, c = 0;
    if (!leading_ones) {
        if (!zero_least_sig) {
            for (int i = modified_q; i >= 0; --i) {
                bitset<128> temp = 0;
                temp[modified_q - i] = 1;
                if ((temp & partition_bitmask) != 0) {
                    if ((temp & gate_bitmask) != 0)
                        projected_bitmask |= 1ull << c;
                    ++c;
                }
            }
        }
        else {
            for (int i = 0; i < num_qubits; ++i) {
                bitset<128> temp = 0, temp1 = 0;
                temp[modified_q - i] = 1;
                temp1[i] = 1;
                if ((temp & partition_bitmask) != 0) {
                    if ((temp1 & gate_bitmask) != 0)
                        projected_bitmask |= 1ull << c;
                    ++c;
                }
            }
        }
    }
    else {
        assert(false);
//        return gate_bitmask & partition_bitmask;
    }//TODO:FIX

    return projected_bitmask;
}

int
ProjectQubit(const int qubit_to_project,
             const bitset<128> partition_bitmask,
             const int num_qubits)
{
    const int modified_q = num_qubits - 1;
    int c = 0;
    for (int i = 0; i < num_qubits; ++i) {
        bitset<128> temp = 0;
        temp[modified_q - i] = 1;
        if ((temp & partition_bitmask) != 0) {
            if (i == qubit_to_project)
                return c;
            ++c;
        }
    }
    assert(false);
    return -1;
}

bool
ProjectCZBitmask(bitset<128>* __restrict projected_bitmasks,
                 const bitset<128> partition_bitmask,
                 const bitset<128>* __restrict gate_bitmasks,
                 const int total_circuit_qubits)
{
    const int modified_q = total_circuit_qubits - 1;
    idx_size c = 0;
    bool bitmask_0 = true;
    for (int i = modified_q; i >= 0; --i) {
        bitset<128> temp = 0;
        temp[modified_q - i] = 1;
        if ((temp & partition_bitmask) != 0) {
            if ((gate_bitmasks[modified_q - i] & partition_bitmask) != 0) {
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

bitset<128>
ScatterGlobalIndex(const bitset<128> i,
                   const bitset<128> partition_bitmask,
                   const idx_size total_qubits)
{
    bitset<128> local_idx = 0;
    int count = 0;

    for (idx_size j = 0; j < total_qubits; ++j) {
        bitset<128> j_bit = 0;
        j_bit[j] = 1;
        if ((j_bit & partition_bitmask) == 0) continue;

        if ((j_bit & i) != 0) local_idx[count++] = 1;
        else ++count;
    }
    return local_idx;
}

TensorProductStateVector::
TensorProductStateVector(const int qubits,
                         const Cuts cut,
                         const int hcut,
                         const int vcut,
                         const Config::SimType sim):
a_qubits_bitmask(0), b_qubits_bitmask(0), num_q_a(0), num_q_b(0), sim_type(sim) 
{
    static int count_h = 0, count_v = 0;
    cut_type = cut;

    if (cut == Cuts::Horizontal)
        HorizontalCut(num_q_a, num_q_b, qubits, hcut);
    else
        VerticalCut(num_q_a, num_q_b, qubits, vcut);
    
    
    state_a = new FullAmpStateVector(num_q_a);
    state_b = new FullAmpStateVector(num_q_b);
    
    if (!count_h && cut == Cuts::Horizontal) {
        string data = "";
        data += "Cut : horizontal " + to_string(num_q_a) + " + " + to_string(num_q_b) + "\n";
        log.push_back(data);
        ++count_h;
    }
    if (!count_v && cut == Cuts::Vertical) {
        string data = "";
        data += "Cut : vertical " + to_string(num_q_a) + " + " + to_string(num_q_b) + "\n";
        log.push_back(data);
        ++count_v;
    }
}

TensorProductStateVector::
TensorProductStateVector(const TensorProductStateVector& rhs)
{
    state_a = new FullAmpStateVector(*(rhs.state_a));
    state_b = new FullAmpStateVector(*(rhs.state_b));
    a_qubits_bitmask = rhs.a_qubits_bitmask;
    b_qubits_bitmask = rhs.b_qubits_bitmask;
    cut_type = rhs.cut_type;
    num_q_a = rhs.num_q_a;
    num_q_b = rhs.num_q_b;
}

TensorProductStateVector::
~TensorProductStateVector()
{
    delete state_a;
    delete state_b;
}

void TensorProductStateVector::
HorizontalCut(int& num_qubits_a,
              int& num_qubits_b,
              const int total_qubits,
              const int cut)
{
    num_qubits_a = !cut ? ceil(total_qubits/2) : cut;
    const int modified_q = total_qubits - 1;
    
    for (int i = 0; i < num_qubits_a; ++i)
        a_qubits_bitmask [modified_q - i] = 1;
    
    for (int i = num_qubits_a; i < total_qubits; ++i) {
        b_qubits_bitmask [modified_q - i] = 1;
        ++num_qubits_b;
    }
}

void TensorProductStateVector::
VerticalCut(int& num_qubits_a,
            int& num_qubits_b,
            const int total_qubits,
            const int cut)
{
    const int x_axis_sz = FindDivisor(total_qubits), y_axis_sz = total_qubits/x_axis_sz,
    modified_q = total_qubits - 1, v_cut = !cut ? ceil(x_axis_sz/2) : cut;
    
    for (int i = 0; i < y_axis_sz; ++i) {
        for (int j = 0; j < v_cut; ++j) {
            a_qubits_bitmask [modified_q - ((i * x_axis_sz) + j)] = 1;
            ++num_qubits_a;
        }
        for (int j = v_cut; j < x_axis_sz; ++j) {
            b_qubits_bitmask [modified_q - ((i * x_axis_sz) + j)] = 1;
            ++num_qubits_b;
        }
    }
}

void TensorProductStateVector::
FindCZGatesBetweenPartitions(vector<pair<int,bitset<128>>>& CZ_bitmasks,
                             const bitset<128>* __restrict gate_bitmasks)
{
    const int qubits_a = state_a -> GetNumQubits();
    const int modified_q = qubits_a + state_b -> GetNumQubits() - 1;
    pair<int,bitset<128>> temp;
    int c = 0;
    for (int i = 0; c <  qubits_a; ++i) {
        bitset<128> temp1 = 0;
        temp1[modified_q - i] = 1;
        if ((temp1 & a_qubits_bitmask) != 0) {
            if ((gate_bitmasks[modified_q - i] & a_qubits_bitmask) != gate_bitmasks[modified_q - i]) {
                temp.second = Project1QBitmask(gate_bitmasks[modified_q - i],
                                               b_qubits_bitmask, modified_q + 1,
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
                               const int qubit_a,
                               const int qubit_b)
{
    state_a -> ApplyCZDecompositions(qubit_a, CZ_D_A);
    state_b -> ApplyCZDecompositions(qubit_b, CZ_D_B);
}


bool TensorProductStateVector::
ApplyBlockOfDiagGates(string& cz_bits,
                      const bitset<128>* __restrict CZ_bitmasks,
                      const bitset<128> __restrict T_bitmasks[2])
{
    clock_t begin = clock();
    const int num_q_a = state_a -> GetNumQubits(), num_q_b = state_b -> GetNumQubits(),
    total_circuit_qubits = num_q_a + num_q_b;
    bitset<128> CZ_bitmasks_a[num_q_a];
    bitset<128> CZ_bitmasks_b[num_q_b];
    bitset<128> T_bitmasks_a[2] = {0};
    bitset<128> T_bitmasks_b[2] = {0};
    
    for (int i = 0; i < num_q_a; ++i)
        CZ_bitmasks_a[i] = 0;
        
    for (int i = 0; i < num_q_b; ++i)
        CZ_bitmasks_b[i] = 0;
    
    bool applyCZ_a = ProjectCZBitmask(CZ_bitmasks_a, a_qubits_bitmask, CZ_bitmasks, total_circuit_qubits);
    bool applyCZ_b = ProjectCZBitmask(CZ_bitmasks_b, b_qubits_bitmask, CZ_bitmasks, total_circuit_qubits);
    for (int i = 0; i < 2; ++i) {
        T_bitmasks_a[i] = Project1QBitmask(T_bitmasks[i], a_qubits_bitmask, total_circuit_qubits, false);
        T_bitmasks_b[i] = Project1QBitmask(T_bitmasks[i], b_qubits_bitmask, total_circuit_qubits, false);
    }
    clock_t end = clock();
    time_by_category.CZ_T +=  double(end - begin) / CLOCKS_PER_SEC;
    
    if (applyCZ_a || T_bitmasks_a[0] != 0)
        state_a -> ApplyBlockOfDiagGates(cz_bits, CZ_bitmasks_a, T_bitmasks_a);
    if (applyCZ_b || T_bitmasks_b[0] != 0)
        state_b -> ApplyBlockOfDiagGates(cz_bits, CZ_bitmasks_b, T_bitmasks_b);
    
    if (sim_type == Config::SimType::Approx2011)
        ApplyXCZGateApprox(CZ_bitmasks, Gate::Type::CZ_D5, Gate::Type::CZ_D3);
    else if (sim_type == Config::SimType::Approx1_101) //compute norm and divide by the norm
        ApplyXCZGateApprox(CZ_bitmasks, Gate::Type::CZ_D1, Gate::Type::CZ_D2);
    else if (sim_type == Config::SimType::Approx1110)
        ApplyXCZGateApprox(CZ_bitmasks, Gate::Type::CZ_D3, Gate::Type::CZ_D4);
    else if (sim_type == Config::SimType::Approx1CutH || (sim_type == Config::SimType::Approx1CutV)) {
        idx_size count = CountXCZGates(CZ_bitmasks);
        data_per_cycles.memory.push_back(GetMemUsage());
        data_per_cycles.addends.push_back(1);
        if (cut_type == Cuts::Horizontal) {
            data_per_cycles.xCZ_H.push_back(count);
            data_per_cycles.xCZ_V.push_back(0);
        }
        else {
            data_per_cycles.xCZ_H.push_back(count);
            data_per_cycles.xCZ_H.push_back(0);
        }
    }
    return false;
}

idx_size TensorProductStateVector::
CountXCZGates(const bitset<128>* __restrict CZ_bitmasks)
{
    vector<pair<int,bitset<128>>> qubits_gates_across;
    FindCZGatesBetweenPartitions(qubits_gates_across, CZ_bitmasks);
    
    idx_size new_count = 0;
    for (idx_size i = 0; i < qubits_gates_across.size(); ++i) {
        while (qubits_gates_across[i].second != 0) {
            ++new_count;
            int first_half = __builtin_ctzl(qubits_gates_across[i].second.to_ulong());
            int second_half = __builtin_ctzl((qubits_gates_across[i].second >> 64).to_ulong());
            const int q1 = first_half ? first_half : second_half ? 64 + second_half : 0;
            qubits_gates_across[i].second[q1] = 0;
        }
    }
    return new_count;
}

void TensorProductStateVector::
ApplyXCZGateApprox(const bitset<128>* __restrict CZ_bitmasks,
                   const Gate::Type CZ_D_A,
                   const Gate::Type CZ_D_B)
{
    clock_t begin = clock();
    vector<pair<int,bitset<128>>> qubits_gates_across;
    FindCZGatesBetweenPartitions(qubits_gates_across, CZ_bitmasks);
    
    const int modified_num_q_B = num_q_a + num_q_b - 1;
    const idx_size prev_CZ_count = count_of_category.decomposed_CZ;
    for (auto& g : qubits_gates_across) {
        while (g.second != 0) {
            int first_half = __builtin_ctzl(g.second.to_ulong());
            int second_half = __builtin_ctzl((g.second >> 64).to_ulong());
            const int q = first_half ? first_half : second_half ? 64 + second_half : 0;
            ++count_of_category.decomposed_CZ;
            ApplyCZGateAcrossTensorFactors(CZ_D_A, CZ_D_B, g.first, modified_num_q_B - q);
            g.second[q] = 0;
        }
    }
    clock_t end = clock();
    time_by_category.decomposed_CZ +=  double(end - begin) / CLOCKS_PER_SEC;
    
    if (sim_type != Config::SimType::Approx2011) {
        data_per_cycles.memory.push_back(GetMemUsage());
        data_per_cycles.addends.push_back(1);
        if (cut_type == Cuts::Horizontal) {
            data_per_cycles.xCZ_H.push_back(count_of_category.decomposed_CZ - prev_CZ_count);
            data_per_cycles.xCZ_V.push_back(0);
        }
        else {
            data_per_cycles.xCZ_V.push_back(count_of_category.decomposed_CZ - prev_CZ_count);
            data_per_cycles.xCZ_H.push_back(0);
        }
    }
}

void TensorProductStateVector::
ApplyNonCGate(const int gate_qubit,
              const Gate::Type gate_type,
              const Gate& g)
{
    const int modified_q = state_a -> GetNumQubits() + state_b -> GetNumQubits() - 1;
    bitset<128> temp = 0;
    temp[gate_qubit] = 1;
    if ((a_qubits_bitmask & temp) != 0) {
        const int projected_gate_q = ProjectQubit(modified_q - gate_qubit, a_qubits_bitmask, modified_q + 1);
        state_a -> ApplyNonCGate(projected_gate_q, gate_type, g);
    }
    else {
        const int projected_gate_q = ProjectQubit(modified_q - gate_qubit, b_qubits_bitmask, modified_q + 1);
        state_b -> ApplyNonCGate(projected_gate_q, gate_type, g);
    }
}

void TensorProductStateVector::
ApplyHGateOnAllAmps()
{
    state_a -> ApplyHGateOnAllAmps();
    state_b -> ApplyHGateOnAllAmps();
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
    bitset<128> temp1 = 0, temp2 = 0;
    temp1[gate1.qubits[0]] = 1;
    temp2[gate2.qubits[0]] = 1;
    
    if ((temp1 & a_qubits_bitmask) != 0 && (temp2 & a_qubits_bitmask) != 0)
        state_a -> ApplyMergedXYGate(gate1, gate2);
    else if ((temp1 & b_qubits_bitmask) != 0 && (temp2 & b_qubits_bitmask) != 0)
        state_b -> ApplyMergedXYGate(gate1, gate2);
    else {
        if ((temp1 & a_qubits_bitmask) != 0)
            state_a -> ApplyNonCGate(gate1.qubits[0], (Gate::Type)gate1.ids.back());
        else
            state_b -> ApplyNonCGate(gate1.qubits[0], (Gate::Type)gate1.ids.back());

        if ((temp2 & b_qubits_bitmask) != 0)
            state_a -> ApplyNonCGate(gate2.qubits[0], (Gate::Type)gate2.ids.back());
        else
            state_b -> ApplyNonCGate(gate2.qubits[0], (Gate::Type)gate2.ids.back());
    }
}

void TensorProductStateVector::
ApplyXYRecursiveTransform(bitset<128> X_bitmask,
                          bitset<128> Y_bitmask,
                          const int th)
{
    bitset<128> temp = a_qubits_bitmask | b_qubits_bitmask;
    int total_circuit_qubits =  (int)temp.count();
    bitset<128> stateA_Xbitmask = Project1QBitmask(X_bitmask, a_qubits_bitmask, total_circuit_qubits, true);
    bitset<128> stateA_Ybitmask = Project1QBitmask(Y_bitmask, a_qubits_bitmask, total_circuit_qubits, true);
    bitset<128> stateB_Xbitmask = Project1QBitmask(X_bitmask, b_qubits_bitmask, total_circuit_qubits, true);
    bitset<128> stateB_Ybitmask = Project1QBitmask(Y_bitmask, b_qubits_bitmask, total_circuit_qubits, true);
    
    state_a -> ApplyXYRecursiveTransform(stateA_Xbitmask, stateA_Ybitmask, th);
    state_b -> ApplyXYRecursiveTransform(stateB_Xbitmask, stateB_Ybitmask, th);
}

cmplx TensorProductStateVector::
operator[](bitset<128> i) const
{
    const idx_size total_q = num_q_a + num_q_b;
    
    if (cut_type == Cuts::Horizontal) {
        bitset<128> temp_i = i;
        bitset<128> a = i >> num_q_b;
        bitset<128> b = temp_i & b_qubits_bitmask;
        return (*state_a)[a.to_ulong()] * (*state_b)[b.to_ulong()];
    }
    else {
        bitset<128> a = ScatterGlobalIndex(i, a_qubits_bitmask, total_q);
        bitset<128> b = ScatterGlobalIndex(i, b_qubits_bitmask, total_q);
        return (*state_a)[a.to_ulong()] * (*state_b)[b.to_ulong()];
    }
}

cmplx TensorProductStateVector::
GetAmpFromGlobalState(const idx_size a,
                      const idx_size b) const
{
    return (*state_a)[a] * (*state_b)[b];
}

double TensorProductStateVector::
GetMinProb() const
{
    return state_a -> GetMinProb() * state_b -> GetMinProb();
}

double TensorProductStateVector::
GetMaxProb() const
{
    return state_a -> GetMaxProb() * state_b -> GetMaxProb();
}

double TensorProductStateVector::
GetAvgProb() const
{
    return 1.0/GetFullStateVectorSize();
}

double TensorProductStateVector::
GetMemUsage() const
{
    return state_a -> GetMemUsage() + state_b -> GetMemUsage();
}

idx_size TensorProductStateVector::
GetSize() const
{
    return (idx_size)(state_a -> GetSize() + state_b -> GetSize());
}

idx_size TensorProductStateVector::
GetFullStateVectorSize() const
{
    bitset<128> temp = 0;
    temp[num_q_a + num_q_b] = 1;
    return temp.to_ulong();
}

int TensorProductStateVector::
GetStateANumQ() const
{
    return num_q_a;
}

int TensorProductStateVector::
GetStateBNumQ() const
{
    return num_q_b;
}

bitset<128> TensorProductStateVector::
GetStateABitmask() const
{
    return a_qubits_bitmask;
}

bitset<128> TensorProductStateVector::
GetStateBBitmask() const
{
    return b_qubits_bitmask;
}

double TensorProductStateVector::
CalculateNormSquared()
{
    return state_a -> CalculateNormSquared() * state_b -> CalculateNormSquared();
}

double TensorProductStateVector::
CalculateAverageInaccuracy(double norm) const
{
    return abs(1.0 - norm) /(double)GetFullStateVectorSize();
}

double TensorProductStateVector::
CalculateMeanEntropy() const
{
    const idx_size a_size = 1ull << num_q_a, b_size = 1ull << num_q_b, range = sampling_factor;
    auto& state_v_a = (*state_a), state_v_b = (*state_b);
    double entropy = 0.0;
    const idx_size num_ranges_a = a_size/range, num_ranges_b = b_size/range;
    
    float rescaling_factor_a = 1.0/pow(2,(state_a -> GetGlobalFactorPower()/2)),
    rescaling_factor_b = 1.0/pow(2,(state_b -> GetGlobalFactorPower()/2));
    if ((state_a -> GetGlobalFactorPower() % 2) == 1)
        rescaling_factor_a *= 1.0/sqrt(2.0);
    if ((state_b -> GetGlobalFactorPower() % 2) == 1)
        rescaling_factor_b *= 1.0/sqrt(2.0);
    
    for (idx_size a = 0; a < num_ranges_a; ++a) {
        auto s_a = state_v_a[(a * range) + (rand() % range)];
        for (idx_size b = 0; b < num_ranges_b; ++b) {
            auto s_b = state_v_b[(b * range) + (rand() % range)];
            if ((real(s_b) > 1e-20 || imag(s_b) > 1e-20) &&
                (real(s_a) > 1e-20 || imag(s_a) > 1e-20))
                entropy += norm(s_a * s_b * rescaling_factor_a * rescaling_factor_b)
               * log2l(norm(s_a * s_b * rescaling_factor_a * rescaling_factor_b));
        }
    }
    
    return -entropy * range * range;
}

double TensorProductStateVector::
CalculateCrossEntropy(int range) const
{
    const idx_size a_size = 1ull << num_q_a, b_size = 1ull << num_q_b;
    auto& state_v_a = (*state_a), state_v_b = (*state_b);
    double xe = 0.0;
    const idx_size num_ranges_a = a_size/range, num_ranges_b = b_size/range;
    
    float rescaling_factor_a = 1.0/pow(2,(state_a -> GetGlobalFactorPower()/2)),
    rescaling_factor_b = 1.0/pow(2,(state_b -> GetGlobalFactorPower()/2));
    if ((state_a -> GetGlobalFactorPower() % 2) == 1)
        rescaling_factor_a *= 1.0/sqrt(2.0);
    if ((state_b -> GetGlobalFactorPower() % 2) == 1)
        rescaling_factor_b *= 1.0/sqrt(2.0);
    
    for (idx_size a = 0; a < num_ranges_a; ++a) {
        auto s_a = state_v_a[(a * range) + (rand() % range)];
        for (idx_size b = 0; b < num_ranges_b; ++b) {
            auto s_b = state_v_b[(b * range) + (rand() % range)];
            if ((real(s_b) > 1e-20 || imag(s_b) > 1e-20) &&
                (real(s_a) > 1e-20 || imag(s_a) > 1e-20))
                xe += log2l(norm(s_a * s_b * rescaling_factor_a * rescaling_factor_b));
        }
    }
    
    return -xe / (num_ranges_a * num_ranges_b);
}

void TensorProductStateVector::
Normalize()
{
    state_a -> Normalize();
    state_b -> Normalize();
}

void TensorProductStateVector::
Rescale()
{
    state_a -> Rescale();
    state_b -> Rescale();
}

void TensorProductStateVector::
RescaleAndApplyGlobalICounter()
{
    state_a -> RescaleAndApplyGlobalICounter();
    state_b -> RescaleAndApplyGlobalICounter();
}

void TensorProductStateVector::
ApplyGlobalICounter()
{
    state_a -> ApplyGlobalICounter();
    state_b -> ApplyGlobalICounter();
}

idx_size TensorProductStateVector::
GetGlobalFactorPower() const
{
    return state_a -> GetGlobalFactorPower() > state_b -> GetGlobalFactorPower() ?
    state_a -> GetGlobalFactorPower() : state_b -> GetGlobalFactorPower();
}

idx_size TensorProductStateVector::
CountZeroAmp() const
{
    const idx_size a_size = 1ull << num_q_a, b_size = 1ull << num_q_b;
    idx_size zero_count = 0;
    
    for (idx_size i = 0; i < a_size; ++i) {
        if ((*state_a)[i] == cmplx(0,0))
            zero_count += b_size;
        else {
            for (idx_size j = 0; j < b_size; ++j) {
                if ((*state_b)[j] == cmplx(0,0))
                    ++zero_count;
            }
        }
    }
    
    return zero_count;
}

void TensorProductStateVector::
PrintStateVector(const string& outfile,
                 const int cycle_num) 
{
    ofstream file;
    file.open(outfile + "_" + to_string(cycle_num) + ".txt");
    
    const idx_size amp_size = GetFullStateVectorSize();
    
    RescaleAndApplyGlobalICounter();
    
    srand(6);
    idx_size off = 0;
    for (idx_size i = 0; i + off < amp_size; i += off) {
        auto amp = (*this)[i];
        
        file << real(amp) ;
        
        if (imag(amp) > 0)
            file << "+" << imag(amp) << "j";
        else if (imag(amp) < 0)
            file << imag(amp) << "j";
        file << "\n";
        
        off = 1 + rand() % sampling_factor;
    }
}

void TensorProductStateVector::
PrintStateVector() 
{
    const idx_size amp_size = GetFullStateVectorSize();
    
    RescaleAndApplyGlobalICounter();
    
    for (idx_size i = 0; i < amp_size; ++i) {
        auto amp =  (*this)[i];
        
        cout << real(amp) ;
        
        if (imag(amp) > 0)
            cout << "+" << imag(amp) << "j";
        else if (imag(amp) < 0)
            cout << imag(amp) << "j";
        cout << "\n";
    }
     cout << "\n\n";
}

void TensorProductStateVector::
PrintProbabilities(const string& out_file,
                   const int cycle_num)
{
    ofstream file;
    file.open(out_file + "_" + to_string(cycle_num) + ".txt");
    
    const idx_size amp_size = GetFullStateVectorSize();
    
    RescaleAndApplyGlobalICounter();
    double norm_f = sqrt(CalculateNormSquared());

    srand(6);
    idx_size off = 0;
    for (idx_size i = 0; i + off < amp_size; i += off) {
        float prob = (norm((*this)[i])/norm_f) * amp_size;
        
        file << prob << "\n";
        
        off = 1 + rand() % sampling_factor;
    }
}
