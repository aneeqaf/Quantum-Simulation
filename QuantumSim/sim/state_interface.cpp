
//  state_interface.cpp
//  vector_state_sim
//
//  Created by Aneeqa Fatima on 1/18/18.
//  Copyright © 2018 Aneeqa Fatima. All rights reserved.
//

#include "state_interface.h"

Times GenericQuantumState::time_by_category({});
Counts GenericQuantumState::count_of_category({});
vector<string> GenericQuantumState::log({});
vector<array<complex<float>*, 2>> GenericQuantumState::compressed_vector_ptrs({});
Data GenericQuantumState::data_per_cycles({});
bool GenericQuantumState::book_keep = true;
Config::SimType GenericQuantumState::sim_type = Config::SimType::FullState;
char GenericQuantumState::partition_to_sim = 'x';

#ifdef Parallel
int GenericQuantumState::num_threads = omp_get_num_procs();
#else
int GenericQuantumState::num_threads = 8;
#endif

ostream& operator<<(ostream& o, const vector<int>& v) {
    for (auto k : v) o << k << " ";
    return o << endl;
}

GenericQuantumState::
GenericQuantumState(int n_threads): compressed(false){
    num_threads = n_threads;
}

idx_size GenericQuantumState::
GetNumAddends() const
{
    return 0;
}
