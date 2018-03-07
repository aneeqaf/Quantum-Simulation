//
//  configuration.h
//  QuantumSimProj
//
//  Created by Aneeqa Fatima on 1/27/18.
//

#ifndef configuration_h
#define configuration_h

#include <fstream>
#include <string>
#include <vector>
#include <bitset>

using namespace std;

using idx_size = size_t;

class Config {
public:
    enum SimType : int {LosslessH, LosslessV, Approx1CutH, Approx1CutV, ApproxOWT, FullState,
        Approx2011, Approx1_101, Approx1110, Approx2011OWT, Approx_i11i, Approx_i11iOWT};
    enum Verbose : int {None, NCCV, NCC, Default, Cycles};
    enum SimMode : int {Phase1, Phase2, Phase3};
    
    vector<bitset<128>> indices;
    string infile;
    string prob_outfile;
    string amp_outfile;
    string report_outfile;
    string misc_outfile;
    string cz_path;
    int dfs_length;
    int depth;
    int th;
    int num_threads;
    int vcut;
    int hcut;
    bool google;
    bool print_amp;
    bool print_idx;
    SimType sim_type;
    Verbose verbose;
    SimMode curr_mode;
    
    void ReadIndices(const string& idx_file);
    void GenerateRandomIndices(const int seed,
                               const int num_idx,
                               const idx_size amp_size);
    
    Config() : indices({}), infile(""), prob_outfile(""), amp_outfile(""), report_outfile(""), misc_outfile(""),
    cz_path("-"), dfs_length(-1), depth(26), th(14), num_threads(1), vcut(0), hcut(0), google(true), print_amp(false), print_idx(false),
    sim_type(FullState), verbose(Default), curr_mode(Phase1) {}
    Config(const idx_size amp_size,
           const string ifile,
           const string pfile,
           const string afile,
           const string rfile,
           const string mfile,
           const string cz_b,
           const int dfs,
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
};

#endif /* configuration_h */
