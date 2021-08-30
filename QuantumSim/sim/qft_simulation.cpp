//
//  qftSimulation.cpp
//  vector_state_sim
//
//  Created by Aneeqa Fatima on 8/8/21.
//  Copyright © 2021 Aneeqa Fatima. All rights reserved.
//

#include "qft_simulation.h"

void QFTSimulation::
Simulate(FullAmpStateVector& amp, string idx_infile)
{
    amp.ApplyHGateOnAllAmps(true);
    for (idx_size i = 0; i < num_qubits; i += 2) {
        amp.ApplyMergedXYGate(create_Y_1_2({static_cast<float>(i)}),
                              create_X_1_2({static_cast<float>(i + 1)}));
    }
    
    vector<idx_size> indices;
    if (idx_infile != "") {
        
        int f_idx;
        ifstream idx_in;
        idx_in.open(idx_infile);
        
        while (idx_in >> f_idx)
            indices.push_back(f_idx);
        
        idx_in.close();
    }
    
    Time qft_time;
    qft_time.StartTime();
    
    idx_size amp_size = 1ull << num_qubits;
    
    amp.ApplyQFT();
    
    double elapsed_time = qft_time.GetElapsedTime();
    
    cout << "\n(C) Igor L. Markov and Aneeqa Fatima  2018 - 2021\n";
    cout << "Entangled Simulator ver 1.0 - a quantum circuit simulator\n\n";
    cout << "Entangled Simulator QFT time : " << elapsed_time << "s\n\n";
    
    if (idx_infile != "") {
        for (idx_size i = 0; i < indices.size(); ++i)
            cout << "amp[" << indices[i] << "] = " << amp[indices[i]] << "\n";
    }
    else if (num_qubits < 15) {
        cout << "amp :\n";
        for (idx_size i = 0; i < amp_size; ++i)
            cout << amp[i] << "\n";
    }
    else {
        cout << "amp[3] = " << amp[3] << "\n";
        cout << "amp[1/4] = " << amp[amp_size/4] << "\n";
        cout << "amp[1/2] = " << amp[amp_size/2] << "\n";
        cout << "amp[3/4] = " << amp[3 * amp_size/4] << "\n";
        cout << "amp[-3] = " << amp[amp_size - 3] << "\n";
    }
    cout << "\n¯\\_(ツ)_/¯ \n\n";
}
