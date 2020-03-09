//
//  RR_configs.hpp
//  vector_state_sim
//
//  Created by Aneeqa Fatima on 3/26/19.
//  Copyright © 2019 Aneeqa Fatima. All rights reserved.
//

#ifndef RR_configs_h
#define RR_configs_h

#include <algorithm>
#include <bitset>
#include <complex>
#include <cstring>
#include <fstream>
#include <fcntl.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <vector>
#include <utility>
#include <dirent.h>

#include "mmap.h"

using namespace std;

static const int ASCIIChars = 1024;

enum SimType : int {LosslessH, LosslessV, Approx1CutH, Approx1CutV, ApproxOWT, FullState,
    Approx2011, Approx1_101, Approx1110, Approx2011OWT, Approx_i11i, Approx_i11iOWT,
    ApproxCZPathH2011, ApproxCZPathV2011};
enum Verbosity : int {None, NCCV, NCC, Default, Cycles};
enum FeymanSchStage : int {ProcPrefix, Ranges, Branch};

typedef struct RRChangingParams {
    size_t CZ_path;
    string outfile;
} RRChangingParams;

typedef struct RRStaticParams {
    string circuit_filename;
    string amp_outdir;
    string amp_outfile;
    string misc_outdir;
    string temp_state_dir;
    double fidelity;
    int ranges_bits;
    int proc_prefix_bits;
    int branch_bits;
    int depth;
    int threshold_low_qubits;
    int num_threads;
    int vcut;
    int hcut;
    bool print_amp;
    bool print_idx;
    bool print_amps_ascii;
    bool approx_sim;
    bool nearest_neighbor_arch;
    bool row_major;
    bool count_zeros;
    bool checkpoint_range;
    bool first_part_of_cut_smaller;
    int save_checkpoint_file;
    SimType sim_type;
    Verbosity verbose;
    FeymanSchStage FeynmanSch_stage;
} RRStaticParams;

class Config {
private:
    
    void ReadKeyValueAndAssign(string key, string value);
    void ParseLine(string line);
    bool CheckIfValid(string str);
    string ConstructDir(string output_dir, string circuit_filename);
    string StripSpace(string line);

public:

    vector<bitset<128>> indices_to_save;
    MMapContent* mmap_obj;
    RRChangingParams changing_configs;
    RRStaticParams static_configs;
    
    void ReadIndices(const string& idx_file);
    void GenerateRandomIndices(const int seed,
                               const int num_idx,
                               const __int128 amp_size);
    void ReadConfigFile(string config_file);
    
    Config();
    Config(const Config& rhs) = delete;
    Config& operator=(const Config& rhs) = delete;
};

#endif /* RR_configs_h */
