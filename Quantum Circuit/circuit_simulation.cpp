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

circuit::circuit_simulation::~circuit_simulation()
{
    cir = nullptr;
}

void circuit::circuit_simulation::initialize_control(int& index, int i)
{
    int count = 0;
    int c_iters = pow(2, cir -> qubits - cir -> gates[i] -> num_controls);
    int transition = pow(2, abs(cir -> gates_to_qubits[i][0] - abs(cir -> qubits - 1)));
    int to_transition = 0;
    while(count < c_iters) {
        if ((index & c_bits) == c_bits) {
            ++count;
            cir -> circuit_state -> amplitudes[index] = cir -> circuit_state -> state_vector[index];
            ++index;
            ++to_transition;
        }
        else if (to_transition < transition) {
            ++to_transition;
            ++index;
        }
        else {
            to_transition = 0;
            index += pow(2, abs(cir -> gates_to_qubits[i][0] - abs(cir -> qubits - 1)));
        }
    }
}

void circuit::circuit_simulation::non_control_sim(vector<bool>& iterated, int i, int n, int num_bits)
{
    auto& amp = cir -> circuit_state -> amplitudes;
    
    amp[n] = cir -> circuit_state -> state_vector[n];
    
    int index = 0;
    for (int j = 0 ; j < num_bits; ++j) {
        map<double, cmplx> temp;
        for (const auto& a : amp) {
            index = a.first + pow(2, abs(cir -> gates_to_qubits[i][j] - abs(cir -> qubits - 1)));
            temp[index] = cir -> circuit_state -> state_vector[index];
            iterated[index] = true;
        }
        amp.insert(temp.begin(), temp.end());
    }
    
    (*cir -> circuit_state) * cir -> gates[i];
}

void circuit::circuit_simulation::control_Not_opt()
{
    auto amp = cir -> circuit_state -> amplitudes;
    
    vector<bool> iterated (amp.size(), false);
    
    int count = 0;
    for (auto iter = amp.begin(); iter != amp.end(); ++iter) {
        if (iterated[count] == false) {
            auto iter_t = find_target(iterated ,iter, count);
            if(iter_t == amp.end()) {
                throw "iter_t is null";
            };
            if(!(cir -> circuit_state -> state_vector[iter -> first] == cmplx(0,0)
               && cir -> circuit_state -> state_vector[iter_t -> first] == cmplx(0,0))) {
                swap(cir -> circuit_state -> state_vector[iter -> first],
                     cir -> circuit_state -> state_vector[iter_t -> first]);
            }
        }
        ++count;
    }
}

void circuit::circuit_simulation::control_Z_opt(int i)
{
    for (auto c: cir -> circuit_state -> amplitudes) {
       if ((c.first & t_bits) == t_bits) {
            cir -> circuit_state -> state_vector[c.first] = (cmplx)(-1.0) * c.second;
        }
    }
}

void circuit::circuit_simulation::control_Phase_opt(int i)
{
    for (auto c: cir -> circuit_state -> amplitudes) {
        if ((c.first & t_bits) == t_bits) {
            cir -> circuit_state -> state_vector[c.first] = cmplx(0.0, 1.0) * c.second;
        }
    }
}

void circuit::circuit_simulation::control_rand_opt(int i)
{
    auto amp = cir -> circuit_state -> amplitudes;
    
    vector<bool> iterated (amp.size(), false);
    
    int count = 0;
    for (auto iter = amp.begin(); iter != amp.end(); ++iter) {
        if (iterated[count] == false) {
            iterated[count] = true;
            auto iter_t = find_target(iterated, iter, count);
            if(iter_t == amp.end()) {
                throw "iter_t is null";
            };
            
            cir -> circuit_state -> iterator = &iter;
            cir -> circuit_state -> iterator1 = &iter_t;
            (*cir -> circuit_state) * cir -> gates[i];
        }
        
        ++count;
    }
}

template <typename Iterator>
inline Iterator circuit::circuit_simulation::find_target(vector<bool>& iterated, Iterator iter, int c)
{
    auto amp = cir -> circuit_state -> amplitudes;
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

circuit::circuit() : qubits(0), google(false)
{
    circuit_state = new state();
    gates_to_qubits = {};
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
    gates_to_qubits = g.gates_to_qubits;
    gates = g.gates;
}

circuit& circuit::operator=(const circuit& g)
{
    circuit temp(g);
    swap(qubits, temp.qubits);
    swap(gates, temp.gates);
    swap(gates_to_qubits, temp.gates_to_qubits);
    swap(circuit_state -> state_vector, temp.circuit_state -> state_vector);
    return *this;
}

// TO DO: apply optimizations for diagonal gates, control Z, toffolli, and control X. 
void circuit::simulate(string& outfile)
{
    clock_t begin = clock();
    int to_print = 0;
    int size = (int)gates_to_qubits.size();
    for (int i = 0; i < size; ++i) {
        int num_bits = (int)gates_to_qubits[i].size();
        if (num_bits == 0) {
            throw "Please enter non-zero number of bits for each gate";
        }
        
        if (google == true && i == clock_cycles[to_print]) {
            clock_t end = clock();
            double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
            cout << elapsed_secs << " secs for cycle " << to_print << "\n";
            ++to_print;
            print_probabilities(outfile);
            begin = clock();
        }
        
        int num_iters = (int)circuit_state -> state_vector.size();//pow(2, num_bits);
        vector<bool> iterated(num_iters, false);
        
        if(gates_to_qubits[i].size() > 1 && gates[i] -> gate_identification[0] != gate::Gates::Control) {
            sort(gates_to_qubits[i].begin(), gates_to_qubits[i].end());
        }
        
        for(int n = 0 ; n < num_iters; ++n) {
            if (iterated[n] == false) {
                
                iterated[n] = true;
                
                circuit_simulation* sim = (new circuit_simulation(this));
                circuit_state -> amplitudes.clear();
                circuit_state -> apply_gate.clear();
                
                if(gates[i] -> gate_identification[0] == gate::Gates::Control) {
                    
                    int index = n + pow(2, abs(gates_to_qubits[i][0] - abs(qubits - 1)));
                    
                    for (int j = 0 ; j < gates[i] -> num_controls; ++j) {
                        sim -> c_bits += pow(2, abs(gates_to_qubits[i][j] - abs(qubits - 1)));
                    }
                    
                    for (int j = gates[i] -> num_controls ; j < gates_to_qubits[i].size(); ++j) {
                        sim -> t_bits += pow(2, abs(gates_to_qubits[i][j] - abs(qubits - 1)));
                    }
                
                    sim -> initialize_control(index, i);
                   
                    if ( gates[i] -> gate_identification[1] == gate::Gates::X) {
                        sim -> control_Not_opt();
                    }
                    else if ( gates[i] -> gate_identification[1] == gate::Gates::Z) {
                        sim -> control_Z_opt(i);
                    }
                    else if (gates[i] -> gate_identification[1] == gate::Gates::Phase) {
                        sim -> control_Phase_opt(i);
                    }
                    else {
                        sim -> control_rand_opt(i);
                    }
                    n = index + 1;
                }
                else if (gates[i] -> gate_identification[0] == gate::Gates::Measurement) {
                    circuit_state -> measure(gates_to_qubits[i][0]);
                    n = num_iters;
                }
                else {
                    sim -> non_control_sim(iterated, i, n, num_bits);
                }
                delete sim;
            }
        }
    }
    clock_t end = clock();
    double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
    cout << elapsed_secs << " secs for cycle " << to_print << "\n";
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

state::state(): iterator(nullptr), iterator1(nullptr)
{
    state_vector = {};
    amplitudes = {};
    apply_gate = {};
}

state::state(const state& rhs)
{
    state_vector = rhs.state_vector;
    amplitudes = rhs.amplitudes;
    apply_gate = rhs.apply_gate;
    
    iterator = rhs.iterator;
    iterator1 = rhs.iterator1;
}

state& state::operator=(const state& rhs)
{
    state temp(rhs);
    swap(state_vector, temp.state_vector);
    swap(amplitudes, temp.amplitudes);
    swap(apply_gate, temp.apply_gate);
    swap(iterator, temp.iterator);
    swap(iterator1, temp.iterator1);
    return *this;
}

void state::operator*(shared_ptr<gate> q_gate)
{
    
    if (iterator && iterator1) {
        apply_gate.push_back((*iterator) -> second);
        apply_gate.push_back((*iterator1) -> second);
    }
    else {
        for(const auto& a : amplitudes) {
            apply_gate.push_back(a.second);
        }
    }
    
    vector<cmplx> test_gate = apply_gate;
    if (!(apply_gate[0] == cmplx(0,0) && apply_gate[1] == cmplx(0,0))) {
        assert(apply_gate.size() == 2);
        
        if (q_gate -> gate_identification[0] == gate::Gates::Z ||
            q_gate -> gate_identification[0] == gate::Gates::T) {

            test_gate[0] = test_gate[0] * q_gate -> rows[0][0];
            apply_gate[1] = apply_gate[1] * q_gate -> rows[1][1];
            apply_gate[0] = test_gate[0];
        }
        else if (q_gate -> gate_identification[0] == gate::Gates::X ||
                 q_gate -> gate_identification[0] == gate::Gates::Y ) {

            test_gate[0] =  q_gate -> rows[0][1] * test_gate[1] ;
            apply_gate[1] =  q_gate -> rows[1][0] * apply_gate[0];
            apply_gate[0] = test_gate[0];
        }
        else {
            apply_gate = matrix_v_mult(q_gate->rows, apply_gate);
        }
        
        if (iterator && iterator1) {
            state_vector[(*iterator) -> first] = apply_gate[0];
            state_vector[(*iterator1) -> first] = apply_gate[1];
        }
        else {
            int c = 0;
            for(const auto& a : amplitudes) {
                state_vector[a.first] = apply_gate[c++];
            }
        }
    }
    
    iterator = nullptr;
    iterator1 = nullptr;
    apply_gate.clear();
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



