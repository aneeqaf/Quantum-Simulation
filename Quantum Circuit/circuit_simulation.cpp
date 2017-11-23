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

gate::gate(const gate& rhs) {
    rows = rhs.rows;
    qubits = rhs.qubits;
    gate_identification = rhs.gate_identification;
    theta = rhs.theta;
    num_controls = rhs.num_controls;
}

gate& gate::operator=(const gate& rhs){
    gate temp(rhs);
    swap(rows, temp.rows);
    swap(qubits, temp.qubits);
    swap(gate_identification, temp.gate_identification);
    swap(theta , temp.theta);
    swap(num_controls , temp.num_controls);
    return *this;
}

circuit::circuit_simulation::~circuit_simulation()
{
    cir = nullptr;
}

void circuit::circuit_simulation::initialize_control(index_size& index, int gate_i, index_size c_bits)
{
    int count = 0;
    int c_iters = pow(2, cir -> qubits - cir -> gates[gate_i] -> num_controls);
    int transition = pow(2, abs(cir -> gates[gate_i] -> qubits[0] - abs(cir -> qubits - 1)));
    int to_transition = 0;
    while(count < c_iters) {
        if ((index & c_bits) == c_bits) {
            ++count;
            cir -> circuit_state -> amp_apply_gate[index] = cir -> circuit_state -> state_vector[index];
            ++index;
            ++to_transition;
        }
        else if (to_transition < transition) {
            ++to_transition;
            ++index;
        }
        else {
            to_transition = 0;
            index += pow(2, abs(cir -> gates[gate_i] -> qubits[0] - abs(cir -> qubits - 1)));
        }
    }
}

template <typename function>
vector<index_size> circuit::circuit_simulation::initialize_gate_set(index_size& index, int& gate_i,
                                                                gate::Gates I, function& tp)
{
    vector<index_size> bit_mask;
    for(; gate_i < cir -> gates.size() &&
        cir -> gates[gate_i] -> gate_identification.front() == gate::Gates::Control &&
        cir -> gates[gate_i] -> gate_identification.back() == I; ++gate_i ) {
        tp();
        index_size bits = 0;
        if (I == gate::Gates::X) {
            for (int j = 0 ; j < cir -> gates[gate_i] -> num_controls; ++j) {
                bits += pow(2, abs(cir -> gates[gate_i] -> qubits[j] - abs(cir -> qubits - 1)));
            }
        }
        else {
            for (int j = 0 ; j < cir -> gates[gate_i] -> qubits.size(); ++j) {
                bits += pow(2, abs(cir -> gates[gate_i] -> qubits[j] - abs(cir -> qubits - 1)));
            }
        }
        
        bit_mask.push_back(bits);
    }
    if (gate_i < cir -> gates.size()) {
        --gate_i;
    }
    return bit_mask;
}

void circuit::circuit_simulation::rand_sim(vector<bool>& iterated, int gate_i,
                                           int num_bits, index_size index)
{
    cir -> circuit_state -> amp_apply_gate[index] =
                cir -> circuit_state -> state_vector[index];
    
    index_size temp_index = 0;
    for (int j = 0 ; j < num_bits; ++j) {
        map<index_size, cmplx> temp;
        for (const auto& a : cir -> circuit_state -> amp_apply_gate) {
            temp_index = a.first + pow(2, abs(cir -> gates[gate_i] -> qubits[j] -
                                              abs(cir -> qubits - 1)));
            temp[temp_index] = cir -> circuit_state -> state_vector[temp_index];
            iterated[temp_index] = true;
        }
        cir -> circuit_state -> amp_apply_gate.insert(temp.begin(), temp.end());
    }
}

template<typename function>
void circuit::circuit_simulation::block_gate_opt(vector<index_size>& cbits, int gate_i,
                                              function& gate_specific_opt)
{
    auto find_smallest_non_control = [&](int j) {
        for(int cq = 0; cq < cir -> gates[gate_i + j] -> num_controls; ++cq) {
            if (cq != cir -> gates[gate_i + j] -> qubits[cq]) {
                return cq;
            }
        }
        return -1;
    };
    
    vector<bool> iterated(cir -> circuit_state -> state_vector.size(), false) ;
    
    index_size size =  cir -> circuit_state -> state_vector.size();
    
    for (index_size index = 0; index < size ; ++index) {
        int c_size = (int)cbits.size();
        
        for (int j = 0; j < c_size; ++j){
            if((index & cbits[j]) == cbits[j]) {
                if (cir -> gates[gate_i + j] -> gate_identification.back() == gate::Gates::X) {
                    if (iterated[index] == false) {
                        iterated[index] = true;
                        
                        int smallest_non_control_bit = find_smallest_non_control(j);
                        if (smallest_non_control_bit == -1) {
                            continue;
                        }
                        
                        index_size swap_index = index +
                            pow(2, abs(smallest_non_control_bit - abs(cir -> qubits - 1)));
                        iterated[swap_index] = true;
                        
                        if (!(cir -> circuit_state -> state_vector[index] == cmplx(0,0) &&
                              cir -> circuit_state -> state_vector[swap_index] == cmplx(0,0)))
                        {
                            gate_specific_opt(swap_index);
                        }
                    }
                }
                else {
                    if (!(cir -> circuit_state -> state_vector[index] == cmplx(0,0))) {
                        gate_specific_opt(index);
                    }
                }
            }
        }
    }
}

void circuit::circuit_simulation::control_rand_sim(int gate_i, int num_bits, index_size c_bits)
{
    vector<bool> iterated(cir -> circuit_state -> state_vector.size(), false);
    index_size iter_count = 0;
    index_size size = cir -> circuit_state -> state_vector.size();
    for (index_size index = 0; iter_count < size/pow(2,num_bits); ++index) {
        
        if (iterated[index] == false) {
            iterated[index] = true;
            iter_count += num_bits;
            
            if((index & c_bits) == c_bits) {
                rand_sim(iterated, gate_i, num_bits - cir -> gates[gate_i] -> num_controls, index);
                (*cir -> circuit_state) * cir -> gates[gate_i];
            }
            else if (cir -> circuit_preprocessed) {
                (*cir -> circuit_state) * cir -> gates[gate_i+1];
            }
        }
    }
}

template <typename Iterator>
inline Iterator circuit::circuit_simulation::find_control_target(vector<bool>& iterated,
                                                         Iterator iter, int c, index_size t_bits)
{
    auto amp = cir -> circuit_state -> amp_apply_gate;
    auto iter_t = iter;
    ++iter_t;
    int count = c + 1;
    for (; iter_t != amp.end(); ++iter_t) {
        if(iterated[count] == false) {
            if ((iter -> first & t_bits) != (iter_t -> first & t_bits)) {
                iterated[count] = true;
                return iter_t;
            }
        }
        ++count;
    }
    return iter_t;
}


circuit::circuit() : qubits(0), google(false), circuit_preprocessed(false)
{
    circuit_state = new state();
    gates = {};
    clock_cycles = {};
}

circuit::~circuit()
{
    delete circuit_state;
}

circuit::circuit(const circuit& g)
{
    qubits = g.qubits;
    circuit_state -> state_vector = g.circuit_state -> state_vector;
    gates = g.gates;
}

circuit& circuit::operator=(const circuit& g)
{
    circuit temp(g);
    swap(qubits, temp.qubits);
    swap(gates, temp.gates);
    swap(circuit_state -> state_vector, temp.circuit_state -> state_vector);
    return *this;
}

void circuit::circuit_preprocessing()
{
    circuit_preprocessed = true;
    deque<shared_ptr<gate>> new_gates;
    
    for (int q = 0; q < qubit_to_gates.size(); ++q) {
        int count = 1, count_controls = 0;
        shared_ptr<gate> merged_gate(new gate());
        shared_ptr<gate> merged_control_gate(new gate());
        merged_gate -> rows = gates[qubit_to_gates[q].back()] -> rows;
        
        for(int g = (int)qubit_to_gates[q].size() - 2; g >= 0; --g) {
            
            if (count == 10) {
                merged_gate -> qubits.push_back(q);
                merged_gate -> gate_identification.push_back(gate::Gates::Random);
                if (count_controls > 1) {
                    merged_control_gate -> qubits.push_back(q);
                    new_gates.push_front(merged_control_gate);
                }
                new_gates.push_front(merged_gate);
                merged_gate.reset(new gate());
                merged_control_gate.reset(new gate());
                merged_gate -> rows = gates[qubit_to_gates[q][g]] -> rows;
                count_controls = 0;
                count = 1;
            }
            else if (gates[qubit_to_gates[q][g]] -> gate_identification[0] == gate::Gates::Control
                && gates[qubit_to_gates[q][g]] -> qubits.back() == q) {
                
                ++count_controls;
                
                if (count_controls > 1) {
                    count = 10;
                    ++g;
                    continue;
                }
                merged_gate -> num_controls = 1;
                merged_gate -> qubits.insert(merged_gate -> qubits.begin(),
                                             gates[qubit_to_gates[q][g]] -> qubits.front());
                merged_gate -> gate_identification.push_back(gate::Gates::Control);
                *merged_control_gate = *merged_gate;
                merged_control_gate -> rows = matrix_mult(gates[qubit_to_gates[q][g]] -> rows,
                                                  merged_control_gate -> rows);
                ++count;
            }
            else {
                merged_gate -> rows = matrix_mult(gates[qubit_to_gates[q][g]] -> rows,
                                          merged_gate -> rows);
                if (count_controls > 0) {
                    merged_control_gate -> rows = matrix_mult(gates[qubit_to_gates[q][g]] -> rows,
                                                              merged_control_gate -> rows);
                }
                
                ++count;
            }
        }
        merged_gate -> qubits.push_back(q);
        merged_gate -> gate_identification.push_back(gate::Gates::Random);
        if (count_controls > 1) {
            new_gates.push_front(merged_control_gate);
        }
        new_gates.push_front(merged_gate);
    }
   // gates = new_gates;
}

// TO DO: apply optimizations for diagonal gates, control Z, toffolli, and control X. 
void circuit::simulate(string outfile)
{
    clock_t begin = clock();
    int to_print = 0;
    int size = (int)gates.size();
    for (int i = 0; i < size; ++i) {
        int num_bits = (int)gates[i] -> qubits.size();
        
        auto timing_and_probability = [&]() {
            if (google && i == clock_cycles[to_print] ) {
                clock_t end = clock();
                double secs = double(end - begin) / CLOCKS_PER_SEC;
                elapsed_secs.push_back(secs);
                cout << secs << " secs for cycle " << to_print << "\n";
                ++to_print;
                if (to_print % 20 == 0) {
                    print_probabilities(outfile);
                }
                begin = clock();
            }
        };
        timing_and_probability();
        
        if(gates[i] -> qubits.size() > 1 &&
           gates[i] -> gate_identification[0] != gate::Gates::Control) {
            sort(gates[i] -> qubits.begin(), gates[i] -> qubits.end());
        }
        
        circuit_simulation* sim = (new circuit_simulation(this));
        circuit_state -> amp_apply_gate.clear();
        circuit_state -> apply_gate.clear();
        
        int initial_i = i;
        index_size index = pow(2, abs(gates[i] -> qubits[0] - abs(qubits - 1)));
    
        if(gates[i] -> gate_identification[0] == gate::Gates::Control) {
           
            if ( gates[i] -> gate_identification[1] == gate::Gates::X) {
                vector<index_size> bit_mask = sim -> initialize_gate_set(index, i,
                                                                   gate::Gates::X,
                                                                  timing_and_probability);
                auto X_opt = [&](index_size swap_index) {
                    if(!(circuit_state -> state_vector[index] == cmplx(0,0)
                         && circuit_state -> state_vector[index] == cmplx(0,0))) {
                        swap(circuit_state -> state_vector[index],
                             circuit_state -> state_vector[swap_index]);
                    }
                };
                
                sim -> block_gate_opt(bit_mask, initial_i, X_opt);
            }
            else if ( gates[i] -> gate_identification[1] == gate::Gates::Z) {
                vector<index_size> bit_mask = sim -> initialize_gate_set(index, i,
                                                                   gate::Gates::Z,
                                                                  timing_and_probability);
                auto Z_opt = [&](index_size index) {
                    circuit_state -> state_vector[index] = -circuit_state -> state_vector[index];
                };
                sim -> block_gate_opt(bit_mask, initial_i, Z_opt);
            }
            else if (gates[i] -> gate_identification[1] == gate::Gates::Phase) {
                vector<index_size> bit_mask = sim -> initialize_gate_set(index, i,
                                                                  gate::Gates::Phase,
                                                                  timing_and_probability);
                auto Phase_opt = [&](index_size index) {
                    circuit_state -> state_vector[index] = cmplx(
                                    -imag(circuit_state -> state_vector[index]),
                                    real(circuit_state -> state_vector[index]));
                };
                sim -> block_gate_opt(bit_mask, initial_i, Phase_opt);
                //print_state();
            }
            else {
                index_size c_bits = 0;
                for (int j = 0 ; j < gates[i] -> num_controls; ++j) {
                    c_bits += pow(2, abs(gates[i] -> qubits[j] - abs(qubits - 1)));
                }
                
                sim -> control_rand_sim(i, num_bits, c_bits);
            }
        }
        else if (gates[i] -> gate_identification[1] == gate::Gates::T) {
            for(; i < gates.size() &&
                gates[i] -> gate_identification.back() == gate::Gates::T; ++i ) {
                ++(circuit_state -> i_count);
            }
            --i;
        }
        else if (gates[i] -> gate_identification[0] == gate::Gates::Measurement) {
            circuit_state -> measure(gates[i] -> qubits[0]);
        }
        else {
            if(google && gates[i] -> gate_identification[0] == gate::Gates::Hadamard) {
                     index_size size = circuit_state -> state_vector.size();
                     for (index_size index = 0; index < size; ++index) {
                         circuit_state -> state_vector[index] = 1;
                     }
                 }
//                 else if(gates[i] -> gate_identification[0] == gate::Gates::X_rotation) {
//
//                 }
//                 else if(gates[i] -> gate_identification[0] == gate::Gates::Y_rotation) {
//
//                 }
             else {
                 vector<bool> iterated(circuit_state -> state_vector.size(), false);
                 index_size iter_count = 0;
                 index_size size = circuit_state -> state_vector.size();
                 for (index_size index = 0; index < size/*iter_count < size/pow(2,num_bits)*/; ++index) {
                     if (iterated[index] == false) {
                         iterated[index] = true;
                         iter_count += num_bits;
                         
                         sim -> rand_sim(iterated, i, num_bits, index);
                         (*circuit_state) * gates[i];
                     }
                 }
             }
        }
        delete sim;
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
    
    print_state();
}


int circuit::test(circuit& result)
{
    for (int i = 0; i < gates.size(); ++i) {
        gates[i] -> rows = matrix_inv(gates[i] -> rows);
    }
    
    //reverse the order of the gates and call simulate;
    //save the initial state vector in test.
    return 0;
}

void circuit::print_probabilities(string& out_file) {
    
    static int count = 0;
    ofstream file;
    file.open(out_file + to_string(count) + ".txt");
    
    for (auto state_v : circuit_state -> state_vector) {
        state_v *= cmplx(pow(HADAMARD_CONST, qubits)) *
        conj(cmplx(pow(HADAMARD_CONST, qubits)) * state_v); /// cmplx(pow(2, qubits));
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

void circuit::print_state() {
    for (auto state_v : circuit_state -> state_vector) {
        state_v *= pow(HADAMARD_CONST, qubits) *
                pow(cmplx(0, 1), circuit_state->i_count);
        cout << real(state_v) ;
        
        if (imag(state_v) > 0) {
            cout << "+" << imag(state_v) << "i";
        }
        else if (imag(state_v) < 0) {
            cout << imag(state_v) << "i";
        }
        cout << " ";
    }
    cout << "\n\n";
}

void circuit::print_state(string outfile) {
    
    static ofstream file;
    file.open(outfile + ".txt");
    
    for (auto state_v : circuit_state -> state_vector) {
        state_v *= pow(HADAMARD_CONST, qubits) *
                pow(cmplx(0.0, 1.0), circuit_state->i_count);
        file << real(state_v) ;
        
        if (imag(state_v) > 0) {
            file << "+" << imag(state_v) << "i";
        }
        else if (imag(state_v) < 0) {
            file << imag(state_v) << "i";
        }
        file << " ";
    }
    file << "\n\n";
}

state::state(): i_count(0)
{
    state_vector = {};
    amp_apply_gate = {};
    apply_gate = {};
}

state::state(const state& rhs)
{
    state_vector = rhs.state_vector;
    amp_apply_gate = rhs.amp_apply_gate;
    apply_gate = rhs.apply_gate;
}

state& state::operator=(const state& rhs)
{
    state temp(rhs);
    swap(state_vector, temp.state_vector);
    swap(amp_apply_gate, temp.amp_apply_gate);
    swap(apply_gate, temp.apply_gate);
    return *this;
}

void state::operator*(shared_ptr<gate> q_gate)
{
    for(const auto& a : amp_apply_gate) {
        apply_gate.push_back(a.second);
    }
    
    vector<cmplx> test_gate = apply_gate;
    if (!(apply_gate[0] == cmplx(0,0) && apply_gate[1] == cmplx(0,0))) {
        assert(apply_gate.size() == 2);
        
        if (q_gate -> gate_identification[0] == gate::Gates::Z ||
            q_gate -> gate_identification[0] == gate::Gates::T) {

            for (int i = 0; i < apply_gate.size(); ++i) {
                apply_gate[i] = apply_gate[i] * q_gate -> rows[i][i];
            }
        }
        else if (q_gate -> gate_identification[0] == gate::Gates::X ||
                 q_gate -> gate_identification[0] == gate::Gates::Y ) {

            auto temp_v = apply_gate;
            short a_size = apply_gate.size();
            for (int i = 0; i < a_size; ++i) {
                apply_gate[i] = temp_v[a_size - i + 1] * q_gate -> rows[i][a_size - i + 1];
            }
        }
        else {
            apply_gate = matrix_v_mult(q_gate->rows, apply_gate);
        }
        
       int c = 0;
        for(const auto& a : amp_apply_gate) {
            state_vector[a.first] = apply_gate[c++];
        }
    }
    apply_gate.clear();
    amp_apply_gate.clear();
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



