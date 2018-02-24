//
//  time.h
//  QuantumSimProj
//
//  Created by Aneeqa Fatima on 1/27/18.
//  Copyright © 2018 Aneeqa Fatima. All rights reserved.
//

#ifndef time_h
#define time_h

using ul = unsigned long;

typedef struct TimesByCategory{
    double H;
    double X1_2;
    double Y1_2;
    double merged_XY1_2;
    double CZ_T;
    double decomposed_CZ;
    double rescale;
    double conversion;
    
    TimesByCategory(): H(0), X1_2(0), Y1_2(0), merged_XY1_2(0), CZ_T(0), decomposed_CZ(0), rescale(0), conversion(0){}
}Times;


typedef struct CountOfCategory{
    ul H;
    ul X1_2;
    ul Y1_2;
    ul merged_XY1_2;
    ul CZ_T;
    ul decomposed_CZ;
    ul rescale;
    ul conversion;
    ul xCZ_not_applied;
    
    CountOfCategory(): H(0), X1_2(0), Y1_2(0), merged_XY1_2(0), CZ_T(0), decomposed_CZ(0), rescale(0), conversion(0),
    xCZ_not_applied(0) {}
}Counts;

#endif /* time_h */
