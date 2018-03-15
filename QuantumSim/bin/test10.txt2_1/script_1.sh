#!/bin/bash
set -e
./bin/rr -i test10.txt --num_threads 1 --sim_type 0 --idx 7,20 --CZ_path 2,2,0,1 --outfile output_2
./bin/rr -i test10.txt --num_threads 1 --sim_type 0 --idx 7,20 --CZ_path 2,3,0,1 --outfile output_2,a
