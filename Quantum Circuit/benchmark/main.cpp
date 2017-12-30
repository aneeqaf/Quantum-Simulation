//
//  main_BM.cpp
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 12/30/17.
//  Copyright © 2017 Aneeqa Fatima. All rights reserved.
//

#include <iostream>
#include <stdio.h>
#include <string>

#include "XYmicroBM.h"

using namespace std;

int main(int argc, char *argv[]) 
{
    if (argc < 3)
        throw "Please specify number of qubits";
    
    const string s_num_qubits(argv[1]);
    const string s_num_gates(argv[2]);
    const int num_qubits = stoi(s_num_qubits);
    const int num_gates = stoi(s_num_gates);
    
    if (num_qubits < 10) 
        throw "Number of qubits should be greater than or equal to 10";
    
    const idx_size amp_size = 1ull << num_qubits;
    
    cmplx* amp;
    posix_memalign((void**)&amp, 64, sizeof(cmplx) * amp_size);
    amp = static_cast<cmplx*>(amp);
    memset(amp, 0, amp_size * sizeof(amp));
    amp[0] = 1;
    
    cout << "Benchmarking results for " + s_num_qubits + " qubits circuit:\n";
    Apply2MergedGatesBM(amp, amp_size, num_qubits, num_gates);
    Apply4MergedGatesBM(amp, amp_size, num_qubits, num_gates);
    
    return 0;
}
