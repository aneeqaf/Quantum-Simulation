//
//  RR_configs.cpp
//  vector_state_sim
//
//  Created by Aneeqa Fatima on 3/26/19.
//  Copyright © 2019 Aneeqa Fatima. All rights reserved.
//

#include "RR_configs.h"

Config::
Config():indices_to_save({}), mmap_obj(nullptr)
{
    changing_configs.CZ_path = 0;
    changing_configs.outfile = "";
    
    static_configs.circuit_filename = "";
    static_configs.amp_outdir = "output/amp_vectors/";
    static_configs.amp_outfile = "rr_output";
    static_configs.misc_outdir = "output/misc/";
    static_configs.fidelity = 1;
    static_configs.ranges_bits = 0;
    static_configs.proc_prefix_bits = 0;
    static_configs.branch_bits = 0;
    static_configs.depth = 26;
    static_configs.threshold_low_qubits = 0;
    static_configs.num_threads = 1;
    static_configs.vcut = 0;
    static_configs.hcut = 0;
    static_configs.print_amp = false;
    static_configs.print_idx = false;
    static_configs.print_amps_ascii = false;
    static_configs.approx_sim = false;
    static_configs.nearest_neighbor_arch = true;
    static_configs.row_major = true;
    static_configs.count_zeros = false;
    static_configs.checkpoint_range = true;
    static_configs.first_part_of_cut_smaller = false;
    static_configs.save_checkpoint_file = false;
    static_configs.temp_state_dir = "";
    static_configs.sim_type = FullState;
    static_configs.verbose = Default;
    static_configs.FeynmanSch_stage = ProcPrefix;
}

string Config::
ConstructDir(string output_dir,
             string circuit_filename)
{
    string dir = output_dir + circuit_filename
    + "_" + to_string(static_configs.depth) + "_" + to_string(static_configs.proc_prefix_bits + static_configs.ranges_bits)
    + "_" + to_string(static_configs.num_threads);
    if (static_configs.approx_sim)
        dir += "_approx_" + to_string(static_configs.fidelity);
    
    return dir;
}

void Config::
ReadIndices(const string& idx_infile)
{
    if (idx_infile != "") {
        
        int f_idx;
        ifstream idx_in;
        idx_in.open(idx_infile);
        
        while (idx_in >> f_idx)
            indices_to_save.push_back(f_idx);
        
        idx_in.close();
    }
    else {
        throw "Empty filename for indices";
    }
    
    //    sort(indices.begin() + 5, indices.end(), [](bitset<128>& first, bitset<128>& second) {
    //        for (int i = 127; i >= 0; i--) {
    //            if (first[i] ^ second[i]) return (bool)second[i];
    //        }
    //        return false;
    //    });
    
    string dir = ConstructDir(static_configs.amp_outdir, static_configs.circuit_filename);
    string command = "mkdir -p " + dir;
    system(command.c_str());
    
    mmap_obj = new MMapContent(dir + static_configs.amp_outfile + ".amps", sizeof(cmplx) * (indices_to_save.size()), true);
}

void Config::
GenerateRandomIndices(const int seed,
                      const int num_idx,
                      const __int128 amp_size)
{
    srand(seed);
    string dir = ConstructDir(static_configs.amp_outdir, static_configs.circuit_filename);
    string command = "mkdir -p " + dir;
    system(command.c_str());
    
    for (int i = 0; i < num_idx; ++i) {
        const size_t first_half = (amp_size << 64) >> 64;
        const size_t second_half = amp_size >> 64;
        const int q = first_half ? __builtin_ctzl(first_half) : second_half ? 64 + __builtin_ctzl(second_half) : 0;
        
        if (q < 64)
            indices_to_save.push_back(rand() % amp_size);
        else {
            bool large_idx = rand() % 2;
            if (large_idx) {
                __int128 n1_64 = rand() % INT64_MAX;
                __int128 n2_64 = rand() % INT64_MAX;
                indices_to_save.push_back((bitset<128>(n1_64) << 64) | bitset<128>(n2_64));
            }
            else {
                indices_to_save.push_back(rand() % INT64_MAX);
            }
        }
    }
    
    mmap_obj = new MMapContent(dir + static_configs.amp_outfile + ".amps", sizeof(cmplx) * (num_idx + 5), true);
}

void Config::
ReadKeyValueAndAssign(const string key,
                      const string value)
{
    if (strcmp(key.c_str(), "CZ_path") == 0) changing_configs.CZ_path = stoll(value);
    else if (strcmp(key.c_str(), "outfile") == 0) changing_configs.outfile = value;
    else if (strcmp(key.c_str(), "circuit_filename") == 0) static_configs.circuit_filename = value;
    else if (strcmp(key.c_str(), "amp_outdir") == 0) static_configs.amp_outdir = value;
    else if (strcmp(key.c_str(), "amp_outfile") == 0) static_configs.amp_outfile = value;
    else if (strcmp(key.c_str(), "misc_outdir") == 0) static_configs.misc_outdir = value;
    else if (strcmp(key.c_str(), "temp_state_dir") == 0) static_configs.temp_state_dir = value;
    else if (strcmp(key.c_str(), "fidelity") == 0) static_configs.fidelity = stod(value);
    else if (strcmp(key.c_str(), "ranges_bits") == 0) static_configs.ranges_bits = stoi(value);
    else if (strcmp(key.c_str(), "proc_prefix_bits") == 0) static_configs.proc_prefix_bits = stoi(value);
    else if (strcmp(key.c_str(), "branch_bits") == 0) static_configs.branch_bits = stoi(value);
    else if (strcmp(key.c_str(), "depth") == 0) static_configs.depth = stoi(value);
    else if (strcmp(key.c_str(), "threshold_low_qubits") == 0) static_configs.threshold_low_qubits = stoi(value);
    else if (strcmp(key.c_str(), "num_threads") == 0) static_configs.num_threads = stoi(value);
    else if (strcmp(key.c_str(), "vcut") == 0) static_configs.vcut = stoi(value);
    else if (strcmp(key.c_str(), "hcut") == 0) static_configs.hcut = stoi(value);
    else if (strcmp(key.c_str(), "print_amp") == 0) istringstream(value) >> static_configs.print_amp;
    else if (strcmp(key.c_str(), "print_idx") == 0) istringstream(value) >> static_configs.print_idx;
    else if (strcmp(key.c_str(), "print_amps_ascii") == 0) istringstream(value) >> static_configs.print_amps_ascii;
    else if (strcmp(key.c_str(), "approx_sim") == 0) istringstream(value) >> static_configs.approx_sim;
    else if (strcmp(key.c_str(), "nearest_neighbor_arch") == 0) istringstream(value) >> static_configs.nearest_neighbor_arch;
    else if (strcmp(key.c_str(), "row_major") == 0) istringstream(value) >> static_configs.row_major;
    else if (strcmp(key.c_str(), "count_zeros") == 0) istringstream(value) >> static_configs.count_zeros;
    else if (strcmp(key.c_str(), "checkpoint_range") == 0) istringstream(value) >> static_configs.checkpoint_range;
    else if (strcmp(key.c_str(), "first_part_of_cut_smaller") == 0) istringstream(value) >> static_configs.first_part_of_cut_smaller;
    else if (strcmp(key.c_str(), "save_checkpoint_file") == 0) istringstream(value) >> static_configs.save_checkpoint_file;
    else if (strcmp(key.c_str(), "sim_type") == 0) static_configs.sim_type = (SimType)stoi(value);
    else if (strcmp(key.c_str(), "verbose") == 0) static_configs.verbose = (Verbosity)stoi(value);
    else if (strcmp(key.c_str(), "FeynmanSch_stage") == 0) static_configs.FeynmanSch_stage = (FeymanSchStage)stoi(value);
    else throw "Incorrect config";
}

void Config::
ParseLine(string line)
{
    if (line[0] != '#' && line != "")
    {
        //Check to see if the '=' comes before '#'
        if (line.find_first_of('=') < line.find_first_of('#'))
        {
            string key = StripSpace(line.substr(0, line.find_first_of('=')));
            string value = StripSpace(line.substr(line.find_first_of('=') + 1, line.find_first_of('#')));
            
            if (CheckIfValid(key) && CheckIfValid(value))
            {
                cout << key << " = " << value << endl;
                ReadKeyValueAndAssign(key, value);
            }
        }
        else
            throw "Line format is undefined. Please make sure it is only alphanumeric, '_', or '/'.";
    }
        
}

inline bool Config::
CheckIfValid(string str)
{
    return find_if(str.begin(), str.end(), [](char c) {
                return !(isalnum(c) || c == '_' || c == '/');
           }) == str.end();
}

inline string Config::
StripSpace(string line)
{
    line.erase(remove(line.begin(), line.end(), ' '), line.end());
    return line;
}

void Config::
ReadConfigFile(string config_file)
{
    ifstream config_in;
    config_in.open(config_file);
    
    string line = {};
    while (getline(config_in, line)) {
        if (line.length() <= ASCIIChars)
            ParseLine(line);
        else
            throw "Line in config file too long\n";
    }
}

