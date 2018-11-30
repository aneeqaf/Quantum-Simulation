//
//  main.cpp
//  Practice
//
//  Created by Aneeqa Fatima on 9/9/18.
//  Copyright © 2018 Aneeqa Fatima. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <cmath>
#include <complex>
#include <cassert>

#include "gnuplot-iostream/gnuplot-iostream.h"

using namespace std;

using cmplx = complex<float>;
using Packed8CharArray = unsigned char[8];

constexpr double PI = M_PI;

void FindPitch(const cmplx* amp,
               size_t size,
               map<double, pair<double, double>>& mag_phases_pitches)
{
    for (size_t i = 0; i < size; ++i) {
        double amp_real = amp[i].real(), amp_imag = amp[i].imag();
        bool neg_real = false, neg_imag = false;
        if (amp_real < 0) {
            amp_real *= -1;
            neg_real = true;
        }
        if (amp_imag < 0) {
            amp_imag *= -1;
            neg_imag = true;
        }
        
        double phase = atan(amp_imag/amp_real);
        double magnitude = abs(amp[i]);
        
        double b = (1/phase) * log(magnitude);
        double pitch = (PI/2) - atan(1/b);
        mag_phases_pitches[magnitude] = pair<double, double>(phase, pitch);
//        double new_b = 1/tan((PI/2) - pitch);
//        double new_magnitude = exp(new_b * phase);
//
//        cout << b << " : " << pitch << " | " << new_b << " : " << magnitude << " -> " << new_magnitude << endl;
        
    }
    
//    for (auto mpp : mag_phases_pitches) {
//        cout << mpp.first << " , " << mpp.second.first << " , " << mpp.second.second << endl;
//    }
}

void PlotLogSpiralAndAmpDensity(cmplx* amp,
                                size_t size)
{
    Gnuplot gp;
    
    vector<pair<float, float> > xy_pts;
    for(size_t i = 0; i < size; ++i)
        xy_pts.push_back(make_pair(amp[i].real(), amp[i].imag()));

    
    gp << "reset\nset nokey\n";
    gp << "set term png size 1000,480\n";
    gp << "set output 'log_spiral.png'\n";
    gp << "set parametric\nset size ratio -1\nset samples 1e5\n";
    gp << "set tics font 'Times New Roman,18'\n";
//    gp << "set xl 'x(t)' enhanced font 'Latin Modern Math, 20'\n";
//    gp << "set yl 'y(t)' enhanced font 'Latin Modern Math, 20'\n";
    gp << "L = 0.04\n";
    gp << "set xr[-L:L]\nset yr[-L:L]\n";
    gp << "set xtics -L , 0.02 , L\nset ytics -L, 0.02, L\nset grid\n";
    
    
    gp << "a=0.01 \nb=0.05\n";
    gp << "x(t) = a*exp(b*t)*cos(t)\n";
    gp << "y(t) = a*exp(b*t)*sin(t)\n";
//    gp << "x(t) = a*exp(b*t)*(1-(t**2))/(1+(t**2))\n";
//    gp << "y(t) = a*exp(b*t)*(2*t)/(1+(t**2))\n";
    gp << "title(a, b) = sprintf(\"\\na=%.2f  b=%.2f \\n\",a, b) \n";
    gp << "set multiplot layout 1, 2 title title(a, b) font 'Latin Modern Math, 20'\n";
    gp << "set title \"Zoom out \"\n";
    gp << "plot [-40*pi:8*pi] " << gp.file1d(xy_pts, "file.dat") << " with points pt 7 ps 0.2 lc 'grey', x(t), y(t) lc 'blue'\n";
    gp << "set termoption enhanced\n";
    
    gp << "set title \"Zoom in \"\n";
    gp << "L = 0.0001\n";
    gp << "set xr[-L:L]\nset yr[-L:L]\n";
    gp << "set xtics -L , 0.00005 , L\nset ytics -L, 0.00005, L\nset grid\n";
    gp << "plot [-40*pi:8*pi] " << gp.file1d(xy_pts, "file.dat") << " with points pt 7 ps 0.2 lc 'grey', x(t), y(t) lc 'blue'\n";
    
}

int main(int argc, const char * argv[]) {
    
    size_t amp_size = 1ull << 16;
    cmplx* amp = nullptr;
    
    if (posix_memalign((void**)&amp, 64, sizeof(cmplx) * amp_size) != 0)
        throw "Unable to allocate";
    memset(amp, 0, amp_size * sizeof(amp));
    amp[0] = 1;
    
    ifstream infile;
    infile.open("exact.txt");
    
    char extra = '\0';
//    getline(infile, cmplx_num);
    float real = 0, imag = 0;
    int i = 0;
    while (infile >> real >> imag >> extra) {
        assert(i < amp_size);
        amp[i++] = cmplx(real, imag);
    }
    infile.close();
    
    map<double, pair<double, double>> mag_phases_pitches;

    FindPitch(amp, amp_size, mag_phases_pitches);
    PlotLogSpiralAndAmpDensity(amp, amp_size);
}
