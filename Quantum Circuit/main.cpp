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
    
    google = false;
    
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
        { "googleInput",    required_argument,       nullptr, 'h' },
        { "outfile",    required_argument,       nullptr, 'o' },
        { "test",    no_argument,       nullptr, 't' },
        { nullptr,  0,                 nullptr, '\0' }
    };
    
    int c = 0;
    bool inputfile = false, googleInput = false, create = false, to_write = false,
    test = false, google_c = false;;
    int idx = 0;
    
    string input_filename = "", out_file = "";
    int numQ = 0, numG = 0;
    vector<int> num_qubits, num_gates;
    
    while ((c = getopt_long(argc, argv, "i:c:o:g:h:t", longopts, &idx)) != -1)
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
            case 'h': {
                googleInput = true;
                if (argc < 2) {
                    cerr << "Please enter filename\n";
                    exit(1);
                }
                input_filename = string(optarg);
                break;
            }
            case 'g':
                google_c = true;
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
                if (create == false && inputfile == false && googleInput == false) {
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
    circuit_generator new_circuit = {};
    if (inputfile || googleInput) {
        
         google = true;
        
        if (inputfile) {
            new_circuit.read_input_file("input/" + input_filename);
        }
        else {
            new_circuit.read_google_input_files(input_filename);
        }
        
        if(googleInput) {
            cout << "Google circuit file: " << input_filename << "\n\n";
            new_circuit.create_quiddpro_script("qpro_scripts/" + out_file + ".qpro");
        }
        
//        new_circuit.q_circuit -> circuit_preprocessing();
        new_circuit.q_circuit -> simulate("probabilities/" + out_file);
        new_circuit.q_circuit -> print_state("state/" + out_file);
    }
    else if(create) {
        for (int i = 0; i < num_qubits.size(); ++i){
            if (google_c) {
                new_circuit.create_google_rand_circuit(num_qubits[i], num_gates[i]);
            }
            else {
                new_circuit.create_rand_circuit(num_qubits[i], num_gates[i]);
            }
            
            if(to_write && num_qubits[0] <= 20) {
                new_circuit.write_circuit_to_file("input/" + out_file + to_string(i) + ".txt");
                new_circuit.create_quiddpro_script("qpro_scripts/" + out_file + to_string(i) + ".qpro");
            }
            
            
//            new_circuit.q_circuit -> circuit_preprocessing();
            new_circuit.q_circuit -> simulate("probabilities/" + out_file);
            new_circuit.q_circuit -> print_state("state/" + out_file);
        }
    }
    return 0;
    
}

