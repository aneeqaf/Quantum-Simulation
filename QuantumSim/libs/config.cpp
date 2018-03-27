//
//  config.cpp
//  vector_state_sim
//
//  Created by Aneeqa Fatima on 1/29/18.
//  Copyright © 2018 Aneeqa Fatima. All rights reserved.
//

#include "config.h"



MMapContent::
MMapContent(string file_n,
            idx_size size,
            bool initialize): map_ptr(nullptr), size(size), filename(file_n)
{
    if (initialize)
        InitializeMemMap(filename, size);
}

MMapContent::
MMapContent(const MMapContent& rhs)
{
    size = rhs.size;
    fd = rhs.fd;
    filename = rhs.filename;
    InitializeMemMap(filename, size);
}

MMapContent& MMapContent::
operator=(const MMapContent& rhs)
{
    MMapContent temp(rhs);
    swap(map_ptr, temp.map_ptr);
    swap(size, temp.size);
    swap(fd, temp.fd);
    return *this;
}

MMapContent::
~MMapContent()
{
    if (map_ptr) {
        if (munmap(map_ptr, size) == -1)
        {
            close(fd);
            perror("Error un-mmapping the file");
            exit(EXIT_FAILURE);
        }
        
        // Un-mmaping doesn't close the file, so we still need to do that.
        close(fd);
    }
}

void MMapContent::
InitializeMemMap(const string file_n,
                 idx_size size)
{
    auto does_file_exist = [](const char *fileName)
    {
        ifstream infile(fileName);
        return (bool)infile.good();
    };
    
    auto is_file_empty = [] (const char *fileName)
    {
        ifstream infile(fileName);
        return infile.peek() == std::ifstream::traits_type::eof();
    };
    
    bool new_file = !does_file_exist(filename.c_str()) || is_file_empty(filename.c_str());
    fd = open(filename.c_str(), O_RDWR | O_CREAT , (mode_t)0600);
    map_ptr = (cmplx*)mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map_ptr == MAP_FAILED)
    {
        close(fd);
        perror("Error mmapping the file");
        exit(EXIT_FAILURE);
    }
    
    if (new_file) {
        
        if (lseek(fd, size - 1, SEEK_SET) == -1)
        {
            close(fd);
            perror("Error calling lseek() to 'stretch' the file");
            exit(EXIT_FAILURE);
        }
        
        if (write(fd, "", 1) == -1)
        {
            close(fd);
            perror("Error writing last byte of the file");
            exit(EXIT_FAILURE);
        }
        for (idx_size i = 0; i < size/sizeof(cmplx); ++i)
            map_ptr[i] = 0;
    }
}

cmplx* MMapContent::
operator[](idx_size i)
{
    return &map_ptr[i];
}


void MMapContent::
WriteToDisk()
{
    if (msync(map_ptr, size, MS_SYNC) == -1)
    {
        perror("Could not sync the file to disk");
    }
}

Config::
Config(const idx_size amp_size,
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
       const bool google) : infile(ifile), prob_outfile(pfile), amp_outfile(afile), report_outfile(rfile),
misc_outfile(mfile), cz_path(cz_p), norm_perc(norm_p), norm_depth(norm_d), czp_append_len(cz_append_l), cz_num_bits(cz_len),
dfs_length(dfs), depth(d), th(t), num_threads(n_threads), vcut(vc), hcut(hc), google(google), print_amp(p_amp), print_idx(p_idx),
ascii(ascii), approx(approx), sim_type(sim), verbose(v), curr_mode(Phase1)
{
    if (!print_amp)
        mmap_obj = new MMapContent();
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
    
    string file_n = "output/amp_vectors/" + infile + "_" + to_string(depth)
    + "_" + to_string(cz_num_bits + czp_append_len) + "_" + to_string(num_threads);
    if (approx)
        file_n += "_approx";
    string command = "mkdir -p " + file_n;
    system(command.c_str());
    mmap_obj = new MMapContent(file_n + amp_outfile.substr(amp_outfile.find_last_of("/")) + ".amps",
                               sizeof(cmplx) * (indices.size()));
}

void Config::
GenerateRandomIndices(const int seed,
                      const int num_idx,
                      const idx_size amp_size)
{
    srand(seed);
    string file_n = "output/amp_vectors/" + infile + "_" + to_string(depth)
    + "_" + to_string(cz_num_bits + czp_append_len) + "_" + to_string(num_threads);
    if (approx)
        file_n += "_approx";
    string command = "mkdir -p " + file_n;
    system(command.c_str());
    
    for (int i = 0; i < num_idx; ++i)
        indices.push_back(rand() % amp_size);
    
    if (print_idx) {
        string idx_outfile = file_n + "/" + amp_outfile + ".idx";
        ofstream idx_out;

        idx_out.open(idx_outfile);

        for (idx_size i = 5; i < indices.size(); ++i)
            idx_out << indices[i].to_ullong() << "\n";
    }

    mmap_obj = new MMapContent(file_n + amp_outfile.substr(amp_outfile.find_last_of("/"))  + ".amps",
                               sizeof(cmplx) * (num_idx + 5));
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
    czp_append_len = rhs.czp_append_len;
    cz_num_bits = rhs.cz_num_bits;
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
    if (print_amp) mmap_obj = new MMapContent(*rhs.mmap_obj);
    else mmap_obj = new MMapContent();
    ascii = rhs.ascii;
    approx = rhs.approx;
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
    czp_append_len = rhs.czp_append_len;
    cz_num_bits = rhs.cz_num_bits;
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
    mmap_obj = rhs.mmap_obj;
    ascii = rhs.ascii;
    approx = rhs.approx;
    if (print_amp) swap(mmap_obj, temp.mmap_obj);
    else mmap_obj = new MMapContent();
    return *this;
}

Config::
~Config() {
    
    delete mmap_obj;
}


