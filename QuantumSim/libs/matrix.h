//
//  matrix.h
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 10/19/17.
//

#ifndef matrix_h
#define matrix_h

#include <complex>
#include <stdio.h>
#include <vector>

using namespace std;
using cmplx = complex<float>;
using idx_size = unsigned long long;

vector<vector<cmplx>> matrix_mult(vector<vector<cmplx>>& g1, vector<vector<cmplx>>& g2);
vector<cmplx> matrix_v_mult(const vector<vector<cmplx>>& matrix,
                                   const vector<cmplx>& vector1);
vector<cmplx> matrix_v_mult_diag(vector<vector<cmplx>> matrix, vector<cmplx> vector1);
vector<cmplx> tensor_v_product(vector<cmplx>& first, vector<cmplx>& second);
vector<vector<cmplx>> tensor_product(vector<vector<cmplx>>& first, vector<vector<cmplx>>& second);
vector<vector<cmplx>> matrix_inv(vector<vector<cmplx>>& g);
vector<vector<cmplx>> tensor_c(vector<vector<cmplx>>& first, vector<vector<cmplx>>& second, cmplx c);

#endif /* matrix_h */
