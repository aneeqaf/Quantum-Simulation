#!/bin/bash

set -e

make clean
make all

./bin/rollright_sim -h inst_6_5_100_4 -t 14 -d 41 -s 5 -o 30q_41d_4exact -v 4
 ./bin/rollright_sim -h inst_6_5_100_4 -t 14 -d 41 -s 2 -o 30q_41d_4approxH -v 4
 ./bin/approxH.sh > output/reports/fccm_30q_4approxH.txt

 ./bin/rollright_sim -h inst_6_5_100_4 -t 14 -d 41 -s 3 -o 30q_41d_4approxV -v 4
 ./bin/approxV.sh > output/reports/fccm_30q_4approxV.txt

 ./bin/rollright_sim -h inst_6_5_100_4 -t 14 -d 41 -s 4 -o 30q_41d_4approx2011OWT -v 4
./bin/approx2011OWT.sh > output/reports/fccm_30q_42011OWT.txt

python3 python_scripts/model_fid.py "Fidelity loss for 30q_41d" \
output/misc/30q_41d_4fid_approxV.txt output/misc/30q_41d_4fid_approxH.txt output/misc/30q_41d_4fid_2011OWT.txt