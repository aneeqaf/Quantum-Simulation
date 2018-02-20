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
#include "state.h"
#include "state_autoconv.h"
#include "state_sum_tensor.h"

using namespace std;

string ReadMultipleArgs(char* optarg) {
    string opt = "";
    int i = 0;
    for (; optarg[i] != '-' && optarg[i] != '_'; ++i) {
        if(optarg[i] == '\0')
            opt += " ";
        else
            opt += optarg[i];
    }
    return opt;
}

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
        { "depth",    required_argument,       nullptr, 'd' },
        { "google_spec",    required_argument,       nullptr, 'g' },
        { "google_input",    required_argument,       nullptr, 'h' },
        { "outfile",    required_argument,       nullptr, 'o' },
        { "sim_type",    required_argument,       nullptr, 's' },
        { "vcut_sizes",    required_argument,       nullptr, 'a' },
        { "hcut_sizes",    required_argument,       nullptr, 'b' },
        { "verbose",    required_argument,       nullptr, 'v' },
        { nullptr,  0,                 nullptr, '\0' }
    };
    
    bool inputfile = false, googleInput = false, create = false, to_write = false;
    string input_filename = "", out_file = "";
    int numQ = 0, numG = 0, threshold = -1, depth = 0, vcut = 0, hcut = 0, idx = 0, c = 0;
    Config::SimType sim_type = Config::FullState;
    Config::Verbose verbose = Config::Default;
    vector<int> num_qubits, num_gates;
    
    while ((c = getopt_long(argc, argv, "i:o:g:h:t:d:s:a:v:b:", longopts, &idx)) != -1)
    {
        switch (c) {
            case 'a': {
                string s_c = string(optarg);
                vcut = stoi(s_c);
                break;
            }
            case 'b': {
                string s_c = string(optarg);
                hcut = stoi(s_c);
                break;
            }
            case 'd': {
                string s_d = string(optarg);
                depth = stoi(s_d);
                break;
            }
            case 'g': {
                create = true;
                if (argc < 3) {
                    cerr << "Please enter number of qubits and number of gates in circuit\n";
                    exit(1);
                }
                string opt = ReadMultipleArgs(optarg);
                istringstream iss(opt);
                
                while (iss >> numQ >> numG) {
                    num_qubits.push_back(numQ);
                    num_gates.push_back(numG);
                }
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
            case 'i': {
                inputfile = true;
                if (argc < 2) {
                    cerr << "Please enter filename\n";
                    exit(1);
                }
                input_filename = string(optarg);
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
            case 's': {
                string s_type = string(optarg);
                sim_type = (Config::SimType)stoi(s_type);
                break;
            }
            case 't': {
                string s_th = string(optarg);
                threshold = stoi(s_th);
                break;
            }
            case 'v': {
                string s_v = string(optarg);
                verbose = (Config::Verbose)stoi(s_v);
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
    Config config(input_filename ,"output/probabilities/" + out_file, "output/amp_vectors/" + out_file,
                  "output/reports/" + out_file, "output/misc/g_" + out_file, sim_type, verbose, vcut, hcut,
                  depth, threshold);
    SequentialSimulation sim(config);
    if (inputfile || googleInput) {
        cmplx* amp_v = nullptr;
        idx_size size = 0;
        //write a function for printing google files.
        if (inputfile) {
            cir.ReadCustomInputFiles("input/random_circuits_rollright/" + input_filename, amp_v, size);
            delete [] amp_v;
            amp_v = nullptr;
        }
        else
            cir.ReadGoogleCircuitFile("input/random_circuits_google/" + input_filename, depth);
//            cir.CreateQuiddProScript("output/qpro_scripts/" + out_file + ".qpro");
    }
    else if(create) {
        for (idx_size i = 0; i < num_qubits.size(); ++i){
            cir.CreateGoogleCircuit(num_qubits[i], num_gates[i]);
            
            if(to_write && num_qubits[0] <= 20) {
                cir.WriteGeneratedCircuitFile("input/random_circuits_rollright/" + out_file + to_string(i) + ".txt",
                                              cir.GetNumQubits());
                cir.CreateQuiddProScript("output/qpro_scripts/" + out_file + to_string(i) + ".qpro");
            }
        }
    }
    
    if (sim_type == Config::FullState) {
        FullAmpStateVector amp(cir.GetNumQubits());
        sim.Simulate(amp, cir);
    }
    else if (sim_type == Config::Approx1CutH || sim_type == Config::Approx2011
             || sim_type == Config::Approx1_101 || sim_type == Config::Approx1110) {
        TensorProductStateVector amp (cir.GetNumQubits(),
                                      TensorProductStateVector::Cuts::Horizontal, hcut, vcut, sim_type);
        sim.Simulate(amp, cir);
    }
    else if (sim_type == Config::Approx1CutV) {
        TensorProductStateVector amp (cir.GetNumQubits(),
                                      TensorProductStateVector::Cuts::Vertical, hcut, vcut, sim_type);
        sim.Simulate(amp, cir);
    }
    else {
        AdaptiveStateVector amp(cir.GetNumQubits(),
                                (Config::SimType)sim_type, hcut, vcut);
        sim.Simulate(amp, cir);
    }
    return 0;
    
}

