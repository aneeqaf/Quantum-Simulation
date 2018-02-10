#!/bin/bash
set -e

case $1 in 
	"approx2cuts")
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_1.txt \
	output/probabilities/30q_41d_approx2cuts_1.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_5.txt \
	output/probabilities/30q_41d_approx2cuts_5.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_7.txt \
	output/probabilities/30q_41d_approx2cuts_7.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_9.txt \
	output/probabilities/30q_41d_approx2cuts_9.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_11.txt \
	output/probabilities/30q_41d_approx2cuts_11.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_13.txt \
	output/probabilities/30q_41d_approx2cuts_13.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_15.txt \
	output/probabilities/30q_41d_approx2cuts_15.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_17.txt \
	output/probabilities/30q_41d_approx2cuts_17.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_19.txt \
	output/probabilities/30q_41d_approx2cuts_19.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_21.txt \
	output/probabilities/30q_41d_approx2cuts_21.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_23.txt \
	output/probabilities/30q_41d_approx2cuts_23.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_25.txt \
	output/probabilities/30q_41d_approx2cuts_25.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_27.txt \
	output/probabilities/30q_41d_approx2cuts_27.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_29.txt \
	output/probabilities/30q_41d_approx2cuts_29.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_31.txt \
	output/probabilities/30q_41d_approx2cuts_31.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_33.txt \
	output/probabilities/30q_41d_approx2cuts_33.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_35.txt \
	output/probabilities/30q_41d_approx2cuts_35.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_37.txt \
	output/probabilities/30q_41d_approx2cuts_37.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_39.txt \
	output/probabilities/30q_41d_approx2cuts_39.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_40.txt \
	output/probabilities/30q_41d_approx2cuts_40.txt
;;

"approx1cutH")
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_1.txt \
	output/probabilities/30q_41d_approx1cutH_1.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_5.txt \
	output/probabilities/30q_41d_approx1cutH_5.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_7.txt \
	output/probabilities/30q_41d_approx1cutH_7.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_9.txt \
	output/probabilities/30q_41d_approx1cutH_9.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_11.txt \
	output/probabilities/30q_41d_approx1cutH_11.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_13.txt \
	output/probabilities/30q_41d_approx1cutH_13.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_15.txt \
	output/probabilities/30q_41d_approx1cutH_15.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_17.txt \
	output/probabilities/30q_41d_approx1cutH_17.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_19.txt \
	output/probabilities/30q_41d_approx1cutH_19.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_21.txt \
	output/probabilities/30q_41d_approx1cutH_21.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_23.txt \
	output/probabilities/30q_41d_approx1cutH_23.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_25.txt \
	output/probabilities/30q_41d_approx1cutH_25.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_27.txt \
	output/probabilities/30q_41d_approx1cutH_27.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_29.txt \
	output/probabilities/30q_41d_approx1cutH_29.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_31.txt \
	output/probabilities/30q_41d_approx1cutH_31.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_33.txt \
	output/probabilities/30q_41d_approx1cutH_33.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_35.txt \
	output/probabilities/30q_41d_approx1cutH_35.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_37.txt \
	output/probabilities/30q_41d_approx1cutH_37.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_39.txt \
	output/probabilities/30q_41d_approx1cutH_39.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_40.txt \
	output/probabilities/30q_41d_approx1cutH_40.txt
;;

"approx1cutV")
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_1.txt \
	output/probabilities/30q_41d_approx1cutV_1.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_5.txt \
	output/probabilities/30q_41d_approx1cutV_5.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_7.txt \
	output/probabilities/30q_41d_approx1cutV_7.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_9.txt \
	output/probabilities/30q_41d_approx1cutV_9.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_11.txt \
	output/probabilities/30q_41d_approx1cutV_11.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_13.txt \
	output/probabilities/30q_41d_approx1cutV_13.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_15.txt \
	output/probabilities/30q_41d_approx1cutV_15.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_17.txt \
	output/probabilities/30q_41d_approx1cutV_17.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_19.txt \
	output/probabilities/30q_41d_approx1cutV_19.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_21.txt \
	output/probabilities/30q_41d_approx1cutV_21.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_23.txt \
	output/probabilities/30q_41d_approx1cutV_23.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_25.txt \
	output/probabilities/30q_41d_approx1cutV_25.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_27.txt \
	output/probabilities/30q_41d_approx1cutV_27.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_29.txt \
	output/probabilities/30q_41d_approx1cutV_29.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_31.txt \
	output/probabilities/30q_41d_approx1cutV_31.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_33.txt \
	output/probabilities/30q_41d_approx1cutV_33.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_35.txt \
	output/probabilities/30q_41d_approx1cutV_35.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_37.txt \
	output/probabilities/30q_41d_approx1cutV_37.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_39.txt \
	output/probabilities/30q_41d_approx1cutV_39.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_40.txt \
	output/probabilities/30q_41d_approx1cutV_40.txt
;;

"approx2011")
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_1.txt \
	output/probabilities/30q_41d_approx2011_1.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_5.txt \
	output/probabilities/30q_41d_approx2011_5.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_7.txt \
	output/probabilities/30q_41d_approx2011_7.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_9.txt \
	output/probabilities/30q_41d_approx2011_9.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_11.txt \
	output/probabilities/30q_41d_approx2011_11.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_13.txt \
	output/probabilities/30q_41d_approx2011_13.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_15.txt \
	output/probabilities/30q_41d_approx2011_15.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_17.txt \
	output/probabilities/30q_41d_approx2011_17.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_19.txt \
	output/probabilities/30q_41d_approx2011_19.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_21.txt \
	output/probabilities/30q_41d_approx2011_21.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_23.txt \
	output/probabilities/30q_41d_approx2011_23.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_25.txt \
	output/probabilities/30q_41d_approx2011_25.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_27.txt \
	output/probabilities/30q_41d_approx2011_27.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_29.txt \
	output/probabilities/30q_41d_approx2011_29.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_31.txt \
	output/probabilities/30q_41d_approx2011_31.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_33.txt \
	output/probabilities/30q_41d_approx2011_33.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_35.txt \
	output/probabilities/30q_41d_approx2011_35.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_37.txt \
	output/probabilities/30q_41d_approx2011_37.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_39.txt \
	output/probabilities/30q_41d_approx2011_39.txt
python3 python_scripts/cosine_similarity.py output/probabilities/30q_41d_exact_40.txt \
	output/probabilities/30q_41d_approx2011_40.txt
;;

esac
