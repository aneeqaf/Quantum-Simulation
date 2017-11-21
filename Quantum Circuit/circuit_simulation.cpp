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

void circuit::circuit_simulation::initialize_control(int& index, int i, int c_bits)
{
    int count = 0;
    int c_iters = pow(2, cir -> qubits - cir -> gates[i] -> num_controls);
    int transition = pow(2, abs(cir -> gates[i] -> qubits[0] - abs(cir -> qubits - 1)));
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
            index += pow(2, abs(cir -> gates[i] -> qubits[0] - abs(cir -> qubits - 1)));
        }
    }
}

template <typename function>
vector<int> circuit::circuit_simulation::initialize_control_set(int& index, int& i,
                                                                gate::Gates I, function& tp)
{
    vector<int> cbits;
    for(; cir -> gates[i] -> gate_identification.back() == I; ++i ) {
        tp();
        int c_bits = 0;
        for (int j = 0 ; j < cir -> gates[i] -> num_controls; ++j) {
            c_bits += pow(2, abs(cir -> gates[i] -> qubits[j] - abs(cir -> qubits - 1)));
        }
        cbits.push_back(c_bits);
    }
    --i;
    return cbits;
}

void circuit::circuit_simulation::rand_sim(vector<bool>& iterated,
                                                  int i, int num_bits, int index)
{
    cir -> circuit_state -> amp_apply_gate[index] =
                cir -> circuit_state -> state_vector[index];
    
    int temp_index = 0;
    for (int j = 0 ; j < num_bits; ++j) {
        map<double, cmplx> temp;
        for (const auto& a : cir -> circuit_state -> amp_apply_gate) {
            temp_index = a.first + pow(2, abs(cir -> gates[i] -> qubits[j] - abs(cir -> qubits - 1)));
            temp[temp_index] = cir -> circuit_state -> state_vector[temp_index];
            iterated[temp_index] = true;
        }
        cir -> circuit_state -> amp_apply_gate.insert(temp.begin(), temp.end());
    }
}

template<typename function>
void circuit::circuit_simulation::control_opt(vector<int>& cbits, int i,
                                              function& gate_specific_opt)
{
    auto find_smallest_non_control = [&](int j) {
        for(int cq = 0; cq < cir -> gates[i + j] -> num_controls; ++cq) {
            if (cq != cir -> gates[i + j] -> qubits[cq]) {
                return cq;
            }
        }
        return -1;
    };
    
    vector<vector<bool>> iterated(cir -> circuit_state -> state_vector.size(),
                                  vector<bool>(cbits.size(), false) );
    
    for (int index = 0; index < cir -> circuit_state -> state_vector.size(); ++index) {
        
        for (int j = 0; j < cbits.size(); ++j){
            if (iterated[index][j] == false) {
                if((index & cbits[j]) == cbits[j]) {
                    
                    iterated[index][j] = true;
                    int smallest_non_control_bit = find_smallest_non_control(j);
                    
                    if (smallest_non_control_bit == -1) {
                        continue;
                    }
                    if (cir -> gates[i + j] -> gate_identification.back() == gate::Gates::X) {
                        int swap_index = index +
                            pow(2, abs(smallest_non_control_bit - abs(cir -> qubits - 1)));
                        iterated[swap_index][j] = true;
                        
                        gate_specific_opt(swap_index, j);
                    }
                    else {
                        gate_specific_opt(index, j);
                        
                    }
                }
            }
        }
    }
}

void circuit::circuit_simulation::control_sim(int i, int num_bits, int c_bits)
{
    vector<bool> iterated(cir -> circuit_state -> state_vector.size(), false);
    for (int index = 0; index < cir -> circuit_state -> state_vector.size(); ++index) {
        
        if (iterated[index] == false) {
            iterated[index] = true;
            
            rand_sim(iterated, i, num_bits - cir -> gates[i] -> num_controls, index);
            if((index & c_bits) == c_bits) {
                (*cir -> circuit_state) * cir -> gates[i];
            }
            else if (cir -> circuit_preprocessed) {
                (*cir -> circuit_state) * cir -> gates[i+1];
            }
        }
    }
}

template <typename Iterator>
inline Iterator circuit::circuit_simulation::find_control_target(vector<bool>& iterated,
                                                         Iterator iter, int c, int t_bits)
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
void circuit::simulate(string& outfile)
{
    clock_t begin = clock();
    int to_print = 0;
    int size = (int)gates.size();
    for (int i = 0; i < size; ++i) {
        int num_bits = (int)gates[i] -> qubits.size();
        if (num_bits == 0) {
            throw "Please enter non-zero number of bits for each gate";
        }
        
        auto timing_and_probability = [&]() {
            if (google == true && i == clock_cycles[to_print] ) {
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
        
        if(gates[i] -> qubits.size() > 1 && gates[i] -> gate_identification[0] != gate::Gates::Control) {
            sort(gates[i] -> qubits.begin(), gates[i] -> qubits.end());
        }
        
        circuit_simulation* sim = (new circuit_simulation(this));
        circuit_state -> amp_apply_gate.clear();
        circuit_state -> apply_gate.clear();
    
        if(gates[i] -> gate_identification[0] == gate::Gates::Control) {
            
            int initial_i = i;
            int index = pow(2, abs(gates[i] -> qubits[0] - abs(qubits - 1)));
           
            if ( gates[i] -> gate_identification[1] == gate::Gates::X) {
                vector<int> cbits = sim -> initialize_control_set(index, i,
                                                                   gate::Gates::X,
                                                                  timing_and_probability);
                auto X_opt = [&](int swap_index, int j) {
                    if(!(circuit_state -> state_vector[index] == cmplx(0,0)
                         && circuit_state -> state_vector[index] == cmplx(0,0))) {
                        swap(circuit_state -> state_vector[index],
                             circuit_state -> state_vector[swap_index]);
                    }
                };
                
                sim -> control_opt(cbits, initial_i, X_opt);
            }
            else if ( gates[i] -> gate_identification[1] == gate::Gates::Z) {
                vector<int> cbits = sim -> initialize_control_set(index, i,
                                                                   gate::Gates::Z,
                                                                  timing_and_probability);
                auto Z_opt = [&](int swap_index, int j) {
                    int t_bits = 0;
                    for (int k = gates[initial_i + j] -> num_controls ;
                         k < gates[initial_i + j] -> qubits.size(); ++k) {
                        t_bits += pow(2, abs(gates[initial_i + j] -> qubits[k] - abs(qubits - 1)));
                    }
                
                    if ((swap_index & t_bits) == t_bits) {
                        circuit_state -> state_vector[swap_index] = (cmplx)(-1.0) *
                        circuit_state -> state_vector[swap_index];
                    }
                };
                sim -> control_opt(cbits, initial_i, Z_opt);
            }
            else if (gates[i] -> gate_identification[1] == gate::Gates::Phase) {
                vector<int> cbits = sim -> initialize_control_set(index, i,
                                                                  gate::Gates::Phase,
                                                                  timing_and_probability);
                auto Phase_opt = [&](int swap_index, int j) {
                    int t_bits = 0;
                    for (int k = gates[initial_i + j] -> num_controls ;
                         k < gates[initial_i + j] -> qubits.size(); ++k) {
                        t_bits += pow(2, abs(gates[initial_i + j] -> qubits[k] - abs(qubits - 1)));
                    }
                    if ((swap_index & t_bits) == t_bits) {
                        circuit_state -> state_vector[swap_index] = cmplx(0.0, 1.0) *
                                    circuit_state -> state_vector[swap_index];
                    }
                };
                sim -> control_opt(cbits, initial_i, Phase_opt);
            }
            else {
                int t_bits = 0;
                for (int j = gates[i] -> num_controls ; j < gates[i] -> qubits.size(); ++j) {
                    t_bits += pow(2, abs(gates[i] -> qubits[j] - abs(qubits - 1)));
                }
                
                sim -> control_sim(i, num_bits, t_bits);
                ++i;
            }
        }
        else if (gates[i] -> gate_identification[0] == gate::Gates::Measurement) {
            circuit_state -> measure(gates[i] -> qubits[0]);
        }
        else {
            vector<bool> iterated(circuit_state -> state_vector.size(), false);
            for (int index = 0; index < circuit_state -> state_vector.size(); ++index) {
                
                if (iterated[index] == false) {
                    iterated[index] = true;
                    
                    sim -> rand_sim(iterated, i, num_bits, index);
                    (*circuit_state) * gates[i];
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
    file.open(to_string(count)  + out_file + ".txt");
    
    for (auto state_v : circuit_state -> state_vector) {
        state_v *= conj(state_v) * cmplx(pow(2, qubits));
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
    
    for (const auto& state_v : circuit_state -> state_vector) {
        cout << real(state_v) ;
        
        if (imag(state_v) > 0) {
            cout << "+" << imag(state_v) << "i";
        }
        else if (imag(state_v) < 0) {
            cout << imag(state_v) << "i";
        }
        cout << " ";
    }
    cout << "\n";
}

state::state()
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
        
        //Fix it for larger diagonal gates.
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



