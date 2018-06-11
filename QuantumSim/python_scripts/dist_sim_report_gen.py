#!/usr/bin/env python3
"""Script for generating report for distributed simulation"""

import os
import click
import re
import dist_util
from math import ceil

@click.command()
@click.argument("cir_file", nargs=1)
@click.option("--est_time", nargs=1, required=False, default=0.0)
@click.option("--max_procs", nargs=1, required=False, default=0)
@click.option("--test_fid", nargs=1, required=False, is_flag=True)
@click.option("--no_checkpoint_with_ranges", nargs=1, required=False, is_flag=True)
def main(cir_file, est_time, max_procs, test_fid, no_checkpoint_with_ranges):

	log_dir = os.path.join("output", "log", cir_file)

	if not os.path.isdir(log_dir):
		print("Directory " + log_dir + " does not exist")
		exit()

	mem_line = ""
	mem_val = 0
	unit = "B"
	qubits = 0
	print_line = True
	categories = {'I_H':0, 'L_H':0, 'CZ & T':0, 'xCZ':0, 'Single X':0, 'Single Y':0, 'H_lo':0, 'H_hi':0,\
	'Merged X & Y':0, 'Rescaling passes':0, 'Copying':0, 'High XY': 0 , 'Low XY': 0}
	num_threads = 0
	cz_path_len = 0
	cz_path_ranges = 0
	dfs_bits = 0
	dfs = False
	epsilon = 0
	hardware_threads = 0
	num_amps = 0
	num_layers_stats = ""
	layers_breakdown = ""
	low_val_q_line = ""

	scripts = os.listdir(log_dir)

	#Copy the initial content of seq run onto the report
	for script_log in scripts:
		if script_log.endswith(".txt"):
			with open(os.path.join(log_dir, script_log), "r") as first_file:
				requested_amps_line = ""
				for line in first_file:
					if "Qubits" in line:
						qubits = int(line.split(":")[1].split()[0].replace(' ',''))
						
					if "Max threads per process" in line:
						num_threads = int(line.split(":")[1].replace(' ','').replace("\n", ""))
						
					if "Hardware threads" in line:
						hardware_threads = int(line.split(":")[1].replace(' ','').replace("\n", ""))

					if "xCZ path breakdown" in line:
						print(line.replace('\n',""), end=" (")
						cz_path_t = line.split(":")[1]
						# if cz_path_t[1].replace(" ", "").replace("\n", "") != "None":
						cz_path = cz_path_t.split("+")
						if len(cz_path) >= 1:
							cz_path_len = int(re.sub('[^0-9]', '', cz_path[0])) 
						if len(cz_path) > 1:
							if "r" in cz_path[1]:
								cz_path_ranges = int(re.sub('[^0-9]', '', cz_path[1]))
						if len(cz_path) > 2: 
							dfs_bits = int(re.sub('[^0-9]', '', cz_path[2]))
						print_line = False

						if len(cz_path) == 3 :
							dfs = True
					if "fidelity" in line:
						epsilon = line.split(':')[1].replace(" ", "").replace("\n", "")
					if "Cycle breakdown" in line:
						print(line.replace('C', 'c').replace('\n',")"))
						if low_val_q_line != "":
							print(low_val_q_line, end="")
					if "Layers breakdown" in line:
						layers_breakdown += line
					elif "Low-value qubits" in line:
						low_val_q_line = line
					elif "Requested num amps" in line:
						num_amps = int(line.split(":")[1].replace(' ','').replace("\n", ""))
						print(line,  end='')
					elif "State representation size" not in line and print_line:
						print(line,  end='')
					elif "State representation size" in line:
						mem_line = line
						mem_usage = line.split(":")[1].replace('\n', '')
						mem_val = float(mem_usage.split(" ")[1].replace(' ',''))
						unit = mem_usage.split(" ")[2].replace(' ','')
						print_line = False
					elif "Layers simulated" in line:
						num_layers_stats += line.replace("\n", "")
					# elif "CZ & T" in line and "(" not in line:
					# 	num_layers_stats += "\t" + line
					# elif "X & Y" in line and "(" not in line:
					# 	num_layers_stats += "\t" + line
					elif "Initial H" in line :
						categories['I_H'] = int(line.split()[2].replace("(","").replace(")",""))
					elif "Last H" in line :
						categories['L_H'] = int(line.split()[2].replace("(","").replace(")",""))
					elif "CZ & T" in line:
						categories['CZ & T'] = int(line.split()[3].replace("(","").replace(")","").replace(",", ""))
						categories['Low XY'] = int(line.split()[8].replace("(","").replace(")",""))
						try:
							categories['H_lo'] = int(line.split()[11].replace("(","").replace(")",""))
						except:
							categories['H_lo'] = 0
					elif "xCZ (" in line:
						categories['xCZ'] = int(line.split()[1].replace("(","").replace(")",""))
					elif "Single X" in line:
						categories['Single X'] = int(line.split()[2].replace("(","").replace(")",""))
						categories['Single Y'] = int(line.split()[5].replace("(","").replace(")",""))
					elif "Merged X & Y" in line:
						categories['Merged X & Y'] = int(line.split()[4].replace("(","").replace(")",""))
					elif "High X & Y" in line:
						categories['High XY'] = int(line.split()[4].replace("(","").replace(")",""))
						try:
							categories['H_hi'] = int(line.split()[7].replace("(","").replace(")",""))
						except:
							categories['H_hi'] = 0
					elif "Rescaling passes" in line:
						categories['Rescaling passes'] = int(line.split()[2].replace("(","").replace(")",""))
					elif "Copying" in line:
						categories['Copying'] = int(line.split()[1].replace("(","").replace(")",""))
					
					elif "¯\_(ツ)_/¯ " in line:
						break
			break

	num_CZ_paths = 0 # 1 << cz_path_len if max_procs == 0 else max_procs
	avg_time_per_category = {'I_H':0.0, 'L_H':0.0, 'CZ & T, Low XY & H':0.0, 'xCZ':0.0, 'Single X & Y':0.0, \
		 'Merged X & Y':0.0, 'Rescaling passes':0.0, 'Copying':0.0, 'Storing amps':0.0,\
		  'High XY':0.0}
	
	amp = {'3':0.0+0.0j, '1/4':0.0+0.0j, '1/2':0.0+0.0j, '3/4':0.0+0.0j, '-3':0.0+0.0j}
	avg_time_per_process = 0.0
	avg_user_time = 0.0
	num_batches = 0	
	peak_mem = 0.0
	avg_cpu_percent = 0.0
	avg_elapsed_time = 0.0
	max_elapsed_time = 0.0
	avg_CPU_uti_per_p = 0.0
	avg_dfs_time = 0.0
	avg_cz_time = 0.0
	avg_residents = 0.0
	avg_major_pagefaults = 0.0
	avg_minor_pagefaults = 0.0
	avg_time_per_gate = 0.0
	avg_mmap_time = 0.0

	# Calculating other statistics for the report 
	for script_log in scripts:
		if script_log.endswith(".txt"):
			num_batches += 1
			max_time = 0
			with open(os.path.join(log_dir, script_log), "r") as sl:
				for line in sl:
					if "amp[3]" in line:
						amp['3'] += complex(line.split('=')[1].replace(' ', '').replace('\n', ''))
					elif "amp[1/4]" in line:
						amp['1/4'] += complex(line.split('=')[1].replace(' ', '').replace('\n', ''))
					elif "amp[1/2]" in line:
						amp['1/2'] += complex(line.split('=')[1].replace(' ', '').replace('\n', ''))
					elif "amp[3/4]" in line:
						amp['3/4'] += complex(line.split('=')[1].replace(' ', '').replace('\n', ''))
					elif "amp[-3]" in line:
						amp['-3'] += complex(line.split('=')[1].replace(' ', '').replace('\n', ''))
					elif "Runtime (" in line:
						num_CZ_paths += 1
						time_r = line.split()[1]
						time_r = time_r.replace("(", "")# re.findall("\d+\.\d+", line)
						avg_time_per_process += float(time_r)
						# else:
						# 	time_r = re.findall("\d+", line)
						# 	avg_time_per_process += float(time_r)
					elif "Initial H" in line :
						avg_time_per_category['I_H'] += float(line.split(":")[1].replace("\t","").replace(" ","").split("=")[0][:-1])
					elif "Last H" in line :
						avg_time_per_category['L_H'] += float(line.split(":")[1].replace("\t","").replace(" ","").split("=")[0][:-1])
					elif "CZ & T " in line and "Low X & Y" in line:
						avg_time_per_category['CZ & T, Low XY & H'] += float(line.split(":")[1].replace("\t","").replace(" ","").split("=")[0][:-1])
					elif "xCZ (" in line:
						avg_time_per_category['xCZ'] += float(line.split(":")[1].replace("\t","").replace(" ","").split("=")[0][:-1])
					elif "Single X" in line:
						avg_time_per_category['Single X & Y'] += float(line.split(":")[1].replace("\t","").replace(" ","").split("=")[0][:-1])
					elif "Merged X & Y" in line:
						avg_time_per_category['Merged X & Y'] += float(line.split(":")[1].replace("\t","").replace(" ","").split("=")[0][:-1])
					elif "High X & Y" in line:
						avg_time_per_category['High XY'] += float(line.split(":")[1].replace("\t","").replace(" ","").split("=")[0][:-1])
					elif "Rescaling passes" in line:
						avg_time_per_category['Rescaling passes'] += float(line.split(":")[1].replace("\t","").replace(" ","").split("=")[0][:-1])
					elif "Copying" in line:
						avg_time_per_category['Copying'] += float(line.split(":")[1].replace("\t","").replace(" ","").split("=")[0][:-1])
					elif "Storing amps" in line:
						avg_time_per_category['Storing amps'] += float(line.split(":")[1].replace("\t","").replace(" ","").split("=")[0][:-1])
					elif "Prefix" in line:
						avg_cz_time += float(line.split(":")[1].split()[0])
					elif "Branches" in line:
						avg_dfs_time += float(line.split(":")[1].split()[0])
					elif "Memory map" in line:
						avg_mmap_time += float(line.split(":")[1].split()[0])
					elif "Average time per gate" in line:
						avg_time_per_gate += float(line.split(":")[1].split()[0])
					elif "elapsed" in line:
						elapsed_time = line.split()[2].split(':')
						e_t = 0.0
						if len(elapsed_time) == 2:
							e_t = float(elapsed_time[0]) * 60 \
							+ float(re.sub('[a-zA-Z_]', '', elapsed_time[1]))
						elif len(elapsed_time) == 3:
							e_t = (float(elapsed_time[0]) * 60 * 60) + (float(elapsed_time[1]) * 60) \
							+ float(re.sub('[a-zA-Z_]', '', elapsed_time[1]))
						avg_elapsed_time += e_t
						max_time += e_t
						
						user_time = line.split()[0]
						avg_user_time += float(re.sub('[a-zA-Z_]', '', user_time))
						avg_cpu_percent += float(line.split()[3].split('%')[0])
						avg_residents += float(re.sub('[^0-9]', '', line.split()[-1])) * 1000
					elif "pagefaults" in line:
						pf_str = line.split()[1].split('+')
						avg_major_pagefaults += float(re.sub('[^0-9]', '', line.split()[1].split('+')[0]))
						avg_minor_pagefaults += float(re.sub('[^0-9]', '', line.split()[1].split('+')[1]))
					elif "real" in line:
						avg_user_time += float(line.split()[2])
						e_t = float(line.split()[0])
						avg_elapsed_time += e_t
						max_time += e_t
				if max_elapsed_time < max_time:
						max_elapsed_time = max_time

	# Fidelity calculations for approximation
	fidelity = 0.0
	if "_approx" in cir_file:
		exact_dir = os.path.join("output", "amp_vectors", re.sub('_approx.*\d', "", cir_file))
		approx_dir = os.path.join("output", "amp_vectors", cir_file)
		exact_res_file= os.path.join(exact_dir, "result.amps")
		approx_res_file = os.path.join(approx_dir, "result.amps")
		# model_dir = os.path.join("output", "misc", cir_file)

		# if not os.path.isdir(model_dir):
		# 	try:
		# 		os.makedirs(model_dir)
		# 	except OSError as e:
		# 		if e.errno != errno.EEXIST:
		# 			raise
		# model_file = os.path.join(model_dir, "model.txt")

		if os.path.isfile(exact_res_file):
			fidelity = dist_util.CalculateFidelity(exact_res_file, approx_res_file)
			# if os.path.isfile(model_file):
			# 	with open(model_file, "a") as file:
			# 		file.write(str(num_CZ_paths) + "," + str(fidelity) + "," + str(epsilon) + "\n")
			# else:
			# 	with open(model_file, "w") as file:
			# 		file.write(str(num_CZ_paths) + "," + str(fidelity) + "," + str(epsilon) + "\n")


		if test_fid:
			exact_path1_file= os.path.join(approx_dir, "test_fid_1_ascii.amps")
			exact_path2_file = os.path.join(approx_dir, "test_fid_2_ascii.amps")
			if os.path.isdir(approx_dir):
				test_fid = dist_util.CalculateFidelity(exact_path1_file, exact_path2_file)
				# print(test_fid)
				# if test_fid > 1e-3:
				# 	fidelity = float("nan")
	
	num_machines = ceil((num_threads * num_batches)/hardware_threads)
	# printing statistics onto reports				
	print("\nMulti-process simulation " , end="")
	if max_procs and max_procs < (1 << cz_path_len):
		print ("(truncated) ", end="")
	print(": ", end="")

	print("\n\t" + str(num_CZ_paths) + " processes (" + \
		str(num_threads) + " threads each) over " + str(num_machines), end="")
	if num_machines > 1 and ceil(num_batches/num_machines) > 1:
		print(" nodes in " + str(ceil(num_batches/num_machines)) + " batches per node ")
	elif num_machines > 1 and ceil(num_batches/num_machines) == 1:
		print(" nodes in " + str(ceil(num_batches/num_machines)) + " batch per node ")
	elif num_machines == 1 and ceil(num_batches/num_machines) > 1:
		print(" node in " + str(ceil(num_batches/num_machines)) + " batches per node ")
	elif num_machines == 1 and ceil(num_batches/num_machines) == 1:
		print(" node in " + str(ceil(num_batches/num_machines)) + " batch per node ")

	print("\t" + mem_line, end='')

	if not no_checkpoint_with_ranges and dfs and cz_path_ranges:
		mem_val *= 3
	elif cz_path_ranges or dfs:
		mem_val += mem_val  
	print("\tPeak memory : " + str(round(mem_val * num_batches,3)) + " " + unit, end="")

	if num_machines > 1:
		print( " (" + str(round((mem_val * num_batches) / num_machines, 3)) + " " + unit + " per node)", end="")
	print()

	if float(est_time):
		print("\tPredicted time : " + str(round(float(est_time), 3)) \
			+ " +- " + str(round(float(est_time) * 0.3, 3)) + " s")

	if num_layers_stats != "":
		print("\t" + num_layers_stats)
	print ("\t" + layers_breakdown, end="")
	
	if round((avg_elapsed_time/num_batches),3) <= 3000 :
		print("\tBatch stats :\n\t\tAvg user time : " + str(round((avg_user_time/num_batches), 3)) \
			+ " s \n\t\tWallclock : " + str(round((avg_elapsed_time/num_batches),3)) + " s (avg), " +\
			str(round(max_elapsed_time, 3))+ " s (max)")
	else:
		print("\tBatch stats :\n\t\tAvg user time : " + str(round((avg_user_time/num_batches)/3600, 3)) \
			+ " hrs \n\t\tWallclock : " + str(round((avg_elapsed_time/num_batches)/3600,3)) + " hrs (avg), " +\
			str(round(max_elapsed_time/3600, 3))+ " hrs (max)")

	if avg_cpu_percent:
		print("\t\tAvg CPU utilization : " + str(round(avg_cpu_percent/num_CZ_paths, 3)) + "% (" \
			+ str(round((avg_cpu_percent/num_CZ_paths)/num_threads, 3)) + "% per thread)")
	# if avg_cz_time:
	# 	print("\t\tAvg simulation runtime breakdown : \n\t\t\tPrefix : " + str(round(avg_cz_time/num_batches, 6)) + " s = " +\
	# 		str(round(((avg_cz_time/num_batches)/(avg_elapsed_time/num_batches)) * 100, 3)) + "%")
	# if avg_dfs_time:
	# 	print("\t\t\tBranches : " + str(round(avg_dfs_time/num_batches, 6)) + " s = " +\
			# str(round(((avg_dfs_time/num_batches)/(avg_elapsed_time/num_batches)) * 100, 3)) + "%")
	if avg_residents:
		print("\t\tAvg resident size : ", end="")
		if (avg_residents)/num_CZ_paths >= pow(2, 30):
			print (str(round(avg_residents/(pow(2, 30) * num_CZ_paths), 3)) + " GiB")
		elif (avg_residents)/num_CZ_paths >= pow(2, 20):
			print (str(round(avg_residents/(pow(2, 20) * num_CZ_paths), 3)) + " MiB")
		elif (avg_residents)/num_CZ_paths >= pow(2, 10):
			print (str(round(avg_residents/(pow(2, 10) * num_CZ_paths), 3)) + " KiB")
		else:
			print (str(round((avg_residents)/(num_CZ_paths), 3)) + " B")
	if avg_major_pagefaults or avg_minor_pagefaults:
		print("\t\tAvg page faults : " + str(round(avg_major_pagefaults/num_CZ_paths, 3)) + " (major), " \
			+ str(round(avg_minor_pagefaults/num_CZ_paths, 3)) + " (minor)")

	if fidelity != 0.0:
		print("\tEstimated end-to-end circuit fidelity : " + str(fidelity))
			# " (epsilon = " + str(round(1/(num_CZ_paths / (1 << cz_path_len)), 3)) + ")")

	print("\tBillable runtime : {:.3e}".format((max_elapsed_time * num_machines)/3600) \
		+ " hrs ({:.3e}".format(((max_elapsed_time * num_machines)/num_amps)/3600) + " hrs per amp)")

	print("\namp[3]  \t= {:.6e}".format(amp['3']))
	print("amp[1/4]\t= {:.6e}".format(amp['1/4']))
	print("amp[1/2]\t= {:.6e}".format(amp['1/2']))
	print("amp[3/4]\t= {:.6e}".format(amp['3/4']))
	print("amp[-3] \t= {:.6e}".format(amp['-3']) + "\n")

	avg_time_per_process /= num_CZ_paths
	avg_dfs_time /= num_CZ_paths
	avg_cz_time /= num_CZ_paths
	avg_mmap_time /= num_CZ_paths
	
	for key, val in avg_time_per_category.items():
		avg_time_per_category[key] = val/num_CZ_paths

	print("Avg runtime (" + str(round(avg_time_per_process, 3)) + " s total) per process by category ")
	if avg_time_per_category['I_H']:
		print("\tInitial H ("+ str(categories['I_H']) + ") \t\t\t\t: " \
			+ str(round(avg_time_per_category['I_H'], 5)) + " s  \t= " +\
		str(round(((avg_time_per_category['I_H'])/avg_time_per_process)*100, 3)) + "%")

	if avg_time_per_category['L_H']:
		print("\tLast H ("+ str(categories['L_H']) + ") \t\t\t\t: " \
			+ str(round(avg_time_per_category['L_H'], 3)) + " s  \t= " +\
		str(round(((avg_time_per_category['L_H'])/avg_time_per_process)*100, 3)) + "%")

	if avg_time_per_category['xCZ']:
		print("\txCZ (" + str(categories['xCZ']) + ") \t\t\t\t: "\
		 + str(round(avg_time_per_category['xCZ'], 3)) + " s  \t= " +\
		str(round(((avg_time_per_category['xCZ'])/avg_time_per_process)*100, 3)) + "%")
	
	if avg_time_per_category['CZ & T, Low XY & H']:
		print("\tCZ & T (" + str(categories['CZ & T']) + "), Low X & Y (" + str(categories['Low XY']) 
			+ ")" , end="")
		if categories['H_lo']:
			print (" & H (" + str(categories['H_lo']) +  ")\t: ", end ="")
		else:
			print ("\t\t: ", end ="")
		print ( str(round(avg_time_per_category['CZ & T, Low XY & H'], 3)) + " s  \t= " +\
		str(round(((avg_time_per_category['CZ & T, Low XY & H'])/avg_time_per_process)*100, 3)) + "%")
	
	if avg_time_per_category['Single X & Y']:
		print("\tSingle X (" + str(categories['Single X']) + ") & Y (" \
			+ str(categories['Single Y']) + ")\t\t\t: " \
			+ str(round(avg_time_per_category['Single X & Y'] , 3)) + " s  \t= " +\
		str(round((avg_time_per_category['Single X & Y']/avg_time_per_process)*100, 3)) + "%")
	
	if avg_time_per_category['Merged X & Y']:
		print("\tMerged X & Y (" + str(categories['Merged X & Y']) + ")\t\t\t: "\
		 + str(round(avg_time_per_category['Merged X & Y'], 3)) + " s  \t= " +\
		str(round(((avg_time_per_category['Merged X & Y'])/avg_time_per_process)*100, 3)) + "%")
	
	if avg_time_per_category['High XY']:
		print("\tHigh X & Y (" + str(categories['High XY']) + ")" , end="")
		if categories['H_hi']:
			print (" & H (" + str(categories['H_hi']) +  ")  \t\t: ", end ="")
		else:
			print ("   \t\t\t: " , end="")
		print (str(round(avg_time_per_category['High XY'], 3)) + " s  \t= " +\
		str(round(((avg_time_per_category['High XY'])/avg_time_per_process)*100, 3)) + "%")

	if avg_time_per_category['Rescaling passes']:
		print("\tRescaling passes (" + str(categories['Rescaling passes']) + ")\t\t\t: "\
		 + str(round(avg_time_per_category['Rescaling passes'], 3)) + " s  \t= " +\
		str(round(((avg_time_per_category['Rescaling passes'])/avg_time_per_process)*100, 3)) + "%")

	if avg_time_per_category['Copying']:
		print("\tCopying (" + str(categories['Copying']) + ")\t\t\t\t: "\
		 + str(round(avg_time_per_category['Copying'], 3)) + " s  \t= " +\
		str(round(((avg_time_per_category['Copying'])/avg_time_per_process)*100, 3)) + "%")

	if avg_time_per_category['Storing amps']:
		print("\tStoring amps \t\t\t\t: "\
		 + str(round(avg_time_per_category['Storing amps'], 3)) + " s  \t= " +\
		str(round(((avg_time_per_category['Storing amps'])/avg_time_per_process)*100, 3)) + "%")

	sum_time = sum(avg_time_per_category.values())

	sum_percen = 0
	for cat, time in avg_time_per_category.items():
		sum_percen += (time/avg_time_per_process) * 100
	
	print("\t\t\t\t\t\t\t\t----------")
	print("\tTotal \t\t\t\t\t\t\t  " + str(round(sum_percen, 3)) + "%\n")
	
	print("Avg time per gate : " + str(round(avg_time_per_gate/num_CZ_paths, 6)) + " s")	
	if avg_cz_time:
		print("Simulation per process runtime breakdown : ")
		print ("\tPrefix\t\t  : " + str(round(avg_cz_time, 3)) + " s = " +\
		str(round((avg_cz_time/avg_time_per_process) * 100, 3)) + "%")
	if avg_dfs_time:
		print("\tBranches\t  : " + str(round(avg_dfs_time, 3)) + " s = " +\
			str(round((avg_dfs_time/avg_time_per_process) * 100, 3)) + "%")
	if avg_mmap_time:
		print("\tMemory map I/O    : " + str(round(avg_mmap_time, 5)) + " s = " +\
			str(round((avg_mmap_time/avg_time_per_process) * 100, 5)) + "%")	
	if avg_CPU_uti_per_p:
		print("Avg CPU utilization per process : " + \
			str(round(avg_CPU_uti_per_p/num_CZ_paths, 3)) + " % ")

	if max_procs != 0:
		print("\033[1m\nThe estimated time for all the processes is " \
			+ str(round((avg_elapsed_time/(max_procs)) * ((1 << cz_path_len)/num_batches), 3)) + " s\n\033[0m")

	print("\n¯\_(ツ)_/¯ \n")

if __name__ == "__main__":
    main()