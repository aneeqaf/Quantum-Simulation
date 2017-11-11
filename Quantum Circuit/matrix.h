//
//  matrix.hpp
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 10/19/17.
//  Copyright © 2017 Aneeqa Fatima. All rights reserved.
//

#ifndef matrix_h
#define matrix_h

#include <vector>
#include <complex>
#include <stdio.h>

using namespace std;
using cplx = std::complex<float>;

extern vector<vector<cplx>> matrix_mult(vector<vector<cplx>>& g1, vector<vector<cplx>>& g2);
extern vector<cplx> matrix_v_mult(vector<vector<cplx>>& matrix, vector<cplx>& vector1);
extern vector<cplx> matrix_v_mult_diag(vector<vector<cplx>>& matrix, vector<cplx>& vector1);
extern vector<cplx> tensor_v_product(vector<cplx>& first, vector<cplx>& second);
extern vector<vector<cplx>> tensor_product(vector<vector<cplx>>& first, vector<vector<cplx>>& second);
extern vector<vector<cplx>> matrix_inv(vector<vector<cplx>>& g);
extern vector<vector<cplx>> tensor_c(vector<vector<cplx>>& first, vector<vector<cplx>>& second, cplx c);

#endif /* matrix_h */
