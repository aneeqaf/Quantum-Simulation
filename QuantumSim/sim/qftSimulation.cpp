//
//  qftSimulation.cpp
//  vector_state_sim
//
//  Created by Aneeqa Fatima on 8/8/21.
//  Copyright © 2021 Aneeqa Fatima. All rights reserved.
//

#include "qftSimulation.h"

void QFTSimulation::
Simulate(FullAmpStateVector& amp)
{
    Time qft_time;
    qft_time.StartTime();
    
    idx_size amp_size = 1ull << num_qubits;
    
    amp.ApplyQFT();
    
    double elapsed_time = qft_time.GetElapsedTime();
    
    cout << "RR QFT Time : " << elapsed_time << "s\n\n";
    
    if (num_qubits < 15) {
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
}
