//
//  state_tensor.cpp
//  vector_state_sim
//
//  Created by Aneeqa Fatima on 1/16/18.
//  Copyright © 2018 Aneeqa Fatima. All rights reserved.
//

#include "state_tensor.h"

idx_size* TensorProductStateVector::global_to_local_a(nullptr);
idx_size* TensorProductStateVector::global_to_local_b(nullptr);
idx_size TensorProductStateVector::num_requested_amps(0);

TensorProductStateVector::
TensorProductStateVector(const int qubits,
                         const QubitPartition::Cuts cut_type,
                         const int hcut,
                         const int vcut,
                         const Config::SimType sim,
                         const int verb):
qp(cut_type == QubitPartition::Cuts::Horizontal ? QubitPartition(cut_type, qubits, hcut)
         : QubitPartition(cut_type, qubits, vcut))
{
    sim_type = sim;
    static int count_h = 0, count_v = 0;
    this -> cut_type = cut_type;
    
//    qp.RenumberLocalQubits();

    int num_q_b0 = qp.getNumQubitsInBlock(0), num_q_b1 = qp.getNumQubitsInBlock(1);
    state_a = new FullAmpStateVector(num_q_b0);
    state_b = new FullAmpStateVector(num_q_b1);
        
    if (!count_h && cut_type == QubitPartition::Cuts::Horizontal) {
        string data = "";
        data += "Cut : horizontal " + to_string(num_q_b0) + "q + " + to_string(num_q_b1) + "q";
        log.push_back(data);
        ++count_h;
    }
    if (!count_v && cut_type == QubitPartition::Cuts::Vertical) {
        string data = "";
        data += "Cut : vertical " + to_string(num_q_b0) + "q + " + to_string(num_q_b1) + "q";
        log.push_back(data);
        ++count_v;
    }
    log.push_back(qp.print(verb));
}

TensorProductStateVector::
TensorProductStateVector(const TensorProductStateVector& rhs) : qp(rhs.qp), cut_type(rhs.cut_type),
state_a(new FullAmpStateVector(*(rhs.state_a))), state_b(new FullAmpStateVector(*(rhs.state_b)))
{
    sim_type = rhs.sim_type;
}

TensorProductStateVector::
~TensorProductStateVector()
{
    delete state_a;
    delete state_b;
}

void TensorProductStateVector::
PopulateGlobalToLocalMap(const vector<bitset<128>>& idxs)
{
    if (global_to_local_a != nullptr && global_to_local_b != nullptr) {
        delete [] global_to_local_a;
        delete [] global_to_local_b;
        global_to_local_a = nullptr;
        global_to_local_b = nullptr;
    }
    
    num_requested_amps = idxs.size();
    
    global_to_local_a = new idx_size[num_requested_amps];
    global_to_local_b = new idx_size[num_requested_amps];
    memset(global_to_local_a, 0, num_requested_amps * sizeof(idx_size));
    memset(global_to_local_b, 0, num_requested_amps * sizeof(idx_size));
    
    for (idx_size i = 0; i < num_requested_amps; ++i) {
        global_to_local_a[i] = qp.IndexScatter(idxs[i], 0);
        global_to_local_b[i] = qp.IndexScatter(idxs[i], 1);
    }
}

void TensorProductStateVector::
UnpopulateGlobalToLocalMap(){
    delete [] global_to_local_a;
    delete [] global_to_local_b;
    global_to_local_a = nullptr;
    global_to_local_b = nullptr;
}

int TensorProductStateVector::
FindCZGatesBetweenPartitions(bitset<128>* __restrict xCZ_bitmasks,
                             const bitset<128>* __restrict gate_bitmasks)
{
    const int qubits_a = state_a -> GetNumQubits(), qubits_a_1 = qubits_a - 1,
    num_q_1 = qp.getNumQubits() - 1;
    const bitset<128> block0_bitmask = qp.getBlockBitmask(0);
    const bitset<128> block1_bitmask = qp.getBlockBitmask(1);
    int count = 0;
    
    // CZ bitmasks array is numbered in the opposite direction
    for (int i = 0; i <  num_q_1 + 1; ++i) {
        if (qp.globalToBlock(num_q_1 - i) == 0) {
//            cout << "bm 1: " << gate_bitmasks[num_q_1 - i].to_string().substr(128-15) << endl;
//            cout << "bm 0: " << gate_bitmasks[num_q_1 - i].to_string().substr(128-30, 15) << endl;
//            cout << "g i 0 :" << num_q_1 - i << endl;
//            cout << "gtl 0 : "<< qp.globalToLocal(num_q_1 - i) << endl;
            
             if ((gate_bitmasks[num_q_1 - i] & block0_bitmask) != gate_bitmasks[num_q_1 - i]) {
                 cout << "xCZ" << endl;
                xCZ_bitmasks[qubits_a_1 - qp.globalToLocal(num_q_1 - i)] =
                        Project1QBitmask(gate_bitmasks[num_q_1 - i] & block1_bitmask, qp, 1) ;
                ++count;
            }
//            cout << "gtl 1: "<< __builtin_ctzl(xCZ_bitmasks[qubits_a_1 - qp.globalToLocal(num_q_1 - i)].to_ulong()) << endl;
        }
    }
    return count;
}

void TensorProductStateVector::
ApplyCZGateAcrossTensorFactors(const Gate::Type CZ_D_A,
                               const Gate::Type CZ_D_B,
                               const int qubit_a,
                               const int qubit_b)
{
    if (partition_to_sim == 'a' || partition_to_sim == 'x')
        state_a -> ApplyCZDecompositions(qubit_a, CZ_D_A);
    if (partition_to_sim == 'b' || partition_to_sim == 'x')
        state_b -> ApplyCZDecompositions(qubit_b, CZ_D_B);
}

void TensorProductStateVector::
ApplyCZGateAcrossTensorFactors(bitset<128>* __restrict xCZ_bitmasks_path0_D1D2,
                               bitset<128>* __restrict xCZ_bitmasks_path0_D2D1,
                               bitset<128>* __restrict xCZ_bitmasks_path1_D3D4,
                               bitset<128>* __restrict xCZ_bitmasks_path1_D4D3)
{
    const int num_q_a = qp.getNumQubitsInBlock(0), num_q_a_1 = num_q_a - 1;
    if (partition_to_sim == 'a' || partition_to_sim == 'x') {
        idx_size state_A_gate_bm[3] = {0};
        for (int i = 0; i < num_q_a; ++i) {
            state_A_gate_bm[0] |= xCZ_bitmasks_path0_D1D2[i].count() % 2 ? (1ull << (num_q_a_1 - i)) : 0;
            state_A_gate_bm[1] |= xCZ_bitmasks_path0_D2D1[i].count() ? (1ull << (num_q_a_1 - i)) : 0;
            state_A_gate_bm[2] |= xCZ_bitmasks_path1_D4D3[i].count() ? (1ull << (num_q_a_1 - i)) : 0;
        }
        state_a -> ApplyCZDecompositionDist(state_A_gate_bm);
    }
   if (partition_to_sim == 'b' || partition_to_sim == 'x') {
        idx_size state_B_gate_bm[3] = {0};
        for (int i = 0; i < num_q_a; ++i) {
            state_B_gate_bm[1] |= xCZ_bitmasks_path0_D1D2[i].to_ulong();
            state_B_gate_bm[0] ^= xCZ_bitmasks_path0_D2D1[i].to_ulong();
            state_B_gate_bm[2] |= xCZ_bitmasks_path1_D3D4[i].to_ulong();
        }
        state_b -> ApplyCZDecompositionDist(state_B_gate_bm);
    }
}

int TensorProductStateVector::
ApplyBlockOfDiagGates(string& cz_bits,
                      idx_size prefix_size,
                      const bitset<128>* __restrict CZ_bitmasks,
                      const bitset<128> T_bitmasks[2])
{
    
    const int num_q_a = state_a -> GetNumQubits(), num_q_b = state_b -> GetNumQubits();
    
    if (partition_to_sim == 'a' || partition_to_sim == 'x') {
        Time time;
        time.StartTime();
        
        bitset<128> CZ_bitmasks_a[num_q_a];
        bitset<128> T_bitmasks_a[2] = {0};
        
        for (int i = 0; i < num_q_a; ++i)
            CZ_bitmasks_a[i] = 0;
        
        bool applyCZ_a = ProjectCZBitmask(CZ_bitmasks_a, qp, 0, CZ_bitmasks);
        for (int i = 0; i < 2; ++i)
            T_bitmasks_a[i] = Project1QBitmask(T_bitmasks[i], qp, 0);
        
        time_by_category.CZ_T += time.GetElapsedTime();
        
        if (applyCZ_a || T_bitmasks_a[0] != 0)
            state_a -> ApplyBlockOfDiagGates(cz_bits, prefix_size, CZ_bitmasks_a, T_bitmasks_a);
    }
    if (partition_to_sim == 'b' || partition_to_sim == 'x') {
        Time time;
        time.StartTime();
        
        bitset<128> CZ_bitmasks_b[num_q_b];
        bitset<128> T_bitmasks_b[2] = {0};
        
        for (int i = 0; i < num_q_b; ++i)
            CZ_bitmasks_b[i] = 0;
        
        bool applyCZ_b = ProjectCZBitmask(CZ_bitmasks_b, qp, 1, CZ_bitmasks);
        for (int i = 0; i < 2; ++i)
            T_bitmasks_b[i] = Project1QBitmask(T_bitmasks[i], qp, 1);
        
        time_by_category.CZ_T += time.GetElapsedTime();

        if (applyCZ_b || T_bitmasks_b[0] != 0)
            state_b -> ApplyBlockOfDiagGates(cz_bits, prefix_size, CZ_bitmasks_b, T_bitmasks_b);
    }
//    state_a -> PrintStateVector() ; cout << endl;
//    state_b -> PrintStateVector() ; cout << endl;
//    // TODO : Fix the book keeping for approximation
    
    if (sim_type == Config::SimType::Approx2011 || sim_type == Config::SimType::Approx2011OWT)
        ApplyXCZGateApprox(CZ_bitmasks, Gate::Type::CZ_D5, Gate::Type::CZ_D3);
    else if (sim_type == Config::SimType::Approx1_101) //compute norm and divide by the norm
        ApplyXCZGateApprox(CZ_bitmasks, Gate::Type::CZ_D1, Gate::Type::CZ_D2);
    else if (sim_type == Config::SimType::Approx1110)
        ApplyXCZGateApprox(CZ_bitmasks, Gate::Type::CZ_D3, Gate::Type::CZ_D4);
    else if (sim_type == Config::SimType::Approx_i11i || sim_type == Config::SimType::Approx_i11iOWT)
        ApplyXCZGateApprox(CZ_bitmasks, Gate::Type::CZ_D6, Gate::Type::CZ_D7);
    else if ((sim_type == Config::SimType::Approx1CutH || (sim_type == Config::SimType::Approx1CutV))
             && sim_mode != Config::SimMode::Phase2){
        idx_size count = CountXCZGates(CZ_bitmasks);
        data_per_cycles.memory.push_back(GetMemUsage());
        data_per_cycles.addends.push_back(1);
        if (cut_type == QubitPartition::Cuts::Horizontal) {
            data_per_cycles.xCZ_H.push_back(count);
            data_per_cycles.xCZ_V.push_back(0);
        }
        else {
            data_per_cycles.xCZ_H.push_back(count);
            data_per_cycles.xCZ_H.push_back(0);
        }
    }
    
    return -1;
}

idx_size TensorProductStateVector::
CountXCZGates(const bitset<128>* __restrict CZ_bitmasks)
{
    const int num_q_a = qp.getNumQubitsInBlock(0);
    bitset<128> xCZ_bitmask[num_q_a];
    for (int i = 0; i < num_q_a; ++i)
        xCZ_bitmask[i] = 0;
    
    FindCZGatesBetweenPartitions(xCZ_bitmask, CZ_bitmasks);
    
    idx_size new_count = 0;
    for (int i = 0; i < num_q_a; ++i) {
        while (xCZ_bitmask[i] != 0) {
            ++new_count;
            const idx_size first_half = xCZ_bitmask[i].to_ulong();
            const idx_size second_half = (xCZ_bitmask[i] >> 64).to_ulong();
            const int q1 = first_half ? __builtin_ctzl(first_half) : second_half ? 63 + __builtin_ctzl(second_half) : 0;
            xCZ_bitmask[i][q1] = 0;
        }
    }
    return new_count;
}

void TensorProductStateVector::
ApplyXCZGateApprox(const bitset<128>* __restrict CZ_bitmasks,
                   const Gate::Type CZ_D_A,
                   const Gate::Type CZ_D_B)
{
    Time time;
    time.StartTime();
    
    const int num_q_a = qp.getNumQubitsInBlock(0), num_q_b = qp.getNumQubitsInBlock(1);
    bitset<128> xCZ_bitmask[num_q_a];
    for (int i = 0; i < num_q_a; ++i)
        xCZ_bitmask[i] = 0;
    FindCZGatesBetweenPartitions(xCZ_bitmask, CZ_bitmasks);
    
    const int modified_num_q_B = num_q_a + num_q_b - 1;
    const idx_size prev_CZ_count = count_of_category.decomposed_CZ;
    for (int i = 0; i < num_q_a; ++i) {
        while (xCZ_bitmask[i] != 0) {
            const idx_size first_half = __builtin_ctzl(xCZ_bitmask[i].to_ulong());
            const idx_size second_half = __builtin_ctzl((xCZ_bitmask[i] >> 64).to_ulong());
            const int q = first_half ? __builtin_ctzl(first_half)
            : second_half ? 63 + __builtin_ctzl(second_half) : 0;
            if (sim_mode != Config::SimMode::Phase2)
                ++count_of_category.decomposed_CZ;
            ApplyCZGateAcrossTensorFactors(CZ_D_A, CZ_D_B, (int)i, modified_num_q_B - q);
            xCZ_bitmask[i][q] = 0;
        }
    }
    
    time_by_category.decomposed_CZ += time.GetElapsedTime();
    
    if (sim_mode != Config::SimMode::Phase2 &&
        (sim_type != Config::SimType::Approx2011OWT && sim_type != Config::SimType::Approx_i11iOWT)) {
        data_per_cycles.memory.push_back(GetMemUsage());
        data_per_cycles.addends.push_back(1);
        if (cut_type == QubitPartition::Cuts::Horizontal) {
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
    int block = qp.globalToBlock(gate_qubit);
    int num_q_1_partition = qp.getNumQubitsInBlock(block) - 1;

    const int projected_gate_q = num_q_1_partition - qp.globalToLocal(gate_qubit);
    if (qp.globalToBlock(gate_qubit) == 0)
        state_a -> ApplyNonCGate(projected_gate_q, gate_type, g);
    else
        state_b -> ApplyNonCGate(projected_gate_q, gate_type, g);
}

void TensorProductStateVector::
ApplyHGateOnAllAmps()
{
    if (partition_to_sim == 'a' || partition_to_sim == 'x')
        state_a -> ApplyHGateOnAllAmps();
    if (partition_to_sim == 'b' || partition_to_sim == 'x')
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
    
    const bitset<128> a_qubits_bitmask = qp.getBlockBitmask(0), b_qubits_bitmask = qp.getBlockBitmask(1);

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
                          int th)
{
    if (partition_to_sim == 'a' || partition_to_sim == 'x') {
        bitset<128> stateA_Xbitmask = Project1QBitmask(X_bitmask, qp, 0, true);
        bitset<128> stateA_Ybitmask = Project1QBitmask(Y_bitmask, qp, 0, true);
        state_a -> ApplyXYRecursiveTransform(stateA_Xbitmask, stateA_Ybitmask, th);
    }
    if (partition_to_sim == 'b' || partition_to_sim == 'x') {
        bitset<128> stateB_Xbitmask = Project1QBitmask(X_bitmask, qp, 1, true);
        bitset<128> stateB_Ybitmask = Project1QBitmask(Y_bitmask, qp, 1, true);
        state_b -> ApplyXYRecursiveTransform(stateB_Xbitmask, stateB_Ybitmask, th);
    }
}

cmplx TensorProductStateVector::
operator[](bitset<128> i) 
{    
//    if (cut_type == QubitPartition::Cuts::Horizontal) {
//        bitset<128> temp_i = i;
//        bitset<128> a = i >> qp.getNumQubitsInBlock(1);
//        bitset<128> b = temp_i & qp.getBlockBitmask(1);
//        return (*state_a)[a.to_ulong()] * (*state_b)[b.to_ulong()];
//    }
//    else {
        return (*state_a)[qp.IndexScatter(i, 0)] * (*state_b)[qp.IndexScatter(i, 1)];
//    }
}

cmplx TensorProductStateVector::
GetGlobalAmpAtInterestingIdx(idx_size i)
{
    return (*state_a)[global_to_local_a[i]] * (*state_b)[global_to_local_b[i]];
}


cmplx TensorProductStateVector::
GetAmpFromGlobalState(const idx_size a,
                      const idx_size b) const
{
    return (*state_a)[a] * (*state_b)[b];
}

double TensorProductStateVector::
GetMinProb()
{
    return state_a -> GetMinProb() * state_b -> GetMinProb();
}

double TensorProductStateVector::
GetMaxProb() 
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
    return 1ull << qp.getNumQubits();
}

int TensorProductStateVector::
GetStateANumQ() const
{
    return qp.getNumQubitsInBlock(0);
}

int TensorProductStateVector::
GetStateBNumQ() const
{
    return qp.getNumQubitsInBlock(1);
}

bitset<128> TensorProductStateVector::
GetStateABitmask() const
{
    return qp.getBlockBitmask(0);
}

bitset<128> TensorProductStateVector::
GetStateBBitmask() const
{
    return qp.getBlockBitmask(1);
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
    const idx_size a_size = 1ull << qp.getNumQubitsInBlock(0), b_size = 1ull << qp.getNumQubitsInBlock(1), range = sampling_factor;
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
    const idx_size a_size = 1ull << qp.getNumQubitsInBlock(0), b_size = 1ull << qp.getNumQubitsInBlock(1);
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
ResetAmpVector()
{
    if (partition_to_sim == 'a' || partition_to_sim == 'x')
        state_a -> ResetAmpVector();
    if (partition_to_sim == 'b' || partition_to_sim == 'x')
        state_b -> ResetAmpVector();
}

void TensorProductStateVector::
Normalize()
{
    if (partition_to_sim == 'a' || partition_to_sim == 'x')
        state_a -> Normalize();
    if (partition_to_sim == 'b' || partition_to_sim == 'x')
        state_b -> Normalize();
}

void TensorProductStateVector::
Rescale()
{
    if (partition_to_sim == 'a' || partition_to_sim == 'x')
        state_a -> Rescale();
    if (partition_to_sim == 'b' || partition_to_sim == 'x')
        state_b -> Rescale();
}

void TensorProductStateVector::
RescaleAndApplyGlobalICounter()
{
    if (partition_to_sim == 'a' || partition_to_sim == 'x')
        state_a -> RescaleAndApplyGlobalICounter();
    if (partition_to_sim == 'b' || partition_to_sim == 'x')
        state_b -> RescaleAndApplyGlobalICounter();
}

void TensorProductStateVector::
ApplyGlobalICounter()
{
    if (partition_to_sim == 'a' || partition_to_sim == 'x')
         state_a -> ApplyGlobalICounter();
    if (partition_to_sim == 'b' || partition_to_sim == 'x')
        state_b -> ApplyGlobalICounter();
}

idx_size TensorProductStateVector::
GetGlobalFactorPower() const
{
    if (partition_to_sim == 'a')
        return state_a -> GetGlobalFactorPower();
    else if (partition_to_sim == 'b')
        return state_b -> GetGlobalFactorPower();
    else
        return state_a -> GetGlobalFactorPower() > state_b -> GetGlobalFactorPower() ?
        state_a -> GetGlobalFactorPower() : state_b -> GetGlobalFactorPower();
}

const QubitPartition& TensorProductStateVector::
GetQp() const
{
    return qp;
}

double TensorProductStateVector::
CountZeroAmpPercentage() const
{
//    const idx_size a_size = 1ull << num_q_a, b_size = 1ull << num_q_b;
//    idx_size zero_count = 0;
//
//    for (idx_size i = 0; i < a_size; ++i) {
//        if ((*state_a)[i] == cmplx(0,0))
//            zero_count += b_size;
//        else {
//            for (idx_size j = 0; j < b_size; ++j) {
//                if ((*state_b)[j] == cmplx(0,0))
//                    ++zero_count;
//            }
//        }
//    }

    static int calls = 0;
    ++calls;
    
    if (calls % 2)
        return state_a -> CountZeroAmpPercentage();
    else return state_b -> CountZeroAmpPercentage();
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
