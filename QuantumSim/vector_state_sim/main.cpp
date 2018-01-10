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
#include "../sim/simulation.h"

using namespace std;

int main(int argc, char *argv[])
{    
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
        { "threshold",    required_argument,       nullptr, 't' },
        { "google",    required_argument,       nullptr, 'g' },
        { "googleInput",    required_argument,       nullptr, 'h' },
        { "outfile",    required_argument,       nullptr, 'o' },
        { nullptr,  0,                 nullptr, '\0' }
    };
    
    int c = 0;
    bool inputfile = false, googleInput = false, create = false, to_write = false;
    int idx = 0;
    
    string input_filename = "", out_file = "";
    int numQ = 0, numG = 0, threshold = 0;
    vector<int> num_qubits, num_gates;
    
    while ((c = getopt_long(argc, argv, "i:o:g:h:t:", longopts, &idx)) != -1)
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
            case 't': {
                string s_th = string(optarg);
                threshold = stoi(s_th);
                break;
            }
            case 'g': {
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
    
    Circuit cir;
    SequentialSimulation sim;
    if (inputfile || googleInput) {
        if(googleInput)
            cout << "Google circuit file: " << input_filename << "\n\n";
        
        cmplx* amp_v = nullptr;
        idx_size size = 0;
        //write a function for printing google files.
        if (inputfile) {
            cir.ReadCustomInputFiles("input/random_circuits_aneeqa/" + input_filename, amp_v, size);
            State amp(amp_v, size);
            sim.Simulate("output/probabilities/" + out_file, amp, cir, threshold);
            delete [] amp_v;
            amp_v = nullptr;
        }
        else {
            cir.ReadGoogleCircuitFile("input/random_circuits_google/" + input_filename, 26);
            State amp(cir.GetNumQubits());
            sim.Simulate("output/probabilities/" + out_file, amp, cir, threshold);
        }
    }
    else if(create) {
        for (idx_size i = 0; i < num_qubits.size(); ++i){
            cir.CreateGoogleCircuit(num_qubits[i], num_gates[i]);
            
            State amp(cir.GetNumQubits());
            if(to_write && num_qubits[0] <= 20) {
                cir.WriteGeneratedCircuitFile("input/random_circuits_aneeqa/" + out_file + to_string(i) + ".txt",
                                              amp.GetAmp(), cir.GetNumQubits());
                cir.CreateQuiddProScript("output/qpro_scripts/" + out_file + to_string(i) + ".qpro");
            }
            
            sim.Simulate("output/probabilities/" + out_file, amp, cir, threshold);
        }
    }
    return 0;
    
}

