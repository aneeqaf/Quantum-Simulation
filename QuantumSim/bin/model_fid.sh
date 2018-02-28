#!/bin/bash

set -e

# make clean
# make all

# ./bin/rollright_sim -h inst_6_5_100_4 -t 14 -d 41 -s 5 -o 30q_41d_4exact -v 4
# ./bin/rollright_sim -h inst_6_5_100_4 -t 14 -d 41 -s 2 -o 30q_41d_4approxH -v 4
 ./bin/approx-i11i.sh > output/reports/fccm_25q_41d_5_-i11i.txt&

# ./bin/rollright_sim -h inst_6_5_100_4 -t 14 -d 41 -s 3 -o 30q_41d_4approxV -v 4
 ./bin/approx-i11iOWT.sh > output/reports/fccm_25q_41d_5_-i11iOWT.txt&

# ./bin/rollright_sim -h inst_6_5_100_4 -t 14 -d 41 -s 4 -o 30q_41d_4approx2011OWT -v 4
./bin/approx2011OWT.sh > output/reports/fccm_25q_41d_5_2011OWT.txt&
./bin/approx2011.sh > output/reports/fccm_25q_41d_5_2011.txt&

python3 python_scripts/model_fid.py "Fidelity loss for 25q_41d circuit inst_5_5_100_5" \
output/misc/25q_41d_fid_-i11i.txt output/misc/25q_41d_fid_2011OWT.txt \
output/misc/25q_41d_fid_-i11iOWT.txt output/misc/25q_41d_fid_2011.txt &