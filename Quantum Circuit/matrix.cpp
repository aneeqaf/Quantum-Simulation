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

vector<vector<cplx>> matrix_mult(vector<vector<cplx>>& g1,
                                            vector<vector<cplx>>& g2) {
    
    vector<vector<cplx>> product;
    
    for (int i = 0; i < g1.size(); ++i) {
        vector<cplx> row;
        for (int k = 0; k < g2[0].size(); ++k){
            cplx sum = 0;
            for (int j = 0; j < g2.size(); ++j) {
                sum += g1[i][j] * g2[j][k];
            }
            row.push_back(sum);
        }
        product.push_back(row);
    }
    return product;
}

vector<cplx> matrix_v_mult(vector<vector<cplx>>& matrix,
                                      vector<cplx>& vector1) {
    
    vector<cplx> product;
    
    for (int i = 0; i < matrix.size(); ++i) {
        cplx sum = 0;
        for (int j = 0; j < vector1.size(); ++j) {
            sum += matrix[i][j] * vector1[j];
        }
        product.push_back(sum);
    }
    return product;
}

vector<cplx> matrix_v_mult_diag(vector<vector<cplx>>& matrix,
                                      vector<cplx>& vector1) {
    
    vector<cplx> product;
    
    for (int i = 0; i < matrix.size(); ++i) {
        cplx sum = 0;
        sum += matrix[i][i] * vector1[i];
        product.push_back(sum);
    }
    return product;
}


vector<cplx> tensor_v_product(vector<cplx>& first,
                                         vector<cplx>& second) {
    
    vector<cplx> tensor;
    
    for (int i = 0; i < first.size(); ++i) {
        for (int j = 0; j < second.size(); ++j){
            tensor.push_back(first[i] * second[j]);
        }
    }
    return tensor;
}

vector<vector<cplx>> tensor_product(vector<vector<cplx>>& first,
                                               vector<vector<cplx>>& second) {
    
    vector<vector<cplx>> tensor;
    
    for (int i = 0; i < first.size(); ++i) {
        for (int j = 0; j < second.size(); ++j){
            vector<cplx> row;
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

vector<vector<cplx>> tensor_c(vector<vector<cplx>>& first,
                                         vector<vector<cplx>>& second, cplx c)
{
    vector<vector<cplx>> tensor;
    
    for (int i = 0; i < first.size(); ++i) {
        for (int j = 0; j < second.size(); ++j){
            vector<cplx> row;
            for(int k = 0; k < first[i].size(); ++k){
                for(int l = 0; l < second[j].size(); ++l) {
                    if(first[i][k] == c) {
                        if(j == l) {
                            row.push_back(first[i][k] * cplx(1,0));
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

vector<vector<cplx>> matrix_inv(vector<vector<cplx>>& g) {
    
    vector<vector<cplx>> inv = g;
    
    inv[0][1] = -g[1][0];
    inv[1][0] = -g[0][1];
    
    cplx determinant = cplx(1,0) / (g[0][0]*g[1][1] - g[0][1]*g[1][0]) ;
    
    for (int i = 0; i < g.size(); ++i) {
        inv[i][0] = determinant * inv[i][0];
        inv[i][1] = determinant * inv[i][1];
    }
    
    return  inv;
}
