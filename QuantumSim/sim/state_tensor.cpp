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
                         const Config* config,
                         const int hcut,
                         const int vcut,
                         const Config::SimType sim,
                         const bool row_major,
                         const bool first_part_small,
                         const int verb):
qp(cut_type == QubitPartition::Cuts::Horizontal ? QubitPartition(cut_type, qubits, row_major, hcut, first_part_small)
         : QubitPartition(cut_type, qubits, row_major, vcut, first_part_small))
{
    sim_type = sim;
    static int count_h = 0, count_v = 0;
    this -> cut_type = cut_type;
    
//    qp.RenumberLocalQubits();

    int num_q_b0 = qp.getNumQubitsInBlock(0), num_q_b1 = qp.getNumQubitsInBlock(1);
    state_a = new FullAmpStateVector(num_q_b0, config);
    state_b = new FullAmpStateVector(num_q_b1, config);
        
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
TensorProductStateVector(const TensorProductStateVector& rhs) : qp(rhs.qp), cut_type(rhs.cut_type)
{
    sim_type = rhs.sim_type;
    compressed = rhs.compressed;
    
    state_a = new FullAmpStateVector(*(rhs.state_a));
    state_b = new FullAmpStateVector(*(rhs.state_b));
}

TensorProductStateVector::
~TensorProductStateVector()
{
    if (state_a != nullptr)
        delete state_a;
    if (state_b != nullptr)
        delete state_b;
}

void TensorProductStateVector::
PopulateGlobalToLocalMap(vector<bitset<128>>& idxs)
{
    if (global_to_local_a != nullptr && global_to_local_b != nullptr) {
        delete [] global_to_local_a;
        delete [] global_to_local_b;
        global_to_local_a = nullptr;
        global_to_local_b = nullptr;
    }
    
    num_requested_amps = idxs.size();
    
    vector<pair<idx_size, idx_size>> temp_global_to_local_a(num_requested_amps,
                                                            pair<idx_size, idx_size>(0,0));
    vector<pair<idx_size, idx_size>> temp_global_to_local_b(num_requested_amps,
                                                            pair<idx_size, idx_size>(0,0));
    
    global_to_local_a = new idx_size[num_requested_amps];
    global_to_local_b = new idx_size[num_requested_amps];
    memset(global_to_local_a, 0, num_requested_amps * sizeof(idx_size));
    memset(global_to_local_b, 0, num_requested_amps * sizeof(idx_size));

//    cout << endl;
//    for (idx_size i = 0; i < num_requested_amps; ++i) {
//        cout << idxs[i].to_ullong() << ": " << qp.IndexScatter(idxs[i], 0) << ", ";
//        cout << qp.IndexScatter(idxs[i], 1) << endl;
//    }
    
    for (idx_size i = 0; i < num_requested_amps; ++i)
        temp_global_to_local_a[i] = pair<idx_size, idx_size>(i, qp.IndexScatter(idxs[i], 0));
    
    sort(temp_global_to_local_a.begin() + 5, temp_global_to_local_a.end(),
         [](pair<idx_size, idx_size>& first, pair<idx_size, idx_size>& second) {
             return first.second < second.second;
         });
    
    for (idx_size i = 0; i < num_requested_amps; ++i)
        temp_global_to_local_b[i] = pair<idx_size, idx_size>(i,
                                                qp.IndexScatter(idxs[temp_global_to_local_a[i].first], 1));
    
    idx_size last_idx_a = 5;
//    for (idx_size i = 5; i < num_requested_amps; ++i) {
//        if (temp_global_to_local_a[i].second != temp_global_to_local_a[last_idx_a].second) {
//            if (i - last_idx_a > 1)
//                sort(temp_global_to_local_b.begin() + last_idx_a, temp_global_to_local_b.begin() + i,
//                     [](pair<idx_size, idx_size>& first, pair<idx_size, idx_size>& second) {
//                         return first.second < second.second;
//                     });
//            last_idx_a = i;
//        }
//    }
//
//    if (num_requested_amps - last_idx_a > 1)
//        sort(temp_global_to_local_b.begin() + last_idx_a, temp_global_to_local_b.begin() + num_requested_amps,
//             [](pair<idx_size, idx_size>& first, pair<idx_size, idx_size>& second) {
//                 return first.second < second.second;
//             });
    
    for (idx_size i = 5; i < num_requested_amps; ++i) {
        if (temp_global_to_local_a[i].second >= (temp_global_to_local_a[last_idx_a].second + (1ull << 12))) {
            if (i - last_idx_a > 1)
                sort(temp_global_to_local_b.begin() + last_idx_a, temp_global_to_local_b.begin() + i,
                     [](pair<idx_size, idx_size>& first, pair<idx_size, idx_size>& second) {
                         return first.second < second.second;
                     });
            last_idx_a = i;
        }
    }
    
    if (num_requested_amps - last_idx_a > 1)
        sort(temp_global_to_local_b.begin() + last_idx_a, temp_global_to_local_b.begin() + num_requested_amps,
             [](pair<idx_size, idx_size>& first, pair<idx_size, idx_size>& second) {
                 return first.second < second.second;
             });
    
    vector<bitset<128>> temp_global_idxs (num_requested_amps, 0);
    for (idx_size i = 0; i < num_requested_amps; ++i)
        temp_global_idxs[i] = idxs[temp_global_to_local_a[temp_global_to_local_b[i].first].first];
    
    for (idx_size i = 0; i < num_requested_amps; ++i) {
        idxs[i] = temp_global_idxs[i];
        global_to_local_a[i] = temp_global_to_local_a[temp_global_to_local_b[i].first].second;
        global_to_local_b[i] = temp_global_to_local_b[i].second;
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
    static const int qubits_a = state_a -> GetNumQubits(), qubits_a_1 = qubits_a - 1,
    num_q_1 = qp.getNumQubits() - 1;
    static const bitset<128> block0_bitmask = qp.getBlockBitmask(0);
    static const bitset<128> block1_bitmask = qp.getBlockBitmask(1);
    int count = 0;
    
    // CZ bitmasks array is numbered in the opposite direction
    for (int i = 0; i <  num_q_1 + 1; ++i) {
        if (qp.globalToBlock(num_q_1 - i) == 0) {
//            cout << "bm 1: " << gate_bitmasks[num_q_1 - i].to_string().substr(128-15) << endl;
//            cout << "bm 0: " << gate_bitmasks[num_q_1 - i].to_string().substr(128-30, 15) << endl;
//            cout << "g i 0 :" << num_q_1 - i << endl;
//            cout << "gtl 0 : "<< qp.globalToLocal(num_q_1 - i) << endl;
            
             if ((gate_bitmasks[num_q_1 - i] & block0_bitmask) != gate_bitmasks[num_q_1 - i]) {
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

void TensorProductStateVector::
HandleCZApprox(const bitset<128> *CZ_bitmasks)
{
    if (sim_type == Config::SimType::Approx2011 || sim_type == Config::SimType::Approx2011OWT)
        ApplyXCZGateApprox(CZ_bitmasks, Gate::Type::cz_d5, Gate::Type::cz_d3);
    else if (sim_type == Config::SimType::Approx1_101) //compute norm and divide by the norm
        ApplyXCZGateApprox(CZ_bitmasks, Gate::Type::cz_d1, Gate::Type::cz_d2);
    else if (sim_type == Config::SimType::Approx1110)
        ApplyXCZGateApprox(CZ_bitmasks, Gate::Type::cz_d3, Gate::Type::cz_d4);
    else if (sim_type == Config::SimType::Approx_i11i || sim_type == Config::SimType::Approx_i11iOWT)
        ApplyXCZGateApprox(CZ_bitmasks, Gate::Type::cz_d6, Gate::Type::cz_d7);
    else if ((sim_type == Config::SimType::Approx1CutH || (sim_type == Config::SimType::Approx1CutV))
             && book_keep){
//        idx_size count = CountXCZGates(CZ_bitmasks);
//        data_per_cycles.memory.push_back(GetMemUsage());
//        data_per_cycles.addends.push_back(1);
//        if (cut_type == QubitPartition::Cuts::Horizontal) {
//            data_per_cycles.xCZ_H.push_back(count);
//            data_per_cycles.xCZ_V.push_back(0);
//        }
//        else {
//            data_per_cycles.xCZ_H.push_back(count);
//            data_per_cycles.xCZ_H.push_back(0);
//        }
    }
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
            const idx_size first_half = ((xCZ_bitmask[i] << 64) >> 64).to_ulong();
            const idx_size second_half = (xCZ_bitmask[i] >> 64).to_ulong();
            const int q1 = first_half ? __builtin_ctzl(first_half) : second_half ? 64 + __builtin_ctzl(second_half) : 0;
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
//    const idx_size prev_CZ_count = count_of_category.decomposed_CZ;
    for (int i = 0; i < num_q_a; ++i) {
        while (xCZ_bitmask[i] != 0) {
            const idx_size first_half = __builtin_ctzl(((xCZ_bitmask[i] << 64) >> 64).to_ulong());
            const idx_size second_half = __builtin_ctzl((xCZ_bitmask[i] >> 64).to_ulong());
            const int q = first_half ? __builtin_ctzl(first_half)
            : second_half ? 64 + __builtin_ctzl(second_half) : 0;
            if (book_keep)
                ++count_of_category.decomposed_CZ;
            ApplyCZGateAcrossTensorFactors(CZ_D_A, CZ_D_B, (int)i, modified_num_q_B - q);
            xCZ_bitmask[i][q] = 0;
        }
    }
    
    time_by_category.decomposed_CZ += time.GetElapsedTime();
    
//    if (book_keep &&
//        (sim_type != Config::SimType::Approx2011OWT && sim_type != Config::SimType::Approx_i11iOWT)) {
//        data_per_cycles.memory.push_back(GetMemUsage());
//        data_per_cycles.addends.push_back(1);
//        if (cut_type == QubitPartition::Cuts::Horizontal) {
//            data_per_cycles.xCZ_H.push_back(count_of_category.decomposed_CZ - prev_CZ_count);
//            data_per_cycles.xCZ_V.push_back(0);
//        }
//        else {
//            data_per_cycles.xCZ_V.push_back(count_of_category.decomposed_CZ - prev_CZ_count);
//            data_per_cycles.xCZ_H.push_back(0);
//        }
//    }
}

void TensorProductStateVector::
ApplyNonCGate(const idx_size gate_qubit,
              const Gate::Type gate_type)
{
    int block = qp.globalToBlock(gate_qubit);
    int num_q_1_partition = qp.getNumQubitsInBlock(block) - 1;

    const int projected_gate_q = num_q_1_partition - qp.globalToLocal(gate_qubit);
    if (qp.globalToBlock(gate_qubit) == 0)
        state_a -> ApplyNonCGate(projected_gate_q, gate_type);
    else
        state_b -> ApplyNonCGate(projected_gate_q, gate_type);
}

void TensorProductStateVector::
ApplyHGateOnAllAmps(bool cycle_0)
{
    if (partition_to_sim == 'a' || partition_to_sim == 'x')
        state_a -> ApplyHGateOnAllAmps(cycle_0);
    if (partition_to_sim == 'b' || partition_to_sim == 'x')
        state_b -> ApplyHGateOnAllAmps(cycle_0);
}

//TODO
void TensorProductStateVector::
ApplyCGate(const idx_size num_controls,
           const vector<idx_size>& gate_qubits,
           const Gate& g,
           const Gate::Type gate_type)
{
    
}

void TensorProductStateVector::
ApplyMergedXYGate(const Gate& gate1,
                  const Gate& gate2)
{
    bitset<128> temp1 = 0, temp2 = 0;
    temp1[gate1.GetQubits()[0]] = 1;
    temp2[gate2.GetQubits()[0]] = 1;
    
    const bitset<128> a_qubits_bitmask = qp.getBlockBitmask(0), b_qubits_bitmask = qp.getBlockBitmask(1);

    if ((temp1 & a_qubits_bitmask) != 0 && (temp2 & a_qubits_bitmask) != 0)
        state_a -> ApplyMergedXYGate(gate1, gate2);
    else if ((temp1 & b_qubits_bitmask) != 0 && (temp2 & b_qubits_bitmask) != 0)
        state_b -> ApplyMergedXYGate(gate1, gate2);
    else {
        if ((temp1 & a_qubits_bitmask) != 0)
            state_a -> ApplyNonCGate(gate1.GetQubits()[0], gate1.GetType());
        else
            state_b -> ApplyNonCGate(gate1.GetQubits()[0], gate1.GetType());

        if ((temp2 & b_qubits_bitmask) != 0)
            state_a -> ApplyNonCGate(gate2.GetQubits()[0], gate2.GetType());
        else
            state_b -> ApplyNonCGate(gate2.GetQubits()[0], gate2.GetType());
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

int TensorProductStateVector::
ApplyLoXYHAndCZTInSamePass(int& remaining_cz_bits,
                           idx_size& cz_path,
                           const idx_size cz_path_len,
                           const idx_size suffix_size,
                           const bitset<128>& X_bitmask,
                           const bitset<128>& Y_bitmask,
                           const bitset<128>& H_bitmask,
                           const bitset<128>* __restrict CZ_bitmasks,
                           const bitset<128> T_bitmasks[2],
                           int th)
{
    const int num_q_a = state_a -> GetNumQubits(), num_q_b = state_b -> GetNumQubits();

    if (partition_to_sim == 'a' || partition_to_sim == 'x') {
        bitset<128> stateA_Xbitmask = Project1QBitmask(X_bitmask, qp, 0, true);
        bitset<128> stateA_Ybitmask = Project1QBitmask(Y_bitmask, qp, 0, true);
        bitset<128> stateA_Hbitmask = Project1QBitmask(H_bitmask, qp, 0, true);
        
        bitset<128> CZ_bitmasks_a[num_q_a + 1];
        bitset<128> T_bitmasks_a[2] = {0};
        
        for (int i = 0; i <= num_q_a; ++i)
            CZ_bitmasks_a[i] = 0;
        
        CZ_bitmasks_a[num_q_a] = ProjectCZBitmask(CZ_bitmasks_a, qp, 0, CZ_bitmasks) ? 1 : 0;
        for (int i = 0; i < 2; ++i)
            T_bitmasks_a[i] = Project1QBitmask(T_bitmasks[i], qp, 0);
        
        state_a -> ApplyLoXYHAndCZTInSamePass(remaining_cz_bits, cz_path,
                                              cz_path_len, suffix_size,
                                              stateA_Xbitmask, stateA_Ybitmask,
                                              stateA_Hbitmask, CZ_bitmasks_a,
                                              T_bitmasks_a, th);
    }
    if (partition_to_sim == 'b' || partition_to_sim == 'x') {
        bitset<128> stateB_Xbitmask = Project1QBitmask(X_bitmask, qp, 1, true);
        bitset<128> stateB_Ybitmask = Project1QBitmask(Y_bitmask, qp, 1, true);
        bitset<128> stateB_Hbitmask = Project1QBitmask(H_bitmask, qp, 1, true);
        
        bitset<128> CZ_bitmasks_b[num_q_b + 1];
        bitset<128> T_bitmasks_b[2] = {0};
        
        for (int i = 0; i <= num_q_b; ++i)
            CZ_bitmasks_b[i] = 0;
        
        CZ_bitmasks_b[num_q_b] = ProjectCZBitmask(CZ_bitmasks_b, qp, 1, CZ_bitmasks) ? 1 : 0;
        for (int i = 0; i < 2; ++i)
            T_bitmasks_b[i] = Project1QBitmask(T_bitmasks[i], qp, 1);
        
        state_b -> ApplyLoXYHAndCZTInSamePass(remaining_cz_bits, cz_path,
                                              cz_path_len, suffix_size,
                                              stateB_Xbitmask, stateB_Ybitmask,
                                              stateB_Hbitmask, CZ_bitmasks_b,
                                              T_bitmasks_b, th);
    }
    
    return -1;
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
GetNumQInBlock(idx_size block) const
{
    if (block == 0)
        return state_a -> GetNumQubits();
    else if (block == 1)
        return  state_b -> GetNumQubits();
    else return NAN;
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
    const idx_size a_size = 1ull << qp.getNumQubitsInBlock(0), b_size = 1ull << qp.getNumQubitsInBlock(1), range = SAMPLING_FACTOR;
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

idx_size TensorProductStateVector::
CountZerosInBlock(int block) const
{
    if (block == 0)
        return state_a -> CountZerosInBlock(0);
    else if (block == 1)
        return state_b -> CountZerosInBlock(1);
    else {
        cerr << "Invalid block number for counting zeros.";
        exit(1);
    }
    return 0;
}

bool TensorProductStateVector::
AreAllAmpsZero() const
{
    return state_a -> AreAllAmpsZero() || state_b  -> AreAllAmpsZero();
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
        
        off = 1 + rand() % SAMPLING_FACTOR;
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
        
        off = 1 + rand() % SAMPLING_FACTOR;
    }
}

void TensorProductStateVector::
WriteAmpToDisk(const string& filename)
{
    if (partition_to_sim == 'a' || partition_to_sim == 'x')
       state_a -> WriteAmpToDisk(filename + "_A");
    if (partition_to_sim == 'b' || partition_to_sim == 'x')
        state_b -> WriteAmpToDisk(filename + "_B");
}

void TensorProductStateVector::
ReadFromDisk(const string& filename)
{
    if (partition_to_sim == 'a' || partition_to_sim == 'x')
        state_a -> ReadFromDisk(filename + "_A");
    if (partition_to_sim == 'b' || partition_to_sim == 'x')
        state_b -> ReadFromDisk(filename + "_B");
}

void TensorProductStateVector::
CopyState(const GenericQuantumState& rhs)
{
    const TensorProductStateVector& t_rhs = (const TensorProductStateVector&)rhs;
    qp = t_rhs.qp;
    cut_type = t_rhs.cut_type;
    sim_type = t_rhs.sim_type;
  
    state_a -> CopyState(*t_rhs.state_a);
    state_b -> CopyState(*t_rhs.state_b);
}

void TensorProductStateVector::
CopyMemberVars(const GenericQuantumState& rhs)
{
    const TensorProductStateVector& t_rhs = (const TensorProductStateVector&)rhs;
    qp = t_rhs.qp;
    cut_type = t_rhs.cut_type;
    compressed = t_rhs.compressed;
    sim_type = t_rhs.sim_type;
    
    state_a -> CopyMemberVars(*t_rhs.state_a);
    state_b -> CopyMemberVars(*t_rhs.state_b);
}

void TensorProductStateVector::
CompressStateVector(idx_size num_codewords,
                    double p_rejection)
{
    if (book_keep)
        compressed_vector_ptrs.push_back({nullptr, nullptr});
        
    partition_to_sim = 'a';
    state_a -> CompressStateVector(num_codewords, p_rejection);
    partition_to_sim = 'b';
    state_b -> CompressStateVector(num_codewords, p_rejection);
    partition_to_sim = 'x';
    
    compressed = true;
}

void TensorProductStateVector::
DecompressStateVector()
{
    state_a -> DecompressStateVector();
    state_b -> DecompressStateVector();
    
    compressed = false;
}

void TensorProductStateVector::
DecompressAndCopyAnotherState(const GenericQuantumState& rhs)
{
    const TensorProductStateVector& t_rhs = (const TensorProductStateVector&)rhs;
    qp = t_rhs.qp;
    cut_type = t_rhs.cut_type;
    sim_type = t_rhs.sim_type;
    
    state_a -> DecompressAndCopyAnotherState(*t_rhs.state_a);
    state_b -> DecompressAndCopyAnotherState(*t_rhs.state_b);
    
    compressed = false;
}
