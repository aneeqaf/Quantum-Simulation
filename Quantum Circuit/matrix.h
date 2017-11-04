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

extern vector<vector<complex<double>>> matrix_mult(vector<vector<complex<double>>>& g1, vector<vector<complex<double>>>& g2);
extern vector<complex<double>> matrix_v_mult(vector<vector<complex<double>>>& matrix, vector<complex<double>>& vector1);
extern vector<complex<double>> tensor_v_product(vector<complex<double>>& first, vector<complex<double>>& second);
extern vector<vector<complex<double>>> tensor_product(vector<vector<complex<double>>>& first, vector<vector<complex<double>>>& second);
extern vector<vector<complex<double>>> matrix_inv(vector<vector<complex<double>>>& g);
extern vector<vector<complex<double>>> tensor_c(vector<vector<complex<double>>>& first, vector<vector<complex<double>>>& second, complex<double> c);

#endif /* matrix_h */
