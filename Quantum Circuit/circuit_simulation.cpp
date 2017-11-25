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

vector<double> elapsed_secs;
bool google = false;
float H_related_gates = 0;

function<void(int)> timing_and_probability;

gate::gate(): rows({}), qubits({}), gate_identification({}),
            theta({}), num_controls(0){}

gate::gate(vector<vector<cmplx>> g): rows(g), qubits({}),
    gate_identification({}),theta({}), num_controls(0){}

gate::gate(const gate& rhs)
{
    rows = rhs.rows;
    qubits = rhs.qubits;
    gate_identification = rhs.gate_identification;
    theta = rhs.theta;
    num_controls = rhs.num_controls;
}

gate& gate::operator=(const gate& rhs)
{
    gate temp(rhs);
    swap(rows, temp.rows);
    swap(qubits, temp.qubits);
    swap(gate_identification, temp.gate_identification);
    swap(theta , temp.theta);
    swap(num_controls , temp.num_controls);
    return *this;
}

circuit::circuit_simulation::
circuit_simulation(const circuit& c, int gate_i): cir(c), gate_i(gate_i) {}

void circuit::circuit_simulation::
initialize_control(index_size& index, index_size c_bits)
{
    int count = 0;
    int c_iters = pow(2, cir.qubits - cir.gates[gate_i].num_controls);
    int transition = pow(2, abs(cir.gates[gate_i].qubits[0] - abs(cir.qubits - 1)));
    int to_transition = 0;
    while(count < c_iters) {
        if ((index & c_bits) == c_bits) {
            ++count;
            cir.circuit_state -> amp_apply_gate[index] = cir.circuit_state -> state_vector[index];
            ++index;
            ++to_transition;
        }
        else if (to_transition < transition) {
            ++to_transition;
            ++index;
        }
        else {
            to_transition = 0;
            index += pow(2, abs(cir.gates[gate_i].qubits[0] - abs(cir.qubits - 1)));
        }
    }
}

vector<index_size> circuit::circuit_simulation::
initialize_gate_set(int& num_X_T_gates)
{
    gate::Gates X = gate::Gates::X;
    gate::Gates P = gate::Gates::Phase;
    gate::Gates Z = gate::Gates::Z;
    gate::Gates T = gate::Gates::T;
    
    auto check_diag_gates =[&](int g_i) {
        return (cir.gates[g_i].gate_identification.back() == X ||
                cir.gates[g_i].gate_identification.back() == P ||
                cir.gates[g_i].gate_identification.back() == Z ||
                cir.gates[g_i].gate_identification.back() == T);
    };
    
    
    vector<index_size> bit_mask {};
    int g_i = gate_i;
    for(; g_i < cir.gates.size() && check_diag_gates(g_i); ++g_i ) {
        timing_and_probability(g_i);
        index_size bits = 0;
        if (cir.gates[g_i].gate_identification.back() == gate::Gates::X) {
            for (int j = 0 ; j < cir.gates[g_i].num_controls; ++j) {
                bits += pow(2, abs(cir.gates[g_i].qubits[j] - abs(cir.qubits - 1)));
            }
            ++num_X_T_gates;
        }
        else {
            for (int j = 0 ; j < cir.gates[g_i].qubits.size(); ++j) {
                bits += pow(2, abs(cir.gates[g_i].qubits[j] - abs(cir.qubits - 1)));
            }
            if (cir.gates[g_i].gate_identification.back() == gate::Gates::T) {
                ++num_X_T_gates;
            }
        }
        
        bit_mask.push_back(bits);
    }
    
    return bit_mask;
}

inline void circuit::circuit_simulation::
rand_sim(vector<bool>& iterated, int g_i, int num_bits, index_size index)
{
    cir.circuit_state -> apply_gate.push_back(cir.circuit_state -> state_vector[index]);
    cir.circuit_state -> indices_for_ag.push_back(index);
    
    index_size temp_index = 0;
    int target = cir.gates[g_i].num_controls;
    assert(abs(target-num_bits) == 1);
    
    for (int j = target ; j < num_bits; ++j) {
        temp_index = index + pow(2, abs(cir.gates[g_i].qubits[j] -
                                              abs(cir.qubits - 1)));
        assert(temp_index < cir.circuit_state -> state_vector.size());
        cir.circuit_state -> apply_gate.push_back(
                            cir.circuit_state -> state_vector[temp_index]);
        iterated[temp_index] = true;
        cir.circuit_state -> indices_for_ag.push_back(temp_index);
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
        
        for (int j = 0; j < c_size; ++j){
            
            if(cir.gates[gate_i + j].gate_identification.back() == gate::Gates::T) {
                if (iterated[index][iter_count] == false) {
                    iterated[index][iter_count] = true;
                    
                    index_size temp_index = index +
                    pow(2, abs(cir.gates[gate_i + j].qubits[0] - abs(cir.qubits - 1)));
                    
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
                            pow(2, abs(smallest_non_control_bit - abs(cir.qubits - 1)));
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
         iter_count < pow(2, abs(cir.qubits - num_controls)); ++index) {
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
    
    for (index_size index = 0; index < size && iter_count < size/pow(2,num_bits); ++index) {
        if (iterated[index] == false) {
            iterated[index] = true;
            iter_count += num_bits;
            
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
        timing_and_probability(gate_i + j);
        ++H_related_gates;
    }

    index_size size = cir.circuit_state -> state_vector.size();
    for (index_size index = 0; index < size; ++index) {
        cir.circuit_state -> state_vector[index] = 1;
    }

    return j;
}

circuit::circuit() : gates({}), qubit_to_gates({}), clock_cycles({}),
    circuit_state(new state()), qubits(0), circuit_preprocessed(false) {}

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
    clock_t begin = clock();
    int to_print = 0;
    int size = (int)gates.size();
    
    circuit_simulation sim (*this, 0);
    
    for (int i = 0; i < size; ++i) {
        
        timing_and_probability = [&](int j) {
            if (google && j == clock_cycles[to_print] ) {
                clock_t end = clock();
                double secs = double(end - begin) / CLOCKS_PER_SEC;
                elapsed_secs.push_back(secs);
                cout << secs << " secs for cycle " << to_print << "\n";
                ++to_print;
//                if (to_print % 20 == 0) {
//                    print_probabilities(outfile);
//                }
                begin = clock();
            }
        };

        timing_and_probability(i);
        
        sim.gate_i = i;
        circuit_state -> apply_gate.clear();
        circuit_state -> indices_for_ag.clear();
        
        if(gates[i].qubits.size() > 1 &&
           gates[i].gate_identification[0] != gate::Gates::Control) {
            sort(gates[i].qubits.begin(), gates[i].qubits.end());
        }
        
        if(gates[i].gate_identification[0] == gate::Gates::Control ||
           gates[i].gate_identification[0] == gate::Gates::T) {
            
            if (gates[i].gate_identification[1] == gate::Gates::X ||
                gates[i].gate_identification[0] == gate::Gates::T ||
                gates[i].gate_identification[1] == gate::Gates::Z ||
                gates[i].gate_identification[1] == gate::Gates::Phase) {
                
                index_size index = pow(2, abs(gates[i].qubits[0] - abs(qubits - 1)));
                int XT_gates = 0;
                vector<index_size> bit_mask = sim.initialize_gate_set(XT_gates);
                
                sim.block_gate_opt(bit_mask, XT_gates);
                i += (int)bit_mask.size() - 1;
            }
            else {
                index_size c_bits = 0;
                for (int j = 0 ; j < gates[i].num_controls; ++j) {
                    c_bits += pow(2, abs(gates[i].qubits[j] - abs(qubits - 1)));
                }
                sim.control_rand_sim(c_bits);
            }
        }
        else if (gates[i].gate_identification[0] == gate::Gates::Measurement) {
            circuit_state -> measure(gates[i].qubits[0]);
        }
        else {
            if(google && gates[i].gate_identification[0] == gate::Gates::Hadamard) {
                int increment = sim.H_google_opt();
                i += increment - 1;
            }
            else {
                sim.non_control_sim();
             }
        }
    }
    
    clock_t end = clock();
    double secs = double(end - begin) / CLOCKS_PER_SEC;
    elapsed_secs.push_back(secs);
    cout << secs << " secs for cycle " << to_print << "\n";
    
    double sum = 0;
    for (auto i : elapsed_secs) {
        sum += i;
    }
    cout << "Total Time: " << sum << " secs\n";
    
    if (qubits < 10) {
        print_state();
    }
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
        state_v *= cmplx(pow(HADAMARD_CONST, H_related_gates));
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

state::state():state_vector({}), amp_apply_gate({}),
        apply_gate({}), indices_for_ag({}) {}

state::state(const state& rhs)
{
    state_vector = rhs.state_vector;
    amp_apply_gate = rhs.amp_apply_gate;
    apply_gate = rhs.apply_gate;
    indices_for_ag = rhs.indices_for_ag;
}

state& state::operator=(const state& rhs)
{
    state temp(rhs);
    swap(state_vector, temp.state_vector);
    swap(amp_apply_gate, temp.amp_apply_gate);
    swap(apply_gate, temp.apply_gate);
    swap(indices_for_ag, temp.indices_for_ag);
    return *this;
}

void state::operator*(const gate& q_gate)
{
    if (!(apply_gate[0] == cmplx(0,0) && apply_gate[1] == cmplx(0,0))) {
        assert(apply_gate.size() == 2);
        
        if (q_gate.gate_identification[0] == gate::Gates::Z ||
            q_gate.gate_identification[0] == gate::Gates::T) {

            for (int i = 0; i < apply_gate.size(); ++i) {
                apply_gate[i] = apply_gate[i] * q_gate.rows[i][i];
            }
        }
        else if (q_gate.gate_identification[0] == gate::Gates::X ||
                 q_gate.gate_identification[0] == gate::Gates::Y ) {

            auto temp_v = apply_gate;
            short a_size = apply_gate.size();
            for (int i = 0; i < a_size; ++i) {
                apply_gate[i] = temp_v[a_size - i + 1] * q_gate.rows[i][a_size - i + 1];
            }
        }
        else if (google &&
                q_gate.gate_identification[0] == gate::Gates::X_rotation) {

            auto temp_v = apply_gate[0];
            temp_v = apply_gate[0] - cmplx(-imag(apply_gate[1]),
                                                  real(apply_gate[1]));
            apply_gate[1] = -cmplx(-imag(apply_gate[0]), real(apply_gate[0])) + apply_gate[1];
            apply_gate[0] = temp_v;
        }
        else if (google &&
                 q_gate.gate_identification[0] == gate::Gates::Y_rotation) {

            auto temp_v = apply_gate[0];
            temp_v = apply_gate[0] - apply_gate[1];
            apply_gate[1] = apply_gate[0] + apply_gate[1];
            apply_gate[0] = temp_v;
        }
        else {
            apply_gate = matrix_v_mult(q_gate.rows, apply_gate);
        }
        
        int c = 0;
        for(const auto& a : indices_for_ag) {
            state_vector[a] = apply_gate[c++];
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



