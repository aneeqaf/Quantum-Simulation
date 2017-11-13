//
//  main.cpp
//  Quantum Circuits
//
//  Created by Aneeqa Fatima on 10/13/17.
//

/*
 * Input File Format:
 * 
 */

#include <sstream>
#include <fstream>
#include <getopt.h>
#include "circuit_simulation.h"
#include "circuit_generator.h"

using namespace std;

int main(int argc, char *argv[]) {
    
#ifdef __APPLE__
    if (getenv("STDIN")) {
        if (!freopen(getenv("STDIN"), "r", stdin)) {
            cerr << "Unable to redirect stdin to " << getenv("STDIN") << endl;
            exit (1);
        }
    }
#endif
    
    static struct option longopts[] = {
        { "inputfile",    required_argument,       nullptr, 'i' },
        { "create",    required_argument,       nullptr, 'c' },
        { "google",    required_argument,       nullptr, 'g' },
        { "outfile",    required_argument,       nullptr, 'o' },
        { "test",    no_argument,       nullptr, 't' },
        { nullptr,  0,                 nullptr, '\0' }
    };
    
    int c = 0;
    bool inputfile = false, create = false, to_write = false,
    test = false, google = false;;
    int idx = 0;
    
    string input_filename = "", out_file = "";
    int numQ = 0, numG = 0;
    vector<int> num_qubits, num_gates;
    
    while ((c = getopt_long(argc, argv, "i:c:o:g:t", longopts, &idx)) != -1)
    {
        switch (c) {
            case 'i': {
                inputfile = true;
                if (argc < 2) {
                    cerr << "Please enter filename\n";
                    exit(1);
                }
                input_filename = string(optarg);
                break;
            }
            case 'g':
                google = true;
            case 'c': {
                create = true;
                if (argc < 3) {
                    cerr << "Please enter number of qubits and number of gates in circuit\n";
                    exit(1);
                }
                string opt = "";
                int i = 0;
                for (; optarg[i] != '-' && optarg[i] != '_'; ++i) {
                    if(optarg[i] == '\0')
                        opt += " ";
                    else
                        opt += optarg[i];
                }
                istringstream iss(opt);
                
                while (iss >> numQ >> numG) {
                    num_qubits.push_back(numQ);
                    num_gates.push_back(numG);
                }
                
                break;
            }
            case 'o': {
                to_write = true;
                if (create == false) {
                    cerr << "Please specify what circuit to create first\n";
                    exit(1);
                }
                out_file = string(optarg);
                break;
            }
            case 't': {
                test = true;
                break;
            }
            default: {
                cerr << "Unknown option " << c << '\n';
                exit(1);
                break;
            }
        } // switch
    } // while
    
    circuit test_circuit;
    circuit_generator new_circuit;
    if (inputfile) {
        new_circuit.read_input_file(input_filename);
        
        if(test) {
            test_circuit = *(new_circuit.q_circuit);
        }
        
        new_circuit.q_circuit -> simulate(out_file);
        new_circuit.q_circuit -> print_state();
        new_circuit.q_circuit -> print_probabilities(out_file);
    }
    if(create) {
        for (int i = 0; i < num_qubits.size(); ++i){
            if (google) {
                new_circuit.create_google_rand_circuit(num_qubits[i], num_gates[i]);
               // new_circuit.create_quiddpro_script(out_file + to_string(i) + ".qpro");
            }
            else {
                new_circuit.create_rand_circuit(num_qubits[i], num_gates[i]);
                new_circuit.create_quiddpro_script(out_file + to_string(i) + ".qpro");
            }
            
            if(test) {
                test_circuit = *(new_circuit.q_circuit);
            }
            
            if(to_write) {
                new_circuit.write_circuit_to_file(out_file + to_string(i) + ".txt");
            }
            
            new_circuit.q_circuit -> simulate(out_file);
            new_circuit.q_circuit -> print_state();
            new_circuit.q_circuit -> print_probabilities(out_file);
        }
    }
//    if(test) {
//        if( test_circuit.test(*(new_circuit.q_circuit)) == 1) {
//            cout << "Pass!\n";
//        }
//        else {
//            test_circuit.print_state();
//            new_circuit.q_circuit -> print_state();
//            cout << "Fail!\n";
//        }
//    }
    
    return 0;
    
}

