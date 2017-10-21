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

vector<vector<complex<double>>> matrix_mult(vector<vector<complex<double>>>& g1, vector<vector<complex<double>>>& g2)
{
    vector<vector<complex<double>>> product;
    
    for (int i = 0; i < g2.size(); ++i) {
        vector<complex<double>> row;
        for (int k = 0; k < g1[0].size(); ++k){
            complex<double> sum = 0;
            for (int j = 0; j < g1.size(); ++j) {
                sum += g2[i][j] * g1[j][k];
            }
            row.push_back(sum);
        }
        product.push_back(row);
    }
    return product;
}

inline vector<double> matrix_v_mult(vector<vector<double>>& matrix, vector<double>& vector1) {
    
    vector<double> product;
    
    for (int i = 0; i < matrix.size(); ++i) {
        long long sum = 0;
        for (int j = 0; j < vector1.size(); ++j) {
            sum += matrix[i][j] * vector1[j];
        }
        product.push_back(sum);
    }
    return product;
}

inline vector<double> Tensor_Product(vector<double>& first, vector<double>& second) {
    
    vector<double> tensor;
    
    for (int i = 0; i < first.size(); ++i) {
        for (int j = 0; j < second.size(); ++j){
            tensor.push_back(first[i] * second[j]);
        }
    }
    return tensor;
}

vector<vector<double>> Tensor_Product(vector<vector<double>>& first, vector<vector<double>>& second) {
    
    vector<vector<double>> tensor;
    
    for (int i = 0; i < first.size(); ++i) {
        for (int j = 0; j < second.size(); ++j){
            vector<double> row;
            for(int k = 0; k < first[i].size(); ++k){
                for(int l = 0; l < second[j].size(); ++l) {
                    row.push_back(first[i][k] * second[j][l]);
                }
            }
            tensor.push_back(row);
        }
    }
    return tensor;
}

#endif /* matrix_h */
