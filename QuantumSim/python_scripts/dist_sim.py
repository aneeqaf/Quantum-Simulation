#!/usr/bin/env python3
"""Script for distributed execution"""

import os
import click
import tempfile
import time
import re
import shutil
import time
import dist_util
import errno
import psutil
from multiprocessing import cpu_count
import numpy as np
import random
from math import sqrt, floor, ceil

@click.command()
@click.argument("circuit", nargs=1, required=True)
@click.option("--depth", nargs=1, required=False, default=0)
@click.option("--v_cut", nargs=1, required=False, default=0)
@click.option("--h_cut", nargs=1, required=False, default=0)
@click.option("--proc_prefix_bits", required=False, nargs=1, default=0)
@click.option("--ranges_bits", nargs=1, required=False, default=0)
@click.option("--branch_bits", nargs=1, required=False, default=0)
@click.option("--column_major", nargs=1, required=False, is_flag=True)
@click.option("--num_batches", nargs=1, required=False, default=0)
@click.option("--num_threads", nargs=1, required=False, default=4)
@click.option("--num_highq", nargs=1, required=False, default=0)
@click.option("--max_procs", nargs=1, required=False, default=0)
@click.option("--idx_seed", nargs=1, required=False, default=7)
@click.option("--idx_file", nargs=1, required=False, default="")
@click.option("--num_idx", nargs=1, required=False, default=1000)
@click.option("--print_idxs", nargs=1, required=False, default=-1)
@click.option("--print_all", nargs=1, required=False, default=-1)
@click.option("--trial", nargs=1, required=False, is_flag=True)
@click.option("--approx", nargs=1, required=False, default=0)
@click.option("--test_fid", nargs=1, required=False, is_flag=True)
@click.option("--multiple_nodes", nargs=1, required=False, is_flag=True)
@click.option("--cont_cz_paths", nargs=1, required=False, is_flag=True)
@click.option("--no_nearest_neighbors", nargs=1, required=False, is_flag=True)
@click.option("--layers_hgates_b4_meas", nargs=1, required=False, default=0)
@click.option("--no_checkpoint_with_ranges", nargs=1, required=False, is_flag=True)
@click.option("--binary_vectors_only", nargs=1, required=False, is_flag=True)
def main(circuit, depth, proc_prefix_bits, branch_bits, num_idx, idx_seed, num_highq, v_cut, h_cut,\
 idx_file, print_idxs, num_batches, num_threads, print_all, max_procs, ranges_bits, trial, \
 approx, test_fid, multiple_nodes, cont_cz_paths, column_major, no_nearest_neighbors,
 layers_hgates_b4_meas, no_checkpoint_with_ranges, binary_vectors_only):

	dist_util.CheckInputFile(circuit)

	epsilon = 1
	max_threads = cpu_count()
	binary = "./bin/rr "
	command = binary + "-i " + circuit 

	if not num_batches:
		num_batches = int(max_threads/num_threads);

	if not multiple_nodes and num_batches * num_threads > cpu_count():
		print("\033[1m Requested too many threads. There are " + str(cpu_count()) + " hardware threads.\033[0m")
		exit()

	# If the entire state vector needs to be printed, specify this command.
	# The value is the number of qubits in the circuit
	if int(print_all) != -1:
		with open(idx_file, "w") as f:
			for q in range(1 << int(print_all)):
				f.write(str(q) + "\n")

	# cir_name = circuit + "_" + str(depth) + "_"
	# os.makedirs(cir_dir)

	commandH = dist_util.BuildDistCommand(command, 0, num_threads, num_highq, approx, column_major,
	depth, no_nearest_neighbors, layers_hgates_b4_meas, no_checkpoint_with_ranges, h_cut = h_cut) 
	commandV = dist_util.BuildDistCommand(command, 1, num_threads, num_highq, approx, column_major,
	depth, no_nearest_neighbors, layers_hgates_b4_meas, no_checkpoint_with_ranges, v_cut = v_cut) 
		
	proc_prefix_bits, branch_bits, t_time, mem, ranges_bits, cut, command, depth = \
	dist_util.PerformTrialRun(commandH, commandV, proc_prefix_bits, 
		ranges_bits, branch_bits, trial, v_cut, h_cut, approx)

	num_bit_strings = (1 << int(proc_prefix_bits)) 

	if t_time > 100 and max_procs:
		max_procs = num_batches

	cir_name = circuit + "_" + str(depth) + "_" + \
	str(proc_prefix_bits + ranges_bits) + "_" + str(num_threads)

	if approx:
		fid = approx
		if not cont_cz_paths:
			epsilon = approx
		if cont_cz_paths:
			num_bit_strings = ceil(num_bit_strings / approx)

		cir_name += "_approx_" + str(approx)

	if int(max_procs)/int(num_batches) > ((1 << int(proc_prefix_bits))/int(num_batches)):
		print("Max processes exceed total number of processes. Setting to default.\033[0m./")
		max_procs = 0
		
	cir_dir = os.path.join("output", "amp_vectors", cir_name)
	if os.path.isdir(cir_dir):
		shutil.rmtree(cir_dir, ignore_errors=True)
	
	# estimate runtime and peak memory usage before proceeding
	dist_util.EvalMemAndRuntime(t_time, proc_prefix_bits, mem, num_batches)

	cz_bits_strings = []
	for bit_comb in range(0, num_bit_strings, epsilon):
		if branch_bits:
			cz_bits_strings.append(str(proc_prefix_bits) + "," + str(bit_comb) + "," + str(ranges_bits) 
				+ "," + str(branch_bits) + " ")
		elif ranges_bits:
			cz_bits_strings.append(str(proc_prefix_bits) + "," + str(bit_comb) + "," + str(ranges_bits) + " ")
		else:
			cz_bits_strings.append(str(proc_prefix_bits) + "," + str(bit_comb) + " ")

	if max_procs or (max_procs and approx and max_procs < approx):
		random.shuffle(cz_bits_strings)
		cz_bits_strings = cz_bits_strings[:max_procs]
		num_bit_strings = len(cz_bits_strings)

	command += dist_util.AddPrintOptToCommand(idx_seed, command, idx_file, print_idxs, num_idx)
	command += " --CZ_path "

	num_batches = len(cz_bits_strings) if len(cz_bits_strings) < num_batches else num_batches

	num_batches = dist_util.LaunchDisParallelSim(proc_prefix_bits, num_batches, branch_bits, cir_name, \
	 cz_bits_strings, command, t_time, num_threads, mem, cut, ranges_bits, max_procs, approx, \
	 multiple_nodes, binary_vectors_only)

	if test_fid:
		if len(cz_bits_strings) > 2:
			temp_dir =  tempfile.mkdtemp()
			os.system(command + cz_bits_strings[0] + "--outfile test_fid_1@ > " + os.path.join(temp_dir, "t1.txt"))
			os.system(command + cz_bits_strings[-1] + "--outfile test_fid_2@ > " + os.path.join(temp_dir, "t2.txt"))

	# Launch post_launch.py if a trial run has been performed
	# otherwise just print the command line to be used for post_launch.py
	# post_launch.py is responsible for checking whether all the batches have been completed, adding
	# up the amps from the batches and generating the report.
	print("\033[1m" + "\nPlease use the following command after all the processes have ended to " + \
		"check whether all the simulations have successfully completed; add all the amplitudes vectors; and " + \
		"produce the multiprocess simulation report." + "\033[0m")

	if multiple_nodes:
		print("\033[1m" + "Please add the batch ranges that are executed on a node to the post_launch.py command" +\
		 "\033[0m")

	num_procs = len(cz_bits_strings) if not max_procs else max_procs 
	log_dir = os.path.join("output", "log", cir_name)
	post_launch_cmd = "./python_scripts/post_launch.py " + str(cir_name) + " --num_procs " + str(num_procs) + \
		" --num_batches " + str(num_batches) + " --t_time " + str(t_time) + \
		" --num_idx " + str(num_idx) + " --max_procs " + str(max_procs)
	if test_fid:
		post_launch_cmd += " --test_fid"
	if no_checkpoint_with_ranges:
		post_launch_cmd += " --no_checkpoint_with_ranges"
	if binary_vectors_only:
		post_launch_cmd += " --binary_vectors_only"
	if multiple_nodes:
		post_launch_cmd += " --not_final_amps"
		post_launch_cmd += "\033[1m --batch_range <inclusive start, exclusive end> \033[0m"

	post_launch_cmd += " > " + str(log_dir) + "/final_report" + str(time.clock()) + ".rep 2>&1 &"
	print(post_launch_cmd)
	if trial:
		os.system(post_launch_cmd)

	if multiple_nodes:
		print("\033[1m" + "\nPlease use the following command once your multinode simulation has completed." +\
		" The script will add all the partial resultant amplitudes and generate the final report " +\
		"from the intermediate multiprocess simulations reports. Make sure that you transfer all files " + \
		"to their designated directories " + \
		"(output/(amp_vectors or log)/<circuit_filename>_<depth>_<prefix_bits>_<num_threads>_approx_<eps>), " +\
		"all intermediate multiprocess simulations reports should end "  +\
		"in '.rep' and all partial resultant amplitude vector files should have 'result' in ther filenames." + \
		" Apart from the transfer, the post_launch.py command above should take care of the naming conventions required.\n" +\
		"\033[0m")

		post_multinode_sim_cmd = "./python_scripts/post_multinode_sim.py "  + str(cir_name) +\
		 " --num_idx " + str(num_idx) + " --max_procs " + str(max_procs)
		if test_fid:
			post_multinode_sim_cmd += " --test_fid"
	
		print(post_multinode_sim_cmd + "\n\n")


if __name__ == "__main__":
	dist_util.EnvCompatibility()
	main()		
