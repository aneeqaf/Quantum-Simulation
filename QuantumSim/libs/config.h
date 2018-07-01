//
//  configuration.h
//  QuantumSimProj
//
//  Created by Aneeqa Fatima on 1/27/18.
//

#ifndef configuration_h
#define configuration_h

#include <algorithm>
#include <bitset>
#include <complex>
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

using namespace std;

using idx_size = unsigned long long;
using cmplx = complex<float>;

class MMapContent {
    cmplx* map_ptr;
    idx_size size;
    int fd;
    string filename;
    
public:
    void InitializeMemMap(const string filename,
                          idx_size size);
    cmplx* operator[](idx_size i);
    void WriteToDisk();
   
    cmplx* GetMapPtr() { return map_ptr; }
    
    MMapContent(): map_ptr(nullptr), size(0), fd(0), filename("") {};
    MMapContent(const string filename,
                const idx_size size,
                bool initialize = false);
    MMapContent(const MMapContent& rhs);
    MMapContent& operator=(const MMapContent& rhs);
    ~MMapContent();
};

class Config {
public:
    enum SimType : int {LosslessH, LosslessV, Approx1CutH, Approx1CutV, ApproxOWT, FullState,
        Approx2011, Approx1_101, Approx1110, Approx2011OWT, Approx_i11i, Approx_i11iOWT,
        ApproxCZPathH2011, ApproxCZPathV2011};
    enum Verbose : int {None, NCCV, NCC, Default, Cycles};
    enum SimMode : int {ProcPrefix, Ranges, Branch};
    
    vector<bitset<128>> indices;
    MMapContent* mmap_obj;
    string infile;
    string prob_outfile;
    string amp_outfile;
    string report_outfile;
    string misc_outfile;
    idx_size cz_path;
    idx_size approx_epsilon;
    float norm_perc;
    int norm_depth;
    int ranges_bits;
    int proc_prefix_bits;
    int dfs_length;
    int depth;
    int th;
    int num_threads;
    int vcut;
    int hcut;
    bool google;
    bool print_amp;
    bool print_idx;
    bool ascii;
    bool approx;
    bool nearest_neighbors;
    bool row_major;
    int last_layers_H;
    bool store_checkpoint_range;
    bool first_part_smaller;
    int save_cp_file;
    string temp_dir;
    SimType sim_type;
    Verbose verbose;
    SimMode curr_mode;
    
    void ReadIndices(const string& idx_file);
    void GenerateRandomIndices(const int seed,
                               const int num_idx,
                               const __int128 amp_size);
    
    Config() : indices({}), mmap_obj(nullptr), infile(""), prob_outfile(""), amp_outfile(""), report_outfile(""), misc_outfile(""),
    cz_path(0), approx_epsilon(0),  norm_perc(0), norm_depth(0), ranges_bits(0), proc_prefix_bits(0),
    dfs_length(0), depth(26), th(0), num_threads(1), vcut(0), hcut(0), google(true), print_amp(false),
    print_idx(false), ascii(false), approx(true), nearest_neighbors(true), row_major(true), last_layers_H(0),
    store_checkpoint_range(true), first_part_smaller(false), sim_type(FullState), verbose(Default),
    curr_mode(ProcPrefix), save_cp_file(0), temp_dir("") {}
    Config(const bitset<128>& amp_size,
           const string ifile,
           const string pfile,
           const string afile,
           const string rfile,
           const string mfile,
           const float norm_p,
           const int norm_d,
           const idx_size cz_p,
           const int ranges_bits,
           const int cz_len,
           const int dfs,
           const idx_size epsilon = 0,
           const bool approx = false,
           const bool ascii = false,
           const bool p_amp = false,
           const bool p_idx = false,
           const SimType sim = FullState,
           const Verbose v = Default,
           const int vc = 0,
           const int hc = 0,
           const int d = 26,
           const int t = 0,
           const int n_threads = 1,
           const bool google = true,
           const bool near_neighbors = true,
           const bool row_maj = true,
           const int layers_last_H = 0,
           const bool checkpoint_r = true,
           const bool first_part_small = false,
           const int sv_cp_file = 0);
    Config(const Config& rhs);
    Config& operator=(const Config& rhs);
    ~Config();
};

#endif /* configuration_h */
