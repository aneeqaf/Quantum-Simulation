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
#include <sys/types.h>
#include <sys/stat.h>

#include "qftSimulation.h"
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
        { "approx",    required_argument,       nullptr, 'a' },
        { "inputfile",    required_argument,       nullptr, 'i' },
        { "idx",    required_argument,       nullptr, 'x' },
        { "high_value_q",    required_argument,       nullptr, 'q' },
        { "num_threads",    required_argument,       nullptr, 't' },
        { "depth",    required_argument,       nullptr, 'd' },
        { "outfile",    required_argument,       nullptr, 'o' },
        { "sim_type",    required_argument,       nullptr, 's' },
        { "vcut",    required_argument,       nullptr, '|' },
        { "hcut",    required_argument,       nullptr, '_' },
        { "verbose",    required_argument,       nullptr, 'v' },
        { "CZ_path",    required_argument,       nullptr, 'c' },
        { "norm_est",    required_argument,       nullptr, 'e' },
        { "grid_type",    required_argument,       nullptr, 'm' },
        { "no_nearest_neighbors",    no_argument,       nullptr, 'n' },
        { "add_concluding_H_gates",    required_argument,       nullptr, 'H' },
        { "no_checkpoint_ranges",    no_argument,       nullptr, 'p' },
        { "first_partition_smaller",    no_argument,       nullptr, 'f' },
        { "save_checkpoint_to_file",    required_argument,       nullptr, 'r' },
        { "count_zeros",    no_argument,       nullptr, '0' },
        { "Cramer",    required_argument,       nullptr, 'z' },
        { "QFT",    required_argument,       nullptr, 'Q' },
        { "circuit_reordering_mode", no_argument, nullptr, 'C'},
        { "help",    no_argument,       nullptr, 'h' },
        { nullptr,  0,                 nullptr, '\0' }
    };
    
    bool to_write = false, print_amp = false, write_circuit_mode = false,
    print_idx = false, valid = false, ascii = false, approx = false, row_major = true, nearest_neighbors = true,
    store_checkpoint_range = true, first_partition_smaller = false, count_zeros = false, compress = false;
    string input_filename = "", out_file = "", idx_filename = "" ;
    int threshold = 0, depth = 0, vcut = 0, hcut = 0, idx = 0, c = 0, seed = -1, num_idx = -1,
    num_threads = 8, dfs_length = 0, cz_len = 0, czp_app_len = 0, norm_depth = 0, layers_H_gates = 0,
    save_cp_to_file = 0, qft = 0;
    float norm_perc = 0;
    idx_size cz_path = 0, epsilon = 0, cramer_cw = 0;
    double cramer_p_reject = 0;
    int sim_type = -1;
    Config::Verbose verbose = Config::Default;
    vector<int> num_qubits, num_gates;
    num_threads = omp_get_num_procs();
    
    while ((c = getopt_long(argc, argv, "a:i:o:ut:d:s:|:v:_:x:q:c:nh:e:m:H:pfr:0z:Q:C", longopts, &idx)) != -1)
    {
        switch (c) {
            case 'a': {
                approx = true;
                string app_str = string(optarg);
                epsilon = stoul(app_str);
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
            case 'e': {
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
            case 'f': {
                first_partition_smaller = true;
                break;
            }
            case 'H': {
                layers_H_gates = stoi(string(optarg));
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
                
                if (!CheckIfGoogleFile(input_filename))
                {
                    ifstream infile(string(input_filename).c_str());
                    if (!infile.good()) {
                        cerr << "Cannot find circuit file.\n";
                        exit(1);
                    }
                }
                input_filename = "input/random_circuits_google/" + input_filename;
                break;
            }
            case 'm': {
                string grid_type = string(optarg);
                
                if (grid_type == "c" || grid_type == "column_major")
                    row_major = false;
                else if (!(grid_type == "r" || grid_type == "row_major")) {
                    cerr << "Incorrect grid type. Please choose between \"r\" (row_major) or \"c\" (column_major).\n";
                    exit(1);
                }
                break;
            }
            case 'n': {
                nearest_neighbors = false;
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
                idx_size p = out_file.find("@");
                if (p != string::npos)
                    out_file = out_file.substr(0, p);
                
                break;
            }
            case 'p': {
                store_checkpoint_range = false;
                break;
            }
            case 'q': {
                string s_th = string(optarg);
                threshold = stoi(s_th);
                break;
            }
            case 'Q': {
                valid = true;
                string s_qft = string(optarg);
                qft = stoi(s_qft);
                break;
            }
            case 'r': {
                string s_cp = string(optarg);
                save_cp_to_file = stoi(s_cp);
                if (save_cp_to_file > 2 || save_cp_to_file < 0) {
                    cerr << "Please enter a value between 0 and 2 for specifying how many checkpoints to save to file\n";
                    exit(1);
                }
                break;
            }
            case 's': {
                string s_type = string(optarg);
                sim_type = (Config::SimType)stoi(s_type);
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
            case 'v': {
                string s_v = string(optarg);
                verbose = (Config::Verbose)stoi(s_v);
                break;
            }
            case 'x': {
                string idx_arg = string(optarg);
                print_amp = true;
                if (idx_arg.find(",") != string::npos) {
                    string seed_str = idx_arg.substr(0, idx_arg.find_first_of(","));
                    string num_idx_str = idx_arg.substr(idx_arg.find_first_of(",") + 1, idx_arg.find_first_of("+"));
                    seed = stoi(seed_str);
                    num_idx = stoi(num_idx_str);
                    //                    if (num_idx > 10000)
                    //                        throw "Cannot print more than 10000 amps";
                    print_idx = idx_arg.find_first_of("+") != string::npos;
                }
                else
                    idx_filename = idx_arg;
                
                break;
            }
            case '0': {
                count_zeros = true;
                break;
            }
            case 'z': {
                compress = true;
                string args = string(optarg);
                if (args.find(",") != string::npos) {
                    string num_cw = args.substr(0, args.find_first_of(","));
                    string p_reject = args.substr(args.find_first_of(",") + 1, args.size());
                    cramer_cw = (1ull << stoul(args)) - 1;
                    p_reject = stod(p_reject);
                }
                else
                    cramer_cw = (1ull << stoul(args)) - 1;
                break;
            }
            case '|': {
                string s_c = string(optarg);
                vcut = stoi(s_c);
                break;
            }
            case '_': {
                string s_c = string(optarg);
                hcut = stoi(s_c);
                break;
            }
            case 'C' : {
                write_circuit_mode = true;
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
    
    if (!to_write && print_amp) {
        cerr << "Please enter ouput filename to print file\n";
        exit(1);
    }
    
    if (to_write && print_amp) {
        struct stat info;
        
        string pathname = string("output/amp_vectors");
        if( stat( pathname.c_str(), &info ) != 0 ) {
            cerr << pathname <<  " cannot be accessed\n";
            exit(1);
        }
        else if(!(info.st_mode & S_IFDIR))
            system(string("mkdir -p " + pathname).c_str());
    }
    
    Circuit cir(input_filename, num_qubits.size() > 0 ? num_qubits[0] : 0, depth);
    
    if (sim_type == -1  && qft == 0) {
        if (cir.GetNumQubits() <= 32) sim_type = 5;
        else if (hcut != 0) sim_type = 0;
        else if (vcut != 0) sim_type = 1;
        else sim_type = 0;
    }
    
    idx_size input_filename_pos = input_filename.find_last_of('/') != string::npos ?
    input_filename.find_last_of('/') + 1 : 0;
    bitset<128> temp_size = 0;
    temp_size[cir.GetNumQubits()] = 1;
    Config* config = new Config (temp_size, input_filename.substr(input_filename_pos) ,
                                 "output/probabilities/" + out_file, "output/amp_vectors/" + out_file,
                                 "output/reports/" + out_file, "output/misc", norm_perc, norm_depth,
                                 cz_path, czp_app_len, cz_len, dfs_length, epsilon, approx, ascii,
                                 print_amp, print_idx, (Config::SimType)sim_type, verbose, vcut, hcut,
                                 depth, threshold, num_threads, true, nearest_neighbors, row_major,
                                 layers_H_gates, store_checkpoint_range, first_partition_smaller,
                                 count_zeros, save_cp_to_file, compress, cramer_cw, cramer_p_reject);
    
    cir.InitializeCircuitConfig(config);
    
    if(write_circuit_mode) {
        cout << "Rearranging and writing circuit to file\n";
        cir.OptimizeCircuitArrangement(config);
        cir.WriteCircuitToFile(input_filename + ".rearranged");
        delete config;
        return 0;
    }
    
    if (print_amp) {
        if (seed != -1)
            config -> GenerateRandomIndices(seed, num_idx, ((__int128)1) << cir.GetNumQubits());
        else
            config -> ReadIndices(idx_filename);
    }
    
    GenericQuantumState::num_threads = num_threads;
    if (qft > 0) {
        FullAmpStateVector amp(qft);
        
        QFTSimulation qft_sim(qft);
        
        qft_sim.Simulate(amp);
        
        delete config;
        return 0;
    }
    
    SequentialSimulation sim(config);
    
    if (config -> verbose)
        sim.PrintSystemReport();
    
    if (sim_type == Config::FullState) {
        FullAmpStateVector amp(cir.GetNumQubits());
        if (threshold == 0)
            sim.SetThreshold(cir.GetNumQubits()/2);
        
        sim.Simulate(amp, cir);
    }
    else if (sim_type == Config::Approx1CutH || sim_type == Config::Approx2011 || sim_type == Config::Approx_i11i
             || sim_type == Config::Approx1_101 || sim_type == Config::Approx1110) {
        TensorProductStateVector amp (cir.GetNumQubits(),
                                      QubitPartition::Cuts::Horizontal, hcut, vcut,
                                      (Config::SimType)sim_type, row_major, first_partition_smaller,
                                      config -> verbose);
        sim.Simulate(amp, cir);
    }
    else if (sim_type == Config::Approx1CutV) {
        TensorProductStateVector amp (cir.GetNumQubits(),
                                      QubitPartition::Cuts::Vertical, hcut, vcut,
                                      (Config::SimType)sim_type, row_major, first_partition_smaller,
                                      config -> verbose);
        
        if (threshold == 0) {
            int num_q = amp.GetNumQInBlock(0) > amp.GetNumQInBlock(1) ?
            amp.GetNumQInBlock(1) : amp.GetNumQInBlock(0);
            sim.SetThreshold(num_q/2);
        }
        
        sim.Simulate(amp, cir);
    }
    else if (sim_type == Config::Approx2011OWT || sim_type == Config::Approx_i11iOWT || cz_len != 0) {
        SumOfTensorsProductsStateVector amp (cir.GetNumQubits(), (Config::SimType)sim_type,
                                             hcut, vcut, row_major, first_partition_smaller,
                                             config -> verbose);
        if (!config -> indices.empty())
            amp.PopulateGlobalToLocalMap(config -> indices);
        
        if (threshold == 0) {
            int num_q = amp.GetNumQInBlock(0) > amp.GetNumQInBlock(1) ?
            amp.GetNumQInBlock(1) : amp.GetNumQInBlock(0);
            sim.SetThreshold(num_q/2);
        }
        
        sim.Simulate(amp, cir);
        if (!config -> indices.empty())
            amp.UnpopulateGlobalToLocalMap();
    }
    else {
        AdaptiveStateVector amp(cir.GetNumQubits(),
                                (Config::SimType)sim_type, hcut, vcut, row_major, first_partition_smaller,
                                config -> verbose);
        sim.Simulate(amp, cir);
    }
    
    delete config;
    return 0;
    
}

