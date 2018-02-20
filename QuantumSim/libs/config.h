//
//  configuration.h
//  QuantumSimProj
//
//  Created by Aneeqa Fatima on 1/27/18.
//

#ifndef configuration_h
#define configuration_h

#include <string>

using namespace std;

class Config {
public:
    enum SimType : int {LosslessH, LosslessV, Approx1CutH, Approx1CutV, Approx2Cuts, FullState,
        Approx2011, Approx1_101, Approx1110};
    enum Verbose : int {None, NCCV, NCC, Default, Cycles};
    
    string infile;
    string prob_outfile;
    string amp_outfile;
    string report_outfile;
    string misc_outfile;
    int depth;
    int th;
    int vcut;
    int hcut;
    bool google;
    SimType sim_type;
    Verbose verbose;
    
    Config() : infile(""), prob_outfile(""), amp_outfile(""), report_outfile(""), misc_outfile("") ,depth(26),
    th(14), vcut(0), hcut(0), google(true), sim_type(FullState), verbose(Default) {}
    Config(string ifile,
           string pfile,
           string afile,
           string rfile,
           string mfile,
           SimType sim = FullState,
           Verbose v = Default,
           int a = 0,
           int b = 0,
           int d = 26,
           int t = 14,
           bool google = true);
    Config(const Config& rhs);
    Config& operator=(const Config& rhs);
};

#endif /* configuration_h */
