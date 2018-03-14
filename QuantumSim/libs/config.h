//
//  configuration.h
//  QuantumSimProj
//
//  Created by Aneeqa Fatima on 1/27/18.
//

#ifndef configuration_h
#define configuration_h

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

using namespace std;

using idx_size = size_t;
using cmplx = complex<float>;

class MMapContent {
    cmplx* map_ptr;
    idx_size num_amps;
    idx_size size;
    int fd;
    string filename;
    
public:
    void InitializeMemMap(const string filename,
                          idx_size size);
    cmplx& operator[](idx_size i);
    void WriteToDisk();
    
    MMapContent(): map_ptr(nullptr), num_amps(0), size(0), fd(0), filename("") {};
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
        Approx2011, Approx1_101, Approx1110, Approx2011OWT, Approx_i11i, Approx_i11iOWT};
    enum Verbose : int {None, NCCV, NCC, Default, Cycles};
    enum SimMode : int {Phase1, Phase2, Phase3};
    
    vector<bitset<128>> indices;
    MMapContent* mmap_obj;
    string infile;
    string prob_outfile;
    string amp_outfile;
    string report_outfile;
    string misc_outfile;
    idx_size cz_path;
    int czp_append_len;
    int cz_num_bits;
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
    SimType sim_type;
    Verbose verbose;
    SimMode curr_mode;
    
    void ReadIndices(const string& idx_file);
    void GenerateRandomIndices(const int seed,
                               const int num_idx,
                               const idx_size amp_size);
    
    Config() : indices({}), mmap_obj(nullptr), infile(""), prob_outfile(""), amp_outfile(""), report_outfile(""), misc_outfile(""),
    cz_path(0), czp_append_len(0), cz_num_bits(0), dfs_length(-1), depth(26), th(14), num_threads(1), vcut(0), hcut(0), google(true), print_amp(false), print_idx(false), ascii(false), sim_type(FullState), verbose(Default), curr_mode(Phase1) {}
    Config(const idx_size amp_size,
           const string ifile,
           const string pfile,
           const string afile,
           const string rfile,
           const string mfile,
           const idx_size cz_p,
           const int cz_append_len,
           const int cz_len,
           const int dfs,
           const bool ascii = false,
           const bool p_amp = false,
           const bool p_idx = false,
           const SimType sim = FullState,
           const Verbose v = Default,
           const int vc = 0,
           const int hc = 0,
           const int d = 26,
           const int t = 16,
           const int n_threads = 1,
           const bool google = true);
    Config(const Config& rhs);
    Config& operator=(const Config& rhs);
    ~Config();
};

#endif /* configuration_h */
