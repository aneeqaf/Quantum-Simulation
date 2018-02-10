#!/bin/bash

set -x
set -e

#First run all the simulations for different depths to produce probabilities 
#and state vector files
case $1 in 
	"simulate")
	./bin/vector_state_sim -h inst_6_5_100_5 -t 14 -d 41 -s 5 -o 30q_41d_exact -v 4 
	./bin/vector_state_sim -h inst_6_5_100_5 -t 14 -d 26 -s 5 -o 30q_26d_exact -v 4 
	./bin/vector_state_sim -h inst_6_5_100_5 -t 14 -d 10 -s 5 -o 30q_10d_exact -v 4
	./bin/vector_state_sim -h inst_6_5_100_5 -t 14 -d 3 -s 5 -o 30q_3d_exact -v 4
	./bin/vector_state_sim -h inst_6_5_100_5 -t 14 -d 41 -s 4 -o 30q_41d_approx2cuts -v 4
	./bin/vector_state_sim -h inst_6_5_100_5 -t 14 -d 26 -s 4 -o 30q_26d_approx2cuts -v 4
	./bin/vector_state_sim -h inst_6_5_100_5 -t 14 -d 10 -s 4 -o 30q_10d_approx2cuts -v 4
	./bin/vector_state_sim -h inst_6_5_100_5 -t 14 -d 3 -s 4 -o 30q_3d_approx2cuts -v 4
	./bin/vector_state_sim -h inst_6_5_100_5 -t 14 -d 41 -s 6 -o 30q_41d_approx2011 -v 4
	./bin/vector_state_sim -h inst_6_5_100_5 -t 14 -d 26 -s 6 -o 30q_26d_approx2011 -v 4
	./bin/vector_state_sim -h inst_6_5_100_5 -t 14 -d 10 -s 6 -o 30q_10d_approx2011 -v 4
	./bin/vector_state_sim -h inst_6_5_100_5 -t 14 -d 3 -s 6 -o 30q_3d_approx2011 -v 4
	./bin/vector_state_sim -h inst_6_5_100_5 -t 14 -d 41 -s 2 -o 30q_41d_approx_ignore -v 4
	./bin/vector_state_sim -h inst_6_5_100_5 -t 14 -d 26 -s 2 -o 30q_26d_approx_ignore -v 4
	./bin/vector_state_sim -h inst_6_5_100_5 -t 14 -d 10 -s 2 -o 30q_10d_approx_ignore -v 4
	./bin/vector_state_sim -h inst_6_5_100_5 -t 14 -d 3 -s 2 -o 30q_3d_approx_ignore -v 4
	;;

	"porterThomas")
	python3 python_scripts/probability_distribution.py 30q_41d 30 output/probabilities/30q_41d_exact_0.txt \
	output/probabilities/30q_41d_approx2cuts_0.txt output/probabilities/30q_41d_approx2011_0.txt\
	output/probabilities/30q_41d_approx_ignore_0.txt
	python3 python_scripts/probability_distribution.py 30q_26d 30 output/probabilities/30q_26d_exact_0.txt \
	output/probabilities/30q_26d_approx2cuts_0.txt output/probabilities/30q_26d_approx2011_0.txt \
	output/probabilities/30q_26d_approx_ignore_0.txt 
	python3 python_scripts/probability_distribution.py 30q_10d 30 output/probabilities/30q_10d_exact_0.txt \
	output/probabilities/30q_10d_approx2cuts_0.txt output/probabilities/30q_10d_approx2011_0.txt  \
	output/probabilities/30q_10d_approx_ignore_0.txt 
	python3 python_scripts/probability_distribution.py 30q_xd 30 output/probabilities/30q_26d_exact_0.txt \
	output/probabilities/30q_10d_exact_0.txt  \
	output/probabilities/30q_41d_exact_0.txt output/probabilities/30q_26d_approx2cuts_0.txt \
	output/probabilities/30q_41d_approx2cuts_0.txt output/probabilities/30q_41d_approx2011_0.txt
	output/probabilities/30q_10d_approx2cuts_0.txt \
	output/probabilities/30q_26d_approx2011_0.txt output/probabilities/30q_10d_approx2011_0.txt  \
	 output/probabilities/30q_26d_approx_ignore_0.txt \
	output/probabilities/30q_10d_approx_ignore_0.txt \
	output/probabilities/30q_41d_approx_ignore_0.txt
	;;

	"cosineSimilarity")
	set +x
	python3 python_scripts/cosine_similarity.py output/amp_vectors/30q_41d_exact_0.txt \
	output/amp_vectors/30q_41d_approx2cuts_0.txt
	python3 python_scripts/cosine_similarity.py output/amp_vectors/30q_41d_exact_0.txt \
	output/amp_vectors/30q_41d_approx2011_0.txt
	python3 python_scripts/cosine_similarity.py output/amp_vectors/30q_41d_exact_0.txt \
	output/amp_vectors/30q_41d_approx_ignore_0.txt
	python3 python_scripts/cosine_similarity.py output/amp_vectors/30q_26d_exact_0.txt \
	output/amp_vectors/30q_26d_approx2cuts_0.txt
	python3 python_scripts/cosine_similarity.py output/amp_vectors/30q_26d_exact_0.txt \
	output/amp_vectors/30q_26d_approx2011_0.txt
	python3 python_scripts/cosine_similarity.py output/amp_vectors/30q_26d_exact_0.txt \
	output/amp_vectors/30q_26d_approx_ignore_0.txt
	python3 python_scripts/cosine_similarity.py output/amp_vectors/30q_10d_exact_0.txt \
	output/amp_vectors/30q_10d_approx2cuts_0.txt
	python3 python_scripts/cosine_similarity.py output/amp_vectors/30q_10d_exact_0.txt \
	output/amp_vectors/30q_10d_approx2011_0.txt
	python3 python_scripts/cosine_similarity.py output/amp_vectors/30q_10d_exact_0.txt \
	output/amp_vectors/30q_10d_approx_ignore_0.txt
	python3 python_scripts/cosine_similarity.py output/amp_vectors/30q_3d_exact_0.txt \
	output/amp_vectors/30q_3d_approx2cuts_0.txt
	python3 python_scripts/cosine_similarity.py output/amp_vectors/30q_3d_exact_0.txt \
	output/amp_vectors/30q_3d_approx2011_0.txt
	python3 python_scripts/cosine_similarity.py output/amp_vectors/30q_3d_exact_0.txt \
	output/amp_vectors/30q_3d_approx_ignore_0.txt
	;;

	"xELoss")
	set +x
	python3 python_scripts/cross_entropy_loss.py 30 output/probabilities/30q_41d_exact_0.txt \
	output/probabilities/30q_41d_approx2cuts_0.txt
	python3 python_scripts/cross_entropy_loss.py 30 output/probabilities/30q_41d_exact_0.txt \
	output/probabilities/30q_41d_approx2011_0.txt
	python3 python_scripts/cross_entropy_loss.py 30 output/probabilities/30q_41d_exact_0.txt \
	output/probabilities/30q_41d_approx_ignore_0.txt
	python3 python_scripts/cross_entropy_loss.py 30 output/probabilities/30q_26d_exact_0.txt \
	output/probabilities/30q_26d_approx2cuts_0.txt
	python3 python_scripts/cross_entropy_loss.py 30 output/probabilities/30q_26d_exact_0.txt \
	output/probabilities/30q_26d_approx2011_0.txt
	python3 python_scripts/cross_entropy_loss.py 30 output/probabilities/30q_26d_exact_0.txt \
	output/probabilities/30q_26d_approx_ignore_0.txt
	python3 python_scripts/cross_entropy_loss.py 30 output/probabilities/30q_10d_exact_0.txt \
	output/probabilities/30q_10d_approx2cuts_0.txt
	python3 python_scripts/cross_entropy_loss.py 30 output/probabilities/30q_10d_exact_0.txt \
	output/probabilities/30q_10d_approx2011_0.txt
	python3 python_scripts/cross_entropy_loss.py 30 output/probabilities/30q_10d_exact_0.txt \
	output/probabilities/30q_10d_approx_ignore_0.txt
	python3 python_scripts/cross_entropy_loss.py 30 output/probabilities/30q_3d_exact_0.txt \
	output/probabilities/30q_3d_approx2cuts_0.txt
	python3 python_scripts/cross_entropy_loss.py 30 output/probabilities/30q_3d_exact_0.txt \
	output/probabilities/30q_3d_approx2011_0.txt
	python3 python_scripts/cross_entropy_loss.py 30 output/probabilities/30q_3d_exact_0.txt \
	output/probabilities/30q_3d_approx_ignore_0.txt
	;;

esac