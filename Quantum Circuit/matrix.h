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
using cmplx = complex<float>;

extern vector<vector<cmplx>> matrix_mult(vector<vector<cmplx>>& g1, vector<vector<cmplx>>& g2);
extern vector<cmplx> matrix_v_mult(const vector<vector<cmplx>>& matrix,
                                   const vector<cmplx>& vector1);
extern vector<cmplx> matrix_v_mult_diag(vector<vector<cmplx>> matrix, vector<cmplx> vector1);
extern vector<cmplx> tensor_v_product(vector<cmplx>& first, vector<cmplx>& second);
extern vector<vector<cmplx>> tensor_product(vector<vector<cmplx>>& first, vector<vector<cmplx>>& second);
extern vector<vector<cmplx>> matrix_inv(vector<vector<cmplx>>& g);
extern vector<vector<cmplx>> tensor_c(vector<vector<cmplx>>& first, vector<vector<cmplx>>& second, cmplx c);

#endif /* matrix_h */
