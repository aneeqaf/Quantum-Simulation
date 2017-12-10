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


function<void(index_size)> increment_cycle;

circuit::
circuit() : gates({}),clock_cycles({}),
circuit_state(new state()), merged(0), X(0), Y(0), CZ_T(0),
qubits(0), num_cycles(0), current_google_cycle(0), g_begin(0),
g_end(0), google(false), global_factor_power(0)
{
    gate_time.resize(5,0);
}

circuit::
circuit(const circuit& g)
{
    qubits = g.qubits;
    circuit_state -> amp = g.circuit_state -> amp;
    gates = g.gates;
    clock_cycles = g.clock_cycles;
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

template<typename function>
pair<index_size, index_size>* circuit::
FormBlockOfGates(const vector<gate>& block_gates,
                 index_size& gate_i,
                 vector<index_size>& T_bit_mask,
                 function comp)
{
    pair<index_size, index_size>* qubits_CZ_bitmask =  new pair<index_size, index_size>[64];
    
    for (int i = 0; i < 64; ++i)
        qubits_CZ_bitmask[i] = pair<index_size, index_size>(0, 0);
    
    for(;(gate_i < gates.size()) && (comp(gate_i)); ++gate_i) {
        
        index_size bits = 0;
        increment_cycle(gate_i);

        if (block_gates[gate_i].gate_identification.back() == gate::Gates::Z) {
            
            for (auto q : block_gates[gate_i].qubits)
                bits |= ( 1ull << ((qubits - 1) - q));
            
            for (auto q : block_gates[gate_i].qubits)
                if (qubits_CZ_bitmask[q].first == 0)
                    qubits_CZ_bitmask[q].first = bits;
                else
                    qubits_CZ_bitmask[q].second = bits;
            
        }
        else if (block_gates[gate_i].gate_identification.back() == gate::Gates::T) {
            index_size t_mask = (1ull << ((qubits - 1) - block_gates[gate_i].qubits[0]));
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
    }
    
    return qubits_CZ_bitmask;
}

void circuit::
ApplyBlockOfGates(const pair<index_size, index_size>* cbits,
                  const index_size cbits_size,
                  vector<cmplx>& amp,
                  const vector<index_size>& T_bit_mask)
{
    index_size size =  amp.size();
    const int qubits = log2(size);
    
    bool t_bit_mask_empty = true;
    if (T_bit_mask.front() != 0)
        t_bit_mask_empty = false;
    
    index_size prev_gc = 0;
    index_size prev_set_bit = 0;
    index_size contributions = 0;
    
    auto rescale = [&](const index_size gc,
                       const cmplx& mutated_amp) {
        if (global_factor_power % 2 == 1)
            amp[gc] = mutated_amp
            / (cmplx(pow(2,(global_factor_power/2))) * cmplx(sqrt(2)));
        else
            amp[gc] = mutated_amp / cmplx(pow(2,(global_factor_power/2)));
    };
    
    if (global_factor_power > 100)
        rescale (0, amp[0]);
    for (index_size count = 1; count < size ; ++count) {
        bool gate_applied = false;
        
        index_size gc = count ^ (count >> 1);
        index_size changed_bit = gc ^ prev_gc;
        index_size set_bit = (qubits - 1) - ((changed_bit == 0) ? 0 : log2(changed_bit));
        bool negate_Z = false;
       
        if(cbits[set_bit].first != 0
           &&(cbits[set_bit].first & prev_gc) != (cbits[set_bit].first & gc)) {
            if ((cbits[set_bit].first & gc) == cbits[set_bit].first)
                ++contributions;
            else if ((cbits[set_bit].first & prev_gc) == cbits[set_bit].first
                     && contributions != 0)
                --contributions;
        }
        if(cbits[set_bit].second != 0
           && (cbits[set_bit].second & prev_gc) != (cbits[set_bit].second & gc)) {
            if ( (cbits[set_bit].second & gc) == cbits[set_bit].second)
                ++contributions;
            else if ((cbits[set_bit].second & prev_gc) == cbits[set_bit].second
                     && contributions != 0)
                --contributions;
        }
        
        if (contributions % 2 == 1) {
            negate_Z = !negate_Z;
            gate_applied = true;
        }
    
        cmplx mutated_amp = amp[gc];
        if (!t_bit_mask_empty && amp[gc] != cmplx(0,0)) {
            index_size gate_c = 0;
            gate_applied = true;
            
            for (auto t : T_bit_mask)
                gate_c += __builtin_popcountll(gc & t);
            
            int k = gate_c & ((1u << 3) - 1);
            if ( k > 0) {
                if (negate_Z)
                   k = k + 4 ;
                
                mutated_amp = ApplyTGateKTimes(k, amp[gc]);
            }
            else if (negate_Z)
                mutated_amp = -mutated_amp;
        }
        else if (negate_Z)
            mutated_amp = -mutated_amp;
        
        if (global_factor_power > 100)
            rescale(gc, mutated_amp);
        else if (gate_applied)
            amp[gc] = mutated_amp;
        
        prev_gc = gc;
        prev_set_bit = set_bit;
    }
    if (global_factor_power > 100)
        global_factor_power = 0;
}


inline void circuit::
FindRelevantAmp(vector<index_size>& indices_amp_gate,
                const index_size gate_i,
                const int num_bits,
                const index_size idx)
{
    indices_amp_gate.push_back(idx);
    
    index_size temp_index = 0;
    int c1 = 1;
    int target = gates[gate_i].num_controls;
    
    for (index_size i = target ; i < num_bits; ++i) {
        short relative_pos = 1;
        short j = 0;
        for (index_size n = 0; n < indices_amp_gate.size() &&
             indices_amp_gate.size() < (1ull << (i+1)); n += 2) {
            
            temp_index = indices_amp_gate[n] +
                    (1ull << ((qubits - 1) - gates[gate_i].qubits[i]));
            
            assert(temp_index < circuit_state -> amp.size());
            
            indices_amp_gate.insert(indices_amp_gate.begin()
                                + j + relative_pos, temp_index);
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
    int num_bits = (int)gates[gate_i].qubits.size();
    index_size iter_count = 0;
    int num_controls = gates[gate_i].num_controls;
    
    index_size gate_bitmask = 0;
    for (index_size i = 0; i < gates[gate_i].qubits.size(); ++i)
        gate_bitmask |= (1ull << ((qubits - 1) - gates[gate_i].qubits[i]));
    
    vector<index_size> indices_amp_gates;
    FindRelevantAmp(indices_amp_gates, gate_i, num_bits, 0);
    index_size idx = 0;
    index_size iag_size = indices_amp_gates.size();
    
    while(iter_count < (1 << (qubits - num_controls))) {
        if ((idx & gate_bitmask) == 0) {
            if((idx & c_bits) == c_bits) {
                iter_count += 2;
                
                index_size temp_amp_gates[iag_size];
                for (index_size i = 0; i < iag_size; ++i)
                    temp_amp_gates[i] = indices_amp_gates[i] + idx;
                
                ApplyGateOnGateSizeAmps(temp_amp_gates, iag_size, gates[gate_i]);
            }
            ++idx;
        }
        else
            idx += (idx & gate_bitmask);
    }
}

inline void circuit::
ApplyNonControlGates(const index_size gate_i)
{
    int num_bits = (int)gates[gate_i].qubits.size();
    index_size iter_count = 0;
    index_size size = circuit_state -> amp.size();
    
    index_size gate_bitmask = 0;
    for (index_size i = 0; i < gates[gate_i].qubits.size(); ++i)
        gate_bitmask |= (1ull << ((qubits - 1) - gates[gate_i].qubits[i]));

    vector<index_size> indices_amp_gates;
    FindRelevantAmp(indices_amp_gates, gate_i, num_bits, 0);
    index_size idx = 0;
    index_size iag_size = indices_amp_gates.size();
    
    while(iter_count < (size/(1 << num_bits))) {
        if ((idx & gate_bitmask) == 0) {
            ++iter_count;
            
            index_size temp_amp_gates[iag_size];
            for (index_size i = 0; i < iag_size; ++i)
                temp_amp_gates[i] = indices_amp_gates[i] + idx;
            
            ApplyGateOnGateSizeAmps(temp_amp_gates, iag_size, gates[gate_i]);
            ++idx;
        }
        else
            idx += (idx & gate_bitmask);
    }
    if (google) {
        global_factor_power += 2;
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

inline cmplx circuit::
ApplyPhaseGate(const index_size i_count,
               const cmplx& amp)
{
    switch (i_count % 4) {
        case 1: {
            return cmplx(-imag(amp), real(amp));
            break;
        }
        case 2: {
            return -cmplx(real(amp), imag(amp));
            break;
        }
        case 3: {
            return -cmplx(-imag(amp), real(amp));
            break;
        }
        default:
            return amp;
            break;
    }
    return amp;
}

inline cmplx circuit::
ApplyTGateKTimes(const index_size gate_c,
                 const cmplx& amp)
{
    switch (gate_c % 8) {
        case 1: {
            //0.707106781 + 0.707106781 i
            return HADAMARD_CONST * cmplx(real(amp) - imag(amp),
                  real(amp) + imag(amp));
            break;
        }
        case 2: {
            //i
            return cmplx(-imag(amp), real(amp));
            break;
        }
        case 3: {
            //-0.707106781 + 0.707106781 i
            return HADAMARD_CONST * cmplx(-real(amp) - imag(amp),
                  real(amp) - imag(amp));
            break;
        }
        case 4: {
            //-1
            return cmplx(-real(amp), -imag(amp));
            break;
        }
        case 5: {
            //-0.707106781 - 0.707106781 i
            return HADAMARD_CONST * cmplx(-real(amp) + imag(amp),
                  -real(amp) -imag(amp));
            break;
        }
        case 6: {
            //- i
            return cmplx(imag(amp), -real(amp));
            break;
        }
        case 7: {
            //0.707106781 - 0.707106781 i
            return HADAMARD_CONST * cmplx(real(amp) + imag(amp),
                  - real(amp) + imag(amp));
            break;
        }
        default:
            return amp;
            break;
    }
    return amp;
}

inline int circuit::
ApplyHOnAllAmp(const index_size gate_i)
{
    int j = 0;
    for(; j < gates.size() &&
        gates[gate_i + j].gate_identification.back() == gate::Gates::Hadamard; ++j )
        ++global_factor_power;

    index_size size = circuit_state -> amp.size();
    for (index_size idx = 0; idx < size; ++idx)
        circuit_state -> amp[idx] = 1;

    return j;
}

inline void circuit::
ApplyXX12Gate(const index_size* const indices)
{
    auto& temp_v = circuit_state -> amp;

    auto t = temp_v[indices[0]] + temp_v[indices[3]];
    auto t1 = temp_v[indices[1]] + temp_v[indices[2]];
    auto t2 = cmplx(-imag(temp_v[indices[0]]), real(temp_v[indices[0]]))
            - cmplx(-imag(temp_v[indices[3]]), real(temp_v[indices[3]]));
    auto t3 = cmplx(-imag(temp_v[indices[1]]), real(temp_v[indices[1]]))
            - cmplx(-imag(temp_v[indices[2]]), real(temp_v[indices[2]]));
    
    temp_v[indices[0]] = t1 + t2;
    temp_v[indices[1]] = t + t3;
    temp_v[indices[2]] = t - t3;
    temp_v[indices[3]] = t1 - t2 ;
}

inline void circuit::
ApplyXY12Gate(const index_size* const indices,
              const index_size size)
{
    cmplx ap_gate[size];
    auto& temp_v = circuit_state -> amp;
    for (index_size i = 0; i < size; ++i)
        ap_gate[i] = temp_v[indices[i]];
    
    temp_v[indices[0]] = cmplx(-imag(ap_gate[0]), real(ap_gate[0]))
                        - cmplx(-imag(ap_gate[1]), real(ap_gate[1]))
                        + ap_gate[2] - ap_gate[3] ;
    
    temp_v[indices[1]] = cmplx(-imag(ap_gate[0]), real(ap_gate[0]))
                        + cmplx(-imag(ap_gate[1]), real(ap_gate[1]))
                        + ap_gate[2] + ap_gate[3] ;
    
    temp_v[indices[2]] = ap_gate[0] - ap_gate[1]
                        + cmplx(-imag(ap_gate[2]), real(ap_gate[2]))
                        - cmplx(-imag(ap_gate[3]), real(ap_gate[3])) ;
    
    temp_v[indices[3]] = ap_gate[0] + ap_gate[1]
                        + cmplx(-imag(ap_gate[2]), real(ap_gate[2]))
                        + cmplx(-imag(ap_gate[3]), real(ap_gate[3]));
}

inline void circuit::
ApplyYY12Gate(const index_size* const indices)
{
    auto& temp_v = circuit_state -> amp;
    
    auto t = cmplx(-imag(temp_v[indices[0]]), real(temp_v[indices[0]]))
            + cmplx(-imag(temp_v[indices[3]]), real(temp_v[indices[3]]));
    auto t1 = cmplx(-imag(temp_v[indices[0]]), real(temp_v[indices[0]]))
            - cmplx(-imag(temp_v[indices[3]]), real(temp_v[indices[3]]));
    auto t2 = cmplx(-imag(temp_v[indices[1]]), real(temp_v[indices[1]]))
            + cmplx(-imag(temp_v[indices[2]]), real(temp_v[indices[2]]));
    auto t3 = cmplx(-imag(temp_v[indices[1]]), real(temp_v[indices[1]]))
            - cmplx(-imag(temp_v[indices[2]]), real(temp_v[indices[2]]));
    
    temp_v[indices[0]] = t - t2;
    temp_v[indices[1]] = t1 + t3;
    temp_v[indices[2]] = t1 - t3;
    temp_v[indices[3]] = t + t2;
}

inline void circuit::
ApplyYX12Gate(const index_size* const indices)
{
    auto& temp_v = circuit_state -> amp;
    
    auto t = cmplx(-imag(temp_v[indices[0]]), real(temp_v[indices[0]]))
            + temp_v[indices[1]];
    auto t1 = temp_v[indices[0]]
            + cmplx(-imag(temp_v[indices[1]]), real(temp_v[indices[1]]));
    auto t2 = cmplx(-imag(temp_v[indices[2]]), real(temp_v[indices[2]]))
            + temp_v[indices[3]];
    auto t3 = temp_v[indices[2]]
            + cmplx(-imag(temp_v[indices[3]]), real(temp_v[indices[3]]));
    
    temp_v[indices[0]] = t - t2;
    temp_v[indices[1]] = t1 - t3;
    temp_v[indices[2]] = t + t2;
    temp_v[indices[3]] = t1 + t3;
}

void circuit::
Merge2QXY12Gates(index_size gate_i)
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

    if(gates[a].gate_identification.back() == gate::Gates::X_1_2 &&
       gates[b].gate_identification.back() == gate::Gates::X_1_2)
        ApplyMergedXY12Gates(0, gate_i);
    
    else if(gates[a].gate_identification.back() == gate::Gates::X_1_2 &&
       gates[b].gate_identification.back() == gate::Gates::Y_1_2)
        ApplyMergedXY12Gates(1, gate_i);
    
    else if(gates[a].gate_identification.back() == gate::Gates::Y_1_2 &&
       gates[b].gate_identification.back() == gate::Gates::X_1_2)
        ApplyMergedXY12Gates(2, gate_i);
    
    else if(gates[a].gate_identification.back() == gate::Gates::Y_1_2 &&
       gates[b].gate_identification.back() == gate::Gates::Y_1_2)
        ApplyMergedXY12Gates(3, gate_i);
}

void circuit::
ApplyMergedXY12Gates(const short type,
                   const index_size gate_i)
{
    index_size gate_bitmask = 0;
    for (index_size i = 0; i < gates[gate_i].qubits.size(); ++i)
        gate_bitmask |= (1ull << ((qubits - 1) - gates[gate_i].qubits[i]));

    int num_bits = (int)gates[gate_i].qubits.size();
    index_size iter_count = 0;
    index_size size = circuit_state -> amp.size();
    
    vector<index_size> indices_amp_gates;
    FindRelevantAmp(indices_amp_gates, gate_i, num_bits, 0);
    index_size idx = 0;
    
    while(iter_count < size/(1 << num_bits)) {
        if ((idx & gate_bitmask) == 0) {
            ++iter_count;
            
            index_size temp_amp_gates[indices_amp_gates.size()];
            for (index_size i = 0; i < indices_amp_gates.size(); ++i)
                temp_amp_gates[i] = indices_amp_gates[i] + idx;
            
            switch (type) {
                case 0: {
                    ApplyXX12Gate(temp_amp_gates);
                    break;
                }
                case 1: {
                    ApplyXY12Gate(temp_amp_gates, indices_amp_gates.size());
                    break;
                }
                case 2: {
                    ApplyYX12Gate(temp_amp_gates);
                    break;
                }
                case 3: {
                    ApplyYY12Gate(temp_amp_gates);
                    break;
                }
                default: {
                    throw "Invalid XY merge\n";
                    break;
                }
            }
            ++idx;
        }
        else
            idx += (idx & gate_bitmask);
            
    }
    global_factor_power += 2;
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
ApplySingleCPhaseGate(const index_size gate_i)
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
                gates[g_i + k].gate_identification.back() == gate::Gates::Z) {
                
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
}

void circuit::
GroupSimilarGates()
{
    int last_CZ = 0, last_T = 0, last_X = 0, last_Y = 0;
    bool saw_CZ = false, saw_T = false, saw_X = false, saw_Y = false;
    int g_i = qubits;
        
    for (int j = qubits; j < gates.size()
         && (g_i + last_Y + last_X + last_T + last_CZ) < gates.size(); ++j) {
            
        if (gates[j].gate_identification.back() == gate::Gates::Z) {
            if(saw_Y || saw_X) {
                g_i = j;
                last_CZ = 0; last_T = 0; last_X = 0; last_Y = 0;
                saw_CZ = false; saw_T = false; saw_X = false; saw_Y = false;
            }
            saw_CZ = true;
            if (saw_T || saw_X || saw_Y)
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
        else if (gates[j].gate_identification.back() == gate::Gates::X_1_2) {
            saw_X = true;
            if (saw_T || saw_CZ || saw_Y)
                swap(gates[g_i + last_X + last_T + last_CZ], gates[j]);
            
            ++last_X;
        }
        else if (gates[j].gate_identification.back() == gate::Gates::Y_1_2) {
            saw_Y = true;
            if (saw_T || saw_X || saw_CZ)
                swap(gates[g_i + last_Y + last_X + last_T + last_CZ], gates[j]);
            
            ++last_Y;
        }
    }
}

float circuit::
CalculateNormOfAmp()
{
    double norm = 0;
    for (auto& state_v : circuit_state -> amp) {
        state_v /= pow(2,(global_factor_power/2));
        if (global_factor_power % 2 == 1)
            state_v /= sqrt(2);
        state_v *= conj(state_v);
        if (real(state_v) > (1/(1ull << qubits)))
            norm += real(state_v);
    }
    
    for (auto state_v : circuit_state -> amp) {
        state_v *= conj(state_v);
        if (real(state_v) <= (1/(1ull << qubits)))
            norm += real(state_v);
    }
    return norm;
}

void circuit::
ApplyGateOnGateSizeAmps(const index_size* indices,
                        const index_size size,
                        const gate& q_gate)
{
    auto& amp = circuit_state -> amp;
    cmplx temp_amp[size] ;
    for (index_size i = 0; i < size; ++i)
        temp_amp[i] = amp[indices[i]];
    
    if (!(amp[indices[0]] == cmplx(0,0) && amp[indices[1]] == cmplx(0,0))) {
        
        if (q_gate.gate_identification.back() == gate::Gates::X_1_2) {
            amp[indices[0]] =  cmplx(real(temp_amp[0]) - imag(temp_amp[0]),
                                     real(temp_amp[0]) + imag(temp_amp[0]))
                            + cmplx(real(temp_amp[1]) + imag(temp_amp[1]),
                                    -real(temp_amp[1]) + imag(temp_amp[1]));
            amp[indices[1]] =  cmplx(real(temp_amp[0]) + imag(temp_amp[0]),
                                     -real(temp_amp[0]) + imag(temp_amp[0]))
                            + cmplx(real(temp_amp[1]) - imag(temp_amp[1]),
                                    real(temp_amp[1]) + imag(temp_amp[1]));
        }
        else if (q_gate.gate_identification.back() == gate::Gates::Y_1_2) {
            amp[indices[0]] =  cmplx(real(temp_amp[0]) - imag(temp_amp[0]),
                                     real(temp_amp[0]) + imag(temp_amp[0]))
                            + cmplx(-real(temp_amp[1]) + imag(temp_amp[1]),
                                    -real(temp_amp[1]) - imag(temp_amp[1]));
            amp[indices[1]] =  cmplx(real(temp_amp[0]) - imag(temp_amp[0]),
                                     real(temp_amp[0]) + imag(temp_amp[0]))
                            + cmplx(real(temp_amp[1]) - imag(temp_amp[1]),
                                    real(temp_amp[1]) + imag(temp_amp[1]));
        }
        else if (q_gate.gate_identification.back() == gate::Gates::Z ||
                 q_gate.gate_identification.back() == gate::Gates::T) {
            
            for (index_size i = 0; i < size; ++i) {
                amp[indices[i]] = temp_amp[i] * q_gate.rows[i][i];
            }
        }
        else if (q_gate.gate_identification.back() == gate::Gates::X ||
                 q_gate.gate_identification.back() == gate::Gates::Y ) {
            for (index_size i = 0; i < size; ++i) {
                amp[indices[i]] = temp_amp[size - i + 1]
                * q_gate.rows[i][size - i + 1];
            }
        }
        else {
            vector<cmplx> m_temp(temp_amp, temp_amp + sizeof(temp_amp) / sizeof(temp_amp[0]));
            m_temp = matrix_v_mult(q_gate.rows, m_temp);
            for (index_size i = 0; i < size; ++i)
                amp[i] = m_temp[i];
        }
    }
}

// TO DO: apply optimizations for diagonal gates, control Z, toffolli, and control X. 
void circuit::
Simulate(const string& outfile)
{
    int size = (int)gates.size();
    
    if (google) {
        if (!clock_cycles.empty())
            GroupAlternateCycles();
        GroupSimilarGates();
    }
    
    increment_cycle = [&](index_size g_i) {
        if (google && !clock_cycles.empty() && g_i == clock_cycles[current_google_cycle] )
            ++current_google_cycle;
    };
    
    clock_t begin = clock();
    for (index_size i = 0; i < size; ++i) {
        
        g_begin = clock();
        increment_cycle(i);
        
        if(gates[i].qubits.size() > 1 &&
           gates[i].gate_identification.front() != gate::Gates::Control)
            sort(gates[i].qubits.begin(), gates[i].qubits.end());
        
        //T bit mask won't work unless ordering of blocks is correct
        if(gates[i].gate_identification.front() == gate::Gates::Control ||
             gates[i].gate_identification.back() == gate::Gates::T) {
            
            auto comparator = [&](index_size g_i) {
                return (gates[g_i].gate_identification.back() == gate::Gates::X ||
                        gates[g_i].gate_identification.back() == gate::Gates::T ||
                        gates[g_i].gate_identification.back() == gate::Gates::Z ||
                        gates[g_i].gate_identification.back() == gate::Gates::Phase);
            };
                if (comparator(i)) {
                    vector<index_size> T_bit_mask(1, 0);
                    index_size gate_i = i;
                    
                    pair<index_size, index_size>* bit_mask = 
                                            FormBlockOfGates(gates ,i,
                                                T_bit_mask, comparator);
                    
                    ApplyBlockOfGates(bit_mask, i - gate_i, circuit_state -> amp, T_bit_mask);
                
                    i -= 1;
                    g_end = clock();
                    gate_time[1] += double(g_end - g_begin)/ CLOCKS_PER_SEC;
                    CZ_T += i - gate_i;
                    delete [] bit_mask;
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
                (gates[i + 1].gate_identification.back() == gate::Gates::X_1_2 ||
                gates[i + 1].gate_identification.back() == gate::Gates::Y_1_2)) {
                    Merge2QXY12Gates(i);
                    ++i;
                    g_end = clock();
                    gate_time[4] += double(g_end - g_begin)/ CLOCKS_PER_SEC;
                    merged++;
            }
            else if(google && gates[i].gate_identification.back() == gate::Gates::Hadamard)
            {
                int increment = ApplyHOnAllAmp(i);
                i += increment - 1;
                g_end = clock();
                gate_time[0] += double(g_end - g_begin)/ CLOCKS_PER_SEC;
            }
            else {
                ApplyNonControlGates(i);
                g_end = clock();
                
                if(gates[i].gate_identification.back() == gate::Gates::X_1_2){
                    gate_time[2] += double(g_end - g_begin)/ CLOCKS_PER_SEC;
                    X++;
                }
                else if(gates[i].gate_identification.back() == gate::Gates::Y_1_2) {
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
PrintGates()
{
    for (auto& g : gates) {
        if(g.gate_identification.back() == gate::Gates::Z)
            cout << "CZ ";
        else if (g.gate_identification.back() == gate::Gates::X_1_2)
            cout << "X ";
        else if (g.gate_identification.back() == gate::Gates::Y_1_2)
            cout << "Y ";
        else if (g.gate_identification.back() == gate::Gates::T)
            cout << "T ";
        else if (g.gate_identification.back() == gate::Gates::Hadamard)
            cout << "H ";
    }
    cout << "\n\n";
}
void circuit::
PrintGatesAndCycles()
{
    int j = 0;
    for (index_size i = 0; i < clock_cycles.size(); ++i) {
        cout << i << endl;

        for(;j < clock_cycles[i]; ++j) {
            auto& g = gates[j];
            if(g.gate_identification.back() == gate::Gates::Z)
                cout << "CZ ";
            else if (g.gate_identification.back() == gate::Gates::X_1_2)
                cout << "X ";
            else if (g.gate_identification.back() == gate::Gates::Y_1_2)
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
    time_t t = time(0);
    struct tm * now = localtime (&t);
    cout << "Executed on : "
         << (now->tm_mon + 1) << "/"
         <<  now->tm_mday << "/"
         << (now->tm_year + 1900) << " "
         <<  now->tm_hour << ":" << now->tm_min << ":"
         << std::setw(2) << std::setfill('0') << now->tm_sec
         <<"\n\n";
    
    cout << "Qubits : " << qubits << "  ";
    cout << "Gates : " << gates.size() << "  ";
    cout << "Cycles : " << num_cycles << "\n\n";
    
    double total_time = double(end - begin) / CLOCKS_PER_SEC;
    cout << setprecision(3);
    cout << "Total runtime : " << total_time << "s\n";
    cout << "Norm : " << CalculateNormOfAmp() << "\n\n";
    
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
    
    char hostname[20] = {};
    gethostname(hostname, 20);
    file << "Hostname : ";
    for(auto h : hostname) {
        file << h;
    }
    file << "\n";
    file << "Compiler : gcc " << __GNUC__  << "." << __GNUC_MINOR__ << "."
    <<  __GNUC_PATCHLEVEL__<< "\n";
    file << "Compiled on : " <<  __DATE__ << " " << __TIME__ << "\n";
    time_t t = time(0);
    struct tm * now = localtime (&t);
    file << "Executed on : "
    << (now->tm_mon + 1) << "/"
    <<  now->tm_mday << "/"
    << (now->tm_year + 1900) << " "
    <<  now->tm_hour << ":" << now->tm_min << ":"
    << std::setw(2) << std::setfill('0') << now->tm_sec
    <<"\n\n";
    
    file << "Qubits : " << qubits << "  ";
    file << "Cycles : " << num_cycles << "\n\n";
    
    double total_time = double(end - begin) / CLOCKS_PER_SEC;
    file << setprecision(3);
    file << "Total runtime : " << total_time << "s\n\n";
    
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
        state_v /= pow(2,(global_factor_power/2));
        if (global_factor_power % 2 == 1)
            state_v /= sqrt(2);
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
        state_v /= pow(2,(global_factor_power/2));
        if (global_factor_power % 2 == 1)
            state_v /= sqrt(2);
        
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
        state_v /= pow(2,(global_factor_power/2));
        if (global_factor_power % 2 == 1)
           state_v /= sqrt(2);
        
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
state():amp({}){}

state::
state(const state& rhs)
{
    amp = rhs.amp;
}

state& state::
operator=(const state& rhs)
{
    state temp(rhs);
    swap(amp, temp.amp);
    return *this;
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



