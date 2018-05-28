#!/bin/bash

set -e

mkdir -p google
mkdir -p google/bin
mkdir -p google/python_scripts

cp python_scripts/dist_sim.py google/python_scripts/
cp python_scripts/dist_sim_report_gen.py google/python_scripts/
cp python_scripts/dist_util.py google/python_scripts/
cp python_scripts/post_launch.py google/python_scripts/
cp python_scripts/add_amps.py google/python_scripts/
cp python_scripts/multinode_report_gen.py google/python_scripts/
cp python_scripts/post_multinode_sim.py google/python_scripts/
cp python_scripts/execute_scripts.py google/python_scripts/
cp python_scripts/resume_batches.py google/python_scripts/
cp python_scripts/README.md google/python_scripts/

cp $1 google/bin/
cp ../README.md google/

zip -r google.zip google