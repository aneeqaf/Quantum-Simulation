//
//  config.cpp
//  vector_state_sim
//
//  Created by Aneeqa Fatima on 1/29/18.
//  Copyright © 2018 Aneeqa Fatima. All rights reserved.
//

#include "config.h"

Config::
Config(const Config& rhs)
{
    infile = rhs.infile;
    outfile = rhs.outfile;
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
    outfile = rhs.outfile;
    depth = rhs.depth;
    th = rhs.th;
    cut = rhs.cut;
    google = rhs.google;
    sim_type = rhs.sim_type;
    verbose = rhs.verbose;
    return *this;
}
