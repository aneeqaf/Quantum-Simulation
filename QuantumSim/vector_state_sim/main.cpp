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

string ReadMultipleArgs(char* optarg)
{
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

bool CheckIfGoogleFile(string filename)
{
    auto does_file_exist = [](const string& fileName)
    {
        ifstream infile(fileName.c_str());
        return (bool)infile.good();
    };
    
    if (does_file_exist("input/random_circuits_google/" + filename))
        return true;
    return false;
}

bool CheckIfRollRightFile(string filename)
{
    auto does_file_exist = [](const string& fileName)
    {
        ifstream infile(fileName.c_str());
        return (bool)infile.good();
    };
    
    if (does_file_exist("input/random_circuits_rollright/" + filename))
        return true;
    return false;
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
    
    if (argc < 2) {
        cerr << "\nUse the -h option to view the command line options necessary for the simulator to run.\n\n";
        exit(1);
    }
    
    static struct option longopts[] = {
        { "approx",    no_argument,       nullptr, 'a' },
        { "inputfile",    required_argument,       nullptr, 'i' },
        { "idx",    required_argument,       nullptr, 'x' },
        { "low_value_q",    required_argument,       nullptr, 'l' },
        { "num_threads",    required_argument,       nullptr, 't' },
        { "depth",    required_argument,       nullptr, 'd' },
        { "google_spec",    required_argument,       nullptr, 'g' },
        { "outfile",    required_argument,       nullptr, 'o' },
        { "sim_type",    required_argument,       nullptr, 's' },
        { "vcut",    required_argument,       nullptr, '|' },
        { "hcut",    required_argument,       nullptr, '_' },
        { "verbose",    required_argument,       nullptr, 'v' },
        { "CZ_path",    required_argument,       nullptr, 'c' },
        { "norm_est",    required_argument,       nullptr, 'n' },
        { "help",    no_argument,       nullptr, 'h' },
        { nullptr,  0,                 nullptr, '\0' }
    };
    
    bool rollrightInput = false, googleInput = false, create = false, to_write = false, print_amp = false,
    print_idx = false, valid = false, ascii = false, approx = false;
    string input_filename = "", out_file = "", idx_filename = "" ;
    int numQ = 0, numG = 0, threshold = 15, depth = 26, vcut = 0, hcut = 0, idx = 0, c = 0, seed = -1, num_idx = -1,
    num_threads = 8, dfs_length = 0, cz_len = 0, czp_app_len = 0, norm_depth = 0;
    float norm_perc = 0;
    idx_size cz_path = 0;
    int sim_type = -1;
    Config::Verbose verbose = Config::Default;
    vector<int> num_qubits, num_gates;
    
#ifdef Parallel
    num_threads = omp_get_num_procs();
#endif
    
    while ((c = getopt_long(argc, argv, "ai:o:g:t:d:s:|:v:_:x:f:c:n:h", longopts, &idx)) != -1)
    {
        switch (c) {
            case 'a': {
                approx = true;
                break;
            }
            case 'c': {
                if (norm_perc) {
                    cerr << "Please specify either norm estimation or CZ_path simulation.\n";
                    exit(1);
                }
                
                string cz_path_temp = string(optarg);
                
                if (cz_path_temp.find(",") != string::npos) {
                    idx_size pos2 = 0;
                    cz_len = stoi(cz_path_temp.substr(0, cz_path_temp.find(",")));
                    auto count_commas =  count(cz_path_temp.begin(), cz_path_temp.end(), ',');
                    
                    if (count_commas < 1) {
                        cerr << "Invalid CZ path";
                        exit(1);
                    }
                    
                    if (cz_len) {
                        idx_size pos1 = cz_path_temp.find(",");
                        
                        if (count_commas > 1) {
                            pos2 = cz_path_temp.find(",", pos1 + 1);
                            cz_path = stoul(cz_path_temp.substr(pos1 + 1, pos2 - pos1));
                        }
                        else cz_path = stoul(cz_path_temp.substr(pos1 + 1));


                        if (count_commas == 2)
                            czp_app_len = stoi(cz_path_temp.substr(pos2 + 1));
                        else if (count_commas > 2)
                            czp_app_len = stoi(cz_path_temp.substr(pos2 + 1,
                                                                   cz_path_temp.find_last_of(",") - pos2));
                        
                        if (count_commas == 3)
                            dfs_length = stoi(cz_path_temp.substr(cz_path_temp.find_last_of(",") + 1));
                    }
                }
                else {
                    cz_path = (int)bitset<128>(cz_path_temp).to_ulong();
                    czp_app_len = 0;
                    if (cz_path_temp.find("*") != string::npos)
                        dfs_length = (int)(cz_path_temp.find_last_of("*") - cz_path_temp.find_first_of("*"));
                }
                break;
            }
            case 'd': {
                string s_d = string(optarg);
                depth = stoi(s_d);
                break;
            }
            case 'f': {
                string s_th = string(optarg);
                threshold = stoi(s_th);
                break;
            }
            case 'g': {
                valid = true;
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
            case 'h':{
                cout
                << "\n--idx, -x       \t: int<seed>,int<num_indices> or int<seed>,int<num_indices>+\n"
                << "--low_value_qubits, -l \t: int<base case threshold for XYFastTransform>\n"
                << "--num_threads, -t \t: int<max_num_threads>\n"
                << "--depth, -d     \t: int<depth of circuit to be simulated>\n"
                << "--inputfile, -i \t: string<circuit file>\n"
                << "--outfile, -o   \t: string<filename to print output to>\n"
                << "--sim_type, -s  \t: int<type of sim> (Please refer to the manual)\n"
                << "--vcut, -a      \t: int<cut_size>\n"
                << "--hcut, -b      \t: int<cut_size>\n"
                << "--verbose, -v   \t: int<verbosity level of report> (Please refer to the manual)\n"
                << "--CZ_path, -c   \t: string<0s and 1s> or int<length_bitstring>,int<value_bitstring>\n\n"
                << "Please refer to the manual for more options.\n\n";
                return 0;
                break;
            }
            case 'i': {
                valid = true;
                if (argc < 2) {
                    cerr << "Please enter filename\n";
                    exit(1);
                }
                input_filename = string(optarg);
                break;
            }
            case 'o': {
                to_write = true;
                if (!valid) {
                    cerr << "Please specify what circuit to create first\n";
                    exit(1);
                }
                out_file = string(optarg);
                auto count_a = count(out_file.begin(), out_file.end(), '@');
                if (count_a)
                    ascii = true;
                idx_size p = out_file.find(",");
                if (p != string::npos)
                    out_file = out_file.substr(0, p);
                break;
            }
            case 'n': {
                if (cz_len) {
                    cerr << "Please specify either norm estimation or CZ_path simulation.\n";
                    exit(1);
                }
                string n_str = string(optarg);
                if (n_str.find(",") == string::npos) {
                    cerr << "Missing argument in the norm estimation mode\n";
                    exit(1);
                }
                norm_perc = stof(n_str.substr(0, n_str.find(",")));
                norm_depth = stoi(n_str.substr(n_str.find(",") + 1));
                break;
            }
            case 's': {
                string s_type = string(optarg);
                sim_type = (Config::SimType)stoi(s_type);
                break;
            }
            case 'v': {
                string s_v = string(optarg);
                verbose = (Config::Verbose)stoi(s_v);
                break;
            }
            case 't': {
                string threads = string(optarg);
                num_threads = stoi(threads);
#ifdef Parallel
                if (num_threads > omp_get_max_threads()) {
                    cerr << "Number of threads specified greater than max number of threads\n";
                    exit(1);
                }
#endif
                break;
            }
            case 'x': {
                if (argc < 2) {
                    cerr << "Please enter filename\n";
                    exit(1);
                }
                string idx_arg = string(optarg);
                print_amp = true;
                if (idx_arg.find(",") != string::npos) {
                    string seed_str = idx_arg.substr(0, idx_arg.find_first_of(","));
                    string num_idx_str = idx_arg.substr(idx_arg.find_first_of(",") + 1, idx_arg.find_first_of("+"));
                    seed = stoi(seed_str);
                    num_idx = stoi(num_idx_str);
                    if (num_idx > 10000)
                        throw "Cannot print more than 10000 amps";
                    print_idx = idx_arg.find_first_of("+") != string::npos;
                }
                else
                    idx_filename = idx_arg;
                
                break;
            }
            default: {
                cerr << "Unknown option " << c << '\n';
                exit(1);
                break;
            }
        } // switch
    } // while
    
    if (!valid) {
        cerr << "\nInvalid command line options. Please specify either the input filename or circuit generation. Use -h for more info.\n\n";
        exit(1);
    }
    
    if (input_filename != "") {
        googleInput = CheckIfGoogleFile(input_filename);
        if (!googleInput)
            rollrightInput = CheckIfRollRightFile(input_filename);
        if (!googleInput && !rollrightInput) {
            cerr << "Input file not found\n";
            exit(1);
        }
    }
    
    Circuit cir;
    if (rollrightInput || googleInput) {
        cmplx* amp_v = nullptr;
        idx_size size = 0;
        //write a function for printing google files.
        if (rollrightInput) {
            cir.ReadCustomInputFiles("input/random_circuits_rollright/" + input_filename, amp_v, size);
            delete [] amp_v;
            amp_v = nullptr;
        }
        else
            cir.ReadGoogleCircuitFile("input/random_circuits_google/" + input_filename, depth);
    }
    else if(create) {
        for (idx_size i = 0; i < num_qubits.size(); ++i){
            cir.CreateGoogleCircuit(num_qubits[i], num_gates[i]);
            
            if(to_write && num_qubits[0] <= 20) {
                cir.WriteGeneratedCircuitFile("input/random_circuits_rollright/" + out_file +
                                              to_string(i) + ".txt", cir.GetNumQubits());
                cir.CreateQuiddProScript("output/qpro_scripts/" + out_file + to_string(i) + ".qpro");
            }
        }
    }
    
    if (sim_type == -1) {
        if (cir.GetNumQubits() <= 32) sim_type = 5;
        else sim_type = 0;
    }
    
    Config config(1ull << cir.GetNumQubits(), input_filename ,"output/probabilities/" + out_file,
                  "output/amp_vectors/" + out_file,  "output/reports/" + out_file, "output/misc", norm_perc, norm_depth,
                  cz_path, czp_app_len, cz_len, dfs_length, approx, ascii, print_amp, print_idx, (Config::SimType) sim_type, verbose, vcut, hcut, depth,
                  threshold, num_threads);
    
    if (print_amp) {
        if (seed != -1)
            config.GenerateRandomIndices(seed, num_idx, 1ull << cir.GetNumQubits());
        else
            config.ReadIndices(idx_filename);
    }
    SequentialSimulation sim(config);
    
    if (config.verbose)
        sim.PrintSystemReport();
    
    GenericQuantumState::num_threads = num_threads;
    if (sim_type == Config::FullState) {
        FullAmpStateVector amp(cir.GetNumQubits());
        sim.Simulate(amp, cir);
    }
    else if (sim_type == Config::Approx1CutH || sim_type == Config::Approx2011 || sim_type == Config::Approx_i11i
             || sim_type == Config::Approx1_101 || sim_type == Config::Approx1110) {
        TensorProductStateVector amp (cir.GetNumQubits(),
                                      QubitPartition::Cuts::Horizontal, hcut, vcut,
                                      (Config::SimType)sim_type);
        sim.Simulate(amp, cir);
    }
    else if (sim_type == Config::Approx1CutV) {
        TensorProductStateVector amp (cir.GetNumQubits(),
                                      QubitPartition::Cuts::Vertical, hcut, vcut,
                                      (Config::SimType)sim_type);
        sim.Simulate(amp, cir);
    }
    else if (sim_type == Config::Approx2011OWT || sim_type == Config::Approx_i11iOWT || cz_len != 0) {
        SumOfTensorsProductsStateVector amp (cir.GetNumQubits(), (Config::SimType)sim_type, hcut, vcut);
        sim.Simulate(amp, cir);
    }
    else {
        AdaptiveStateVector amp(cir.GetNumQubits(),
                                (Config::SimType)sim_type, hcut, vcut);
        sim.Simulate(amp, cir);
    }
    return 0;
    
}

