//
//  utility.h
//  QuantumSimProj
//
//  Created by Aneeqa Fatima on 6/28/18.
//  Copyright © 2018 Aneeqa Fatima. All rights reserved.
//

#ifndef utility_h
#define utility_h

#include <bitset>
#include <stdio.h>
#include <string>
#include <inttypes.h>

using namespace std;

using idx_size = unsigned long long;

static inline void print_u128_u(__int128 u128,
                                string &bit128_str)
{
    if (u128 == 0)
        return;

    __int128 leading = u128 / 10;
    print_u128_u(leading, bit128_str);
    uint64_t trailing = u128 % 10;
    bit128_str += to_string(trailing);
}

template <typename mem_type>
static inline void allocate_aligned_mem(mem_type *&mem_ptr, idx_size size)
{
    if (int err = posix_memalign((void **)&mem_ptr, 64, sizeof(mem_type) * size) != 0)
    {
        idx_size memory = sizeof(mem_type) * size;
        cerr << "Memory requirement exceeds availiable memory for aligned storage. Requested ";
        if (memory >= (1 << 30))
        {
            cerr << memory / (1 << 30) << " GiB \n";
        }
        else if (memory >= (1 << 20))
        {
            cerr << memory / (1 << 20) << " MiB \n";
        }
        else if (memory >= (1 << 10))
        {
            cerr << memory / (1 << 10) << " KiB \n";
        }
        else
            cerr << memory << " B \n";
        free(mem_ptr);
        mem_ptr = nullptr;
        exit(err);
    }
    memset(mem_ptr, 0, size * sizeof(mem_type));
}

#endif /* utility_h */
