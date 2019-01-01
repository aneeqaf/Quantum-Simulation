//
//  SZ_helper.h
//  vector_state_sim
//
//  Created by Aneeqa Fatima on 12/30/18.
//  Copyright © 2018 Aneeqa Fatima. All rights reserved.
//

#ifndef SZ_helper_h
#define SZ_helper_h

#include <stdio.h>
#include <cstring>
#include <iostream>
#include <complex>

#include "sz.h"

#define DATATYPE SZ_FLOAT
#define ERR_BOUND_MODE REL

using namespace std;

using cmplx = complex<float>;

class SZ_Helper {
    
    unsigned char** compressed_vector_ptrs;
    size_t* compressed_out_sizes;
    
    string sz_cnfg;
    double error_bound;
    size_t actual_vector_size;
    size_t num_threads;
    
public:
    SZ_Helper(string cnfg_file,
              size_t vector_size,
              size_t threads): compressed_vector_ptrs(nullptr), compressed_out_sizes(nullptr), sz_cnfg(cnfg_file),
                                error_bound(1e-2), actual_vector_size(vector_size), num_threads(threads) {}
    SZ_Helper(string cnfg_file,
              size_t vector_size,
              size_t threads,
              double err_bound): compressed_vector_ptrs(nullptr), compressed_out_sizes(nullptr), sz_cnfg(cnfg_file),
                                error_bound(err_bound), actual_vector_size(vector_size), num_threads(threads) {}
    SZ_Helper(const SZ_Helper& rhs);
    ~SZ_Helper();
    
    void Compress(const cmplx* original_vector);
    cmplx* Decompress();
};

#endif /* SZ_helper_h */
