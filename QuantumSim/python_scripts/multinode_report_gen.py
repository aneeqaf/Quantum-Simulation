#!/usr/bin/env python3
"""Script for generating report for distributed simulation"""

import os
import click
import re
import dist_util
from math import ceil

@click.command()
@click.argument("cir_file", nargs=1)
@click.option("--max_procs", nargs=1, required=False, default=0)
@click.option("--test_fid", nargs=1, required=False, is_flag=True)
def main(cir_file, max_procs, test_fid):

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

	reports = os.listdir(log_dir)
	
	#Copy the initial content of seq run onto the report
	for rep in reports:
		if rep.endswith(".rep"):
			with open(os.path.join(log_dir, rep), "r") as first_file:
				requested_amps_line = ""
				for line in first_file:
					if "Qubits" in line:
						qubits = int(line.split(":")[1].split()[0].replace(' ',''))
						
					if "Max threads per process" in line:
						num_threads = int(line.split(":")[1].replace(' ','').replace("\n", ""))
						
					if "Hardware threads" in line:
						hardware_threads = int(line.split(":")[1].replace(' ','').replace("\n", ""))

					if "fidelity" in line:
						epsilon = line.split(':')[1].replace(" ", "").replace("\n", "")

					if "xCZ path breakdown" in line:
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

						if len(cz_path) == 3 :
							dfs = True

					if "Requested number of amplitudes" not in line and print_line:
						print(line,  end='')
					elif "Requested number of amplitudes" in line:
						num_amps = int(line.split(":")[1].replace(' ','').replace("\n", ""))
						print(line,  end='')
						print_line = False
					elif "Layers simulated" in line:
						pass
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

	num_processes = 0 # 1 << cz_path_len if max_procs == 0 else max_procs
	avg_time_per_category = {'I_H':0.0, 'L_H':0.0, 'CZ & T, Low XY & H':0.0, 'xCZ':0.0, 'Single X & Y':0.0, \
		 'Merged X & Y':0.0, 'Rescaling passes':0.0, 'Copying':0.0, 'Storing amps':0.0,\
		  'High XY':0.0}
	
	amp = {'3':0.0+0.0j, '1/4':0.0+0.0j, '1/2':0.0+0.0j, '3/4':0.0+0.0j, '-3':0.0+0.0j}
	avg_time_per_process = 0.0
	avg_user_time = 0.0
	num_reports = 0	
	peak_mem_per_node = 0.0
	avg_cpu_percent = 0.0
	avg_elapsed_time = 0.0
	max_elapsed_time = 0.0
	avg_dfs_time = 0.0
	avg_cz_time = 0.0
	avg_residents = 0.0
	avg_residents_unit = ""
	avg_major_pagefaults = 0.0
	avg_minor_pagefaults = 0.0
	avg_time_per_gate = 0.0
	avg_mmap_time = 0.0
	num_batches = 0
	layers_breakdown_line = ""
	layers_simulated_line = ""
	peak_mem_line = ""

	# Calculating other statistics for the report 
	for rep in reports:
		if rep.endswith(".rep"):
			num_reports += 1
			max_time = 0
			with open(os.path.join(log_dir, rep), "r") as r:
				for line in r:
					if "processes" and "batch" in line:
						words = line.split()
						num_processes += int(words[0])
						num_batches += int(words[6])
					elif "State representation size" in line:
						mem_line = line
					elif "Peak memory" in line:
						peak_mem_per_node = float(line.split()[3])
						peak_mem_unit = line.split()[4]
						peak_mem_line = line.replace("\n", "")
						if peak_mem_unit == "GiB" :
							peak_mem_per_node *= pow(2, 30)
						elif peak_mem_unit == "MiB":
							peak_mem_per_node *= pow(2, 20)
						elif peak_mem_unit == "KiB":
							peak_mem_per_node *= pow(2,10)
					elif "Layers simulated" in line:
						layers_simulated_line = line
					elif "Layers breakdown" in line:
						layers_breakdown_line = line
					elif "Avg user time" in line:
						avg_user_time += float(line.split()[4])
					elif "Wallclock" in line:
						avg_elapsed_time += float(line.split()[2])
						max_time = float(line.split()[5])
					elif "Avg CPU utilization" in line:
						avg_cpu_percent += float(line.split()[4].replace("%", ""))
					elif "Avg resident size" in line:
						avg_residents += float(line.split()[4])
						avg_residents_unit = line.split()[5]
					elif "Avg page faults" in line:
						avg_minor_pagefaults += float(line.split()[6])
						avg_major_pagefaults += float(line.split()[4])
					elif "amplitudes[3]" in line:
						amp['3'] += complex(line.split('=')[1].replace(' ', '').replace('\n', ''))
					elif "amplitudes[1/4]" in line:
						amp['1/4'] += complex(line.split('=')[1].replace(' ', '').replace('\n', ''))
					elif "amplitudes[1/2]" in line:
						amp['1/2'] += complex(line.split('=')[1].replace(' ', '').replace('\n', ''))
					elif "amplitudes[3/4]" in line:
						amp['3/4'] += complex(line.split('=')[1].replace(' ', '').replace('\n', ''))
					elif "amplitudes[-3]" in line:
						amp['-3'] += complex(line.split('=')[1].replace(' ', '').replace('\n', ''))
					elif "Avg runtime (" in line:
						time_r = line.split()[2]
						time_r = time_r.replace("(", "")# re.findall("\d+\.\d+", line)
						avg_time_per_process += float(time_r)
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
					elif "Storing and retrieving amplitudes" in line:
						avg_time_per_category['Storing amps'] += float(line.split(":")[1].replace("\t","").replace(" ","").split("=")[0][:-1])
					elif "Prefix" in line:
						avg_cz_time += float(line.split(":")[1].split()[0])
					elif "Branches" in line:
						avg_dfs_time += float(line.split(":")[1].split()[0])
					elif "Memory map" in line:
						avg_mmap_time += float(line.split(":")[1].split()[0])
					elif "Avg time per gate" in line:
						avg_time_per_gate += float(line.split(":")[1].split()[0]) 
					
				if max_elapsed_time < max_time:
					max_elapsed_time = max_time

	# Fidelity calculations for approximation
	fidelity = 0.0
	if "_approx" in cir_file:
		exact_dir = os.path.join("output", "amp_vectors", re.sub('_approx.*\d', "", cir_file))
		approx_dir = os.path.join("output", "amp_vectors", cir_file)
		exact_res_file= os.path.join(exact_dir, "result.amps")
		approx_res_file = os.path.join(approx_dir, "result.amps")
		
		if os.path.isfile(exact_res_file):
			fidelity = dist_util.CalculateFidelity(exact_res_file, approx_res_file)
			
		if test_fid:
			exact_path1_file= os.path.join(approx_dir, "test_fid_1_ascii.amps")
			exact_path2_file = os.path.join(approx_dir, "test_fid_2_ascii.amps")
			if os.path.isdir(approx_dir):
				test_fid = dist_util.CalculateFidelity(exact_path1_file, exact_path2_file)
	elif "_compress" in cir_file:
		exact_dir = os.path.join("output", "amp_vectors", re.sub('_compress.*\d', "", cir_file))
		compress_dir = os.path.join("output", "amp_vectors", cir_file)
		exact_res_file= os.path.join(exact_dir, "result.amps")
		compress_res_file = os.path.join(compress_dir, "result.amps")
		
		if os.path.isfile(exact_res_file):
			fidelity = dist_util.CalculateFidelity(exact_res_file, approx_res_file)

	# printing statistics onto reports				
	print("\nMulti-process simulation " , end="")
	if max_procs and max_procs < (1 << cz_path_len):
		print ("(truncated) ", end="")
	print(": ", end="")

	print("\n\t" + str(num_processes) + " processes (" + \
		str(num_threads) + " threads each) in " + str(num_batches) + " batch(es) over "
		+ str(num_reports) + " node(s)")
	print(mem_line, end='')

	print("\tPeak memory : " , end="")
	total_peak_mem = peak_mem_per_node * num_reports
	if total_peak_mem >= pow(2, 30):
		unit = "GiB"
		peak_mem_per_node /= pow(2, 10)
		print (str(round(total_peak_mem/pow(2, 30), 3)) + " GiB", end="")
	elif total_peak_mem >= pow(2, 20):
		unit = "MiB"
		peak_mem_per_node /= pow(2, 10)
		print (str(round(total_peak_mem/pow(2, 20), 3)) + " MiB", end="")
	elif total_peak_mem >= pow(2, 10):
		unit = "KiB"
		peak_mem_per_node /= pow(2, 10)
		print (str(round(total_peak_mem/pow(2, 10), 3)) + " KiB", end="")
	else:
		print (str(round(total_peak_mem, 3)) + " B", end="")
	
	if num_reports > 1:
		print( " (" + str(round(peak_mem_per_node, 3)) + " " + unit + " per node)", end="")
	print()
	print("\tCompression ratio : " + str(compression_ratio))

	if layers_simulated_line != "":
		print(layers_simulated_line, end="")
	print (layers_breakdown_line, end="")
	
	print("\tBatch stats :\n\t\tAvg user time : " + str(round((avg_user_time/num_reports), 3)) \
		+ " s \n\t\tWallclock : " + str(round((avg_elapsed_time/num_reports),3)) + " s (avg), " +\
		str(round(max_elapsed_time, 3))+ " s (max)")
	if avg_cpu_percent:
		print("\t\tAvg CPU utilization : " + str(round(avg_cpu_percent/num_reports, 3)) + "% (" \
			+ str(round((avg_cpu_percent/num_reports)/num_threads, 3)) + "% per thread)")
	# if avg_cz_time:
	# 	print("\t\tAvg simulation runtime breakdown : \n\t\t\tPrefix : " + str(round(avg_cz_time/num_reports, 6)) + " s = " +\
	# 		str(round(((avg_cz_time/num_reports)/(avg_elapsed_time/num_reports)) * 100, 3)) + "%")
	# if avg_dfs_time:
	# 	print("\t\t\tBranches : " + str(round(avg_dfs_time/num_reports, 6)) + " s = " +\
			# str(round(((avg_dfs_time/num_reports)/(avg_elapsed_time/num_reports)) * 100, 3)) + "%")
	if avg_residents:
		print("\t\tAvg resident size : " + str(round(avg_residents/num_reports, 3)) + " " + avg_residents_unit)
	if avg_major_pagefaults or avg_minor_pagefaults:
		print("\t\tAvg page faults : " + str(round(avg_major_pagefaults/num_reports, 3)) + " (major), " \
			+ str(round(avg_minor_pagefaults/num_reports, 3)) + " (minor)")

	if fidelity != 0.0:
		print("\tEstimated end-to-end circuit fidelity : " + str(fidelity))
			# " (epsilon = " + str(round(1/(num_processes / (1 << cz_path_len)), 3)) + ")")

	print ("\tTotal runtime: {:.3e}".format(max_elapsed_time) + " s")

	print("\tBillable runtime (total runtime * number of machines): {:.3e}".format((max_elapsed_time * num_machines)/3600) \
		+ " hrs ({:.3e}".format(((max_elapsed_time * num_machines)/num_amps)/3600) + " hrs per amp)")

	print("\namplitudes[3]  \t= {:.6e}".format(amp['3']))
	print("amplitudes[1/4]\t= {:.6e}".format(amp['1/4']))
	print("amplitudes[1/2]\t= {:.6e}".format(amp['1/2']))
	print("amplitudes[3/4]\t= {:.6e}".format(amp['3/4']))
	print("amplitudes[-3] \t= {:.6e}".format(amp['-3']) + "\n")

	avg_time_per_process /= num_reports
	avg_dfs_time /= num_reports
	avg_cz_time /= num_reports
	avg_mmap_time /= num_reports
	
	for key, val in avg_time_per_category.items():
		avg_time_per_category[key] = val/num_reports

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
		print("\tStoring and retrieving amplitudes \t: "\
		 + str(round(avg_time_per_category['Storing amps'], 3)) + " s  \t= " +\
		str(round(((avg_time_per_category['Storing amps'])/avg_time_per_process)*100, 3)) + "%")

	sum_time = sum(avg_time_per_category.values())

	sum_percen = 0
	for cat, time in avg_time_per_category.items():
		sum_percen += (time/avg_time_per_process) * 100
	
	print("\t\t\t\t\t\t\t\t----------")
	print("\tTotal \t\t\t\t\t\t\t  " + str(round(sum_percen, 3)) + "%\n")
	
	print("Avg time per gate : " + str(round(avg_time_per_gate/num_reports, 6)) + " s")	
	if avg_cz_time:
		print("Simulation per process runtime breakdown : ")
		print ("\tPrefix\t\t : " + str(round(avg_cz_time, 3)) + " s = " +\
		str(round((avg_cz_time/avg_time_per_process) * 100, 3)) + "%")
	if avg_dfs_time:
		print("\tBranches\t : " + str(round(avg_dfs_time, 3)) + " s = " +\
			str(round((avg_dfs_time/avg_time_per_process) * 100, 3)) + "%")
	if avg_mmap_time:
		print("\tMemory mapped I/O\t : " + str(round(avg_mmap_time, 5)) + " s = " +\
			str(round((avg_mmap_time/avg_time_per_process) * 100, 5)) + "%")	
	
	if max_procs != 0:
		print("\033[1m\nThe estimated time for all the processes is " \
			+ str(round((avg_elapsed_time/(max_procs)) * ((1 << cz_path_len)/num_reports), 3)) + " s\n\033[0m")

	print("\n¯\_(ツ)_/¯ \n")

if __name__ == "__main__":
    main()