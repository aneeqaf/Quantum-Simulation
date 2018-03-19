//
//  circuit.cpp
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 12/12/17.
//

#include "circuit.h"

vector<string> Circuit::quiddpro_func;

Circuit::
Circuit() : gates({}),clock_cycles({}), qubits(0)
{
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

Circuit::
Circuit(const Circuit& g)
{
    qubits = g.qubits;
    gates = g.gates;
    clock_cycles = g.clock_cycles;
}

Circuit& Circuit::
operator=(const Circuit& g)
{
    Circuit temp(g);
    swap(qubits, temp.qubits);
    swap(gates, temp.gates);
    return *this;
}

void Circuit::
GroupAlternateCycles()
{
    bool saw_xy = false;
    idx_size g_i = 0;
    
    for (idx_size i = 2; i < clock_cycles.size(); ++i) {
        int last_CZT_gate = 0;
        if (i % 2 == 1)
            g_i = clock_cycles[i] - 1;
        
        else
            g_i = clock_cycles[i - 1];
        
        for (int j = 0; j < (int)(clock_cycles[i] - clock_cycles[i-1]); ++j) {
            int k = j;
            if (i % 2 == 1)
                k = -j;
            
            if (gates[g_i + k].ids.back() == Gate::Type::T ||
                gates[g_i + k].ids.back() == Gate::Type::Z) {
                
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

void Circuit::
GroupSimilarGates()
{
    idx_size last_CZ = 0, last_T = 0, last_X = 0, last_Y = 0;
    bool saw_CZ = false, saw_T = false, saw_X = false, saw_Y = false;
    idx_size g_i = (idx_size)qubits;
    
    for (idx_size j = qubits; j < gates.size()
         && (g_i + last_Y + last_X + last_T + last_CZ) < gates.size(); ++j) {
        
        if (gates[j].ids.back() == Gate::Type::Z) {
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
        else if (gates[j].ids.back() == Gate::Type::T) {
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
        else if (gates[j].ids.back() == Gate::Type::X_1_2) {
            saw_X = true;
            if (saw_T || saw_CZ || saw_Y)
                swap(gates[g_i + last_X + last_T + last_CZ], gates[j]);
            
            ++last_X;
        }
        else if (gates[j].ids.back() == Gate::Type::Y_1_2) {
            saw_Y = true;
            if (saw_T || saw_X || saw_CZ)
                swap(gates[g_i + last_Y + last_X + last_T + last_CZ], gates[j]);
            
            ++last_Y;
        }
    }
#ifdef PrintG
    PrintGates();
#endif
}

int Circuit::
MovexCZGates(bitset<128>& a_qubits_bitmask,
             bitset<128>& b_qubits_bitmask)
{
    const int modified_q = qubits - 1;
    int total_xCZ_count = 0, count_CZ = 0;
  
    for (idx_size i = qubits; i < gates.size(); ++i) {
         if (gates[i].ids.back() == Gate::Type::Z) {
             idx_size count_xCZ = 0;
             idx_size j = i;
             for (; j < gates.size() && gates[j].ids.back() == Gate::Type::Z; ++j) {
                 ++count_CZ;
                 bitset<128> a_bm = 0, b_bm = 0;
                 a_bm[modified_q - gates[j].qubits.front()] = 1;
                 b_bm[modified_q - gates[j].qubits.back()] = 1;
                 if ((a_bm & a_qubits_bitmask) != 0) {
                     if ((b_bm & a_qubits_bitmask) == 0) {
//                         cout << "Gate " << j << " : " << gates[j].qubits[0] << ", " << gates[j].qubits[1] << " q\n";
                         swap(gates[i + count_xCZ++], gates[j]);
                     }
                 }
                 else if ((b_bm & a_qubits_bitmask) != 0) {
                     if ((a_bm & a_qubits_bitmask) == 0) {
//                         cout << "Gate " << j << " : " << gates[j].qubits[0] << ", " << gates[j].qubits[1] << " q\n";
                         swap(gates[i + count_xCZ++], gates[j]);
                     }
                 }
                 else if ((b_bm & b_qubits_bitmask) != 0) {
                     if ((a_bm & b_qubits_bitmask) == 0) {
//                         cout << "Gate " << j << " : " << gates[j].qubits[0] << ", " << gates[j].qubits[1] << " q\n";
                         swap(gates[i + count_xCZ++], gates[j]);
                     }
                 }
                 else if ((a_bm & b_qubits_bitmask) != 0) {
                     if ((b_bm & b_qubits_bitmask) == 0) {
//                         cout << "Gate " << j << " : " << gates[j].qubits[0] << ", " << gates[j].qubits[1] << " q\n";
                         swap(gates[i + count_xCZ++], gates[j]);
                     }
                 }
             }
             sort(gates.begin() + i, gates.begin() + i + count_xCZ,
                  [](Gate& g1, Gate& g2){ return g1.qubits.front() < g2.qubits.front();});
             sort(gates.begin() + i, gates.begin() + i + count_xCZ,
                  [](Gate& g1, Gate& g2) {
                        if (g1.qubits.front() == g2.qubits.front())
                            return g1.qubits.back() > g2.qubits.back();
                        else
                            return false;
                  });
             i = j;
             total_xCZ_count += count_xCZ;
         }
    }
//    cout << "The circuit has " << total_xCZ_count << " xCZ gates\n";
//    cout << "The circuit has " << count_CZ << " CZ gates\n";
    return total_xCZ_count;
}

void Circuit::
PrintGates() const
{
    for (auto& g : gates) {
        if(g.ids.back() == Gate::Type::Z)
            cout << "CZ ";
        else if (g.ids.back() == Gate::Type::X_1_2)
            cout << "X ";
        else if (g.ids.back() == Gate::Type::Y_1_2)
            cout << "Y ";
        else if (g.ids.back() == Gate::Type::T)
            cout << "T ";
        else if (g.ids.back() == Gate::Type::Hadamard)
            cout << "H ";
    }
    cout << "\n\n";
}

void Circuit::
PrintGatesAndCycles() const
{
    idx_size j = 0;
    for (idx_size i = 0; i < clock_cycles.size(); ++i) {
        cout << i << endl;
        
        for(;j < clock_cycles[i]; ++j) {
            auto& g = gates[j];
            if(g.ids.back() == Gate::Type::Z)
                cout << "CZ ";
            else if (g.ids.back() == Gate::Type::X_1_2)
                cout << "X ";
            else if (g.ids.back() == Gate::Type::Y_1_2)
                cout << "Y ";
            else if (g.ids.back() == Gate::Type::T)
                cout << "T ";
            else if (g.ids.back() == Gate::Type::Hadamard)
                cout << "H ";
        }
        cout << "\n";
    }
    cout << "\n\n";
}

void Circuit::
CreateGoogleCircuit(int q, int num_clock_cycles)
{
    srand(time(NULL));
    google = true;
    
    short GS_gates[3] = {Gate::Type::X_rotation, Gate::Type::Y_rotation, Gate::Type::T};
    
    int GS_gates_num = 3;
    
    //Each row represents a qubit. This is to keep track of the
    //gates applied to this qubit.
    qubits = q;
    vector<vector<idx_size>> qubit_to_gates(qubits);
  
    auto count_wires_gate = [&](vector<short>& CZ_pairs) {
        int count = 0;
        for (idx_size i = 0; i < qubit_to_gates.size(); ++i) {
            if (gates[qubit_to_gates[i].back()].
                ids[0] == Gate::Type::Control) {
                CZ_pairs.push_back(i);
                count += 1;
            }
        }
        return count;
    };
    
    vector<bool> T_gate_allowed(qubits, true);
    
    //Start by applying Hadamard Gates
    for (int i = 0; i < qubits; ++i) {
        classical_bits.push_back(0);
        Gate temp = create_hadamard();
        temp.qubits.push_back(i);
        gates.push_back(move(temp));
        qubit_to_gates[i].push_back(gates.size());
    }
    
    clock_cycles.push_back(gates.size());
    
    for (int i = 0; i < num_clock_cycles; i+=2) {
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
            qubit_to_gates[CZ_q].push_back(gates.size());
            
            current_CZ_pairs[j] = ++CZ_q;
        }
        qubit_to_gates[CZ_q].push_back(gates.size());
        
        //control phase Gate
        int k = 0;
        for (idx_size j = 0; j < current_CZ_pairs.size()/2; ++j) {
            Gate temp = create_Z();
            temp.ids.insert(temp.ids.begin()
                                            , Gate::Type::Control);
            temp.num_controls = 1;
            temp.qubits.push_back(current_CZ_pairs[k++]);
            temp.qubits.push_back(current_CZ_pairs[k++]);
            gates.push_back(move(temp));
        }
        
        if (current_CZ_pairs.size() > 0) {
            clock_cycles.push_back(gates.size());
        }
        
        /*
         • Place a Gate at qubit q only if this qubit is occupied by a CZ Gate in the previous cycle.
         • Place a T Gate at qubit q if there are no single- qubit gates in the previous cycles at
         qubit q except for the initial cycle of Hadamard gates.
         • Any Gate at qubit q should be different from the Gate at qubit q in the previous cycle.
         */
        
        vector<short> CZ_pairs;
        int qubits_for_gates = 1 + rand() % (count_wires_gate(CZ_pairs) - 1);
        vector<short> complied_qubits(qubits_for_gates, -1);
        
        for (int j = 0; j < qubits_for_gates; ++j) {
            int q = rand() % qubits_for_gates;
            while(gates[qubit_to_gates[CZ_pairs[q]].back()].ids[0]
                  != Gate::Type::Control) {
                q = rand() % qubits_for_gates;
            }
            complied_qubits[j] = CZ_pairs[q];
            
            short gate_to_apply;
            if (T_gate_allowed[CZ_pairs[q]]) {
                gate_to_apply = rand() % GS_gates_num;
                while (gates[qubit_to_gates[CZ_pairs[q]].back()].ids[0]
                       == GS_gates[gate_to_apply]) {
                    gate_to_apply = rand() % GS_gates_num;
                }
                if (gate_to_apply == 2) {
                    T_gate_allowed[CZ_pairs[q]] = false;
                }
            }
            else {
                gate_to_apply = rand() % (GS_gates_num - 1);
                while (gates[qubit_to_gates[CZ_pairs[q]].back()].
                       ids[0] == GS_gates[gate_to_apply]) {
                    gate_to_apply = rand() % (GS_gates_num - 1);
                }
            }
            
            Gate temp;
            
            if (gate_to_apply == 0) {
                temp = create_X_1_2();
            }
            else if (gate_to_apply == 1) {
                temp = create_Y_1_2();
            }
            else if (gate_to_apply == 2) {
                temp = create_T();
            }
            temp.qubits.push_back(CZ_pairs[q]);
            qubit_to_gates[CZ_pairs[q]].push_back(gates.size());
            gates.push_back(move(temp));
        }
        if(qubits_for_gates > 0)
           clock_cycles.push_back(gates.size());
    }
    
    //    g_t_q_size = gates_to_qubits.size();
    //    q_circuit -> gates_to_qubits.resize(q_circuit -> gates_to_qubits.size() + qubits);
    //    for (idx_size i = 0; i < qubits; ++i) {
    //        Gate temp = create_I();
    //        temp -> id.push_back(Gate::Type::Measurement);
    //        q_circuit -> gates.push_back(temp);
    //        q_circuit -> gates_to_qubits[g_t_q_size + i].push_back(i);
    //    }
}

void Circuit::
CreateQuiddProScript(const string& out_file)
{
    ofstream file;
    file.open(out_file);
    
    file << "state = cb(\"" + to_string((int)real(classical_bits[0]))<< "\");\n";;
    
    for (idx_size q = 1; q < classical_bits.size(); ++q) {
        file << "state = kron(state, cb(\"" + to_string(classical_bits[q]) << "\"));\n";
    }
    
    for (idx_size i = 0; i < gates.size(); ++i) {
        if(i != 0) {
            file << ";\n";
        }
        int op_count = 0;
        bool control = false;
        bool continue_ = false;
        for (idx_size gt = 0; gt < gates[i].ids.size(); ++gt) {
            auto g = gates[i].ids[gt];
            
            if ( g == Gate::Type::Measurement) {
                file << "measure_sv(" + to_string(gates[i].qubits[0] + 1)
                + ", state)";
                continue_ = true;
                continue;
            }
            
            if (g == Gate::Type::T) {
                file << "T = [1 0 ; 0 " + to_string(0.707106781) + "+i*" + to_string(0.707106781) + "];\n";
                file << "op" + to_string(op_count++) + " = cu_gate (T, \"";
            }
            else if (g == Gate::Type::X_1_2) {
                file << "X_1_2 = [0.5+i*0.5 0.5-i*0.5 ; 0.5-i*0.5 0.5+i*0.5];\n";
                file << "op" + to_string(op_count++) + " = cu_gate (X_1_2, \"";
            }
            else if (g == Gate::Type::Y_1_2) {
                file << "Y_1_2 = [0.5+i*0.5 -0.5-i*0.5 ; 0.5+i*0.5 0.5+i*0.5];\n";
                file << "op" + to_string(op_count++) + " = cu_gate (Y_1_2, \"";
            }
            else if (g != Gate::Type::Control ) {
                file << "op" + to_string(op_count++)  + " = cu_gate(" + quiddpro_func[g] + "(";
                
                if (g < Gate::Type::X_rotation || g == Gate::Type::Phase) {
                    file << "1), \"";
                }
                else if (g < Gate::Type::Control) {
                    file << to_string(gates[i].theta[gt]) + " * pi, 1), \"";
                }
            }
            
            if (g != Gate::Type::Control) {
                int q = 0;
                if (control) {
                    for (; q < gates[i].num_controls; ++q) {
                        file << "c" + to_string(gates[i].qubits[q] + 1);
                    }
                }
                for (; q < (int)gates[i].qubits.size(); ++q) {
                    file << "x" + to_string(gates[i].qubits[q] + 1);
                }
                file << "\", " + to_string(qubits) << ");\n";
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
                if (n != 0)
                    file << "-";
                
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

void Circuit::
WriteGeneratedCircuitFile(const string& out_file,
                          const idx_size size_q)
{
    if ( size_q == 0)
        throw "Create Circuit first by entering qubits";
    
    ofstream file;
    file.open(out_file);
    
    file << qubits << "\n";
    
    for (idx_size i = 0; i < classical_bits.size(); ++i)
        file << classical_bits[0] << " ";
    file << "\n";
    for (idx_size i = 0; i < size_q; ++i) {
        file << 0;
        file << " " ;
    }
    file << "\n";
    
    idx_size size_g = gates.size();
    
    if ( size_g == 0)
        throw "Create Circuit first by entering gates";
    
    for (idx_size i = 0; i < size_g; ++i) {
        idx_size size_r = gates[i].rows.size();
        for(size_t j = 0; j < size_r; ++j) {
            int size_c = (int)gates[i].rows[j].size();
            for (int k = 0; k < size_c; ++k) {
                file << real(gates[i].rows[j][k]);
                
                if (imag(gates[i].rows[j][k]) != 0)
                    file << " + " << imag(gates[i].rows[j][k]) << " i";
                
                if (k != size_c-1)
                    file << " , ";
                
            }
            file << " & ";
        }
        file << "\n";
        file << "g ";
        for ( auto gate_num : gates[i].ids)
            file << gate_num << " ";
        
        file << "\nt ";
        for ( auto theta : gates[i].theta)
            file << theta << " ";
        
        
        file << "\n";
        file << "q ";
        for (const auto& q : gates[i].qubits) {
            file << q << " ";
        }
        
        file << "\n";
        file << "c " << gates[i].num_controls << "\n";
    }
    file.close();
}

//TO DO:add support for clock ccyles
void Circuit::
ReadCustomInputFiles(const string& input_file,
                     cmplx*& amp,
                     idx_size& size)
{
    google = true;
    ifstream file;
    file.open(input_file);
    string input;
    
    file >> qubits;
    
    getline(file, input);
    getline(file, input);
    int classical;
    istringstream css(input);
    while (css >> classical)
        classical_bits.push_back(classical);
    
    amp = new cmplx [1ull << qubits];
    size = 1ull << qubits;
    getline(file, input);
    cmplx amp_temp(0,0);
    double theta;
    int q, gt;
    char delim = '_';
    istringstream ss(input);
    int vi = 0;
    while (ss >> amp_temp) {
        amp[vi] = amp_temp;
        ++vi;
    }
    
    while(getline(file, input)) {
        istringstream iss(input);
        
        double re, im;
        char sign = '_' , i;
        delim = '_';
        Gate g = Gate();
        
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
                g.ids.push_back(gt);
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
        }
        
        getline(file, input);
        istringstream cis(input);
        cis >> type;
        if (type == 'c') {
            cis >> g.num_controls;
        }
        gates.push_back(g);
    }
}

void Circuit::
ReadGoogleCircuitFile(const string& input_file,
                      const int depth)
{
    google = true;
    ifstream file;
    file.open(input_file);
    
    file >> qubits;
    
    for (int i = 0; i < qubits; ++i)
        classical_bits.push_back(0);
    
    short current_cycle = 0, prev_cycle = 0;
    string gate_type;
    short q1, q2;
    while (file >> current_cycle >> gate_type) {
        
        if (current_cycle >= depth && depth != 0)
            break;
        if (prev_cycle != current_cycle) {
            clock_cycles.push_back(gates.size());
            prev_cycle = current_cycle;
        }
        
        if(gate_type == "h")
            gates.push_back(create_hadamard());
        
        else if (gate_type == "t")
            gates.push_back(create_T());
        
        else if (gate_type == "y_1_2")
            gates.push_back(create_Y_1_2());
        
        else if (gate_type == "x_1_2")
            gates.push_back(create_X_1_2());
        
        else {
            file >> q1 >> q2;
            gates.push_back(create_Z());
            gates[gates.size() - 1].
            ids.insert(gates[gates.size() - 1].ids.begin(),
                                       Gate::Type::Control);
            gates[gates.size() - 1].num_controls = 1;
            gates[gates.size() - 1].qubits.push_back(qubits - 1 - q2);
            gates[gates.size() - 1].qubits.push_back(qubits - 1 - q1);
        }
        
        if (gate_type != "cz") {
            file >> q1;
            gates[gates.size() - 1].qubits.push_back(qubits - 1 - q1);
        }
    }
    clock_cycles.push_back(gates.size());
}

int Circuit::
GetNumQubits() const
{
    return qubits;
}

idx_size Circuit::
GetTotalNumGates() const
{
    return gates.size();
}

bool Circuit::
ClockCycleEmpty() const
{
    return clock_cycles.empty();
}

int Circuit::
GateIndexForCycle(int cycle_num) const
{
    if (cycle_num < 0 || cycle_num > (int)clock_cycles.size())
        return -1;
    
    return (int)clock_cycles[cycle_num] - 1;
}

Gate& Circuit::
GetGateFromIndex(idx_size i)
{
    return gates[i];
}

idx_size Circuit::
GetNumCycles() const
{
    return clock_cycles.size();
}

const vector<Gate>& Circuit::
GetGates() const
{
    return gates;
}
