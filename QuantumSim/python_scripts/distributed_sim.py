#!/usr/bin/env python3
"""Script for distributed execution"""

import os
import click
from itertools import product
import numpy as np
import tempfile
import time
import re
import shutil

max_th = 10

def add_print_opt(seed, command, idx_file, p_idx, num_idx):
	if int(seed) != -1:
		command += " -x " + str(seed) + "," + str(num_idx)
		if p_idx != -1:
			command += "+"
	elif idx_file != "":
		command += " -x " + idx_file
	else:
		command += " -x " + 7 + "," + str(num_idx)
		if int(p_idx) != -1:
			command += "+"

	return command


@click.command()
@click.argument("num_czh", nargs=1)
@click.argument("num_czv", nargs=1)
@click.argument("num_idx", nargs=1)
@click.option("--command", nargs=1, required=True)
@click.option("--print_all", nargs=1, required=False, default=-1)
@click.option("--seed", nargs=1, required=False, default=-1)
@click.option("--idx_file", nargs=1, required=False, default="")
@click.option("--p_idx", nargs=1, required=False, default=-1)
def main(num_czh, num_czv, num_idx, command, print_all, seed, idx_file, p_idx):
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

	num_czv = int(num_czv)
	num_czh = int(num_czh)
	dirpath = tempfile.mkdtemp()

	print ("\033[1m" + "Running exact full state-vector simulation " + "\033[0m")
	# Exact full state-vector sim
	full_amp_dir = "test_full_" + str(os.getpid())
	full_sim_command = command + " -t " + max_th +  " -s 5 -o " + full_amp_dir;
	full_sim_command = add_print_opt(seed, full_sim_command, idx_file, p_idx, num_idx)
	print(full_sim_command)
	os.system(full_sim_command)

	# If the entire state vector needs to be printed, specify this command.
	# The value is the number of qubits in the circuit
	if int(print_all) != -1:
		with open(os.path.join(dirpath, idx_file), "w") as f:
			for q in range(1 << int(print_all)):
				f.write(str(q) + "\n")

	# Form the commands to be executed
	cz_bits_stringsH = []
	for bit_comb in range((1 << int(num_czh))):
		cz_bits_stringsH.append(str(num_czh) + "," + str(bit_comb))

	cz_bits_stringsV = []
	for bit_comb in range((1 << int(num_czv))):
		cz_bits_stringsV.append(str(num_czv) + "," + str(bit_comb))

	tempH_dir = "test_H" + str(os.getpid())
	outdirH = "output/amp_vectors/" + tempH_dir
	commandH = command + " -t " + max_th + " -s 0 -o " + tempH_dir
	commandH = add_print_opt(seed, commandH, idx_file, p_idx, num_idx) + " -c "
	
	tempV_dir = "test_V" + str(os.getpid())
	outdirV = "output/amp_vectors/" + tempV_dir;
	commandV = command + " -t " + max_th + " -s 1 -o " + tempV_dir
	commandV = add_print_opt(seed, commandV, idx_file, p_idx, num_idx) + " -c "
	
	print ("\033[1m" + "Performing a trial simulation run with a horizontal cut and length " + str(num_czh) + " CZ path " + "\033[0m")
	# Horizontal Trial run
	start_time = time.time()

	tempH_file = os.path.join(dirpath, "H_phase1_rep.txt")
	print(commandH + cz_bits_stringsH[0])
	os.system(commandH + cz_bits_stringsH[0] + " > " + str(tempH_file))
	os.system("cat " + tempH_file)
	shutil.rmtree(outdirH)

	end_time = time.time()

	# Horizontal trial run evaluation
	rep_time = "" #re.compile(r'(?<=Runtime)\w+')
	cz_path_trunc = "" #re.compile(r'Truncated CZ path\w+')
	with open(tempH_file, "r") as file:
		for line in file:
			if "Runtime" in line:
				rep_time = re.findall("\d+\.\d+", line)[0]
			if "Truncated CZ path" in line:
				cz_path_trunc = line.split(":")[1].replace(' ','').replace('\n', '')
	
	if rep_time == "":
		print ("\033[1m" + "Horizontal trial run failed" + "\033[0m")
		exit()
	else:
		if float(rep_time) < 0.0:
			print ("\033[1m" + "Horizontal simulation runtime too short for distributed execution" + "\033[0m")
			exit()
		elif float(rep_time) > 3600.0: 
			print ("\033[1m" + "Horizontal simulation runtime too long for distributed execution" + "\033[0m")
			exit()

		if num_czh - len(cz_path_trunc) <= num_czh:
			num_czh = num_czh - len(cz_path_trunc)
			cz_bits_stringsH = []
			for bit_comb in range((1 << int(num_czh))):
				cz_bits_stringsH.append(str(num_czh) + "," + str(bit_comb))
			print("\033[1m" + "The input CZ path for a horizontal cut was truncated to length " + str(num_czh) + "." + "\033[0m")

		if float(rep_time) > 2*(end_time - start_time):
			print("\033[1m" + "Simulation runtime unreliable" + "\033[0m")
			exit()

		if float(end_time - start_time) * (1 << num_czh) > 172800:
			print ("\033[1m" + "Horizontal simulation is expected to take too long for execution" + "\033[0m")
			exit()

	print ("\033[1m" + "Launching " + str(len(cz_bits_stringsH)) + " horizontal-cut simulations with upto " + max_th + \
		" threads each, that are estimated to take " \
		+ str(round(float(end_time - start_time) * (1 << num_czh), 3)) + " s\nThe CZ path length is " + \
		str(num_czh) + "\033[0m")
	# Horizontal simulation launch
	for cz_bits_str in cz_bits_stringsH:
		print(commandH + cz_bits_str)
		os.system(commandH + cz_bits_str + " -v 1")

	res_ampsH = []
	for i in range(int(num_idx)):
		res_ampsH.append(0);

	print("\033[1m" + "Checking for accuracy of horizontal simulation " + "\033[0m")
	# Load the horizontal sim output from files, process it and check for accuracy
	for filename in os.listdir(outdirH):
		if filename.endswith(".amps"):
			with open(os.path.join(outdirH, filename), "r") as f:
				lines = f.readlines()
				temp = np.loadtxt(lines, dtype=complex)
			for i, amp in enumerate(temp):
				res_ampsH[i] += amp;

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

	print(res_ampsH)
	if not failH:
		print("\033[1m" + "H test passed!" + "\033[0m")
	else: 
		print("\033[1m" + "H test failed!" + "\033[0m")

	print()
	print ("\033[1m" + "Performing a trial simulation run with a vertical cut and length " + str(num_czv) + " CZ path " + "\033[0m")
	# Vertical trial run
	start_time = time.time()

	tempV_file = os.path.join(dirpath, "V_phase1_rep.txt")
	print(commandV + cz_bits_stringsV[0])
	os.system(commandV + cz_bits_stringsV[0] + " > " + str(tempV_file))
	os.system("cat " + tempV_file)
	shutil.rmtree(outdirV)
	
	end_time = time.time()

	# Vertical trial run evaluation
	rep_time = "" #re.compile(r'(?<=Runtime)\w+')
	cz_path_trunc = "" #re.compile(r'Truncated CZ path\w+')
	with open(tempV_file, "r") as file:
		for line in file:
			if "Runtime" in line:
				rep_time = re.findall("\d+\.\d+", line)[0]
			if "Truncated CZ path" in line:
				cz_path_trunc = line.split(":")[1].replace(' ','').replace('\n', '')
	
	if rep_time == "":
		print ("\033[1m" + "Vertical trial run failed" + "\033[0m")
		exit()
	else:
		if float(rep_time) < 0.0:
			print ("\033[1m" + "Vertical simulation runtime too short for distributed execution" + "\033[0m")
			exit()
		elif float(rep_time) > 3600.0: 
			print ("\033[1m" + "Vertical simulation runtime too long for distributed execution" + "\033[0m")
			exit()

		if num_czv - len(cz_path_trunc) <= num_czv:
			num_czv = num_czv - len(cz_path_trunc)
			cz_bits_stringsV = []
			for bit_comb in range((1 << int(num_czv))):
				cz_bits_stringsV.append(str(num_czv) + "," + str(bit_comb))
			print("\033[1m" + "The input CZ path for a vertical cut was truncated to length " + str(num_czv) + "\033[0m")


		if float(rep_time) > 2*(end_time - start_time):
			print("\033[1m" + "Simulation runtime unreliable" + "\033[0m")
			exit()
			
		if float(rep_time) * (1 << num_czv) > 172800:
			print ("\033[1m" + "Vertical simulation is expected to take too long for execution" + "\033[0m")
			exit()

	print ("\033[1m" + "Launching " + str(len(cz_bits_stringsV)) + " vertical-cut simulations with upto " + max_th + \
		" threads each, that are estimated to take " \
	 + str(round(float(end_time - start_time) * (1 << num_czv), 3)) + "s\nThe CZ path length is " + \
		str(num_czv) + "\033[0m")
	# Vertical simulation launch
	for cz_bits_str in cz_bits_stringsV:
		print(commandV + cz_bits_str)
		os.system(commandV + cz_bits_str + " -v 1")

	res_ampsV = []
	for i in range(int(num_idx)):
		res_ampsV.append(0);

	print("\033[1m" + "Checking for accuracy of vertical simulation " + "\033[0m")
	for filename in os.listdir(outdirV):
		if filename.endswith(".amps"):
			with open(os.path.join(outdirV, filename), "r") as f:
				lines = f.readlines()
				temp = np.loadtxt(lines, dtype=complex)
			for i, amp in enumerate(temp):
				res_ampsV[i] += amp;

	failV = False
	for filename in os.listdir(full_outdir):
		if filename.endswith(".amps"):
			with open(os.path.join(full_outdir, filename), "r") as f:
				lines = f.readlines()
				temp = np.loadtxt(lines, dtype=complex)
			for i, amp in enumerate(temp):
				if abs(res_ampsV[i] - amp) > 1.0e-4:
					failV = True

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
