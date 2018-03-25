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
    double H;
    double X1_2;
    double Y1_2;
    double merged_XY1_2;
    double CZ_T;
    double decomposed_CZ;
    double rescale;
    double conversion;
    double copying;
    double norm;
    
    TimesByCategory(): H(0), X1_2(0), Y1_2(0), merged_XY1_2(0), CZ_T(0), decomposed_CZ(0), rescale(0), conversion(0), copying(0), norm(0) {}
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
    ul copying;
    
    CountOfCategory(): H(0), X1_2(0), Y1_2(0), merged_XY1_2(0), CZ_T(0), decomposed_CZ(0), rescale(0), conversion(0), xCZ_not_applied(0), copying(0) {}
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
