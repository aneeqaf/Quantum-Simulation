//
//  time.h
//  QuantumSimProj
//
//  Created by Aneeqa Fatima on 1/27/18.
//  Copyright © 2018 Aneeqa Fatima. All rights reserved.
//

#ifndef profile_h
#define profile_h

using ul = unsigned long;

typedef struct TimesByCategory{
    double initial_H;
    double last_H;
    double X1_2;
    double Y1_2;
    double merged_XY1_2;
    double CZ_T;
    double decomposed_CZ;
    double rescale;
    double conversion;
    double copying;
    double norm;
    double amp_storage;
    double low_q_XY_CZT;
    double high_q_XY1_2;
    
    TimesByCategory(): initial_H(0), last_H(0), X1_2(0), Y1_2(0), merged_XY1_2(0), CZ_T(0), decomposed_CZ(0), rescale(0), conversion(0), copying(0), norm(0), amp_storage(0), low_q_XY_CZT(0), high_q_XY1_2(0) {}
}Times;


typedef struct CountOfCategory{
    ul initial_H;
    ul last_H;
    ul H_merged_lo;
    ul H_merged_hi;
    ul X1_2;
    ul Y1_2;
    ul merged_XY1_2;
    ul CZ_T;
    ul decomposed_CZ;
    ul rescale;
    ul conversion;
    ul xCZ_not_applied;
    ul copying;
    ul low_q_XY1_2;
    ul high_q_XY1_2;
    int cycle_p;
    int cycle_r;
    int cycle_d;
    
    CountOfCategory(): initial_H(0), last_H(0), H_merged_lo(0), H_merged_hi(0), X1_2(0), Y1_2(0), merged_XY1_2(0), CZ_T(0), decomposed_CZ(0), rescale(0), conversion(0), xCZ_not_applied(0), copying(0), low_q_XY1_2(0), high_q_XY1_2(0),
        cycle_p(0), cycle_r(0), cycle_d(0){}
}Counts;

class Time {
public:
    struct timespec start, end;
    
    Time():start({}), end({}) {}
    
    inline void StartTime()
    {
        clock_gettime(CLOCK_MONOTONIC, &start);
    }
    
    inline void EndTime()
    {
        clock_gettime(CLOCK_MONOTONIC, &end);
    }
    
    inline double GetElapsedTime()
    {
        clock_gettime(CLOCK_MONOTONIC, &end);
        return (end.tv_sec - start.tv_sec) + ((end.tv_nsec - start.tv_nsec)/1.0e9);
    }
};

#endif /* profile_h */
