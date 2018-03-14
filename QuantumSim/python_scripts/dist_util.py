import os
from itertools import product
import numpy as np
import tempfile
import time
import re
import shutil
import psutil
import random

def AddPrintOptToCommand(seed, command, idx_file, p_idx, num_idx):
	print_opt = ""
	if int(seed) != -1:
		print_opt += " --idx " + str(seed) + "," + str(num_idx)
		if p_idx != -1:
			print_opt += "+"
	elif idx_file != "":
		print_opt += " --idx " + idx_file
	else:
		print_opt += " --idx " + str(7) + "," + str(num_idx)
		# if int(p_idx) != -1:
		# 	print_opt += "+"

	return print_opt

def BuildDistCommand(command, sim_type, num_threads):
	return command + " --num_threads " + str(num_threads) + " --sim_type " + str(sim_type)
	

def TrialRunEval(report_file, num_CZ, start_time, end_time, app_cz_len = 0, dfs_len = 0):
	rep_time = "" #re.compile(r'(?<=Runtime)\w+')
	cz_path_trunc = 0 #re.compile(r'Truncated CZ path\w+')
	dfs_trunc = 0
	mem_val = 0.0
	short_path = False
	total_cz_len = int(num_CZ) + int(app_cz_len)
	with open(report_file, "r") as file:
		for line in file:
			if "Runtime" in line:
				t_rep_time = re.findall("\d+\.\d+", line)
				if len(rep_time):
					rep_time += t_rep_time[0]
				else:
					rep_time = re.findall("\d+", line)[0]
			if "Truncated CZ path" in line:
				cz_path_trunc = line.split(":")[1].replace(' ','').replace('\n', '')
			if "Truncated DFS" in line:
				dfs_trunc = int(line.split(":")[1].replace(' ','').replace('\n', ''))
			if "exhausted early" in line:
				short_path = True
				num_CZ = str(int(num_CZ) + 1)
			if "State representation size" in line:
				mem_usage = line.split(":")[1].replace('\n', '')
				mem_val = float(mem_usage.split(" ")[1].replace(' ',''))
				unit = mem_usage.split(" ")[2].replace(' ','')
				if unit == "KiB":
					mem_val = mem_val * pow(2,10)
				elif unit == "MiB":
					mem_val = mem_val * pow(2,20)
				elif unit == "GiB":
					mem_val = mem_val * pow(2,30)
 	
	if rep_time == "":
		print ("\033[1m" + "Trial run failed" + "\033[0m")
		exit()
	else:
		if float(rep_time) < 0.0:
			print ("\033[1m" + "Simulation runtime too short for distributed execution" + "\033[0m")
			exit()
		elif float(rep_time) > 3600.0: 
			print ("\033[1m" + "Simulation runtime too long for distributed execution" + "\033[0m")
			exit()

		if int(total_cz_len) - int(cz_path_trunc) < int(total_cz_len):
			app_cz_len = int((int(total_cz_len) - int(cz_path_trunc))/3)
			num_CZ = (int(total_cz_len) - int(cz_path_trunc)) - app_cz_len
			print("\033[1m" + "The input CZ path was truncated to length " + str(num_CZ) + "." + "\033[0m\n")

		if int(dfs_len) - dfs_trunc < int(dfs_len):
			dfs_len = int(dfs_len) - dfs_trunc
			print("\033[1m" + "The DFS length was truncated to " + str(dfs_len) + "." + "\033[0m\n")

		# if float(rep_time) > 2*(end_time - start_time):
		# 	print("\033[1m" + "Simulation runtime unreliable" + "\033[0m")
		# 	exit()

		return int(num_CZ), mem_val, short_path, dfs_len, int(app_cz_len)


def PerformTrialRun(commandH, commandV, num_cz, app_cz_len = 0, dfs_len = 0):
	dirpath = tempfile.mkdtemp()

	# Horizontal trial run
	print ("\033[1m" + "Performing a trial simulation run with a horizontal cut and length " + str(num_cz) + " CZ path " + "\033[0m")
	
	start_timeH = time.time()

	tempH_file = os.path.join(dirpath, "H_phase1_rep.txt")

	cz_p = ""
	for i in range(int(num_cz)):
		cz_p += str(i % 2);

	if dfs_len:
		commandH += " --CZ_path " + str(num_cz) + "," + str(int(cz_p, 2)) + "," + str(app_cz_len) \
		+ "," + str(dfs_len) + " > " + str(tempH_file)
	else:
		commandH += " --CZ_path " + str(num_cz) + "," + str(int(cz_p, 2)) + "," + str(app_cz_len) + " > " + str(tempH_file)

	print(commandH)
	os.system(commandH)
	os.system("cat " + tempH_file)

	end_timeH = time.time()

	# Horizontal trial run evaluation
	print("\033[1m" + "Horizontal trial run took " + str (end_timeH - start_timeH) +  " s \033[0m")
	num_czh, H_mem, short_pathH, dfs_lenH, app_cz_lenH = TrialRunEval(tempH_file, num_cz, \
		start_timeH, end_timeH, app_cz_len, dfs_len)

	# Vertical trial run
	print ("\033[1m" + "Performing a trial simulation run with a vertical cut and length " + str(num_cz) + " CZ path " + "\033[0m")
	
	start_timeV = time.time()

	tempV_file = os.path.join(dirpath, "V_phase1_rep.txt")

	if dfs_len:
		commandV += " --CZ_path " + str(num_cz) + "," + str(int(cz_p, 2)) + "," + str(app_cz_len) + "," + str(dfs_len) + " > " + str(tempV_file)
	else:
		commandV += " --CZ_path " + str(num_cz) + "," + str(int(cz_p, 2)) + "," + str(app_cz_len) +  " > " + str(tempV_file)

	print(commandV)
	os.system(commandV)
	os.system("cat " + tempV_file)

	end_timeV = time.time()

	# Vertical trial run evaluation
	print("\033[1m" + "Vertical trial run took " + str (end_timeV - start_timeV) + " s \033[0m")
	num_czv, V_mem, short_pathV, dfs_lenV, app_cz_lenV = TrialRunEval(tempV_file, num_cz, \
		start_timeV, end_timeV, app_cz_len, dfs_len)


	return num_czh, num_czv, dfs_lenH, dfs_lenV, (end_timeH - start_timeH), \
	(end_timeV - start_timeV), H_mem, V_mem, app_cz_lenH, app_cz_lenV

def EvalMemAndRuntime(t_time, num_cz, mem, num_batches = 1, sim_type = "Simulation"):

	if (float(t_time) * (1 << int(num_cz)))/int(num_batches) > 172800:
		print ("\033[1m" + sim_type + " is expected to take too int for execution\n" + \
			"The expected time of execution for a distributed run is " \
			+ str((float(t_time) * (1 << int(num_cz)))/int(num_batches)) + "s \nThe CZ path is of length "\
			+ str(num_cz) + "\033[0m")
		exit() 
	if float(mem * num_batches) >= psutil.virtual_memory().total:
		print("\033[1m" + sim_type + "  is expected to have a peak memory usage of " + \
			str((mem * num_batches)/1e9) + " GB, which is expected to exceed system memory of " + \
			str(psutil.virtual_memory().total/1e9) + " GB\033[0m")
		exit()

def LaunchDisParallelSim(num_cz, num_batches, dfs_len, cir_dir, cz_bits_strings, \
	command, t_time, num_threads, mem, cut, app_cz_len = 0, truncated = 0):

	# Generating scripts for each parallel run
	print("\033[1m" + "Generating scripts for execution" + "\033[0m\n")
	script_dir = os.path.join("bin", cir_dir)
	if not os.path.isdir(script_dir):
		try:
			os.makedirs(script_dir)
		except OSError as e:
			if e.errno != errno.EEXIST:
				raise
	else:
		shutil.rmtree(script_dir)
		try:
			os.makedirs(script_dir)
		except OSError as e:
			if e.errno != errno.EEXIST:
					raise
	
	num_procs = len(cz_bits_strings) if truncated == 0 else truncated
	proc_per_script = int(num_procs/ num_batches);
	proc_c = 0
	if proc_per_script:
		for i in range(int(num_batches - 1)):
			proc_c += 1
			with open(os.path.join(script_dir, "script_" + str(i) + ".sh"), "w") as script:
				script.write("#!/bin/bash\nset -e\n")
				start_idx = i * proc_per_script
				for j in range(start_idx, start_idx + proc_per_script):
					if j < (start_idx + proc_per_script - 1):
						script.write(command + cz_bits_strings[j] + "--outfile output_" + str(proc_c) + "\n")
					else:
						script.write(command + cz_bits_strings[j] + "--outfile output_" + str(proc_c) + ",a\n")
	else:
		proc_per_script = 1
	with open(os.path.join(script_dir, "script_" + str(proc_c) + ".sh"), "w") as script:
			script.write("#!/bin/bash\nset -e\n")
			start_idx = proc_c * proc_per_script
			proc_c += 1
			for j in range(start_idx, num_procs):
				if j < (num_procs - 1):
					script.write(command + cz_bits_strings[j] + "--outfile output_" + str(proc_c) + "\n")
				else:
					script.write(command + cz_bits_strings[j] + "--outfile output_" + str(proc_c) + ",a\n")


	os.system("chmod +x " + script_dir + "/*")

	est_time =  round(float(t_time) * int(num_procs) / int(num_batches), 3)
	print ("\033[1m" + "Launching " +  str(num_procs) + " " + cut + " simulations with " +\
		str(num_batches) + " parallel processes and with upto " + str(num_threads) + \
		" threads each, that are estimated to take " + str(float(est_time) + (0.4 * float(est_time))) + " +- " \
	 + str(round(0.3 * float(est_time), 3)) + \
		" s in a distributed run.\nThe peak memory usage is expected to be " + \
		 str(mem * num_batches) + " B\nThe CZ path length is " + str(num_cz) + "\033[0m")

	# Create the log directory if it doesn't already exist
	log_dir = os.path.join("output", "log")
	if not os.path.isdir(log_dir):
		try:
			os.makedirs("output/log")
		except OSError as e:
			if e.errno != errno.EEXIST:
				raise

	if os.path.isdir(log_dir):
		log_dir = os.path.join(log_dir, cir_dir)
		if not os.path.isdir(log_dir):
			try:
				os.makedirs(log_dir)
			except OSError as e:
				if e.errno != errno.EEXIST:
					raise
		else:
			shutil.rmtree(log_dir)
			try:
				os.makedirs(log_dir)
			except OSError as e:
				if e.errno != errno.EEXIST:
					raise

	# Launch the scripts and print the logs generated by the processes in each script into a file 
	# in the log directory
	for p in range(num_batches):
		print("/usr/bin/time ./" + os.path.join(script_dir, "script_" + str(p) + ".sh") +\
		  " > " + os.path.join(log_dir, "log_script_" + str(p))+ ".txt 2>&1 &")
		os.system("/usr/bin/time ./" + os.path.join(script_dir, "script_" + str(p) + ".sh") + \
		  " > " + os.path.join(log_dir, "log_script_" + str(p)) + ".txt 2>&1 &")

