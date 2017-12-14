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
#include "simulation.h"


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
        { "googleInput",    required_argument,       nullptr, 'h' },
        { "outfile",    required_argument,       nullptr, 'o' },
        { nullptr,  0,                 nullptr, '\0' }
    };
    
    int c = 0;
    bool inputfile = false, googleInput = false, create = false, to_write = false,
    google_c = false;
    int idx = 0;
    
    string input_filename = "", out_file = "";
    int numQ = 0, numG = 0;
    vector<int> num_qubits, num_gates;
    
    while ((c = getopt_long(argc, argv, "i:c:o:g:h:", longopts, &idx)) != -1)
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
            default: {
                cerr << "Unknown option " << c << '\n';
                exit(1);
                break;
            }
        } // switch
    } // while
    
    circuit cir;
    sequentialSimulation sim;
    if (inputfile || googleInput) {
        if(googleInput)
            cout << "Google circuit file: " << input_filename << "\n\n";
        
        valarray<cmplx> amp_v;
        //write a function for printing google files.
        if (inputfile) {
            cir.ReadCustomInputFiles("input/" + input_filename, amp_v);
            cir.CreateQuiddProScript("qpro_scripts/" + out_file + ".qpro");
            state amp(amp_v);
            sim.Simulate("probabilities/" + out_file, amp, cir);
        }
        else {
            cir.ReadGoogleCircuitFile(input_filename);
            state amp(cir.GetNumQubits());
            sim.Simulate("probabilities/" + out_file, amp, cir, 26);
        }
    }
    else if(create) {
        for (idx_size i = 0; i < num_qubits.size(); ++i){
            cir.CreateGoogleCircuit(num_qubits[i], num_gates[i]);
            
            state amp(cir.GetNumQubits());
            if(to_write && num_qubits[0] <= 20) {
                cir.WriteGeneratedCircuitFile("input/" + out_file + to_string(i) + ".txt",
                                              amp.GetAmp());
                cir.CreateQuiddProScript("qpro_scripts/" + out_file + to_string(i) + ".qpro");
            }
            
            sim.Simulate("probabilities/" + out_file, amp, cir);
        }
    }
    return 0;
    
}

