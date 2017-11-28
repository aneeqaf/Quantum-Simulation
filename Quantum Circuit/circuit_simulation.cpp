//
//  circuit_simulation.cpp
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 10/21/17.
//  Copyright © 2017 Aneeqa Fatima. All rights reserved.
//

#include <stdio.h>

#include "circuit_simulation.h"

using namespace std;

// 0:H, 1:CZ & T, 2:X, 3:Y, 4:Merged X & Y
vector<double> gate_time(5,0);
clock_t g_begin;
clock_t g_end;
bool google = false;
float H_related_gates = 0;

function<void(int)> timing_and_probability;
function<void(int)> increment_cycle;

circuit::circuit_simulation::
circuit_simulation(circuit& c, int gate_i): cir(c), gate_i(gate_i), google_cycle(0) {}

void circuit::circuit_simulation::
diagonalize_XY()
{
    enum to_CNOT : short {red, yellow, green};
    int start_clock_cycle = 0;
    gate H = create_hadamard();
    
    vector<vector<int>> qubit_HCZH(cir.qubits);
    vector<int> gates_to_remove;
    vector<int> r_qubits;
    vector<int> l_qubits;
    
    auto check_green = [&](int i) {
        if(qubit_HCZH[cir.gates[i].qubits.back()].size() == green) {
            cir.gates[qubit_HCZH[cir.gates[i].qubits.back()].back()].gate_identification.back()
                                                                = gate::Gates::X;
            cir.gates[qubit_HCZH[cir.gates[i].qubits.back()].back()].rows = create_X().rows;
            gates_to_remove.push_back(qubit_HCZH[cir.gates[i].qubits.back()].front());
        }
        qubit_HCZH[cir.gates[i].qubits.back()].clear();
    };
    
    int i = 0;
    for (auto& c : cir.clock_cycles) {
        if (i < cir.qubits) {
            i += c;
            continue;
        }
        c += r_qubits.size() + l_qubits.size();
        r_qubits.clear();
        l_qubits.clear();
        for (; i < c; ++i) {
            
                if (cir.gates[i].gate_identification.back() == gate::Gates::X_rotation) {
                    check_green(i);
                    cir.gates[i].gate_identification.back() = gate::Gates::D_X12;
                    l_qubits.push_back(cir.gates[i].qubits.back());
                    r_qubits.push_back(cir.gates[i].qubits.back());
                }
                else if (cir.gates[i].gate_identification.back() == gate::Gates::Y_rotation) {
                    check_green(i);
                    cir.gates[i].gate_identification.back() = gate::Gates::D_Y12;
                    r_qubits.push_back(cir.gates[i].qubits.back());
                }
                else if (cir.gates[i].gate_identification.back() == gate::Gates::Phase) {
                    if(qubit_HCZH[cir.gates[i].qubits.back()].size() == yellow) {
                        qubit_HCZH[cir.gates[i].qubits.back()].push_back(i);
                    }
                }
        }
        
        for (int j = 0; j < l_qubits.size(); ++j) {
            H.qubits.push_back(l_qubits[j]);
            cir.gates.insert(cir.gates.begin() + start_clock_cycle + j, H);
            qubit_HCZH[l_qubits[j]].push_back(start_clock_cycle + j);
            H.qubits.clear();
            ++c;
        }
        for (int j = 0; j < r_qubits.size(); ++j) {
            H.qubits.push_back(r_qubits[j]);
            cir.gates.insert(cir.gates.begin() + c, H);
            H.qubits.clear();
            ++c;
        }
        
        i += r_qubits.size() + l_qubits.size();
        start_clock_cycle = c;
    }
    for (auto g : gates_to_remove) {
        cir.gates.erase(cir.gates.begin() + g);
    }
}

template<typename function>
vector<index_size> circuit::circuit_simulation::
initialize_gate_set(int& num_X_T_gates, function comp)
{
    bool saw_xy = false;
    
    auto check_xy = [&](int g_i) {
        if ((g_i < cir.gates.size()) &&
            (cir.gates[g_i].gate_identification[0] == gate::Gates::X_rotation ||
            cir.gates[g_i].gate_identification[0] == gate::Gates::Y_rotation) ) {
            saw_xy = true;
        }
    };
    
    vector<index_size> bit_mask {};
    int g_i = gate_i;
    for(;(g_i < cir.gates.size()) && (comp(g_i)); ++g_i) {
        
        index_size bits = 0;
    
        if (cir.gates[g_i].gate_identification.back() == gate::Gates::X) {
            for (int j = 0 ; j < cir.gates[g_i].num_controls; ++j) {
                bits += (1 << abs(cir.gates[g_i].qubits[j] - abs(cir.qubits - 1)));
            }
            ++num_X_T_gates;
        }
        else {
            for (int j = 0 ; j < cir.gates[g_i].qubits.size(); ++j) {
                bits += ( 1 << abs(cir.gates[g_i].qubits[j] - abs(cir.qubits - 1)));
            }
            if (cir.gates[g_i].gate_identification.back() == gate::Gates::T) {
                ++num_X_T_gates;
            }
        }
        
        bit_mask.push_back(bits);

        if (!saw_xy) {
            check_xy(g_i + 1);
        }
    }
    
    return bit_mask;
}

inline void circuit::circuit_simulation::
rand_sim(vector<bool>& iterated, int g_i, int num_bits, index_size index)
{
    index_size c = 1 << num_bits;
    cir.circuit_state -> apply_gate.resize(1 << num_bits);
    cir.circuit_state -> apply_gate[0] = cir.circuit_state -> state_vector[index];
    cir.circuit_state -> indices_for_ag.push_back(index);
    
    index_size temp_index = 0;
    int c1 = 1;
    int target = cir.gates[g_i].num_controls;
    
    for (int i = target ; i < num_bits; ++i) {
        short relative_pos = 1;
        short j = 0;
        for (int n = 0; n < cir.circuit_state -> indices_for_ag.size() &&
             cir.circuit_state -> indices_for_ag.size() < (1 << (i+1)); n += 2) {
            
            temp_index = cir.circuit_state -> indices_for_ag[n] +
                    (1 << abs(cir.gates[g_i].qubits[i] - abs(cir.qubits - 1)));
            
            assert(temp_index < cir.circuit_state -> state_vector.size());
            iterated[temp_index] = true;
            
            cir.circuit_state -> indices_for_ag.insert(
                               cir.circuit_state -> indices_for_ag.begin()
                                + j + relative_pos, temp_index);
            cir.circuit_state -> apply_gate[c/(1 << c1) * (j + relative_pos)] =
                    cir.circuit_state -> state_vector[temp_index];
            ++relative_pos;
            ++j;
        }
        ++c1;
    }
}

void circuit::circuit_simulation::
block_gate_opt(vector<index_size>& cbits, int XT_gates)
{
    auto find_smallest_non_control = [&](int j) {
        for(int cq = 0; cq < cir.gates[gate_i + j].num_controls; ++cq) {
            if (cq != cir.gates[gate_i + j].qubits[cq]) {
                return cq;
            }
        }
        return -1;
    };
    vector<vector<bool>> iterated;
    if (XT_gates > 0) {
            iterated.resize(cir.circuit_state -> state_vector.size(),
                            vector<bool>(XT_gates,false)) ;
    }
    
    index_size size =  cir.circuit_state -> state_vector.size();
    
    for (index_size index = 0; index < size ; ++index) {
        int c_size = (int)cbits.size();
        int iter_count = 0;
        if (google_cycle % 5 == 0) {
            cir.circuit_state -> state_vector[index] *=
                        cmplx(pow(HADAMARD_CONST, H_related_gates));
        }
        
        for (int j = 0; j < c_size; ++j){
            
            if(cir.gates[gate_i + j].gate_identification.back() == gate::Gates::T) {
                if (iterated[index][iter_count] == false) {
                    iterated[index][iter_count] = true;
                    
                    index_size temp_index = index +
                    (1 << abs(cir.gates[gate_i + j].qubits[0] - abs(cir.qubits - 1)));
                    
                    iterated[temp_index][iter_count] = true;
                    
                    if(cir.circuit_state->state_vector[temp_index] != cmplx(0,0)) {
                        T_opt(temp_index);
                    }
                }
                ++iter_count;
            }
            else if((index & cbits[j]) == cbits[j]) {
                if (cir.gates[gate_i + j].gate_identification.back() == gate::Gates::X) {
                    if (iterated[index][iter_count] == false) {
                        iterated[index][iter_count] = true;
                        
                        int smallest_non_control_bit = find_smallest_non_control(j);
                        if (smallest_non_control_bit == -1) {
                            continue;
                        }
                        
                        index_size swap_index = index +
                            (1 << abs(smallest_non_control_bit - abs(cir.qubits - 1)));
                        iterated[swap_index][iter_count] = true;
                        
                        if (!(cir.circuit_state -> state_vector[index] == cmplx(0,0) &&
                              cir.circuit_state -> state_vector[swap_index] == cmplx(0,0)))
                        {
                            CX_opt(index, swap_index);
                        }
                    }
                     ++iter_count;
                }
                else if (cir.circuit_state -> state_vector[index] != cmplx(0,0)) {
                    if (cir.gates[gate_i + j].gate_identification.back()
                                                == gate::Gates::Phase) {
                        Phase_opt(index);
                    }
                    else if (cir.gates[gate_i + j].gate_identification.back()
                             == gate::Gates::Z) {
                        CZ_opt(index);
                    }
                }
            }
        }
    }
    if (google_cycle % 5 == 0) {
         H_related_gates = 0;
    }
}

inline void circuit::circuit_simulation::
control_rand_sim(index_size c_bits)
{
    vector<bool> iterated(cir.circuit_state -> state_vector.size(), false);
    int num_bits = (int)cir.gates[gate_i].qubits.size();
    index_size iter_count = 0;
    index_size size = cir.circuit_state -> state_vector.size();
    int num_controls = cir.gates[gate_i].num_controls;
    
    for (index_size index = 0; index < size &&
         iter_count < (1 << abs(cir.qubits - num_controls)); ++index) {
        if (iterated[index] == false) {
            iterated[index] = true;

            if((index & c_bits) == c_bits) {
                iter_count += 2;
                rand_sim(iterated, gate_i, num_bits, index);
               (*cir.circuit_state) * (cir.gates[gate_i]);
            }
        }
    }
}

inline void circuit::circuit_simulation::
non_control_sim()
{
    int num_bits = (int)cir.gates[gate_i].qubits.size();
    vector<bool> iterated(cir.circuit_state -> state_vector.size(), false);
    index_size iter_count = 0;
    index_size size = cir.circuit_state -> state_vector.size();
    
    for (index_size index = 0; index < size && iter_count < size/(1 << num_bits); ++index) {
        if (iterated[index] == false) {
            iterated[index] = true;
            ++iter_count;
            
            rand_sim(iterated, gate_i, num_bits, index);
            (*cir.circuit_state) * (cir.gates[gate_i]);
        }
    }
    if(google) {
        ++H_related_gates;
    }
}

inline void circuit::circuit_simulation::
CX_opt(index_size index, index_size swap_index)
{
    if(!(cir.circuit_state -> state_vector[index] == cmplx(0,0)
             && cir.circuit_state -> state_vector[index] == cmplx(0,0))) {
            swap(cir.circuit_state -> state_vector[index],
                 cir.circuit_state -> state_vector[swap_index]);
    }
}

inline void circuit::circuit_simulation::
CZ_opt(index_size index)
{
    cir.circuit_state -> state_vector[index] =
                        -cir.circuit_state -> state_vector[index];
}

inline void circuit::circuit_simulation::
Phase_opt(index_size index)
{
   cir.circuit_state -> state_vector[index] =
                    cmplx(-imag(cir.circuit_state -> state_vector[index]),
                    real(cir.circuit_state -> state_vector[index]));
}

inline void circuit::circuit_simulation::
T_opt(index_size temp_index)
{
    cir.circuit_state->state_vector[temp_index] *= HADAMARD_CONST;
    cir.circuit_state->state_vector[temp_index] =
    cmplx(real(cir.circuit_state->state_vector[temp_index]) +
          (-imag(cir.circuit_state->state_vector[temp_index])),
          real(cir.circuit_state->state_vector[temp_index]) +
          imag(cir.circuit_state->state_vector[temp_index]));
}

inline int circuit::circuit_simulation::
H_google_opt()
{
    int j = 0;
    for(; j < cir.gates.size() &&
        cir.gates[gate_i + j].gate_identification.back() == gate::Gates::Hadamard; ++j ) {
        ++H_related_gates;
    }

    index_size size = cir.circuit_state -> state_vector.size();
    for (index_size index = 0; index < size; ++index) {
        cir.circuit_state -> state_vector[index] = 1;
    }

    return j;
}

inline void circuit::circuit_simulation::
XX_opt()
{
    auto& ap_gate = cir.circuit_state -> state_vector;
    auto& indices = cir.circuit_state -> indices_for_ag;

    auto t = ap_gate[indices[0]] - ap_gate[indices[3]];
    auto t1 = - cmplx(-imag(ap_gate[indices[1]]), real(ap_gate[indices[1]]))
            - cmplx(-imag(ap_gate[indices[2]]), real(ap_gate[indices[2]]));
    auto t2 = ap_gate[indices[1]] - ap_gate[indices[2]];
    auto t3 = - cmplx(-imag(ap_gate[indices[0]]), real(ap_gate[indices[0]]))
            - cmplx(-imag(ap_gate[indices[3]]), real(ap_gate[indices[3]]));
    
    ap_gate[indices[0]] = t + t1;
    ap_gate[indices[1]] = t3 + t2;
    ap_gate[indices[2]] = t3 - t2;
    ap_gate[indices[3]] = -t + t1;
}

inline void circuit::circuit_simulation::
XY_opt()
{
    auto& ap_gate = cir.circuit_state -> apply_gate;
    auto& temp_v = cir.circuit_state -> state_vector;
    auto& indices = cir.circuit_state -> indices_for_ag;
    
    temp_v[indices[0]] = ap_gate[0] - ap_gate[1] - cmplx(-imag(ap_gate[2]), real(ap_gate[2]))
                + cmplx(-imag(ap_gate[3]), real(ap_gate[3]));
    
    temp_v[indices[1]] = ap_gate[0] + ap_gate[1] - cmplx(-imag(ap_gate[2]), real(ap_gate[2]))
                - cmplx(-imag(ap_gate[3]), real(ap_gate[3]));
    
    temp_v[indices[2]] = - cmplx(-imag(ap_gate[0]), real(ap_gate[0]))
                + cmplx(-imag(ap_gate[1]), real(ap_gate[1])) + ap_gate[2] - ap_gate[3];
    
    temp_v[indices[3]] = - cmplx(-imag(ap_gate[0]), real(ap_gate[0]))
                - cmplx(-imag(ap_gate[1]), real(ap_gate[1])) + ap_gate[2] + ap_gate[3];
}

inline void circuit::circuit_simulation::
YY_opt()
{
    auto& temp_v = cir.circuit_state -> state_vector;
    auto& indices = cir.circuit_state -> indices_for_ag;
    
    auto t = temp_v[indices[0]] + temp_v[indices[1]];
    auto t1 = temp_v[indices[0]] - temp_v[indices[1]];
    auto t2 = temp_v[indices[2]] + temp_v[indices[3]];
    auto t3 = temp_v[indices[2]] - temp_v[indices[3]];
    
    temp_v[indices[0]] = t1 - t3;
    temp_v[indices[1]] = t - t2;
    temp_v[indices[2]] = t1 + t3;
    temp_v[indices[3]] = t + t2;
}

inline void circuit::circuit_simulation::
YX_opt()
{
    auto& temp_v = cir.circuit_state -> state_vector;
    auto& indices = cir.circuit_state -> indices_for_ag;
    
    auto t = temp_v[indices[0]]
             - cmplx(-imag(temp_v[indices[1]]), real(temp_v[indices[1]]));
    auto t1 = -cmplx(-imag(temp_v[indices[0]]), real(temp_v[indices[0]]))
                + temp_v[indices[1]];
    auto t2 = temp_v[indices[2]] -
            cmplx(-imag(temp_v[indices[3]]), real(temp_v[indices[3]]));
    auto t3 = temp_v[indices[3]]
                - cmplx(-imag(temp_v[indices[2]]), real(temp_v[indices[2]]));
    
    temp_v[indices[0]] = t - t2;
    temp_v[indices[1]] = t1 - t3;
    temp_v[indices[2]] = t + t2;
    temp_v[indices[3]] = t1 + t3;
}

void circuit::circuit_simulation::
XY_merge_opt()
{
    int a, b;
    if (cir.gates[gate_i].qubits.back() < cir.gates[gate_i + 1].qubits.back()) {
        cir.gates[gate_i].qubits.push_back(cir.gates[gate_i + 1].qubits.back());
        a = gate_i;
        b = gate_i + 1;
    }
    else {
        cir.gates[gate_i + 1].qubits.push_back(cir.gates[gate_i].qubits.back());
        a = gate_i + 1;
        b = gate_i++;
    }
    
    if(cir.gates[a].gate_identification[0] == gate::Gates::X_rotation &&
       cir.gates[b].gate_identification[0] == gate::Gates::X_rotation) {
        XY_merge_sim(0);
    }
    else if(cir.gates[a].gate_identification[0] == gate::Gates::X_rotation &&
       cir.gates[b].gate_identification[0] == gate::Gates::Y_rotation) {
        XY_merge_sim(1);
    }
    else if(cir.gates[a].gate_identification[0] == gate::Gates::Y_rotation &&
       cir.gates[b].gate_identification[0] == gate::Gates::X_rotation) {
        XY_merge_sim(2);
    }
    else if(cir.gates[a].gate_identification[0] == gate::Gates::Y_rotation &&
       cir.gates[b].gate_identification[0] == gate::Gates::Y_rotation) {
        XY_merge_sim(3);
    }
}

void circuit::circuit_simulation::
XY_merge_sim(short type)
{
    int smallest_non_control_bit = 0;
    for(int cq = 0; cq < cir.gates[gate_i].num_controls; ++cq) {
        if (cq != cir.gates[gate_i].qubits[cq]) {
            smallest_non_control_bit = cq;
        }
    }

    int num_bits = (int)cir.gates[gate_i].qubits.size();
    vector<bool> iterated(cir.circuit_state -> state_vector.size(), false);
    index_size iter_count = 0;
    index_size size = cir.circuit_state -> state_vector.size();
    
    for (index_size index = 0; index < size && iter_count < size/(1 << num_bits); ++index) {
        if (iterated[index] == false) {
            iterated[index] = true;
            ++iter_count;
            
            rand_sim(iterated, gate_i, num_bits, index);
            
//#ifdef DO_PREFETCH
//            int next_index = index + pow(2, smallest_non_control_bit);
//            __builtin_prefetch(&cir.circuit_state -> state_vector[next_index],1,0);
//            __builtin_prefetch(&cir.circuit_state -> state_vector[next_index],1,0);
//            __builtin_prefetch(&cir.circuit_state -> state_vector[next_index],1,0);
//            __builtin_prefetch(&cir.circuit_state -> state_vector[next_index],1,0);
//#endif
            
            switch (type) {
                case 0: {
                    XX_opt();
                    break;
                }
                case 1: {
                    XY_opt();
                    break;
                }
                case 2: {
                    YX_opt();
                    break;
                }
                case 3: {
                    YY_opt();
                    break;
                }
                default: {
                    throw "Invalid XY merge\n";
                    break;
                }
            }
            
            cir.circuit_state -> apply_gate.clear();
            cir.circuit_state -> indices_for_ag.clear();
        }
    }
    H_related_gates += 2;
}

void circuit::circuit_simulation::
CZ_T_preprocess()
{
    bool saw_xy = false;
    int g_i = 0;
    cir.num_cycles = cir.clock_cycles.size();
    
    for (int i = 2; i < cir.clock_cycles.size(); ++i) {
        int last_CZT_gate = 0;
        if (i % 2 == 1) {
            g_i = cir.clock_cycles[i] - 1;
        }
        else {
            g_i = cir.clock_cycles[i - 1];
        }
        for (int j = 0; j < cir.clock_cycles[i] - cir.clock_cycles[i-1]; ++j) {
            int k = j;
            if (i % 2 == 1) {
                k = -j;
            }
            if (cir.gates[g_i + k].gate_identification[0] == gate::Gates::T ||
                cir.gates[g_i + k].gate_identification[1] == gate::Gates::Phase) {
                
                if (i % 2 == 1) {
                    if(saw_xy) {
                        swap(cir.gates[g_i + k], cir.gates[g_i - last_CZT_gate]);
                    }
                }
                else {
                    if (saw_xy) {
                        swap(cir.gates[g_i + k], cir.gates[g_i + last_CZT_gate]);
                    }
                }
                ++last_CZT_gate;
            }
            else {
                saw_xy = true;
            }
        }
    }
    
//    int j = 0;
//    for (int i = 0; i < cir.clock_cycles.size(); ++i) {
//        cout << i << endl;
//
//        for(;j < cir.clock_cycles[i]; ++j) {
//            auto& g = cir.gates[j];
//            if(g.gate_identification.back() == gate::Gates::Phase)
//                cout << "CZ ";
//            else if (g.gate_identification.back() == gate::Gates::X_rotation)
//                cout << "X ";
//            else if (g.gate_identification.back() == gate::Gates::Y_rotation)
//                cout << "Y ";
//            else if (g.gate_identification.back() == gate::Gates::T)
//                cout << "T ";
//            else if (g.gate_identification.back() == gate::Gates::Hadamard)
//                cout << "H ";
//        }
//        cout << endl;
//    }
//    cout << endl << endl;
    cir.clock_cycles.clear();
}

circuit::circuit() : gates({}), qubit_to_gates({}), clock_cycles({}),
    circuit_state(new state()), merged(0), X(0), Y(0), CZ_T(0),
    qubits(0), num_cycles(0), circuit_preprocessed(false) {}

circuit::circuit(const circuit& g)
{
    qubits = g.qubits;
    circuit_state -> state_vector = g.circuit_state -> state_vector;
    gates = g.gates;
    clock_cycles = g.clock_cycles;
    circuit_preprocessed = g.circuit_preprocessed;
    qubit_to_gates = g.qubit_to_gates;
}

circuit& circuit::operator=(const circuit& g)
{
    circuit temp(g);
    swap(qubits, temp.qubits);
    swap(gates, temp.gates);
    swap(circuit_state -> state_vector, temp.circuit_state -> state_vector);
    return *this;
}

circuit::~circuit()
{
    delete circuit_state;
}

// TO DO: apply optimizations for diagonal gates, control Z, toffolli, and control X. 
void circuit::simulate(string outfile)
{
    int size = (int)gates.size();
    
    circuit_simulation sim (*this, 0);
    sim.CZ_T_preprocess();
    int cycle_num = 0;
    
    auto increment_cycle = [&](int g_i) {
        if (google && !clock_cycles.empty() && g_i == clock_cycles[cycle_num] ) {
            ++cycle_num;
        }
    };
    
    clock_t begin = clock();
    for (int i = 0; i < size; ++i) {
        
        g_begin = clock();
        increment_cycle(i);
        
        sim.gate_i = i;
        circuit_state -> apply_gate.clear();
        circuit_state -> indices_for_ag.clear();
        
        if(gates[i].qubits.size() > 1 &&
           gates[i].gate_identification[0] != gate::Gates::Control) {
            sort(gates[i].qubits.begin(), gates[i].qubits.end());
        }
        
        int XT_gates = 0;

        if(gates[i].gate_identification[0] == gate::Gates::Control ||
             gates[i].gate_identification[0] == gate::Gates::T) {
            
                vector<index_size> bit_mask;
                if (gates[i].gate_identification[1] == gate::Gates::X ||
                    gates[i].gate_identification[0] == gate::Gates::T ||
                    gates[i].gate_identification[1] == gate::Gates::Z ||
                    gates[i].gate_identification[1] == gate::Gates::Phase) {
                    
                    bit_mask = sim.initialize_gate_set(XT_gates,
                    [&](int g_i) {
                        return (gates[g_i].gate_identification[1] == gate::Gates::X ||
                           gates[g_i].gate_identification[0] == gate::Gates::T ||
                           gates[g_i].gate_identification[1] == gate::Gates::Z ||
                           gates[g_i].gate_identification[1] == gate::Gates::Phase);});

                    sim.block_gate_opt(bit_mask, XT_gates);
                    i += bit_mask.size() - 1;
//                    print_state();
                    g_end = clock();
                    gate_time[1] += double(g_end - g_begin)/ CLOCKS_PER_SEC;
                    CZ_T += bit_mask.size();
                }
                else {
                    index_size c_bits = 0;
                    for (int j = 0 ; j < gates[i].num_controls; ++j) {
                        c_bits += (1 << abs(gates[i].qubits[j] - abs(qubits - 1)));
                    }
                    sim.control_rand_sim(c_bits);
                }
        }
        else if (gates[i].gate_identification[0] == gate::Gates::Measurement) {
            circuit_state -> measure(gates[i].qubits[0]);
        }
        else {
            if (google && i < gates.size() - 1 &&
                (gates[i + 1].gate_identification[0] == gate::Gates::X_rotation ||
                gates[i + 1].gate_identification[0] == gate::Gates::Y_rotation)) {
                    sim.XY_merge_opt();
                    if (i + 1 == clock_cycles[cycle_num]) {
                        assert(0!=1);
                        //if true will have to modify cycle boundaries;
                    }
//                    print_state();
                    ++i;
                    g_end = clock();
                    gate_time[4] += double(g_end - g_begin)/ CLOCKS_PER_SEC;
                    merged++;
            }
            else if(google && gates[i].gate_identification[0] == gate::Gates::Hadamard) {
                int increment = sim.H_google_opt();
                i += increment - 1;
                g_end = clock();
                gate_time[0] += double(g_end - g_begin)/ CLOCKS_PER_SEC;
            }
            else {
                sim.non_control_sim();
                g_end = clock();
                
                if(gates[i].gate_identification[0] == gate::Gates::X_rotation){
                    gate_time[2] += double(g_end - g_begin)/ CLOCKS_PER_SEC;
                    X++;
                }
                else if(gates[i].gate_identification[0] == gate::Gates::Y_rotation) {
                    gate_time[3] += double(g_end - g_begin)/ CLOCKS_PER_SEC;
                    Y++;
                }
             }
        }
    }
    
    clock_t end = clock();
   
    if (qubits <= 16) {
        print_state();
    }
    
     print_stats(end , begin);
    
}

void circuit::print_stats(clock_t end, clock_t begin) {
    double total_time = double(end - begin) / CLOCKS_PER_SEC;
    cout << setprecision(3);
    cout << "Total Time : " << total_time << "s\n\n";
    
    cout << "Runtimes by gate type\n";
    cout << "   H (" << qubits << ") : " << gate_time[0]
    << "s = " << (gate_time[0]/total_time) * 100 << "%\n";
    
    cout << "   CZ & T (" << CZ_T << ") : " << gate_time[1]
    << "s = " << (gate_time[1]/total_time) * 100 << "%\n";
    
    cout << "   X (" << X << ") : " << gate_time[2] << "s = "
    << (gate_time[2]/total_time) * 100 << "%\n";
    
    cout << "   Y (" << Y << ") : " << gate_time[3]
    << "s = " << (gate_time[3]/total_time) * 100 << "%\n";
    
    cout << "   Merged X & Y (" << 2 * merged << ") : " << gate_time[4]
    << "s = " << (gate_time[4]/total_time) * 100 << "%\n\n";;
    
    char hostname[20];
    gethostname(hostname, 20);
    cout << "Qubits : " << qubits << "\n";
    cout << "Gates : " << gates.size() << "\n";
    cout << "Cycles : " << num_cycles << "\n";
    cout << "Hostname : ";
    for(auto h : hostname) {
        cout << h;
    }
    cout << "\n";
    cout << "Compiler : " << __GNUC__ << "\n";
    cout << "Compilation Time : " << __TIME__ << "\n";
    cout << "Current Date : " << __DATE__ << "\n\n";

}

void circuit::print_stats(string& outfile, clock_t end, clock_t begin)
{
    ofstream file;
    file.open("simulation_stats/" + outfile);
    
    double total_time = double(end - begin) / CLOCKS_PER_SEC;
    file << setprecision(3);
    file << "Total Time : " << total_time << "s\n\n";
    
    file << "Runtimes by gate type\n";
    file << "   H (" << qubits << ") : " << gate_time[0]
    << "s = " << (gate_time[0]/total_time) * 100 << "%\n";
    
    file << "   CZ & T (" << CZ_T << ") : " << gate_time[1]
    << "s = " << (gate_time[1]/total_time) * 100 << "%\n";
    
    file << "   X (" << X << ") : " << gate_time[2] << "s = "
    << (gate_time[2]/total_time) * 100 << "%\n";
    
    file << "   Y (" << Y << ") : " << gate_time[3]
    << "s = " << (gate_time[3]/total_time) * 100 << "%\n";
    
    file << "   Merged X & Y (" << 2 * merged << ") : " << gate_time[4]
    << "s = " << (gate_time[4]/total_time) * 100 << "%\n\n";;
    
    char hostname[20];
    gethostname(hostname, 20);
    file << "Qubits : " << qubits << "\n";
    file << "Gates : " << gates.size() << "\n";
    file << "Cycles : " << num_cycles << "\n";
    file << "Hostname : ";
    for(auto h : hostname) {
        file << h;
    }
    file << "\n";
    file << "Compiler : " << __GNUC__ << "\n";
    file << "Compilation Time : " << __TIME__ << "\n";
    file << "Current Date : " << __DATE__ << "\n\n";
    
    file.close();
}


int circuit::test(circuit& result)
{
    for (int i = 0; i < gates.size(); ++i) {
        gates[i].rows = matrix_inv(gates[i].rows);
    }
    
    //reverse the order of the gates and call simulate;
    //save the initial state vector in test.
    return 0;
}

void circuit::print_probabilities(string& out_file)
{
    static int count = 0;
    ofstream file;
    file.open(out_file + to_string(count) + ".txt");
    
    for (auto state_v : circuit_state -> state_vector) {
        state_v *= cmplx((HADAMARD_CONST, H_related_gates));
        state_v *= conj(state_v); /// cmplx(pow(2, qubits));
        
        file << real(state_v) ;
        
        if (imag(state_v) > 0) {
            file << "+" << imag(state_v) << "i";
        }
        else if (imag(state_v) < 0) {
            file << imag(state_v) << "i";
        }
        file << "\n";
    }
    //file << "\n";
    ++count;
}

void circuit::print_state()
{
    for (auto state_v : circuit_state -> state_vector) {
        state_v *= cmplx(pow(HADAMARD_CONST, H_related_gates));
        
        cout << real(state_v) ;
        
        if (imag(state_v) > 0) {
            cout << "+" << imag(state_v) << "i";
        }
        else if (imag(state_v) < 0) {
            cout << imag(state_v) << "i";
        }
        cout << "\n";
    }
    cout << "\n\n";
}

void circuit::print_state(string outfile)
{
    static ofstream file;
    file.open(outfile + ".txt");
    
    for (auto state_v : circuit_state -> state_vector) {
        state_v *= cmplx(pow(HADAMARD_CONST, H_related_gates));
        file << real(state_v) ;
        
        if (imag(state_v) > 0) {
            file << "+" << imag(state_v) << "i";
        }
        else if (imag(state_v) < 0) {
            file << imag(state_v) << "i";
        }
        file << "\n";
    }
    file << "\n\n";
}

state::state():state_vector({}),
        apply_gate({}), indices_for_ag({}) {}

state::state(const state& rhs)
{
    state_vector = rhs.state_vector;
    apply_gate = rhs.apply_gate;
    indices_for_ag = rhs.indices_for_ag;
}

state& state::operator=(const state& rhs)
{
    state temp(rhs);
    swap(state_vector, temp.state_vector);
    swap(apply_gate, temp.apply_gate);
    swap(indices_for_ag, temp.indices_for_ag);
    return *this;
}

void state::operator*(const gate& q_gate)
{
    if (!(apply_gate[0] == cmplx(0,0) && apply_gate[1] == cmplx(0,0))) {

        if (google &&
                q_gate.gate_identification[0] == gate::Gates::X_rotation) {
            state_vector[indices_for_ag[1]] = -cmplx(-imag(apply_gate[0]), real(apply_gate[0]))
                                            + apply_gate[1];
            state_vector[indices_for_ag[0]] = apply_gate[0] -
                                cmplx(-imag(apply_gate[1]), real(apply_gate[1]));
        }
        else if (google &&
                 q_gate.gate_identification[0] == gate::Gates::Y_rotation) {
            state_vector[indices_for_ag[1]] = apply_gate[0] + apply_gate[1];
            state_vector[indices_for_ag[0]] = apply_gate[0] - apply_gate[1];
        }
        else if (q_gate.gate_identification[0] == gate::Gates::Z ||
            q_gate.gate_identification[0] == gate::Gates::T) {
            
            for (int i = 0; i < indices_for_ag.size(); ++i) {
                state_vector[indices_for_ag[i]] = apply_gate[i] * q_gate.rows[i][i];
            }
        }
        else if (q_gate.gate_identification[0] == gate::Gates::X ||
                 q_gate.gate_identification[0] == gate::Gates::Y ) {
            short a_size = indices_for_ag.size();
            for (int i = 0; i < a_size; ++i) {
                state_vector[indices_for_ag[i]] = apply_gate[a_size - i + 1]
                * q_gate.rows[i][a_size - i + 1];
            }
        }
        else {
            apply_gate = matrix_v_mult(q_gate.rows, apply_gate);
            int c = 0;
            for(const auto& a : indices_for_ag) {
                state_vector[a] = apply_gate[c++];
            }
        }
    }
    apply_gate.clear();
    indices_for_ag.clear();
}

//add optimization for 2n/2 iterations
cmplx state::measure_0(short qubit)
{
    int m_bit = pow(2, abs(qubit - (sqrt(state_vector.size()) - 1)));
    map<int, cmplx> measurement_0;
    cmplx prob_0 = 0;
    
    for (int i = 0; i < state_vector.size() ; ++i) {
        if ((i & m_bit) != m_bit) {
            measurement_0[i] = (state_vector[i] * conj(state_vector[i]));
        }
        else {
            i += pow(2, abs(qubit - (sqrt(state_vector.size()) - 1)));
        }
    }
    
    for (auto m : measurement_0) {
        prob_0 += m.second;
    }
    
    prob_0 = cmplx(1) / sqrt(prob_0);
    for (auto m : measurement_0) {
        state_vector[m.first] = prob_0 * state_vector[m.first];
    }
    
    return prob_0;
}

cmplx state::measure_1(short qubit)
{
    int m_bit = pow(2, abs(qubit - (sqrt(state_vector.size()) - 1)));
    map<int, cmplx> measurement_1;
    cmplx prob_1 = 0;
    
    for (int i = 0; i < state_vector.size() ; ++i) {
        if ((i & m_bit) == m_bit) {
            measurement_1[i] = (state_vector[i] * conj(state_vector[i]));
        }
        else {
            i += pow(2, abs(qubit - (sqrt(state_vector.size()) - 1)));
        }
    }
    
    for (auto m : measurement_1) {
        prob_1 += m.second;
    }
    
    prob_1 = cmplx(1) / sqrt(prob_1);
    for (auto m : measurement_1) {
        state_vector[m.first] = prob_1 * state_vector[m.first];
    }
    
    return prob_1;
}

void state::measure(short qubit)
{
    int m_bit = pow(2, abs(qubit - (sqrt(state_vector.size()) - 1)));
    map<int, cmplx> measurement_0;
    cmplx prob_0 = 0;
    map<int, cmplx> measurement_1;
    cmplx prob_1 = 0;
    
    for (int i = 0; i < state_vector.size() ; ++i) {
        if ((i & m_bit) == m_bit) {
            measurement_1[i] = (state_vector[i] * conj(state_vector[i]));
        }
        else {
            measurement_0[i] = (state_vector[i] * conj(state_vector[i]));
        }
    }
    
    for (auto m : measurement_0) {
        prob_0 += m.second;
    }
    
    for (auto m : measurement_1) {
        prob_1 += m.second;
    }
    
    prob_0 = cmplx(1) / sqrt(prob_0);
    for (auto m : measurement_0) {
        state_vector[m.first] = prob_0 * state_vector[m.first];
    }
    
    prob_1 = cmplx(1) / sqrt(prob_1);
    for (auto m : measurement_1) {
        state_vector[m.first] = prob_1 * state_vector[m.first];
    }
}



