#!/bin/bash
set -e
./bin/rr -i test10.txt --num_threads 1 --sim_type 0 --idx 7,64 --CZ_path 1,0,0,2 --outfile output_1,a
