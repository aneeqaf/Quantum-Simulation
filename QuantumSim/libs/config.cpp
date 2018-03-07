//
//  config.cpp
//  vector_state_sim
//
//  Created by Aneeqa Fatima on 1/29/18.
//  Copyright © 2018 Aneeqa Fatima. All rights reserved.
//

#include "config.h"

Config::
Config(const idx_size amp_size,
       const string ifile,
       const string pfile,
       const string afile,
       const string rfile,
       const string mfile,
       const string cz_b,
       const int dfs,
       const bool p_amp,
       const bool p_idx,
       const SimType sim,
       const Verbose v,
       const int vc,
       const int hc,
       const int d,
       const int t,
       const int n_threads,
       const bool google) : infile(ifile), prob_outfile(pfile), amp_outfile(afile), report_outfile(rfile),
misc_outfile(mfile), cz_path(cz_b), dfs_length(dfs), depth(d), th(t), num_threads(n_threads), vcut(vc), hcut(hc),
google(google), print_amp(p_amp), print_idx(p_idx), sim_type(sim), verbose(v), curr_mode(Phase1)
{
    if (t == -1)
        th = 16;
    indices.push_back(3);
    indices.push_back(amp_size/4);
    indices.push_back(amp_size/2);
    indices.push_back(3 * amp_size/4);
    indices.push_back(amp_size - 3);
}

void Config::
ReadIndices(const string& idx_infile)
{
    if (idx_infile != "") {
        
        int f_idx;
        ifstream idx_in;
        idx_in.open(idx_infile);
        
        while (idx_in >> f_idx)
            indices.push_back(f_idx);
        
        idx_in.close();
    }
    else {
        throw "Empty filename for indices";
    }
}

void Config::
GenerateRandomIndices(const int seed,
                      const int num_idx,
                      const idx_size amp_size)
{
    srand(seed);
    for (int i = 0; i < num_idx; ++i)
        indices.push_back(rand() % amp_size);
}

Config::
Config(const Config& rhs)
{
    indices = rhs.indices;
    infile = rhs.infile;
    prob_outfile = rhs.prob_outfile;
    amp_outfile = rhs.amp_outfile;
    report_outfile = rhs.report_outfile;
    misc_outfile = rhs.misc_outfile;
    cz_path = rhs.cz_path;
    dfs_length = rhs.dfs_length;
    print_amp = rhs.print_amp;
    print_idx = rhs.print_idx;
    depth = rhs.depth;
    th = rhs.th;
    num_threads = rhs.num_threads;
    vcut = rhs.vcut;
    hcut = rhs.hcut;
    google = rhs.google;
    sim_type = rhs.sim_type;
    verbose = rhs.verbose;
    curr_mode = rhs.curr_mode;
}

Config& Config::
operator=(const Config& rhs)
{
    indices = rhs.indices;
    infile = rhs.infile;
    prob_outfile = rhs.prob_outfile;
    amp_outfile = rhs.amp_outfile;
    report_outfile = rhs.report_outfile;
    misc_outfile = rhs.misc_outfile;
    cz_path = rhs.cz_path;
    dfs_length = rhs.dfs_length;
    print_amp = rhs.print_amp;
    print_idx = rhs.print_idx;
    depth = rhs.depth;
    th = rhs.th;
    num_threads = rhs.num_threads;
    vcut = rhs.vcut;
    hcut = rhs.hcut;
    google = rhs.google;
    sim_type = rhs.sim_type;
    verbose = rhs.verbose;
    curr_mode = rhs.curr_mode;
    return *this;
}
