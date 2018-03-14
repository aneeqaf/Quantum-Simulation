#!/usr/bin/env python3
"""Script for distributed execution"""

import os
import click
import tempfile
import time
import re
import shutil
import datetime
import dist_util
import errno
import psutil
from multiprocessing import cpu_count
import numpy as np

@click.command()
@click.argument("num_cz", nargs=1)
@click.argument("num_idx", nargs=1)
@click.option("--dfs_len", nargs=1, required=False, default=0)
@click.option("--command", nargs=1, required=True)
@click.option("--seed", nargs=1, required=False, default=-1)
@click.option("--idx_file", nargs=1, required=False, default="")
@click.option("--p_idx", nargs=1, required=False, default=-1)
@click.option("--num_procs", nargs=1, required=False, default=1)
@click.option("--num_threads", nargs=1, required=False, default=8)
@click.option("--print_all", nargs=1, required=False, default=-1)
@click.option("--truncated", nargs=1, required=False, default=0)
@click.option("--app_czpath", nargs=1, required=False, default=0)
def main(num_cz, dfs_len, num_idx, command, seed,\
 idx_file, p_idx, num_procs, num_threads, print_all, truncated, app_czpath):

	if num_procs * num_threads > cpu_count():
		print("Requested too many threads. There are " + str(cpu_count()) + " hardware threads.")
		exit()

	# If the entire state vector needs to be printed, specify this command.
	# The value is the number of qubits in the circuit
	if int(print_all) != -1:
		with open(idx_file, "w") as f:
			for q in range(1 << int(print_all)):
				f.write(str(q) + "\n")

	# Do trial run to choose better cut
	d_idx = [pos for pos, char in enumerate(command) if char in "-d"][-1]
	file_name = re.findall(r'\binst\w+', command)
	cir_name = "test"

	if len(file_name):
		cir_name = re.findall(r'\binst\w+', command)[0] + "_" \
		+ command[d_idx + 2 :] + "_"
	# os.makedirs(cir_dir)

	commandH = dist_util.BuildDistCommand(command, 0, num_threads) 
	commandV = dist_util.BuildDistCommand(command, 1, num_threads) 
		
	num_czh, num_czv, dfs_lenH, dfs_lenV, H_time, V_time, H_mem, V_mem, app_cz_lenH, app_cz_lenV= \
	dist_util.PerformTrialRun(commandH, commandV, num_cz, app_czpath, dfs_len)

	num_cz = 0
	t_time = 0
	cut = "" 
	command = ""
	mem = 0
	dfs_len = 0

	total_czv = num_czv + app_cz_lenV
	total_czh = num_czh + app_cz_lenH
	
	if total_czv == total_czh:
		num_cz = num_czh if (float(H_time) <= float(V_time)) else num_czv
		app_czpath = app_cz_lenH if (float(H_time) <= float(V_time)) else app_cz_lenV
		t_time = H_time if (float(H_time) <= float(V_time)) else V_time
		cut = "horizontal-cut" if (float(H_time) <= float(V_time)) else "vertical-cut"
		command = commandH if (float(H_time) <= float(V_time)) else commandV
		mem = H_mem if (float(H_time) <= float(V_time)) else V_mem
		dfs_len = dfs_lenH if (float(H_time) <= float(V_time)) else dfs_lenV
	else:
		num_cz = num_czh if (total_czv >= total_czh) else num_czv
		app_czpath = app_cz_lenH if (total_czv >= total_czh) else app_cz_lenV
		t_time = H_time if (total_czv >= total_czh) else V_time
		cut = "horizontal-cut" if (total_czv >= total_czh) else "vertical-cut"
		command = commandH if (total_czv >= total_czh) else commandV
		mem = H_mem if (total_czv >= total_czh) else V_mem
		dfs_len = dfs_lenH if (total_czv >= total_czh) else dfs_lenV

	cir_name += str(num_cz + app_czpath) + "_" + str(num_threads)
	cir_dir = os.path.join("output", "amp_vectors", cir_name)
	
	if os.path.isdir(cir_dir):
		shutil.rmtree(cir_dir, ignore_errors=True)
	
	dist_util.EvalMemAndRuntime(t_time, num_cz, mem, num_procs)

	cz_bits_strings = []
	for bit_comb in range(0, (1 << int(num_cz))):
		if dfs_len:
			cz_bits_strings.append(str(num_cz) + "," + str(bit_comb) + "," + str(app_czpath) 
				+ "," + str(dfs_len) + " ")
		else:
			cz_bits_strings.append(str(num_cz) + "," + str(bit_comb) + "," + str(app_czpath) + " ")

	command += dist_util.AddPrintOptToCommand(seed, command, idx_file, p_idx, num_idx) + " --CZ_path "
	num_procs = 1 if len(cz_bits_strings) == 1 else num_procs

	dist_util.LaunchDisParallelSim(num_cz, num_procs, dfs_len, cir_name, cz_bits_strings, \
		command, t_time, num_threads, mem, cut, app_czpath, truncated)

	# Sleep 5 * time taken in trial run. Check if the logs are changing in size. Once the logs
	# stop changing, verify they completed without errors, then launch reporting script.
	proc_per_script = int(len(cz_bits_strings)/num_procs) if len(cz_bits_strings) > 1 else 1
	log_dir = os.path.join("output", "log", cir_name)
	changing = True
	logs_prev_mem = np.zeros(num_procs)
	log_files = []
	for i in range(num_procs):
		log_files.append(os.path.join(log_dir, "log_script_" + str(i) + ".txt"))

	os.system("chmod +x python_scripts/add_amps.py")
	while changing:
		time.sleep(5 * t_time) # Put script to sleep for substantial changes to take place
		any_log_changed = False
		for i, lf in enumerate(log_files):
			if os.stat(lf).st_size > logs_prev_mem[i]:
				any_log_changed = True
				logs_prev_mem[i] = os.stat(lf).st_size
				# os.system("./python_scripts/add_amps.py " + cir_name + " " + num_idx)

		if not any_log_changed:
			changing = False

	# Check logs for completion
	if truncated == 0 : 
		for i in range(int(num_procs - 1)):
			with open(log_files[i], "r") as lf:
				log_content = lf.read()
				count_runtimes = sum(1 for _ in re.finditer(r'\b%s\b' % re.escape("Runtime"), log_content))
				if count_runtimes != proc_per_script:
					print ("The simulations in script_" + str(i) + " did not complete")
					exit()
			
		with open(log_files[num_procs - 1], "r") as lf:
			log_content = lf.read()
			proc_per_script = len(cz_bits_strings) - ((num_procs - 1) * proc_per_script)
			count_runtimes = sum(1 for _ in re.finditer(r'\b%s\b' % re.escape("Runtime"), log_content))
			if count_runtimes != proc_per_script:
				print ("The simulations in script_" + str(num_procs - 1) + " did not complete")
				exit()

	per_proc = (1 << int(num_cz)) if truncated == 0 else truncated
	est_time = str(round(float(t_time) * int(per_proc)/int(num_procs), 3))
	os.system("chmod +x python_scripts/dist_sim_report_gen.py")
	os.system("./python_scripts/dist_sim_report_gen.py " + cir_name + " " + est_time\
	 + " --truncated " + str(truncated))
	os.system("./python_scripts/add_amps.py " + cir_name + " " + num_idx)

if __name__ == "__main__":
    main()		
