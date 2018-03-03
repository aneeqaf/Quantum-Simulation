import os
from itertools import product
import numpy as np
import tempfile
import time
import re
import shutil
import psutil

def AddPrintOptToCommand(seed, command, idx_file, p_idx, num_idx):
	print_opt = ""
	if int(seed) != -1:
		print_opt += " --idx " + str(seed) + "," + str(num_idx)
		if p_idx != -1:
			print_opt += "+"
	elif idx_file != "":
		print_opt += " --idx " + idx_file
	else:
		print_opt += " --idx " + 7 + "," + str(num_idx)
		# if int(p_idx) != -1:
		# 	print_opt += "+"

	return print_opt

def BuildDistCommand(command, sim_type, num_threads, outfile):
	return command + " --num_threads " + str(num_threads) + " --sim_type " + str(sim_type) \
	+ " --outfile " + outfile  

def TrialRunEval(report_file, num_CZ, start_time, end_time):
	rep_time = "" #re.compile(r'(?<=Runtime)\w+')
	cz_path_trunc = "" #re.compile(r'Truncated CZ path\w+')
	mem_val = 0.0
	with open(report_file, "r") as file:
		for line in file:
			if "Runtime" in line:
				rep_time = re.findall("\d+\.\d+", line)[0]
			if "Truncated CZ path" in line:
				cz_path_trunc = line.split(":")[1].replace(' ','').replace('\n', '')
			if "State representation size" in line:
				mem_usage = line.split(":")[1].replace('\n', '')
				mem_val = float(mem_usage.split(" ")[1].replace(' ',''))
				unit = mem_usage.split(" ")[2].replace(' ','')
				if unit == "KB":
					mem_val = mem_val * 1e3
				elif unit == "MB":
					mem_val = mem_val * 1e6
				elif unit == "GB":
					mem_val = mem_val * 1e9
 	
	if rep_time == "":
		print ("\033[1m" + "Trial run failed" + "\033[0m")
		exit()
	else:
		if float(rep_time) < 0.0:
			print ("\033[1m" + "Simulation runtime too short for distributed execution" + "\033[0m")
			exit()
		elif float(rep_time) > 3600.0: 
			print ("\033[1m" + "Simulation runtime too int for distributed execution" + "\033[0m")
			exit()

		if int(num_CZ) - len(cz_path_trunc) < int(num_CZ):
			num_CZ = int(num_CZ) - len(cz_path_trunc)
			print("\033[1m" + "The input CZ path was truncated to length " + str(num_CZ) + "." + "\033[0m\n")

		if float(rep_time) > 2*(end_time - start_time):
			print("\033[1m" + "Simulation runtime unreliable" + "\033[0m")
			exit()

		return num_CZ, mem_val


def PerformTrialRun(commandH, commandV, num_cz):
	dirpath = tempfile.mkdtemp()

	# Horizontal trial run
	print ("\033[1m" + "Performing a trial simulation run with a horizontal cut and length " + str(num_cz) + " CZ path " + "\033[0m")
	
	start_timeH = time.time()

	tempH_file = os.path.join(dirpath, "H_phase1_rep.txt")
	print(commandH + str(num_cz) + ",0 > " + str(tempH_file))
	os.system(commandH + str(num_cz) + ",0 > " + str(tempH_file))
	os.system("cat " + tempH_file)
	
	end_timeH = time.time()

	# Horizontal trial run evaluation
	print()
	num_czh, H_mem = TrialRunEval(tempH_file, num_cz, start_timeH, end_timeH)


	# Vertical trial run
	print ("\033[1m" + "Performing a trial simulation run with a vertical cut and length " + str(num_cz) + " CZ path " + "\033[0m")
	
	start_timeV = time.time()

	tempV_file = os.path.join(dirpath, "V_phase1_rep.txt")
	print(commandV + str(num_cz) + ",0 > " + str(tempV_file))
	os.system(commandV + str(num_cz) + ",0 > " + str(tempV_file))
	os.system("cat " + tempV_file)
	
	end_timeV = time.time()

	# Vertical trial run evaluation
	num_czv, V_mem = TrialRunEval(tempV_file, num_cz, start_timeV, end_timeV)

	return num_czh, num_czh, end_timeH - start_timeH, end_timeV - start_timeV, H_mem, V_mem

def EvalMemAndRuntime(t_time, num_cz, mem, num_procs = 1, sim_type = "Simulation"):

	if (float(t_time) * (1 << int(num_cz)))/int(num_procs) > 172800:
		print ("\033[1m" + sim_type + " is expected to take too int for execution\n" + \
			"The expected time of execution for a distributed run is " \
			+ str((float(t_time) * (1 << int(num_cz)))/int(num_procs)) + "s \nThe CZ path is of length "\
			+ str(num_cz) + "\033[0m")
		exit() 
	if float(mem * num_procs) >= psutil.virtual_memory().total:
		print("\033[1m" + sim_type + "  is expected to have a peak memory usage of " + \
			str((mem * num_procs)/1e9) + " GB, which is expected to exceed system memory of " + \
			str(psutil.virtual_memory().total/1e9) + " GB\033[0m")
		exit()
