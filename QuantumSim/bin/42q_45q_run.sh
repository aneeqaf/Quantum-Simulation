#!/bin/bash

set -e

./python_scripts/dist_sim.py inst_7_6_100_5 --depth=27 --num_threads=4 
./python_scripts/dist_sim.py inst_100_9_5_0 --depth=27 --num_threads=4 
