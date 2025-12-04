//
//  kernels_spec.h
//  QuantumSimProj
//
//  Created by Aneeqa Fatima on 6/22/21.
//  Copyright © 2021 Aneeqa Fatima. All rights reserved.
//

#ifndef kernels_spec_h
#define kernels_spec_h

#include <string>
#include <tuple>

using namespace std;


template <class TFunc, size_t TNumArgs, class...Ts>
class KernelSpec {
    TFunc kernel;
    size_t num_args;
    tuple<Ts...> args;
    
public:
    
    KernelSpec(TFunc kernel_func, Ts... vals): kernel(kernel_func), num_args(TNumArgs), args(tuple<Ts...>(vals...)) {}
    
    pair<size_t, size_t> InvokeKernel()
    {
        return apply(kernel, args);
    }
};

#endif /* kernels_spec_h */
