//
//  qftSimulation.h
//  vector_state_sim
//
//  Created by Aneeqa Fatima on 8/8/21.
//  Copyright © 2021 Aneeqa Fatima. All rights reserved.
//

#ifndef qftSimulation_h
#define qftSimulation_h

#include <stdio.h>

#include "kernelsAVX.h"
#include "state_interface.h"
#include "state.h"

class QFTSimulation {
    idx_size num_qubits;
    
public:
    QFTSimulation(idx_size num_qubits): num_qubits(num_qubits) {}
    
    void Simulate(FullAmpStateVector& amp, string idx_infile = "");
};

#endif /* qftSimulation_h */
