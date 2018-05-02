import os
from itertools import product
import numpy as np
import tempfile
import time
import datetime
import re
import shutil
import psutil
import random
import pip
from distutils.version import LooseVersion, StrictVersion
from math import ceil, sqrt

def CheckInputFile(input_file):
	if not os.path.isfile(os.path.join("input", "random_circuits_google", input_file)):
		if not os.path.isfile(input_file):
			print("Input file cannot be found")
			exit()

def EnvCompatibility():
	for m in pip.get_installed_distributions():
		if m.project_name == 'click':
			if StrictVersion(m.version) < StrictVersion("6.7"):
				print("Please upgrade your click module version to 6.7 or above.")
				exit()
		if m.project_name == 'psutil':
			if StrictVersion(m.version) < StrictVersion("5.4.2"):
				print("Please upgrade your psutil module version to 5.4.2 or above.")
				exit()
		if m.project_name == 'numpy':
			if StrictVersion(m.version) < StrictVersion("1.14.0"):
				print("Please upgrade your numpy module version to 1.14.0 or above.")
				exit()

	if not os.path.isdir("output"):
		try:
			os.makedirs(script_dir)
		except OSError as e:
			if e.errno != errno.EEXIST:
				raise

	log_dir = os.path.join("output", "log")
	if not os.path.isdir(log_dir):
		try:
			os.makedirs(log_dir)
		except OSError as e:
			if e.errno != errno.EEXIST:
				raise

	amp_dir = os.path.join("output", "amp_vectors")
	if not os.path.isdir(amp_dir):
		try:
			os.makedirs(amp_dir)
		except OSError as e:
			if e.errno != errno.EEXIST:
				raise

def FormatE(n):
    a = '%E' % n
    return a.split('E')[0].rstrip('0').rstrip('.') + 'E' + a.split('E')[1]

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

def BuildDistCommand(command, sim_type, num_threads, num_highq, h_cut = 0, v_cut = 0):
	if h_cut:
		return command + " --num_threads " + str(num_threads) + " --sim_type " \
		+ str(sim_type) + " --hcut " + str(h_cut) + " --high_value_q " + str(num_highq)
	elif v_cut:
		return command + " --num_threads " + str(num_threads) + " --sim_type " \
		+ str(sim_type) + " --vcut " + str(v_cut) + " --high_value_q " + str(num_highq)
	else:
		return command + " --num_threads " + str(num_threads) + " --sim_type " + str(sim_type) \
		+ " --high_value_q " + str(num_highq)
	

def TrialRunEval(report_file, num_CZ, start_time, end_time, app_cz_len = 0, dfs_len = 0):
	rep_time = "" #re.compile(r'(?<=Runtime)\w+')
	cz_path_trunc = 0 #re.compile(r'Truncated CZ path\w+')
	dfs_trunc = 0
	mem_val = 0.0
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

		return int(num_CZ), mem_val, dfs_len, int(app_cz_len)

def CheckxCZGates(report_file):
	num_xCZ = 0
	with open(report_file, "r") as file:
		for line in file:
			if "Cut" in line:
				temp = line.split(":")[1].split("(")[1]
				num_xCZ = int(re.sub('[^0-9]', '', temp)) 
	return num_xCZ

def ChooseSimCutBasedOnNumxCZ(commandH, commandV, num_cz):

	num_cz = 1 if not num_cz else num_cz

	dirpath = tempfile.mkdtemp()

	# Horizontal trial run
	print ("\033[1m" + "\nPerforming a trial simulation run with a horizontal cut and length " \
		+ str(num_cz) + " CZ path, so expect the CZ path to be exhausted early.\n" + \
		"This is to find the best cut with the least number of xCZ gates.\n"  + "\033[0m")
	
	start_timeH = time.time()

	tempH_file = os.path.join(dirpath, "H_phase1_rep.txt")

	cz_p = ""
	for i in range(int(num_cz)):
		cz_p += str(i % 2);
	
	commandH += " --CZ_path " + str(num_cz) + "," + str(int(cz_p, 2)) + " > " + str(tempH_file) + " 2>&1"

	print("/usr/bin/time " + commandH)
	os.system("/usr/bin/time " + commandH)
	os.system("cat " + tempH_file)

	end_timeH = time.time()

	# Horizontal trial run evaluation
	print("\033[1m" + "Horizontal trial run took " + str (round(end_timeH - start_timeH, 3)) +  " s \033[0m")
	num_xCZH = CheckxCZGates(tempH_file) 

	# Vertical trial run
	print ("\033[1m" + "\nPerforming a trial simulation run with a vertical cut and length " \
		+ str(num_cz) + " CZ path, so expect the CZ path to be exhausted early.\n" + \
		"This is to find the best cut with the least number of xCZ gates. " + "\033[0m")
	
	start_timeV = time.time()

	tempV_file = os.path.join(dirpath, "V_phase1_rep.txt")
	
	commandV += " --CZ_path " + str(num_cz) + "," + str(int(cz_p, 2)) + " > " + str(tempV_file) + " 2>&1"

	print("/usr/bin/time " + commandV) 
	os.system("/usr/bin/time " + commandV)
	os.system("cat " + tempV_file)

	end_timeV = time.time()

	# Vertical trial run evaluation
	print("\033[1m" + "Vertical trial run took " + str (round(end_timeV - start_timeV, 3)) + " s \033[0m")
	num_xCZV = CheckxCZGates(tempV_file)

	return num_xCZH, num_xCZV, (end_timeH - start_timeH), (end_timeV - start_timeV)

def PerformTrialRun(commandH, commandV, proc_prefix_bits, ranges_bits = 0, branch_bits = 0, trial=True, \
	v_cut = 0, h_cut = 0, approx=False):

	num_xCZH, num_xCZV, H_time, V_time = ChooseSimCutBasedOnNumxCZ(commandH, commandV, proc_prefix_bits)

	range_specified = False
	if approx:
		if ranges_bits != 0:
			range_specified = True

	cut = ""
	command = ""
	num_xCZ = 0
	if h_cut or v_cut:
		if h_cut:
			cut = "horizontal-cut" 
			command = commandH 
			num_xCZ = num_xCZH 
		elif v_cut:
			cut = "vertical-cut" 
			command = commandV 
			num_xCZ = num_xCZV 
	elif H_time > 1e-1 or V_time > 1e-1:
		cut = "horizontal-cut" if (float(H_time) <= float(V_time)) else "vertical-cut"
		command = commandH if (float(H_time) <= float(V_time)) else commandV
		num_xCZ = num_xCZH if (float(H_time) <= float(V_time)) else num_xCZV
	else:
		cut = "horizontal-cut" if (num_xCZV >= num_xCZH) else "vertical-cut"
		command = commandH if (num_xCZV >= num_xCZH) else commandV
		num_xCZ = num_xCZH if (num_xCZV >= num_xCZH) else num_xCZV

	dirpath = tempfile.mkdtemp()
	command_to_pass = command

	if (proc_prefix_bits and ranges_bits and branch_bits) and \
	(proc_prefix_bits + ranges_bits + branch_bits < num_xCZ) :
		proc_prefix_bits = 0
		ranges_bits = 0
		branch_bits = 0

	if not proc_prefix_bits and not ranges_bits and not branch_bits:
		branch_bits = ceil(num_xCZ / 3)
		proc_prefix_bits = ceil((num_xCZ - branch_bits)/2)
		ranges_bits = num_xCZ - branch_bits - proc_prefix_bits 
	elif approx and not ranges_bits: 
		if proc_prefix_bits and not branch_bits:
			branch_bits = num_xCZ - proc_prefix_bits - ranges_bits
		elif branch_bits and not proc_prefix_bits:
			proc_prefix_bits = num_xCZ - branch_bits - ranges_bits
	elif not ranges_bits and not branch_bits:
		temp_num_xCZ = num_xCZ - max(proc_prefix_bits, 0)
		if temp_num_xCZ:
			branch_bits = ceil(temp_num_xCZ / 2)
			ranges_bits = num_xCZ - branch_bits - proc_prefix_bits
	elif not ranges_bits and not proc_prefix_bits:
		temp_num_xCZ = num_xCZ - max(branch_bits, 0)
		if temp_num_xCZ:
			proc_prefix_bits = ceil(temp_num_xCZ / 2)
			ranges_bits = num_xCZ - branch_bits - proc_prefix_bits
	elif not branch_bits and not proc_prefix_bits:
		temp_num_xCZ = num_xCZ - max(ranges_bits, 0)
		if temp_num_xCZ:
			branch_bits = ceil(temp_num_xCZ / 2)
			proc_prefix_bits = num_xCZ - branch_bits - ranges_bits
	elif not ranges_bits:
		ranges_bits = max(num_xCZ - proc_prefix_bits - branch_bits, 0)
	elif not branch_bits:
		branch_bits = max(num_xCZ - proc_prefix_bits - ranges_bits, 0)
	elif not proc_prefix_bits:
		proc_prefix_bits = max(num_xCZ - branch_bits - ranges_bits, 0)

	trial_time = 0.0
	mem = 0.0
	if trial:
		start_time = time.time()
		print ("\033[1m" + "\nPerforming a complete trial simulation run with a " +  cut + \
		 " and " + str(proc_prefix_bits) + "p + " + str(ranges_bits) +"r + " + str(branch_bits) +"b CZ path " + "\033[0m")

		temp_file = os.path.join(dirpath, "trial.txt")

		cz_p = ""
		for i in range(int(proc_prefix_bits)):
			cz_p += str(i % 2);

		if branch_bits:
			command += " --CZ_path " + str(proc_prefix_bits) + "," + str(int(cz_p, 2)) + "," + str(ranges_bits) \
			+ "," + str(branch_bits) + " > " + str(temp_file) + " 2>&1"
		elif ranges_bits:
			command += " --CZ_path " + str(proc_prefix_bits) + "," + str(int(cz_p, 2)) + "," + str(ranges_bits)\
			 + " > " + str(temp_file) + " 2>&1"
		else:
			command += " --CZ_path " + str(proc_prefix_bits) + "," + str(int(cz_p, 2)) + " > " + str(temp_file) + " 2>&1"

		print("/usr/bin/time " + command)
		os.system("/usr/bin/time " + command)
		os.system("cat " + temp_file)

		end_time = time.time()

		trial_time = end_time - start_time

		# Trial run evaluation
		print("\033[1m" + "Trial run took " + str (round(trial_time, 3)) +  " s \033[0m")
		proc_prefix_bits, mem, branch_bits, ranges_bits = TrialRunEval(temp_file, proc_prefix_bits, \
			start_time, end_time, ranges_bits, branch_bits)

	if approx and not range_specified:
		proc_prefix_bits += ranges_bits
		ranges_bits = 0

	return proc_prefix_bits, branch_bits, trial_time, mem, ranges_bits, cut, command_to_pass 

def EvalMemAndRuntime(t_time, num_cz, mem, num_batches = 1, sim_type = "Simulation"):

	if (float(t_time) * (1 << int(num_cz)))/int(num_batches) > 172800:
		print ("\033[1m" + sim_type + " is expected to take too long for execution\n" + \
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
	command, t_time, num_threads, mem, cut, app_cz_len = 0, truncated = 0, approx = False, \
	multiple_nodes=False):

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

	# if t_time < 100:
	# 	proc_per_script = 1
	# else:
	proc_per_script = ceil(float(num_procs)/ float(num_batches));

	batch_count= 0
	if proc_per_script:
		while (batch_count + 1) * proc_per_script < len(cz_bits_strings): 
			with open(os.path.join(script_dir, "script_" + str(batch_count) + ".sh"), "w") as script:
				script.write("#!/bin/bash\nset -e\nexport OMP_DISPLAY_ENV=true\n\nPROCS=" \
					+ str(proc_per_script) + "\n\n")
				start_idx = batch_count * proc_per_script
				proc_count = 0
				batch_count += 1
				for j in range(start_idx, start_idx + proc_per_script):
					proc_count += 1
					script.write("START_TIME=$SECONDS\n")
					if j < (start_idx + proc_per_script - 1):
						script.write("echo /usr/bin/time " + command + cz_bits_strings[j] + "--outfile output_" + str(batch_count) + "\n")
						script.write("/usr/bin/time " + command + cz_bits_strings[j] + "--outfile output_" + str(batch_count) + "\n")
					else:
						script.write("echo /usr/bin/time " + command + cz_bits_strings[j] + "--outfile output_" + str(batch_count) + "@\n")
						script.write("/usr/bin/time " + command + cz_bits_strings[j] + "--outfile output_" + str(batch_count) + "@\n")
					script.write("ELAPSED_TIME=$((($PROCS - " + str(proc_count) + ")*($SECONDS - $START_TIME)))\n" + \
					"echo \"\n" + str(proc_count)  + " out of " + str(proc_per_script) \
					+ " processes completed.\n$(($ELAPSED_TIME/60)) min $(($ELAPSED_TIME%60)) sec left for "\
						 + str(proc_per_script - proc_count) + " processes to complete\n\"\n\n")
	else:
		proc_per_script = 1

	if batch_count * proc_per_script < len(cz_bits_strings):
		with open(os.path.join(script_dir, "script_" + str(batch_count) + ".sh"), "w") as script:
				start_idx = batch_count* proc_per_script
				script.write("#!/bin/bash\nset -e\nexport OMP_DISPLAY_ENV=true\n\nPROCS=" \
						+ str(num_procs - start_idx) + "\n\n")
				batch_count+= 1
				proc_count = 0
				for j in range(start_idx, num_procs):
					proc_count += 1
					script.write("START_TIME=$SECONDS\n")
					if j < (num_procs - 1):
						script.write("echo /usr/bin/time " + command + cz_bits_strings[j] + "--outfile output_" + str(batch_count) + "\n")
						script.write("/usr/bin/time " + command + cz_bits_strings[j] + "--outfile output_" + str(batch_count) + "\n")
					else:
						script.write("echo /usr/bin/time " + command + cz_bits_strings[j] + "--outfile output_" + str(batch_count) + "@\n")
						script.write("/usr/bin/time " + command + cz_bits_strings[j] + "--outfile output_" + str(batch_count) + "@\n")
					script.write("ELAPSED_TIME=$((($PROCS - " + str(proc_count) + ")*($SECONDS - $START_TIME)))\n" + \
							"echo \"\n" + str(proc_count)  + " out of " + str(num_procs - start_idx) \
							+ " processes completed.\n$(($ELAPSED_TIME/60)) min $(($ELAPSED_TIME%60)) sec left for "\
							 + str(num_procs - start_idx - proc_count) + " processes to complete\n\"\n\n")

	os.system("chmod +x " + script_dir + "/*")

	est_time =  round((float(t_time) * int(num_procs)) / int(num_batches), 3)
	print ("\033[1m" + str(datetime.datetime.now()) + " : Launching " +  str(num_procs) + " " + cut + " simulations with " +\
		str(batch_count) + " batches and with upto " + str(num_threads) + \
		" threads each. The following processes will be launched in the background.")
	if est_time:
		print("The distributed run is estimated to take " + str(round(float(est_time) \
			+ (0.3 * float(est_time)),3)) + " +- " + str(round(0.3 * float(est_time), 3)) + " s.")

	if mem:
		print("The peak memory usage is expected to be ", end = "") 
		if mem * num_batches >= pow(2, 30):
			print(str(round((mem * num_batches) / pow(2,30), 3)) + " GiB")
		elif mem * num_batches >= pow(2, 20):
			print(str(round((mem * num_batches) / pow(2,20), 3)) + " MiB")
		elif mem * num_batches >= pow(2, 10):
			print(str(round((mem * num_batches) / pow(2,10), 3)) + " KiB")
		else:
			print(str(round((mem * num_batches), 3)) + " B")

	print("The CZ path breakdown is " + str(num_cz) + "p + " + str(app_cz_len) + "r + " + str(dfs_len) + "b\033[0m")

	# Create the log directory if it doesn't already exist
	log_dir = os.path.join("output", "log")
	if not os.path.isdir(log_dir):
		try:
			os.makedirs(log_dir)
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
			shutil.rmtree(log_dir, ignore_errors=True)
			try:
				os.makedirs(log_dir)
			except OSError as e:
				if e.errno != errno.EEXIST:
					raise

	# Launch the scripts and print the logs generated by the processes in each script into a file 
	# in the log directory
	if not multiple_nodes:
		for p in range(batch_count):
			print("./" + os.path.join(script_dir, "script_" + str(p) + ".sh") +\
			  " > " + os.path.join(log_dir, "log_script_" + str(p))+ ".txt 2>&1 &")
			os.system("./" + os.path.join(script_dir, "script_" + str(p) + ".sh") + \
			  " > " + os.path.join(log_dir, "log_script_" + str(p)) + ".txt 2>&1 &")

	return batch_count

def CalculateFidelity(exact_amp_file, approx_amp_file):

	if not os.path.isfile(exact_amp_file) or not os.path.isfile(approx_amp_file):
		return float('nan');

	exact_amps = []
	with open(exact_amp_file, "r") as f:
		lines = f.readlines()
		exact_amps = np.loadtxt(lines, dtype=complex)

	approx_amps = []
	with open(approx_amp_file, "r") as f:
		lines = f.readlines()
		approx_amps = np.loadtxt(lines, dtype=complex)

	dotp_exact_approx = np.vdot(exact_amps, approx_amps) / (np.linalg.norm(exact_amps) * np.linalg.norm(approx_amps))
	fidelity = round(pow(np.linalg.norm(dotp_exact_approx), 2), 5)

	return fidelity

