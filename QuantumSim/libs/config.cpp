//
//  config.cpp
//  vector_state_sim
//
//  Created by Aneeqa Fatima on 1/29/18.
//  Copyright © 2018 Aneeqa Fatima. All rights reserved.
//

#include "config.h"




Config::
Config(const bitset<128>& amp_size,
       const string ifile,
       const string pfile,
       const string afile,
       const string rfile,
       const string mfile,
       const float norm_p,
       const int norm_d,
       const idx_size cz_p,
       const int cz_append_l,
       const int cz_len,
       const int dfs,
       const idx_size epsilon,
       const bool approx,
       const bool ascii,
       const bool p_amp,
       const bool p_idx,
       const SimType sim,
       const Verbose v,
       const int vc,
       const int hc,
       const int d,
       const int t,
       const int n_threads,
       const bool google,
       const bool near_neighbors,
       const bool row_maj,
       const int layers_last_H,
       const bool store_r,
       const bool first_part_small,
       const bool count_0,
       const int sv_cp_file)
: mmap_obj(nullptr), infile(ifile), prob_outfile(pfile), amp_outfile(afile),
report_outfile(rfile), misc_outfile(mfile), cz_path(cz_p), approx_epsilon(epsilon),
norm_perc(norm_p), norm_depth(norm_d), ranges_bits(cz_append_l), proc_prefix_bits(cz_len),
dfs_length(dfs), depth(d), th(t), num_threads(n_threads), vcut(vc), hcut(hc),
google(google), print_amp(p_amp), print_idx(p_idx), ascii(ascii), approx(approx),
nearest_neighbors(near_neighbors), row_major(row_maj), count_zeros(count_0),
last_layers_H(layers_last_H), store_checkpoint_range(store_r), first_part_smaller(first_part_small),
save_cp_file(sv_cp_file), sim_type(sim), verbose(v), curr_mode(ProcPrefix)
{
    temp_dir = "output/amp_vectors/" + infile + "_" + to_string(depth)
    + "_" + to_string(proc_prefix_bits + ranges_bits) + "_" + to_string(num_threads);
    if (approx)
        temp_dir += "_approx_" + to_string(approx_epsilon);
    temp_dir += "/temp_" + to_string(getpid()) + "/";
    
    if (!print_amp)
        mmap_obj = new MMapContent();
    if (t == -1)
        th = 16;
    
    if (save_cp_file > 0) {
        string command = "mkdir -p " +  temp_dir;
        system(command.c_str());
    }
        
    indices.push_back(3);
    indices.push_back(amp_size >> 2);
    indices.push_back(amp_size >> 1);
    indices.push_back((amp_size >> 1) | (amp_size >> 2));
    //    indices.push_back(3 * (amp_size >> 2).to_ullong());
    
    const idx_size first_half = ((amp_size << 64) >> 64).to_ulong();
    const idx_size second_half = (amp_size >> 64).to_ulong();
    const int q = first_half ? __builtin_ctzl(first_half) : second_half ? 64 + __builtin_ctzl(second_half) : 0;
    
    bitset<128> amp_3 = 0;
    for (int i = 0; i < q; ++i)
        if (i != 1)
            amp_3[i] = 1;
    indices.push_back(amp_3);
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
    norm_depth = rhs.norm_depth;
    norm_perc = rhs.norm_perc;
    cz_path = rhs.cz_path;
    ranges_bits = rhs.ranges_bits;
    proc_prefix_bits = rhs.proc_prefix_bits;
    approx_epsilon = rhs.approx_epsilon;
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
    if (print_amp && mmap_obj -> GetMapPtr()) mmap_obj = new MMapContent(*rhs.mmap_obj);
    else mmap_obj = new MMapContent();
    ascii = rhs.ascii;
    approx = rhs.approx;
    nearest_neighbors = rhs.nearest_neighbors;
    row_major = rhs.row_major;
    last_layers_H = rhs.last_layers_H;
    store_checkpoint_range = rhs.store_checkpoint_range;
    first_part_smaller = rhs.first_part_smaller;
    save_cp_file = rhs.save_cp_file;
}

Config& Config::
operator=(const Config& rhs)
{
    Config temp(rhs);
    indices = rhs.indices;
    infile = rhs.infile;
    prob_outfile = rhs.prob_outfile;
    amp_outfile = rhs.amp_outfile;
    report_outfile = rhs.report_outfile;
    misc_outfile = rhs.misc_outfile;
    norm_depth = rhs.norm_depth;
    norm_perc = rhs.norm_perc;
    cz_path = rhs.cz_path;
    ranges_bits = rhs.ranges_bits;
    proc_prefix_bits = rhs.proc_prefix_bits;
    dfs_length = rhs.dfs_length;
    approx_epsilon = rhs.approx_epsilon;
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
    mmap_obj = rhs.mmap_obj;
    ascii = rhs.ascii;
    approx = rhs.approx;
    nearest_neighbors = rhs.nearest_neighbors;
    row_major = rhs.row_major;
    last_layers_H = rhs.last_layers_H;
    store_checkpoint_range = rhs.store_checkpoint_range;
    first_part_smaller = rhs.first_part_smaller;
    save_cp_file = rhs.save_cp_file;
    if (print_amp) swap(mmap_obj, temp.mmap_obj);
    else mmap_obj = new MMapContent();
    return *this;
}

Config::
~Config() {
    
    delete mmap_obj;
}


