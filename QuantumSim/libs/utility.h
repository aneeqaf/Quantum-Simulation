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

static inline void print_u128_u(__int128 u128,
                                string& bit128_str)
{
    if (u128 == 0)
        return;
    
    __int128 leading  = u128/10;
    print_u128_u(leading, bit128_str);
    uint64_t trailing = u128 % 10;
    bit128_str += to_string(trailing);
}
#endif /* utility_h */
