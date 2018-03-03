#!/usr/bin/env python3
"""Script for distributed execution - demo"""

import os
import click
from itertools import product
import numpy as np
import tempfile
import time
import re
import shutil
import datetime
import dist_util

@click.command()
@click.argument("num_cz", nargs=1)
@click.argument("num_idx", nargs=1)
@click.option("--command", nargs=1, required=True)
@click.option("--print_all", nargs=1, required=False, default=-1)
@click.option("--seed", nargs=1, required=False, default=-1)
@click.option("--idx_file", nargs=1, required=False, default="")
@click.option("--p_idx", nargs=1, required=False, default=-1)
@click.option("--num_threads", nargs=1, required=False, default=8)
def main(num_cz, num_idx, command, print_all, seed, idx_file, p_idx, num_threads):
	"""
	This script illustrates how to configure simulator parameters for 
	distributed simulation, launch multiple simulator runs, and then collect results. 
	The script first launches a trial run and looks at runtime. If runtime is <1s or if it 
	cannot find runtime in the simulator output, it quits and complains (in case the run 
	failed for some reason, such as bad input or an internal error). The same happens if the runtime 
	is too large. Otherwise, the script can estimate how many CPU hours will be used by the entire 
	simulation.
	The script, also, prevents redundant simulation runs by adjusting the input CZ path length 
	after performing a trial run of vertical and horizontal cut simulation. 
	"""
	print ("\033[1m" + "This script illustrates how to configure simulator parameters for distributed simulation, launch multiple simulator runs, and then collect results. The script first launches a trial run and looks at runtime. If runtime is <1s or if it cannot find runtime in the simulator output, it quits and complains (in case the run failed for some reason, such as bad input or an internal error). The same happens if the runtime is too large. Otherwise, the script can estimate how many CPU hours will be used by the entire simulation. The script also prevents redundant simulation runs by adjusting the input CZ path length after performing a trial simulation runs with a vertical and horizontal cut, each.\n" + "\033[0m")

	dirpath = tempfile.mkdtemp()

	print ("\033[1m" + "Running exact full state-vector simulation " + "\033[0m")
	# Exact full state-vector sim
	full_amp_dir = "test_full_" + str(os.getpid()) + str(datetime.datetime.now().time())
	full_sim_command = dist_util.BuildDistCommand(command, 5, num_threads, full_amp_dir) + \
	dist_util.AddPrintOptToCommand(seed, command, idx_file, p_idx, num_idx)
	print(full_sim_command)
	os.system(full_sim_command)

	# If the entire state vector needs to be printed, specify this command.
	# The value is the number of qubits in the circuit
	if int(print_all) != -1:
		with open(os.path.join(dirpath, idx_file), "w") as f:
			for q in range(1 << int(print_all)):
				f.write(str(q) + "\n")

	tempH_dir = "test_H" + str(os.getpid()) + str(datetime.datetime.now().time())
	outdirH = "output/amp_vectors/" + tempH_dir
	commandH = dist_util.BuildDistCommand(command, 0, num_threads, tempH_dir) \
	+ dist_util.AddPrintOptToCommand(seed, command, idx_file, p_idx, num_idx) + " --CZ_path "
	
	tempV_dir = "test_V" + str(os.getpid()) + str(datetime.datetime.now().time())
	outdirV = "output/amp_vectors/" + tempV_dir;
	commandV = dist_util.BuildDistCommand(command, 1, num_threads, tempV_dir) \
	+ dist_util.AddPrintOptToCommand(seed, command, idx_file, p_idx, num_idx) + " --CZ_path "
		
	num_czh, num_czv, H_time, V_time, H_mem, V_mem = dist_util.PerformTrialRun(commandH, commandV, num_cz)
	shutil.rmtree(outdirH)
	shutil.rmtree(outdirV)

	dist_util.EvalMemAndRuntime(H_time, num_czh, H_mem, sim_type="Horizontal simulation")
	dist_util.EvalMemAndRuntime(V_time, num_czv, V_mem, sim_type="Vertical simulation")

	# Form the commands to be executed
	cz_bits_stringsH = []
	for bit_comb in range((1 << int(num_czh))):
		cz_bits_stringsH.append(str(num_czh) + "," + str(bit_comb))

	cz_bits_stringsV = []
	for bit_comb in range((1 << int(num_czv))):
		cz_bits_stringsV.append(str(num_czv) + "," + str(bit_comb))

	# Horizontal simulation launch
	print ("\033[1m" + "Launching " + str(len(cz_bits_stringsH)) + " horizontal-cut simulations with upto " + str(num_threads) + \
		" threads each, that are estimated to take " \
		+ str(round(float(H_time) * (1 << int(num_czh)), 3)) + " s\nThe CZ path length is " + \
		str(num_czh) + "\033[0m")

	for cz_bits_str in cz_bits_stringsH:
		print(commandH + cz_bits_str)
		os.system(commandH + cz_bits_str )

	res_ampsH = np.zeros(int(num_idx))

	# Vertical simulation launch
	print ("\033[1m" + "Launching " + str(len(cz_bits_stringsV)) + " vertical-cut simulations with upto " + str(num_threads) + \
	" threads each, that are estimated to take " \
 	+ str(round(float(V_time) * (1 << int(num_czv)), 3)) + "s\nThe CZ path length is " + \
	str(num_czv) + "\033[0m")

	for cz_bits_str in cz_bits_stringsV:
		print(commandV + cz_bits_str)
		os.system(commandV + cz_bits_str )

	res_ampsV = np.zeros(int(num_idx))

	# Load the horizontal and vertical sim output from files, process it and check for accuracy
	print("\033[1m" + "Checking for accuracy of horizontal simulation " + "\033[0m")
	for filename in os.listdir(outdirH):
		if filename.endswith(".amps"):
			with open(os.path.join(outdirH, filename), "r") as f:
				lines = f.readlines()
				temp = np.loadtxt(lines, dtype=complex)
			for i, amp in enumerate(temp):
				res_ampsH[i] += amp;

	print("\033[1m" + "Checking for accuracy of vertical simulation " + "\033[0m")
	for filename in os.listdir(outdirV):
		if filename.endswith(".amps"):
			with open(os.path.join(outdirV, filename), "r") as f:
				lines = f.readlines()
				temp = np.loadtxt(lines, dtype=complex)
			for i, amp in enumerate(temp):
				res_ampsV[i] += amp;

	failH = False
	full_outdir = "output/amp_vectors/" + full_amp_dir
	for filename in os.listdir(full_outdir):
		if filename.endswith(".amps"):
			with open(os.path.join(full_outdir, filename), "r") as f:
				lines = f.readlines()
				temp = np.loadtxt(lines, dtype=complex)
			for i, amp in enumerate(temp):
				if abs(res_ampsH[i] - amp) > 1.0e-4:
					failH = True
				if abs(res_ampsV[i] - amp) > 1.0e-4:
					failV = True

	print(res_ampsH)
	if not failH:
		print("\033[1m" + "H test passed!" + "\033[0m")
	else: 
		print("\033[1m" + "H test failed!" + "\033[0m")

	print(res_ampsV)
	if not failV:
		print("\033[1m" + "V test passed!" + "\033[0m")
	else: 
		print("\033[1m" + "V test failed!" + "\033[0m")


if __name__ == "__main__":
    main()

# for bit_comb in product(range(2), repeat=int(num_cz)):
# 	bit_str = ""
# 	for char in bit_comb:
# 		bit_str = bit_str + str(char)
# 	cz_bits_strings.append(bit_str)

# with open("output/amp_vectors/res_output_" + str(os.getpid()) + ".txt", "w") as f:
# 	for amp in res_amps:
# 		f.write(str(amp) + "\n")
