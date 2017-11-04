//
//  matrix.cpp
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 10/21/17.
//  Copyright © 2017 Aneeqa Fatima. All rights reserved.
//

#include <stdio.h>

#include <vector>
#include <complex>
#include <stdio.h>

#include "matrix.h"

using namespace std;

vector<vector<complex<double>>> matrix_mult(vector<vector<complex<double>>>& g1,
                                            vector<vector<complex<double>>>& g2) {
    
    vector<vector<complex<double>>> product;
    
    for (int i = 0; i < g1.size(); ++i) {
        vector<complex<double>> row;
        for (int k = 0; k < g2[0].size(); ++k){
            complex<double> sum = 0;
            for (int j = 0; j < g2.size(); ++j) {
                sum += g1[i][j] * g2[j][k];
            }
            row.push_back(sum);
        }
        product.push_back(row);
    }
    return product;
}

vector<complex<double>> matrix_v_mult(vector<vector<complex<double>>>& matrix,
                                      vector<complex<double>>& vector1) {
    
    vector<complex<double>> product;
    
    for (int i = 0; i < matrix.size(); ++i) {
        complex<double> sum = 0;
        for (int j = 0; j < vector1.size(); ++j) {
            sum += matrix[i][j] * vector1[j];
        }
        product.push_back(sum);
    }
    return product;
}

vector<complex<double>> tensor_v_product(vector<complex<double>>& first,
                                         vector<complex<double>>& second) {
    
    vector<complex<double>> tensor;
    
    for (int i = 0; i < first.size(); ++i) {
        for (int j = 0; j < second.size(); ++j){
            tensor.push_back(first[i] * second[j]);
        }
    }
    return tensor;
}

vector<vector<complex<double>>> tensor_product(vector<vector<complex<double>>>& first,
                                               vector<vector<complex<double>>>& second) {
    
    vector<vector<complex<double>>> tensor;
    
    for (int i = 0; i < first.size(); ++i) {
        for (int j = 0; j < second.size(); ++j){
            vector<complex<double>> row;
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

vector<vector<complex<double>>> tensor_c(vector<vector<complex<double>>>& first,
                                         vector<vector<complex<double>>>& second, complex<double> c)
{
    vector<vector<complex<double>>> tensor;
    
    for (int i = 0; i < first.size(); ++i) {
        for (int j = 0; j < second.size(); ++j){
            vector<complex<double>> row;
            for(int k = 0; k < first[i].size(); ++k){
                for(int l = 0; l < second[j].size(); ++l) {
                    if(first[i][k] == c) {
                        if(j == l) {
                            row.push_back(first[i][k] * complex<double>(1,0));
                        }
                        else {
                            row.push_back(0);
                        }
                    }
                    else {
                        row.push_back(first[i][k] * second[j][l]);
                    }
                }
            }
            tensor.push_back(row);
        }
    }
    return tensor;
}

vector<vector<complex<double>>> matrix_inv(vector<vector<complex<double>>>& g) {
    
    vector<vector<complex<double>>> inv = g;
    
    inv[0][1] = -g[1][0];
    inv[1][0] = -g[0][1];
    
    complex<double> determinant = complex<double>(1,0) / (g[0][0]*g[1][1] - g[0][1]*g[1][0]) ;
    
    for (int i = 0; i < g.size(); ++i) {
        inv[i][0] = determinant * inv[i][0];
        inv[i][1] = determinant * inv[i][1];
    }
    
    return  inv;
}
