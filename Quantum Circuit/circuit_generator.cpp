//
//  circuit_generator_impl.cpp
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 10/19/17.
//  Copyright © 2017 Aneeqa Fatima. All rights reserved.
//

#include <stdio.h>
#include <cmath>
#include <math.h>
#include <fstream>
#include "circuit_generator.h"
#include "circuit_gates.h"

using namespace std;

vector<func_t> circuit_generator::gates{};
vector<rot_f> circuit_generator::rot_gates{};
vector<string> circuit_generator::quiddpro_func;
cmplx circuit_generator::c = cmplx(-0.99989190,0);


circuit_generator::circuit_generator(): q_circuit(new circuit()){
    
    gates.push_back(create_hadamard);
    gates.push_back(create_X);
    gates.push_back(create_Y);
    gates.push_back(create_Z);
    gates.push_back(random_gate);
    
    rot_gates.push_back(create_X_rotation);
    rot_gates.push_back(create_Y_rotation);
    rot_gates.push_back(create_Z_rotation);
    rot_gates.push_back(create_phase_gate);
//     gates.push_back(create_T);
    
    quiddpro_func.push_back("hadamard");
    quiddpro_func.push_back("sigma_x");
    quiddpro_func.push_back("sigma_y");
    quiddpro_func.push_back("sigma_z");
    quiddpro_func.push_back("");
    quiddpro_func.push_back("rx");
    quiddpro_func.push_back("ry");
    quiddpro_func.push_back("rz");
    quiddpro_func.push_back("phase");
    
}

//TO DO:qubits to gate vector
void circuit_generator::create_rand_circuit(int qubits, int num_gates)
{
    srand(time(NULL));
    if(qubits <= 0 || num_gates <= 0) {
        throw "Please request a valid circuit";
    }

    q_circuit -> qubits = qubits;
    for (int i = 0; i < qubits; ++i) {
        int q = rand() % 2;
        vector<cmplx> bits;
        
        if (q == 0) {
            bits.push_back(cmplx(1,0));
            bits.push_back(cmplx(0,0));
        }
        else {
            bits.push_back(cmplx(0,0));
            bits.push_back(cmplx(1,0));
        }
     
        classical_bits.push_back(q);
        
        if (i > 0) {
            q_circuit -> circuit_state -> amp = tensor_v_product(q_circuit -> circuit_state -> amp, bits);
        }
        else {
            q_circuit -> circuit_state -> amp = bits;
        }
    }
    
    for (int i = 0 ; i < num_gates; ++i) {
        int q = rand() % qubits;
        gate::Gates g = static_cast<gate::Gates>(rand() % NUM_GATES);
        
        gate temp = gate();
        
        if (g == gate::Gates::Control) {
            vector<short> num_c(rand() % (qubits-1) + 1, -1); //change 3 to qubits
            vector<short> num_t(1, -1); // change to rand() % qubits - num_c - 1
            q = -1;
            
            temp.gate_identification.insert(temp.gate_identification.begin(), g);
            temp.theta.insert(temp.theta.begin(), 0);
            temp.num_controls = (int)num_c.size();
            g = static_cast<gate::Gates>(rand() % gate::Gates::Control);
            
            auto check_reoccurance = [&num_c, &num_t](int i) {
                for(auto n : num_c) {
                    if(n == i) {
                        return false;
                    }
                }
                for(auto n : num_t) {
                    if(n == i) {
                        return false;
                    }
                }
                return true;
            };
            
            auto rand_bit_gen = [&check_reoccurance, &qubits](vector<short>& to_gen) {
                int rbg = rand() % qubits;
                for(auto& gen : to_gen) {
                    while(!check_reoccurance(rbg)) {
                        rbg = rand() % qubits;
                    }
                    gen = rbg;
                }
            };
            
            rand_bit_gen(num_c);
            rand_bit_gen(num_t);
            
            sort(num_c.begin(), num_c.end());
            sort(num_t.begin(), num_t.end());
            temp.qubits = num_c;
            temp.qubits.insert(temp.qubits.end(),
                                num_t.begin(), num_t.end());
        }
        
        if ( g <= gate::Gates::Random) {
            gate n_temp = gates[g]();
            if (q == -1) {
                temp.rows = n_temp.rows;
                temp.gate_identification.push_back(n_temp.gate_identification[0]);
            }
            else {
                temp = move(n_temp);
                temp.qubits.push_back(q);
            }
            q_circuit -> gates.push_back(move(temp));
        }
        else if (g < gate::Gates::Control) {
            double theta = (double)rand() / double(RAND_MAX);
            gate n_temp = rot_gates[g - (gate::Gates::X_rotation)](theta);
            if (q == -1) {
                temp.rows = n_temp.rows;
                temp.gate_identification.push_back(n_temp.gate_identification[0]);
                temp.theta.push_back(n_temp.theta[0]);
            }
            else {
                temp = move(n_temp);
                temp.qubits.push_back(q);
            }
            q_circuit -> gates.push_back(move(temp));
        }
    }
}

void circuit_generator::create_google_rand_circuit(int qubits, int clock_cycles)
{
    srand(time(NULL));
    
    google = true;
    short GS_gates[3] = {gate::Gates::X_rotation, gate::Gates::Y_rotation, gate::Gates::T};
    
    int GS_gates_num = 3;
    
    auto check_reoccurance = [](vector<short>& to_check, short q) {
        for(size_t i = 0; i < to_check.size(); ++i) {
            if(to_check[i] == q) {
                return false;
            }
        }
        return true;
    };
    
    //Each row represents a qubit. This is to keep track of the
    //gates applied to this qubit.
    vector<vector<int>> qubit_to_gates(qubits);
    q_circuit -> qubits = qubits;
    
    auto count_wires_gate = [&](vector<short>& CZ_pairs) {
        int count = 0;
        for (int i = 0; i < qubit_to_gates.size(); ++i) {
            if ( q_circuit -> gates[qubit_to_gates[i].back()].
                gate_identification[0] == gate::Gates::Control) {
                CZ_pairs.push_back(i);
                count += 1;
            }
        }
        return count;
    };
    auto count_wires_gate1 = [&]() {
        auto& gates = q_circuit -> gates;
        int count = 0;
        for (int i = 0; i < qubit_to_gates.size(); ++i) {
            if ( gates[qubit_to_gates[i].back()].gate_identification[0]
                                == gate::Gates::Control) {
                count += 2;
            }
        }
        return count;
    };
    
    vector<bool> T_gate_allowed(qubits, true);
    q_circuit -> circuit_state -> amp.push_back(cmplx(1,0));
    for (int i = 1; i < pow(2, qubits); ++i) {
        q_circuit -> circuit_state -> amp.push_back(cmplx(0,0));
    }
    
    //Start by applying Hadamard Gates
    for (int i = 0; i < qubits; ++i) {
        classical_bits.push_back(0);
        gate temp = create_hadamard();
        temp.qubits.push_back(i);
        q_circuit -> gates.push_back(move(temp));
        qubit_to_gates[i].push_back((int)q_circuit -> gates.size());
    }
    
    q_circuit -> clock_cycles.push_back(q_circuit -> gates.size());
    
    for (int i = 0; i < clock_cycles; i+=2) {
        vector<short> current_CZ_pairs;
        current_CZ_pairs.resize((1 + (rand() % (qubits - 1))) * 2);
        
        int CZ_size = (int) current_CZ_pairs.size();
        int CZ_q = abs(qubits - CZ_size/2);
        CZ_q = rand() % CZ_q;
        for (int j = 0; j < CZ_size; ++j) {
//            int CZ_q = rand () % (qubits - 1) ;
//            while(!check_reoccurance(current_CZ_pairs, CZ_q, CZ_q + 1)) {
//                CZ_q = rand() % (qubits - 1);
//            }
            
            current_CZ_pairs[j++] = CZ_q;
            qubit_to_gates[CZ_q].push_back((int)q_circuit -> gates.size());
            
            current_CZ_pairs[j] = ++CZ_q;
        }
            qubit_to_gates[CZ_q].push_back((int)q_circuit -> gates.size());
        
        //control phase gate
        int k = 0;
        for (int j = 0; j < current_CZ_pairs.size()/2; ++j) {
            gate temp = create_phase_gate(0.0);
            temp.gate_identification.insert(temp.gate_identification.begin()
                                               , gate::Gates::Control);
            temp.num_controls = 1;
            temp.qubits.push_back(current_CZ_pairs[k++]);
            temp.qubits.push_back(current_CZ_pairs[k++]);
            q_circuit -> gates.push_back(move(temp));
        }
        
        if (current_CZ_pairs.size() > 0) {
            q_circuit -> clock_cycles.push_back(q_circuit -> gates.size());
        }
        
        /*
         • Place a gate at qubit q only if this qubit is occupied by a CZ gate in the previous cycle.
         • Place a T gate at qubit q if there are no single- qubit gates in the previous cycles at
         qubit q except for the initial cycle of Hadamard gates.
         • Any gate at qubit q should be different from the gate at qubit q in the previous cycle.
         */
        
        vector<short> CZ_pairs;
        int qubits_for_gates = 1 + rand() % (count_wires_gate(CZ_pairs) - 1);
        vector<short> complied_qubits(qubits_for_gates, -1);
        
        for (int j = 0; j < qubits_for_gates; ++j) {
            int q = rand() % qubits_for_gates;
            while(q_circuit -> gates[
                 qubit_to_gates[CZ_pairs[q]].back()].gate_identification[0]
                  != gate::Gates::Control) {
                q = rand() % qubits_for_gates;
            }
            complied_qubits[j] = CZ_pairs[q];
            
            short gate_to_apply;
            if (T_gate_allowed[CZ_pairs[q]]) {
                gate_to_apply = rand() % GS_gates_num;
                while (q_circuit -> gates[
                      qubit_to_gates[CZ_pairs[q]].back()].gate_identification[0]
                       == GS_gates[gate_to_apply]) {
                    gate_to_apply = rand() % GS_gates_num;
                }
                if (gate_to_apply == 2) {
                    T_gate_allowed[CZ_pairs[q]] = false;
                }
            }
            else {
                gate_to_apply = rand() % (GS_gates_num - 1);
                while (q_circuit -> gates[
                        qubit_to_gates[CZ_pairs[q]].back()]. gate_identification[0]
                        == GS_gates[gate_to_apply]) {
                    gate_to_apply = rand() % (GS_gates_num - 1);
                }
            }
            
            gate temp;
            
            if (gate_to_apply == 0) {
                temp = create_X_rotation(0.5);
            }
            else if (gate_to_apply == 1) {
                temp = create_Y_rotation(0.5);
            }
            else if (gate_to_apply == 2) {
                temp = create_T();
            }
            temp.qubits.push_back(CZ_pairs[q]);
            qubit_to_gates[CZ_pairs[q]].push_back((int)q_circuit -> gates.size());
            q_circuit -> gates.push_back(move(temp));
        }
        if(qubits_for_gates > 0) {
            q_circuit -> clock_cycles.push_back(q_circuit -> gates.size());
        }
    }
    
//    g_t_q_size = (int)q_circuit -> gates_to_qubits.size();
//    q_circuit -> gates_to_qubits.resize(q_circuit -> gates_to_qubits.size() + qubits);
//    for (int i = 0; i < qubits; ++i) {
//        gate temp = create_I();
//        temp -> gate_identification.push_back(gate::Gates::Measurement);
//        q_circuit -> gates.push_back(temp);
//        q_circuit -> gates_to_qubits[g_t_q_size + i].push_back(i);
//    }
}

void circuit_generator::create_quiddpro_script(const string& input_file)
{
    ofstream file;
    file.open(input_file);
    
    file << "state = cb(\"" + to_string((int)real(classical_bits[0]))<< "\");\n";;
    
    for (int q = 1; q < classical_bits.size(); ++q) {
        file << "state = kron(state, cb(\"" + to_string(classical_bits[q]) << "\"));\n";
    }
    
    for (int i = 0; i < q_circuit -> gates.size(); ++i) {
        if(i != 0) {
            file << ";\n";
        }
        int op_count = 0;
        bool control = false;
        bool continue_ = false;
        for (int gt = 0; gt < q_circuit -> gates[i].gate_identification.size(); ++gt) {
            auto g = q_circuit -> gates[i].gate_identification[gt];
            
            if ( g == gate::Gates::Measurement) {
                file << "measure_sv(" + to_string(q_circuit -> gates[i].qubits[0] + 1)
                + ", state)";
                continue_ = true;
                continue;
            }
            
            if ( g == gate::Gates::T) {
                file << "T = [1 0 ; 0 " + to_string(0.707106781) + "+i*" + to_string(0.707106781) + "];\n";
                file << "op" + to_string(op_count++) + " = cu_gate (T, \"";
            }
            else if (g != gate::Gates::Control ) {
                file << "op" + to_string(op_count++)  + " = cu_gate(" + quiddpro_func[g] + "(";
                
                if (g < gate::Gates::X_rotation || g == gate::Gates::Phase) {
                    file << "1), \"";
                }
                else if (g < gate::Gates::Control) {
                    file << to_string(q_circuit -> gates[i].theta[gt]) + " * pi, 1), \"";
                }
            }
            
            if (g != gate::Gates::Control) {
                int q = 0;
                if (control) {
                    for (; q < q_circuit -> gates[i].num_controls; ++q) {
                        file << "c" + to_string(q_circuit -> gates[i].qubits[q] + 1);
                    }
                }
                for (; q < q_circuit -> gates[i].qubits.size(); ++q) {
                    file << "x" + to_string(q_circuit -> gates[i].qubits[q] + 1);
                }
                file << "\", " + to_string(q_circuit -> qubits) << ");\n";
            }
            else {
                control = true;
            }
        }
        
        if (continue_) {
            continue;
        }
        if(op_count > 1) {
            file << "op = ";
            for (int n = 0; n < op_count; ++n) {
                if (n != 0) {
                    file << "*";
                }
                file << "op" + to_string(n);
            }
            file << ";\n";
            file << "state = op * state";
        }
        else {
            file << "state = op0 * state";
        }
    }
}

/*
 $: separates all the different qubits
 &: separates the rows in a gate
 ,: separates columns within a row in a gate
 
 e.g output
 
 0 1 $ 1 0 $
 1 , 1 + 9 i & 0 , 2 &
 1
 
 The above example represents this gate: [1  1 + 9i
                                          0  2     ]
 */
void circuit_generator::write_circuit_to_file(const string& input_file)
{
    int size_q = (int) q_circuit -> circuit_state -> amp.size();
    
    if ( size_q == 0) {
        throw "Create circuit first by entering qubits";
    }
    ofstream file;
    file.open(input_file);
    
    file << q_circuit -> qubits << "\n";
    
    for (int i = 0; i < classical_bits.size(); ++i) {
        file << classical_bits[0] << " ";
    }
    file << "\n";
    for (int i = 0; i < size_q; ++i) {
        
        file << q_circuit -> circuit_state -> amp[i];
        file << " " ;
    }
    file << "\n";
    
    int size_g = (int) q_circuit -> gates.size();
    
    if ( size_g == 0) {
        throw "Create circuit first by entering gates";
    }
    
    for (int i = 0; i < size_g; ++i) {
        int size_r = (int)q_circuit -> gates[i].rows.size();
        for(size_t j = 0; j < size_r; ++j) {
            int size_c = (int)q_circuit -> gates[i].rows[j].size();
            for (int k = 0; k < size_c; ++k) {
                file << real(q_circuit -> gates[i].rows[j][k]);
                
                if (imag(q_circuit -> gates[i].rows[j][k]) != 0) {
                    file << " + " << imag(q_circuit -> gates[i].rows[j][k]) << " i";
                }
                if (k != size_c-1) {
                    file << " , ";
                }
            }
            file << " & ";
        }
        file << "\n";
        file << "g ";
        for ( auto gate_num : q_circuit -> gates[i].gate_identification) {
            file << gate_num << " ";
        }
        file << "\nt ";
        for ( auto theta : q_circuit -> gates[i].theta) {
            file << theta << " ";
        }
        
        file << "\n";
        file << "q ";
        for (const auto& q : q_circuit -> gates[i].qubits) {
            file << q << " ";
        }
        
        file << "\n";
        file << "c " << q_circuit -> gates[i].num_controls << "\n";
    }
    file.close();
}

//TO DO:add suport for clock ccyles
void circuit_generator::read_input_file(const string& input_file)
{
    ifstream file;
    file.open(input_file);
    string input;
    
    file >> q_circuit -> qubits;
    vector<vector<int>> qubit_to_gates(q_circuit -> qubits);
    
    getline(file, input);
    getline(file, input);
    int classical;
    istringstream css(input);
    while (css >> classical) {
        classical_bits.push_back(classical);
    }
    
    getline(file, input);
    cmplx amp(0,0);
    double theta;
    int q, gt;
    char delim = '_';
    istringstream ss(input);
    while (ss >> amp) {
        q_circuit -> circuit_state -> amp.push_back(amp);
    }
    
    while(getline(file, input)) {
        istringstream iss(input);
        
        double re, im;
        char sign = '_' , i;
        delim = '_';
        gate g = gate();
        
        vector<cmplx> row;
        while(iss >> re) {
            
            if(delim == '&' || sign == '&') {
                g.rows.push_back(row);
                row.clear();
            }
            
            iss >> sign;
            if (sign != '+') {
                im = 0;
            }
            else {
                iss >> im >> i;
                iss >> delim;
            }
            
            cmplx c1(re, im);
            row.push_back(c1);
        }
        
        if(delim == '&' || sign == '&') {
            g.rows.push_back(row);
        }
        
        char type;
        getline(file, input);
        istringstream gis(input);
        gis >> type;
        if (type == 'g') {
            while(gis >> gt) {
                g.gate_identification.push_back(gt);
            }
        }
        
        getline(file, input);
        istringstream tis(input);
        tis >> type;
        if (type == 't') {
            while(tis >> theta) {
                g.theta.push_back(theta);
            }
        }
        
        
        getline(file, input);
        istringstream qis(input);
        qis >> type;
        if (type == 'q') {
            while(qis >> q) {
                g.qubits.push_back(q);
            }
            qubit_to_gates[g.qubits.back()].push_back(
                                                (int)q_circuit -> gates.size());
        }
        
        getline(file, input);
        istringstream cis(input);
        cis >> type;
        if (type == 'c') {
            cis >> g.num_controls;
        }
        q_circuit -> gates.push_back(g);
    }
}

void circuit_generator::read_google_input_files(const string& input_file)
{
    ifstream file;
    file.open(input_file);
    
    google = true;
    file >> q_circuit -> qubits;
    
    q_circuit -> circuit_state -> amp.push_back(cmplx(1,0));
    for (int i = 1; i < pow(2, q_circuit -> qubits); ++i) {
        q_circuit -> circuit_state -> amp.push_back(cmplx(0,0));
    }
    
    for (int i = 0; i < q_circuit -> qubits; ++i) {
        classical_bits.push_back(0);
    }
    
    short current_cycle = 0, prev_cycle = 0;
    string gate_type;
    short q1, q2;
    while (file >> current_cycle >> gate_type) {
        
        if (prev_cycle != current_cycle) {
            q_circuit -> clock_cycles.push_back(q_circuit -> gates.size());
            prev_cycle = current_cycle;
        }
        
        if(gate_type == "h") {
            q_circuit -> gates.push_back(create_hadamard());
        }
        else if (gate_type == "t") {
            q_circuit -> gates.push_back(create_T());
        }
        else if (gate_type == "y_1_2") {
            q_circuit -> gates.push_back(create_Y_rotation(0.5));
        }
        else if (gate_type == "x_1_2") {
            q_circuit -> gates.push_back(create_X_rotation(0.5));
        }
        else {
            file >> q1 >> q2;
            q_circuit -> gates.push_back(create_phase_gate(0.0));
            q_circuit -> gates[q_circuit -> gates.size() - 1].
                    gate_identification.insert(
                    q_circuit -> gates[q_circuit -> gates.size() - 1]. gate_identification.begin()
                    , gate::Gates::Control);
            q_circuit -> gates[q_circuit -> gates.size() - 1].num_controls = 1;
            q_circuit -> gates[q_circuit -> gates.size() - 1].qubits.push_back(q1);
            q_circuit -> gates[q_circuit -> gates.size() - 1].qubits.push_back(q2);
        }
        
        
        if (gate_type != "cz") {
            file >> q1;
            q_circuit -> gates[q_circuit -> gates.size() - 1].qubits.push_back(q1);
        }
    }
    q_circuit -> clock_cycles.push_back(q_circuit -> gates.size());
}
