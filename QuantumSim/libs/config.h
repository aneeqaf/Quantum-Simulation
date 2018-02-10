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
    int cut;
    bool google;
    SimType sim_type;
    Verbose verbose;
    
    Config() : infile(""), prob_outfile(""), amp_outfile(""), report_outfile(""), misc_outfile("") ,depth(26),
    th(14), cut(0), google(true), sim_type(FullState), verbose(Default) {}
    Config(string ifile,
           string pfile,
           string afile,
           string rfile,
           string mfile,
           SimType sim = FullState,
           Verbose v = Default,
           int cut = 0,
           int depth = 26,
           int th = 14,
           bool google = true) : infile(ifile), prob_outfile(pfile), amp_outfile(afile), report_outfile(rfile),
        misc_outfile(mfile), depth(depth), th(th), cut(cut), google(google), sim_type(sim), verbose(v) {}
    Config(const Config& rhs);
    Config& operator=(const Config& rhs);
};

#endif /* configuration_h */
