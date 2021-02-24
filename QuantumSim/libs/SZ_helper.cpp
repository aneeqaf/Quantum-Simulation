//
//  SZ_helper.cpp
//  vector_state_sim
//
//  Created by Aneeqa Fatima on 12/30/18.
//  Copyright © 2018 Aneeqa Fatima. All rights reserved.
//

#include "SZ_helper.h"

SZ_Helper::
~SZ_Helper()
{
    if (!compressed_vector_ptrs.empty())
        for (auto ptrs : compressed_vector_ptrs)
            if(ptrs) free(ptrs);
}

SZ_Helper::
SZ_Helper(const SZ_Helper& rhs)
{
    for (size_t i = 0; i < num_threads; ++i) {
        compressed_out_sizes[i] = rhs.compressed_out_sizes[i];
        compressed_vector_ptrs[i] = (unsigned char*) malloc(sizeof(unsigned char) * compressed_out_sizes[i]);

        for (size_t j = 0; j < compressed_out_sizes[i]; ++j)
            compressed_vector_ptrs[i][j] = rhs.compressed_vector_ptrs[i][j];
    }
}

void SZ_Helper::
Compress(const cmplx* original_vector)
{
    int status = SZ_Init(sz_cnfg.c_str());
    if(status == SZ_NSCS)
        exit(1);
    
    size_t size_of_each_compression = actual_vector_size/num_threads;
    
    for (size_t i = 0; i < num_threads; ++i) {
        unsigned char* compressed_ptr = (unsigned char*) SZ_compress_args(DATATYPE,
                                                                          (float *)(original_vector + (i * size_of_each_compression)),
                                                                          &compressed_out_sizes[i], ERR_BOUND_MODE, error_bound, error_bound,
                                                                          error_bound, 0, 0, 0, 0, (2 * size_of_each_compression));
       compressed_vector_ptrs[i] = compressed_ptr;
    }
}

cmplx* SZ_Helper::
Decompress()
{
    cmplx* decompressed_amp = nullptr;
    if (int err = posix_memalign((void**)&decompressed_amp, 64, sizeof(cmplx) * actual_vector_size) != 0) {
        size_t memory = sizeof(cmplx) * actual_vector_size;
        cerr << "Memory requirement exceeds availiable memory for aligned storage. Requested ";
        if (memory >= (1 << 30)) {
            cerr << memory / (1 << 30) << " GiB \n";
        }
        else if (memory >= (1 << 20)) {
            cerr << memory / (1 << 20) << " MiB \n";
        }
        else if (memory >= (1 << 10)) {
            cerr << memory / (1 << 10) << " KiB \n";
        }
        else
            cerr << memory << " B \n";
        exit(err);
    }
    memset(decompressed_amp, 0, sizeof(cmplx) * actual_vector_size);
    
    size_t size_of_each_compression = actual_vector_size/num_threads;
    
    for (size_t i = 0; i < num_threads; ++i) {
        SZ_decompress_args(DATATYPE, compressed_vector_ptrs[i], compressed_out_sizes[i],
                           (float *)(decompressed_amp + (i * size_of_each_compression)), 0, 0, 0, 0,
                           2 * size_of_each_compression);
    }
    
    return decompressed_amp;
}
