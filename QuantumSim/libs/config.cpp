//
//  config.cpp
//  vector_state_sim
//
//  Created by Aneeqa Fatima on 1/29/18.
//  Copyright © 2018 Aneeqa Fatima. All rights reserved.
//

#include "config.h"

Config::
Config(string ifile,
       string pfile,
       string afile,
       string rfile,
       string mfile,
       SimType sim,
       Verbose v,
       int c,
       int d,
       int t,
       bool google) : infile(ifile), prob_outfile(pfile), amp_outfile(afile), report_outfile(rfile),
misc_outfile(mfile), depth(d), th(t), cut(c), google(google), sim_type(sim), verbose(v)
{
    if (t == -1)
        th = 14;
}

Config::
Config(const Config& rhs)
{
    infile = rhs.infile;
    prob_outfile = rhs.prob_outfile;
    amp_outfile = rhs.amp_outfile;
    report_outfile = rhs.report_outfile;
    misc_outfile = rhs.misc_outfile;
    depth = rhs.depth;
    th = rhs.th;
    cut = rhs.cut;
    google = rhs.google;
    sim_type = rhs.sim_type;
    verbose = rhs.verbose;
}

Config& Config::
operator=(const Config& rhs)
{
    infile = rhs.infile;
    prob_outfile = rhs.prob_outfile;
    amp_outfile = rhs.amp_outfile;
    report_outfile = rhs.report_outfile;
    misc_outfile = rhs.misc_outfile;
    depth = rhs.depth;
    th = rhs.th;
    cut = rhs.cut;
    google = rhs.google;
    sim_type = rhs.sim_type;
    verbose = rhs.verbose;
    return *this;
}
