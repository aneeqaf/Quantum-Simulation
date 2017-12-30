//
//  matrix.cpp
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 10/21/17.
//

#include <stdio.h>

#include <vector>
#include <complex>
#include <stdio.h>

#include "matrix.h"

using namespace std;

vector<vector<cmplx>> matrix_mult(vector<vector<cmplx>>& g1,
                                            vector<vector<cmplx>>& g2) {
    
    vector<vector<cmplx>> product;
    
    for (idx_size i = 0; i < g1.size(); ++i) {
        vector<cmplx> row;
        for (idx_size k = 0; k < g2[0].size(); ++k){
            cmplx sum = 0;
            for (idx_size j = 0; j < g2.size(); ++j) {
                sum += g1[i][j] * g2[j][k];
            }
            row.push_back(sum);
        }
        product.push_back(row);
    }
    return product;
}

vector<cmplx> matrix_v_mult(const vector<vector<cmplx>>& matrix,
                            const vector<cmplx>& vector1) {
    
    vector<cmplx> product;
    
    for (idx_size i = 0; i < matrix.size(); ++i) {
        cmplx sum = 0;
        for (idx_size j = 0; j < vector1.size(); ++j) {
            sum += matrix[i][j] * vector1[j];
        }
        product.push_back(sum);
    }
    return product;
}

vector<cmplx> matrix_v_mult_diag(vector<vector<cmplx>>& matrix,
                                      vector<cmplx>& vector1) {
    
    vector<cmplx> product;
    
    for (idx_size i = 0; i < matrix.size(); ++i) {
        cmplx sum = 0;
        sum += matrix[i][i] * vector1[i];
        product.push_back(sum);
    }
    return product;
}


vector<cmplx> tensor_v_product(vector<cmplx>& first,
                                         vector<cmplx>& second) {
    
    vector<cmplx> tensor;
    
    for (idx_size i = 0; i < first.size(); ++i) {
        for (idx_size j = 0; j < second.size(); ++j){
            tensor.push_back(first[i] * second[j]);
        }
    }
    return tensor;
}

vector<vector<cmplx>> tensor_product(vector<vector<cmplx>>& first,
                                               vector<vector<cmplx>>& second) {
    
    vector<vector<cmplx>> tensor;
    
    for (idx_size i = 0; i < first.size(); ++i) {
        for (idx_size j = 0; j < second.size(); ++j){
            vector<cmplx> row;
            for (idx_size k = 0; k < first[i].size(); ++k){
                for (idx_size l = 0; l < second[j].size(); ++l) {
                    row.push_back(first[i][k] * second[j][l]);
                }
            }
            tensor.push_back(row);
        }
    }
    return tensor;
}

vector<vector<cmplx>> tensor_c(vector<vector<cmplx>>& first,
                                         vector<vector<cmplx>>& second, cmplx c)
{
    vector<vector<cmplx>> tensor;
    
    for (idx_size i = 0; i < first.size(); ++i) {
        for (idx_size j = 0; j < second.size(); ++j){
            vector<cmplx> row;
            for (idx_size k = 0; k < first[i].size(); ++k){
                for (idx_size l = 0; l < second[j].size(); ++l) {
                    if(first[i][k] == c) {
                        if(j == l) {
                            row.push_back(first[i][k] * cmplx(1,0));
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

vector<vector<cmplx>> matrix_inv(vector<vector<cmplx>>& g) {
    
    vector<vector<cmplx>> inv = g;
    
    inv[0][1] = -g[1][0];
    inv[1][0] = -g[0][1];
    
    cmplx determinant = cmplx(1,0) / (g[0][0]*g[1][1] - g[0][1]*g[1][0]) ;
    
    for (idx_size i = 0; i < g.size(); ++i) {
        inv[i][0] = determinant * inv[i][0];
        inv[i][1] = determinant * inv[i][1];
    }
    
    return  inv;
}
