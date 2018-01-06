//
//  main_BM.cpp
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 12/30/17.
//

#include <iostream>
#include <stdio.h>
#include <string>

#include "XYmicroBM.h"

using namespace std;

int main(int argc, char *argv[]) 
{
    if (argc < 2)
        throw "Please specify number of qubits";
    
    const string s_num_qubits(argv[1]);
    const int num_qubits = stoi(s_num_qubits);
   
    if (num_qubits < 10) 
        throw "Number of qubits should be greater than or equal to 10";
    
    const idx_size amp_size = 1ull << num_qubits;
    
    cmplx* amp;
    posix_memalign((void**)&amp, 64, sizeof(cmplx) * amp_size);
    memset(amp, 0, amp_size * sizeof(amp));
    amp[0] = 1;
    
    char hostname[20] = {};
    gethostname(hostname, 20);
    cout << "Hostname : ";
    for(auto h : hostname) {
        cout << h;
    }
    cout << "\n";
    cout << "Benchmarking results for " + s_num_qubits + " qubits circuit:\n";
    Apply2MergedGatesBM(amp, amp_size, num_qubits);
    Apply4MergedGatesBM(amp, amp_size, num_qubits);
    
    return 0;
}
