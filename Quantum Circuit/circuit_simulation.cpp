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
function<void(index_size)> increment_cycle;

template<typename function>
vector<index_size> circuit::
FormBlockOfGates(int& num_X_gates,
                 index_size gate_i,
                 vector<index_size>& T_bit_mask,
                 function comp)
{
    vector<index_size> bit_mask {};
    for(;(gate_i < gates.size()) && (comp(gate_i)); ++gate_i) {
        
        index_size bits = 0;
        increment_cycle(gate_i);

        if (gates[gate_i].gate_identification.back() == gate::Gates::Phase ||
            gates[gate_i].gate_identification.back() == gate::Gates::Z) {
            for (index_size j = 0 ; j < gates[gate_i].qubits.size(); ++j)
                bits |= ( 1ull << ((qubits - 1) - gates[gate_i].qubits[j]));
            
            bit_mask.push_back(bits);
        }
        else if (gates[gate_i].gate_identification.back() == gate::Gates::T) {
            index_size t_mask = (1ull << ((qubits - 1) - gates[gate_i].qubits[0]));
            if ((T_bit_mask.front() & t_mask) != t_mask)
                T_bit_mask[0] |= t_mask;
            
            else {
                bool found = false;
                for (auto& t : T_bit_mask)
                    if ((t & t_mask) != t_mask) {
                        t |= t_mask;
                        found = true;
                    }
                if (!found) {
                    T_bit_mask.push_back(0 | t_mask);
                }
            }
        }
        else if (gates[gate_i].gate_identification.back() == gate::Gates::X) {
            for (index_size j = 0 ; j < gates[gate_i].num_controls; ++j)
                bits |= (1ull << ((qubits - 1) - gates[gate_i].qubits[j]));
            
            ++num_X_gates;
        }
    }
    
    return bit_mask;
}

void circuit::
ApplyBlockOfGates(const vector<index_size>& cbits,
                  const index_size gate_i,
                  const vector<index_size>& T_bit_mask,
                  const int X_gates)
{
    auto find_smallest_non_control = [&](index_size j) {
        for(int cq = 0; cq < gates[gate_i + j].num_controls; ++cq)
            if (cq != gates[gate_i + j].qubits[cq])
                return cq;
        
        return -1;
    };
    vector<vector<bool>> iterated;
    if (X_gates > 0)
        iterated.resize(circuit_state -> amp.size(),
                        vector<bool>(X_gates,false)) ;
    
    index_size size =  circuit_state -> amp.size();
    int c_size = (int)cbits.size();
    
    for (index_size idx = 0; idx < size ; ++idx) {
        int iter_count = 0;
        
        if (current_google_cycle % 50 == 0)
            circuit_state -> amp[idx] *=
            pow(HADAMARD_CONST, H_related_gates);
        
        bool negate_Z = false;
        int i_count = 0;
        for (index_size j = 0; j < c_size; ++j){
            
            if((idx & cbits[j]) == cbits[j]) {
                if (circuit_state -> amp[idx] != cmplx(0,0)) {
                    if (gates[gate_i + j].gate_identification.back() == gate::Gates::Phase)
                        ++i_count;
                    
                    else if (gates[gate_i + j].gate_identification.back() == gate::Gates::Z)
                        negate_Z = ~negate_Z;
                }
                else if (gates[gate_i + j].gate_identification.back() == gate::Gates::X) {
                    if (iterated[idx][iter_count] == false) {
                        iterated[idx][iter_count] = true;
                        
                        int smallest_non_control_bit = find_smallest_non_control(j);
                        if (smallest_non_control_bit == -1)
                            continue;
                        
                        index_size swap_index = idx +
                        (1 << ((qubits - 1) - smallest_non_control_bit));
                        iterated[swap_index][iter_count] = true;
                        
                        if (!(circuit_state -> amp[idx] == cmplx(0,0) &&
                              circuit_state -> amp[swap_index] == cmplx(0,0)))
                            ApplyCXGate(idx, swap_index);
                    }
                    ++iter_count;
                }
            }
        }
        
        if (i_count > 0)
            ApplyPhaseGate(i_count, idx);
        if (negate_Z)
            circuit_state -> amp[idx] = -circuit_state -> amp[idx];
        if (T_bit_mask.front() != 0 && circuit_state->amp[idx] != cmplx(0,0)) {
            index_size gate_c = 0;
            for (auto t : T_bit_mask)
                gate_c += __builtin_popcountll(idx & t);
            
            if (gate_c > 0)
                ApplyTGateKTimes(gate_c, idx);
        }
    }
    if (current_google_cycle % 50 == 0)
        H_related_gates = 0;
}


inline void circuit::
ApplyRandomGate(vector<bool>& iterated,
                const index_size gate_i,
                const int num_bits,
                const index_size idx)
{
    index_size c = 1ull << num_bits;
    circuit_state -> apply_gate.resize(1ull << num_bits);
    circuit_state -> apply_gate[0] = circuit_state -> amp[idx];
    circuit_state -> indices_for_ag.push_back(idx);
    
    index_size temp_index = 0;
    int c1 = 1;
    int target = gates[gate_i].num_controls;
    
    for (index_size i = target ; i < num_bits; ++i) {
        short relative_pos = 1;
        short j = 0;
        for (index_size n = 0; n < circuit_state -> indices_for_ag.size() &&
             circuit_state -> indices_for_ag.size() < (1ull << (i+1)); n += 2) {
            
            temp_index = circuit_state -> indices_for_ag[n] +
                    (1ull << ((qubits - 1) - gates[gate_i].qubits[i]));
            
            assert(temp_index < circuit_state -> amp.size());
            iterated[temp_index] = true;
            
            circuit_state -> indices_for_ag.insert(
                               circuit_state -> indices_for_ag.begin()
                                + j + relative_pos, temp_index);
            circuit_state -> apply_gate[c/(1 << c1) * (j + relative_pos)] =
                    circuit_state -> amp[temp_index];
            ++relative_pos;
            ++j;
        }
        ++c1;
    }
}


inline void circuit::
ApplyAnyCGate(const index_size c_bits,
              const index_size gate_i)
{
    vector<bool> iterated(circuit_state -> amp.size(), false);
    int num_bits = (int)gates[gate_i].qubits.size();
    index_size iter_count = 0;
    index_size size = circuit_state -> amp.size();
    int num_controls = gates[gate_i].num_controls;
    
    for (index_size idx = 0; idx < size &&
         iter_count < (1 << (qubits - num_controls)); ++idx) {
        if (iterated[idx] == false) {
            iterated[idx] = true;

            if((idx & c_bits) == c_bits) {
                iter_count += 2;
                ApplyRandomGate(iterated, gate_i, num_bits, idx);
               (*circuit_state) * (gates[gate_i]);
            }
        }
    }
}

inline void circuit::
ApplyNonControlGates(const index_size gate_i)
{
    int num_bits = (int)gates[gate_i].qubits.size();
    vector<bool> iterated(circuit_state -> amp.size(), false);
    index_size iter_count = 0;
    index_size size = circuit_state -> amp.size();
    
    for (index_size idx = 0; idx < size && iter_count < size/(1 << num_bits); ++idx) {
        if (iterated[idx] == false) {
            iterated[idx] = true;
            ++iter_count;
            
            ApplyRandomGate(iterated, gate_i, num_bits, idx);
            (*circuit_state) * (gates[gate_i]);
        }
    }
    if(google) {
        ++H_related_gates;
    }
}

inline void circuit::
ApplyCXGate(const index_size idx,
            const index_size swap_index)
{
    if(!(circuit_state -> amp[idx] == cmplx(0,0)
             && circuit_state -> amp[idx] == cmplx(0,0))) {
            swap(circuit_state -> amp[idx],
                 circuit_state -> amp[swap_index]);
    }
}

inline void circuit::
ApplyPhaseGate(const int i_count,
               const index_size idx)
{
    switch (i_count % 4) {
        case 1: {
            circuit_state -> amp[idx] =
                cmplx(-imag(circuit_state -> amp[idx]),
                      real(circuit_state -> amp[idx]));
            break;
        }
        case 2: {
            circuit_state -> amp[idx] =
            -cmplx(real(circuit_state -> amp[idx]),
                   imag(circuit_state -> amp[idx]));
            break;
        }
        case 3: {
            circuit_state -> amp[idx] =
            -cmplx(-imag(circuit_state -> amp[idx]),
                  real(circuit_state -> amp[idx]));
            break;
        }
        default:
            break;
    }
}

inline void circuit::
ApplyTGateKTimes(const index_size gate_c,
                 const index_size idx)
{
    switch (gate_c % 8) {
        case 1: {
            //0.707106781 + 0.707106781 i
            circuit_state->amp[idx] *= HADAMARD_CONST;
            circuit_state->amp[idx] =
            cmplx(real(circuit_state->amp[idx])
                  - imag(circuit_state->amp[idx]),
                  real(circuit_state->amp[idx])
                  + imag(circuit_state->amp[idx]));
            break;
        }
        case 2: {
            //i
            circuit_state -> amp[idx] =
            cmplx(-imag(circuit_state -> amp[idx]),
                  real(circuit_state -> amp[idx]));
            break;
        }
        case 3: {
            //-0.707106781 + 0.707106781 i
            circuit_state->amp[idx] *= HADAMARD_CONST;
            circuit_state->amp[idx] =
            cmplx(-real(circuit_state->amp[idx])
                  -imag(circuit_state->amp[idx]),
                  real(circuit_state->amp[idx])
                  -imag(circuit_state->amp[idx]));
            break;
        }
        case 4: {
            //-1
            circuit_state -> amp[idx] =
            -cmplx(real(circuit_state -> amp[idx]),
                   imag(circuit_state -> amp[idx]));
            break;
        }
        case 5: {
            //-0.707106781 - 0.707106781 i
            circuit_state->amp[idx] *= HADAMARD_CONST;
            circuit_state->amp[idx] =
            cmplx(-real(circuit_state->amp[idx])
                  + imag(circuit_state->amp[idx]),
                  -real(circuit_state->amp[idx])
                  -imag(circuit_state->amp[idx]));
            break;
        }
        case 6: {
            //- i
            circuit_state -> amp[idx] =
            -cmplx(-imag(circuit_state -> amp[idx]),
                  real(circuit_state -> amp[idx]));
            break;
        }
        case 7: {
            //0.707106781 - 0.707106781 i
            circuit_state->amp[idx] *= HADAMARD_CONST;
            circuit_state->amp[idx] =
            cmplx(real(circuit_state->amp[idx])
                  + imag(circuit_state->amp[idx]),
                  -real(circuit_state->amp[idx])
                  + imag(circuit_state->amp[idx]));
            break;
        }
        default:
            break;
    }
}

inline int circuit::
ApplyHOnAllAmp(const index_size gate_i)
{
    int j = 0;
    for(; j < gates.size() &&
        gates[gate_i + j].gate_identification.back() == gate::Gates::Hadamard; ++j )
        ++H_related_gates;

    index_size size = circuit_state -> amp.size();
    for (index_size idx = 0; idx < size; ++idx)
        circuit_state -> amp[idx] = 1;

    return j;
}

inline void circuit::
ApplyXXGate()
{
    auto& ap_gate = circuit_state -> amp;
    auto& indices = circuit_state -> indices_for_ag;

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

inline void circuit::
ApplyXYGate()
{
    auto& ap_gate = circuit_state -> apply_gate;
    auto& temp_v = circuit_state -> amp;
    auto& indices = circuit_state -> indices_for_ag;
    
    temp_v[indices[0]] = ap_gate[0] - ap_gate[1] - cmplx(-imag(ap_gate[2]), real(ap_gate[2]))
                + cmplx(-imag(ap_gate[3]), real(ap_gate[3]));
    
    temp_v[indices[1]] = ap_gate[0] + ap_gate[1] - cmplx(-imag(ap_gate[2]), real(ap_gate[2]))
                - cmplx(-imag(ap_gate[3]), real(ap_gate[3]));
    
    temp_v[indices[2]] = - cmplx(-imag(ap_gate[0]), real(ap_gate[0]))
                + cmplx(-imag(ap_gate[1]), real(ap_gate[1])) + ap_gate[2] - ap_gate[3];
    
    temp_v[indices[3]] = - cmplx(-imag(ap_gate[0]), real(ap_gate[0]))
                - cmplx(-imag(ap_gate[1]), real(ap_gate[1])) + ap_gate[2] + ap_gate[3];
}

inline void circuit::
ApplyYYGate()
{
    auto& temp_v = circuit_state -> amp;
    auto& indices = circuit_state -> indices_for_ag;
    
    auto t = temp_v[indices[0]] + temp_v[indices[1]];
    auto t1 = temp_v[indices[0]] - temp_v[indices[1]];
    auto t2 = temp_v[indices[2]] + temp_v[indices[3]];
    auto t3 = temp_v[indices[2]] - temp_v[indices[3]];
    
    temp_v[indices[0]] = t1 - t3;
    temp_v[indices[1]] = t - t2;
    temp_v[indices[2]] = t1 + t3;
    temp_v[indices[3]] = t + t2;
}

inline void circuit::
ApplyYXGate()
{
    auto& temp_v = circuit_state -> amp;
    auto& indices = circuit_state -> indices_for_ag;
    
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

void circuit::
Merge2QXYGates(index_size gate_i)
{
    index_size a, b;
    if (gates[gate_i].qubits.back() < gates[gate_i + 1].qubits.back()) {
        gates[gate_i].qubits.push_back(gates[gate_i + 1].qubits.back());
        a = gate_i;
        b = gate_i + 1;
    }
    else {
        gates[gate_i + 1].qubits.push_back(gates[gate_i].qubits.back());
        a = gate_i + 1;
        b = gate_i++;
    }
    
    if(gates[a].gate_identification.back() == gate::Gates::X_rotation &&
       gates[b].gate_identification.back() == gate::Gates::X_rotation)
        ApplyMergedXYGates(0, gate_i);
    
    else if(gates[a].gate_identification.back() == gate::Gates::X_rotation &&
       gates[b].gate_identification.back() == gate::Gates::Y_rotation)
        ApplyMergedXYGates(1, gate_i);
    
    else if(gates[a].gate_identification.back() == gate::Gates::Y_rotation &&
       gates[b].gate_identification.back() == gate::Gates::X_rotation)
        ApplyMergedXYGates(2, gate_i);
    
    else if(gates[a].gate_identification.back() == gate::Gates::Y_rotation &&
       gates[b].gate_identification.back() == gate::Gates::Y_rotation)
        ApplyMergedXYGates(3, gate_i);
}

void circuit::
ApplyMergedXYGates(const short type,
                   const index_size gate_i)
{
    int smallest_non_control_bit = 0;
    for(int cq = 0; cq < gates[gate_i].num_controls; ++cq) {
        if (cq != gates[gate_i].qubits[cq])
            smallest_non_control_bit = cq;
    }

    int num_bits = (int)gates[gate_i].qubits.size();
    vector<bool> iterated(circuit_state -> amp.size(), false);
    index_size iter_count = 0;
    index_size size = circuit_state -> amp.size();
    
    for (index_size idx = 0; idx < size && iter_count < size/(1 << num_bits); ++idx) {
        if (iterated[idx] == false) {
            iterated[idx] = true;
            ++iter_count;
            
            ApplyRandomGate(iterated, gate_i, num_bits, idx);
            
            switch (type) {
                case 0: {
                    ApplyXXGate();
                    break;
                }
                case 1: {
                    ApplyXYGate();
                    break;
                }
                case 2: {
                    ApplyYXGate();
                    break;
                }
                case 3: {
                    ApplyYYGate();
                    break;
                }
                default: {
                    throw "Invalid XY merge\n";
                    break;
                }
            }
            
            circuit_state -> apply_gate.clear();
            circuit_state -> indices_for_ag.clear();
        }
    }
    H_related_gates += 2;
}

void circuit::
ApplySingleTGate(const index_size gate_i)
{
    index_size gap = (1ull << ((qubits - 1) - gates[gate_i].qubits[0]));
    index_size idx = gap;
    for (; idx < circuit_state -> amp.size(); ++idx) {
        if ((idx & gap) != gap) {
            idx += gap - 1;
            continue;
        }
        ApplyTGateKTimes(1, idx);
    }
}

void circuit::
ApplySingleCZGate(const index_size gate_i)
{
    int num_controls = gates[gate_i].num_controls;
    
    index_size c_bits = 0;
    for (index_size j = 0 ; j < num_controls; ++j)
        c_bits |= (1ull << ((qubits - 1) - (gates[gate_i].qubits[j])));
    
    index_size t_bits = c_bits;
        for (index_size j = num_controls ;
             j < gates[gate_i].qubits.size(); ++j)
            t_bits |= (1ull << ((qubits - 1) - (gates[gate_i].qubits[j])));
    
    index_size size = circuit_state -> amp.size();
    
    index_size idx = (1ull << ((qubits - 1) - (gates[gate_i].qubits[0])));
    index_size gap = (1ull << ((qubits - 1) - (gates[gate_i].qubits[1])));
    
    idx += gap;
    for (;idx < size ; ++idx) {
        if ((idx & c_bits) != c_bits) {
            idx += (1ull << ((qubits - 1) - (gates[gate_i].qubits[0])));
            idx += gap;
        }
        if((idx & t_bits) == t_bits) {
            ApplyPhaseGate(1, idx);
        }
    }
}

void circuit::
GroupAlternateCycles()
{
    bool saw_xy = false;
    int g_i = 0;
    num_cycles = clock_cycles.size();
    
    for (index_size i = 2; i < clock_cycles.size(); ++i) {
        int last_CZT_gate = 0;
        if (i % 2 == 1)
            g_i = clock_cycles[i] - 1;
        
        else
            g_i = clock_cycles[i - 1];
        
        for (int j = 0; j < clock_cycles[i] - clock_cycles[i-1]; ++j) {
            int k = j;
            if (i % 2 == 1)
                k = -j;
            
            if (gates[g_i + k].gate_identification.back() == gate::Gates::T ||
                gates[g_i + k].gate_identification.back() == gate::Gates::Phase) {
                
                if (i % 2 == 1) {
                    if(saw_xy)
                        swap(gates[g_i + k], gates[g_i - last_CZT_gate]);
                    
                }
                else {
                    if (saw_xy)
                        swap(gates[g_i + k], gates[g_i + last_CZT_gate]);
                    
                }
                ++last_CZT_gate;
            }
            else
                saw_xy = true;
            
        }
    }
    PrintGatesAndCycles();
}

void circuit::
GroupSimilarGates()
{
    int last_CZ = 0, last_T = 0, last_X = 0, last_Y = 0;
    bool saw_CZ = false, saw_T = false, saw_X = false, saw_Y = false;
    int g_i = qubits;
        
    for (int j = qubits; j < gates.size()
         && (g_i + last_Y + last_X + last_T + last_CZ) < gates.size(); ++j) {
            
            if (gates[j].gate_identification.back() == gate::Gates::Phase) {
                if(saw_Y || saw_X) {
                    g_i = j;
                    last_CZ = 0; last_T = 0; last_X = 0; last_Y = 0;
                    saw_CZ = false; saw_T = false; saw_X = false; saw_Y = false;
                }
                saw_CZ = true;
                if (last_CZ != 0 && (saw_T || saw_X || saw_Y))
                    swap(gates[j], gates[g_i + last_CZ]);
                
                ++last_CZ;
            }
            else if (gates[j].gate_identification.back() == gate::Gates::T) {
                if(saw_Y || saw_X) {
                    g_i = j;
                    last_CZ = 0; last_T = 0; last_X = 0; last_Y = 0;
                    saw_CZ = false; saw_T = false; saw_X = false; saw_Y = false;
                }
                saw_T = true;
                if (saw_CZ || saw_X || saw_Y)
                    swap(gates[g_i + last_T + last_CZ], gates[j]);
                
                ++last_T;
            }
            else if (gates[j].gate_identification.back() == gate::Gates::X_rotation) {
                saw_X = true;
                if (saw_T || saw_CZ || saw_Y)
                    swap(gates[g_i + last_X + last_T + last_CZ], gates[j]);
                
                ++last_X;
            }
            else if (gates[j].gate_identification.back() == gate::Gates::Y_rotation) {
                saw_Y = true;
                if (saw_T || saw_X || saw_CZ)
                    swap(gates[g_i + last_Y + last_X + last_T + last_CZ], gates[j]);
                
                ++last_Y;
            }
        }
    PrintGatesAndCycles();
}

circuit::
circuit() : gates({}),clock_cycles({}),
    circuit_state(new state()), merged(0), X(0), Y(0), CZ_T(0),
    qubits(0), num_cycles(0), circuit_preprocessed(false) {}

circuit::
circuit(const circuit& g)
{
    qubits = g.qubits;
    circuit_state -> amp = g.circuit_state -> amp;
    gates = g.gates;
    clock_cycles = g.clock_cycles;
    circuit_preprocessed = g.circuit_preprocessed;
}

circuit& circuit::
operator=(const circuit& g)
{
    circuit temp(g);
    swap(qubits, temp.qubits);
    swap(gates, temp.gates);
    swap(circuit_state -> amp, temp.circuit_state -> amp);
    return *this;
}

circuit::
~circuit()
{
    delete circuit_state;
}

// TO DO: apply optimizations for diagonal gates, control Z, toffolli, and control X. 
void circuit::
Simulate(const string& outfile)
{
    int size = (int)gates.size();
    
//    if (google) {
//        if (!clock_cycles.empty())
//            GroupAlternateCycles();
//        GroupSimilarGates();
//    }
    
    increment_cycle = [&](index_size g_i) {
        if (google && !clock_cycles.empty() && g_i == clock_cycles[current_google_cycle] )
            ++current_google_cycle;
    };
    
    clock_t begin = clock();
    for (index_size i = 0; i < size; ++i) {
        
        g_begin = clock();
        increment_cycle(i);
        
        circuit_state -> apply_gate.clear();
        circuit_state -> indices_for_ag.clear();
        
        if(gates[i].qubits.size() > 1 &&
           gates[i].gate_identification.front() != gate::Gates::Control)
            sort(gates[i].qubits.begin(), gates[i].qubits.end());
        
        //T bit mask won't work if multiple T gates on same qubit (general case)
        if(gates[i].gate_identification.front() == gate::Gates::Control ||
             gates[i].gate_identification.back() == gate::Gates::T) {
            
            auto comparator = [&](index_size g_i) {
                return (gates[g_i].gate_identification.back() == gate::Gates::X ||
                        gates[g_i].gate_identification.back() == gate::Gates::T ||
                        gates[g_i].gate_identification.back() == gate::Gates::Z ||
                        gates[g_i].gate_identification.back() == gate::Gates::Phase);
            };
                if (comparator(i)) {
                    
                    int XT_gates = 0;
                    vector<index_size> bit_mask;
                    vector<index_size> T_bit_mask(1, 0);
                    
                    bit_mask = FormBlockOfGates(XT_gates, i, T_bit_mask, comparator);
                    
//                    if ((bit_mask.size() == 0 && T_bit_mask.size() == 1
//                         && __builtin_popcountll(T_bit_mask.front()) == 1) ||
//                        (bit_mask.size() == 1 && __builtin_popcountll(T_bit_mask.front()) == 0)) {
//                        if (gates[i].gate_identification.back() == gate::Gates::T) {
//                            ApplySingleTGate(i);
//                        }
//                        else if (gates[i].gate_identification.back() == gate::Gates::Phase) {
//                            ApplySingleCZGate(i);
//                        }
//                    }
//                    else {
                        ApplyBlockOfGates(bit_mask, i, T_bit_mask, XT_gates);
                   // }

                    i += bit_mask.size();
                    for (auto t : T_bit_mask)
                        i += __builtin_popcountll(t);
                    i -= 1;
                    PrintStateVector();
                    g_end = clock();
                    gate_time[1] += double(g_end - g_begin)/ CLOCKS_PER_SEC;
                    CZ_T += bit_mask.size();
                    for (auto t : T_bit_mask)
                        CZ_T += __builtin_popcountll(t);
                }
                else {
                    index_size c_bits = 0;
                    for (index_size j = 0 ; j < gates[i].num_controls; ++j)
                        c_bits += (1 << ((qubits - 1) - (gates[i].qubits[j])));
                    
                    ApplyAnyCGate(c_bits, i);
                }
        }
        else if (gates[i].gate_identification.back() == gate::Gates::Measurement) {
            circuit_state -> measure(gates[i].qubits[0]);
        }
        else {
            if (google && i < gates.size() - 1 &&
                (gates[i + 1].gate_identification.back() == gate::Gates::X_rotation ||
                gates[i + 1].gate_identification.back() == gate::Gates::Y_rotation)) {
                    Merge2QXYGates(i);
                    ++i;
                    g_end = clock();
                    gate_time[4] += double(g_end - g_begin)/ CLOCKS_PER_SEC;
                    merged++;
            }
            else if(google && gates[i].gate_identification.back() == gate::Gates::Hadamard) {
                int increment = ApplyHOnAllAmp(i);
                i += increment - 1;
                g_end = clock();
                gate_time[0] += double(g_end - g_begin)/ CLOCKS_PER_SEC;
            }
            else {
                ApplyNonControlGates(i);
                g_end = clock();
                
                if(gates[i].gate_identification.back() == gate::Gates::X_rotation){
                    gate_time[2] += double(g_end - g_begin)/ CLOCKS_PER_SEC;
                    X++;
                }
                else if(gates[i].gate_identification.back() == gate::Gates::Y_rotation) {
                    gate_time[3] += double(g_end - g_begin)/ CLOCKS_PER_SEC;
                    Y++;
                }
             }
        }
    }
    
    clock_t end = clock();
   
    if (qubits <= 16)
        PrintStateVector();
    
     PrintReport(end , begin);
    
}

void circuit::
PrintGatesAndCycles()
{
    int j = 0;
    for (index_size i = 0; i < clock_cycles.size(); ++i) {
        cout << i << endl;

        for(;j < clock_cycles[i]; ++j) {
            auto& g = gates[j];
            if(g.gate_identification.back() == gate::Gates::Phase)
                cout << "CZ ";
            else if (g.gate_identification.back() == gate::Gates::X_rotation)
                cout << "X ";
            else if (g.gate_identification.back() == gate::Gates::Y_rotation)
                cout << "Y ";
            else if (g.gate_identification.back() == gate::Gates::T)
                cout << "T ";
            else if (g.gate_identification.back() == gate::Gates::Hadamard)
                cout << "H ";
        }
        cout << "\n";
    }
    cout << "\n\n";
}

void circuit::
PrintReport(const clock_t end, const clock_t begin)
{
    char hostname[20] = {};
    gethostname(hostname, 20);
    cout << "Hostname : ";
    for(auto h : hostname) {
        cout << h;
    }
    cout << "\n";
    cout << "Compiler : gcc " << __GNUC__  << "." << __GNUC_MINOR__ << "."
         <<  __GNUC_PATCHLEVEL__<< "\n";
    cout << "Compiled on : " <<  __DATE__ << " " << __TIME__ << "\n";
    cout << "Executed on : " << __DATE__ << " " << __TIME__ << "\n\n";
    
    cout << "Qubits : " << qubits << "  ";
    cout << "Gates : " << gates.size() << "  ";
    cout << "Cycles : " << num_cycles << "\n\n";
    
    double total_time = double(end - begin) / CLOCKS_PER_SEC;
    cout << setprecision(3);
    cout << "Total runtime : " << total_time << "s\n\n";
    
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
}

void circuit::
PrintReport(const string &outfile, const clock_t end, const clock_t begin)
{
    ofstream file;
    file.open("simulation_stats/" + outfile);
    
    char hostname[20];
    gethostname(hostname, 20);
    file << "Hostname : ";
    for(auto h : hostname) {
        file << h;
    }
    file << "\n";
    file << "Compiler (major): " << __GNUC__  << __GNUC_MINOR__
         <<  __GNUC_PATCHLEVEL__<< "\n";
    file << "Compilated on : " <<  __DATE__ << " , " << __TIME__ << "\n";
    file << "Current Date : " << __DATE__ << "\n\n";
    
    file << "Qubits : " << qubits << "  ";
    file << "Gates : " << gates.size() << "  ";
    file << "Cycles : " << num_cycles << "\n\n";
    
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
    
    file.close();
}

void circuit::
PrintProbabilities(const string &out_file)
{
    static int count = 0;
    ofstream file;
    file.open(out_file + to_string(count) + ".txt");
    
    for (auto state_v : circuit_state -> amp) {
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

void circuit::
PrintStateVector()
{
    for (auto state_v : circuit_state -> amp) {
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

void circuit::
PrintStateVector(const string& outfile)
{
    static ofstream file;
    file.open(outfile + ".txt");
    
    for (auto state_v : circuit_state -> amp) {
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

state::
state():amp({}),
        apply_gate({}), indices_for_ag({}) {}

state::
state(const state& rhs)
{
    amp = rhs.amp;
    apply_gate = rhs.apply_gate;
    indices_for_ag = rhs.indices_for_ag;
}

state& state::
operator=(const state& rhs)
{
    state temp(rhs);
    swap(amp, temp.amp);
    swap(apply_gate, temp.apply_gate);
    swap(indices_for_ag, temp.indices_for_ag);
    return *this;
}

void state::
operator*(const gate& q_gate)
{
    if (!(apply_gate[0] == cmplx(0,0) && apply_gate[1] == cmplx(0,0))) {

        if (google &&
                q_gate.gate_identification.back() == gate::Gates::X_rotation) {
            amp[indices_for_ag[1]] = -cmplx(-imag(apply_gate[0]), real(apply_gate[0]))
                                            + apply_gate[1];
            amp[indices_for_ag[0]] = apply_gate[0] -
                                cmplx(-imag(apply_gate[1]), real(apply_gate[1]));
        }
        else if (google &&
                 q_gate.gate_identification.back() == gate::Gates::Y_rotation) {
            amp[indices_for_ag[1]] = apply_gate[0] + apply_gate[1];
            amp[indices_for_ag[0]] = apply_gate[0] - apply_gate[1];
        }
        else if (q_gate.gate_identification.back() == gate::Gates::Z ||
            q_gate.gate_identification.back() == gate::Gates::T) {
            
            for (index_size i = 0; i < indices_for_ag.size(); ++i) {
                amp[indices_for_ag[i]] = apply_gate[i] * q_gate.rows[i][i];
            }
        }
        else if (q_gate.gate_identification.back() == gate::Gates::X ||
                 q_gate.gate_identification.back() == gate::Gates::Y ) {
            short a_size = indices_for_ag.size();
            for (index_size i = 0; i < a_size; ++i) {
                amp[indices_for_ag[i]] = apply_gate[a_size - i + 1]
                * q_gate.rows[i][a_size - i + 1];
            }
        }
        else {
            apply_gate = matrix_v_mult(q_gate.rows, apply_gate);
            int c = 0;
            for(const auto& a : indices_for_ag) {
                amp[a] = apply_gate[c++];
            }
        }
    }
    apply_gate.clear();
    indices_for_ag.clear();
}

//add optimization for 2n/2 iterations
cmplx state::
measure_0(const short qubit)
{
    int m_bit = pow(2, ((sqrt(amp.size()) - 1) - qubit));
    map<int, cmplx> measurement_0;
    cmplx prob_0 = 0;
    
    for (index_size i = 0; i < amp.size() ; ++i) {
        if ((i & m_bit) != m_bit) {
            measurement_0[i] = (amp[i] * conj(amp[i]));
        }
        else {
            i += pow(2, (sqrt(amp.size()) - 1) - qubit);
        }
    }
    
    for (auto m : measurement_0) {
        prob_0 += m.second;
    }
    
    prob_0 = cmplx(1) / sqrt(prob_0);
    for (auto m : measurement_0) {
        amp[m.first] = prob_0 * amp[m.first];
    }
    
    return prob_0;
}

cmplx state::
measure_1(const short qubit)
{
    int m_bit = pow(2, ((sqrt(amp.size()) - 1) - qubit));
    map<int, cmplx> measurement_1;
    cmplx prob_1 = 0;
    
    for (index_size i = 0; i < amp.size() ; ++i) {
        if ((i & m_bit) == m_bit) {
            measurement_1[i] = (amp[i] * conj(amp[i]));
        }
        else {
            i += pow(2, ((sqrt(amp.size()) - 1) - qubit));
        }
    }
    
    for (auto m : measurement_1) {
        prob_1 += m.second;
    }
    
    prob_1 = cmplx(1) / sqrt(prob_1);
    for (auto m : measurement_1) {
        amp[m.first] = prob_1 * amp[m.first];
    }
    
    return prob_1;
}

void state::
measure(const short qubit)
{
    int m_bit = pow(2, ((sqrt(amp.size()) - 1) - qubit));
    map<int, cmplx> measurement_0;
    cmplx prob_0 = 0;
    map<int, cmplx> measurement_1;
    cmplx prob_1 = 0;
    
    for (index_size i = 0; i < amp.size() ; ++i) {
        if ((i & m_bit) == m_bit) {
            measurement_1[i] = (amp[i] * conj(amp[i]));
        }
        else {
            measurement_0[i] = (amp[i] * conj(amp[i]));
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
        amp[m.first] = prob_0 * amp[m.first];
    }
    
    prob_1 = cmplx(1) / sqrt(prob_1);
    for (auto m : measurement_1) {
        amp[m.first] = prob_1 * amp[m.first];
    }
}



