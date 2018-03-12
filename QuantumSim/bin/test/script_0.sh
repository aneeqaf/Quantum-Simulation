#!/bin/bash
set -e
./bin/rr -p test --num_threads 4 --sim_type 0 --outfile test --idx 7,5 --CZ_path 2,0 
./bin/rr -p test --num_threads 4 --sim_type 0 --outfile test --idx 7,5 --CZ_path 2,1 
