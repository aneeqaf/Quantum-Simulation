#!/usr/bin/env python3
"""Script for generating report for distributed simulation"""

import os
import click
import re

@click.command()
@click.argument("cir_file", nargs=1)
@click.argument("est_time", nargs=1)
def main(cir_file, est_time):
	log_dir = os.path.join("output", "log", cir_file)

	if not os.path.isdir(log_dir):
		print("Directory " + log_dir + " does not exist")
		exit()

	mem_line = ""
	mem_val = 0
	unit = "B"
	qubits = 0
	print_line = True
	categories = {'H':0, 'CZ & T':0, 'xCZ':0, 'Single X':0, 'Single Y':0,\
	'Merged X & Y':0, 'Rescaling passes':0, 'Copying':0}
	num_threads = 0
	cz_path_len = 0
	dfs = False
	#Copy the initial content of seq run onto the report
	with open(os.path.join(log_dir, "log_script_0.txt"), "r") as first_file:
		for line in first_file:
			if "Qubits" in line:
				qubits = int(line.split(":")[1].split()[0].replace(' ',''))
			if "Max threads per process" in line:
				num_threads = int(line.split(":")[1].replace(' ','').replace("\n", ""))

			if "Phase 1 CZ path" in line:
				cz_path_t = line.split(":")
				if cz_path_t[1].replace(" ", "").replace("\n", "") != "None":
					cz_path = cz_path_t[1].split()[1].replace('(','').replace(')','')
				else:
					cz_path = 0
				cz_path_len = int(cz_path)
				print("Phase 1 CZ path length : " + str(cz_path_len))
				print_line = False
			elif "Phase 2 CZ path length" in line:
				dfs = True
				print(line, end='')
			elif "State representation size" not in line and print_line:
				print(line,  end='')
			elif "State representation size" in line:
				mem_line = line
				mem_usage = line.split(":")[1].replace('\n', '')
				mem_val = float(mem_usage.split(" ")[1].replace(' ',''))
				unit = mem_usage.split(" ")[2].replace(' ','')
				print_line = False
			elif "H (" in line:
				categories['H'] = int(line.split()[1].replace("(","").replace(")",""))
			elif "CZ & T" in line:
				categories['CZ & T'] = int(line.split()[3].replace("(","").replace(")",""))
			elif "xCZ (" in line:
				categories['xCZ'] = int(line.split()[1].replace("(","").replace(")",""))
			elif "Single X" in line:
				categories['Single X'] = int(line.split()[2].replace("(","").replace(")",""))
				categories['Single Y'] = int(line.split()[5].replace("(","").replace(")",""))
			elif "Merged X & Y" in line:
				categories['Merged X & Y'] = int(line.split()[4].replace("(","").replace(")",""))
			elif "Rescaling passes" in line:
				categories['Rescaling passes'] = int(line.split()[2].replace("(","").replace(")",""))
			elif "Copying" in line:
				categories['Copying'] = int(line.split()[1].replace("(","").replace(")",""))
			
			elif "¯\_(ツ)_/¯ " in line:
				break

	num_CZ_paths = 1 << cz_path_len
	avg_time_per_category = {'H':0.0, 'CZ & T':0.0, 'xCZ':0.0, 'Single X':0.0, 'Single Y':0.0,\
		 'Merged X & Y':0.0, 'Rescaling passes':0.0, 'Copying':0.0}
	amp = {'3':0.0+0.0j, '1/4':0.0+0.0j, '1/2':0.0+0.0j, '3/4':0.0+0.0j, '-3':0.0+0.0j}
	avg_time_per_process = 0.0
	avg_user_time = 0.0
	num_batches = 0	
	peak_mem = 0.0
	avg_cpu_percent = 0.0
	avg_elapsed_time = 0.0
	max_elapsed_time = 0.0
	avg_CPU_uti_per_p = 0.0
	scripts = os.listdir(log_dir)
	avg_dfs_time = 0.0
	avg_cz_time = 0.0

	for script_log in scripts:
		num_batches += 1
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
				elif "Runtime" in line:
					time_r = re.findall("\d+\.\d+", line)
					if len(time_r):
						avg_time_per_process += float(time_r[0])
					else:
						time_r = re.findall("\d+", line)
						avg_time_per_process += float(time_r[0])
				elif "H (" in line:
					avg_time_per_category['H'] += float(line.split(":")[1].replace("\t","").replace(" ","").split("=")[0][:-1])
				elif "CZ & T" in line:
					avg_time_per_category['CZ & T'] += float(line.split(":")[1].replace("\t","").replace(" ","").split("=")[0][:-1])
				elif "xCZ (" in line:
					avg_time_per_category['xCZ'] += float(line.split(":")[1].replace("\t","").replace(" ","").split("=")[0][:-1])
				elif "Single X" in line:
					avg_time_per_category['Single X'] += float(line.split(":")[1].replace("\t","").replace(" ","").split("=")[0][:-1])
					avg_time_per_category['Single Y'] += float(line.split(":")[1].replace("\t","").replace(" ","").split("=")[0][:-1])
				elif "Merged X & Y" in line:
					avg_time_per_category['Merged X & Y'] += float(line.split(":")[1].replace("\t","").replace(" ","").split("=")[0][:-1])
				elif "Rescaling passes" in line:
					avg_time_per_category['Rescaling passes'] += float(line.split(":")[1].replace("\t","").replace(" ","").split("=")[0][:-1])
				elif "Copying" in line:
					avg_time_per_category['Copying'] += float(line.split(":")[1].replace("\t","").replace(" ","").split("=")[0][:-1])
				elif "Phase 1 runtime" in line:
					temp_str = line.split(":")[1].split()[0]
					avg_cz_time += float(temp_str)
				elif "Phase 2 runtime" in line:
					temp_str = line.split(":")[1].split()[0]
					avg_dfs_time += float(temp_str)
				elif "CPU utilization" in line:
					temp_str = line.split(":")[1].split()[0]
					avg_CPU_uti_per_p += float(temp_str)
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
					if max_elapsed_time < e_t:
						max_elapsed_time = e_t

					user_time = line.split()[0]
					avg_user_time += float(re.sub('[a-zA-Z_]', '', user_time))
					avg_cpu_percent += float(line.split()[3].split('%')[0])
				elif "real" in line:
					avg_user_time += float(line.split()[2])
					e_t = float(line.split()[0])
					avg_elapsed_time += e_t
					if max_elapsed_time < e_t:
						max_elapsed_time = e_t
					
	print("\nDistributed simulation : " + str(num_CZ_paths) + " processes (" + \
		str(num_threads) + " threads each) in " + str(num_batches) + " batches")
	print("\t" + mem_line, end='')

	if dfs:
		mem_val *= 2
	print("\tPeak memory : " + str(round(mem_val * num_batches,3)) + " " + unit)
	print("\tPredicted time : " + str(round(float(est_time) + 0.4 * float(est_time), 3)) + " +- "\
	 + str(round(0.3 * float(est_time), 3)) + " s")
	print("\tBatch time :\n\t\tUser : " + str(round((avg_user_time/num_batches), 3)) \
		+ " s (avg)\n\t\tWallclock : " + str(round((avg_elapsed_time/num_batches),3)) + " s (avg), " +\
		str(round(max_elapsed_time, 3))+ " s (max)")
	if avg_cpu_percent:
		print("\t\tCPU utilization : " + str(round(avg_cpu_percent/num_batches, 3)) + "% (avg)")
	if avg_cz_time:
		print("\t\tAvg phase 1 time : " + str(round(avg_cz_time/num_batches, 6)) + " s = " +\
			str(round(((avg_cz_time/num_batches)/(avg_elapsed_time/num_batches)) * 100, 3)) + "%")
	if avg_dfs_time:
		print("\t\tAvg phase 2 time : " + str(round(avg_dfs_time/num_batches, 6)) + " s = " +\
			str(round(((avg_dfs_time/num_batches)/(avg_elapsed_time/num_batches)) * 100, 3)) + "%")

	print("\namp[3]  \t= {:.8f}".format(amp['3']))
	print("amp[1/4]\t= {:.8f}".format(amp['1/4']))
	print("amp[1/2]\t= {:.8f}".format(amp['1/2']))
	print("amp[3/4]\t= {:.8f}".format(amp['3/4']))
	print("amp[-3] \t= {:.8f}".format(amp['-3']) + "\n")

	avg_time_per_process /= num_CZ_paths
	avg_dfs_time /= num_CZ_paths
	avg_cz_time /= num_CZ_paths
	
	for key, val in avg_time_per_category.items():
		avg_time_per_category[key] = val/num_CZ_paths

	print("Avg runtime (" + str(round(avg_time_per_process, 6)) + " s total) per process by category ")
	if avg_time_per_category['H']:
		print("\tH ("+ str(categories['H']) + ")\t\t\t: " \
			+ str(round(avg_time_per_category['H'], 6)) + " s  \t= " +\
		str(round(((avg_time_per_category['H'])/avg_time_per_process)*100, 3)) + "%")
	
	if avg_time_per_category['CZ & T']:
		print("\tCZ & T (" + str(categories['CZ & T']) + ")\t\t: " \
			+ str(round(avg_time_per_category['CZ & T'], 6)) + " s  \t= " +\
		str(round(((avg_time_per_category['CZ & T'])/avg_time_per_process)*100, 3)) + "%")
	
	if avg_time_per_category['xCZ']:
		print("\txCZ (" + str(categories['xCZ']) + ") \t\t: "\
		 + str(round(avg_time_per_category['xCZ'], 6)) + " s  \t= " +\
		str(round(((avg_time_per_category['xCZ'])/avg_time_per_process)*100, 3)) + "%")
	
	if avg_time_per_category['Single X']:
		print("\tSingle X (" + str(categories['Single X']) + ") & Y (" \
			+ str(categories['Single Y']) + ")\t: " \
			+ str(round((avg_time_per_category['Single X'] + \
			avg_time_per_category['Single Y']), 6)) + " s  \t= " +\
		str(round((((avg_time_per_category['Single X'] + \
			avg_time_per_category['Single Y']))/avg_time_per_process)*100, 3)) + "%")
	
	if avg_time_per_category['Merged X & Y']:
		print("\tMerged X & Y (" + str(categories['Merged X & Y']) + ")\t: "\
		 + str(round(avg_time_per_category['Merged X & Y'], 6)) + " s  \t= " +\
		str(round(((avg_time_per_category['Merged X & Y'])/avg_time_per_process)*100, 3)) + "%")
	
	if avg_time_per_category['Rescaling passes']:
		print("\tRescaling passes (" + str(categories['Rescaling passes']) + ")\t: "\
		 + str(round(avg_time_per_category['Rescaling passes'], 6)) + " s  \t= " +\
		str(round(((avg_time_per_category['Rescaling passes'])/avg_time_per_process)*100, 3)) + "%")

	if avg_time_per_category['Copying']:
		print("\tCopying (" + str(categories['Copying']) + ")\t\t: "\
		 + str(round(avg_time_per_category['Copying'], 6)) + " s  \t= " +\
		str(round(((avg_time_per_category['Copying'])/avg_time_per_process)*100, 3)) + "%")

	sum_time = sum(avg_time_per_category.values())/num_CZ_paths

	sum_percen = 0
	for cat, time in avg_time_per_category.items():
		sum_percen += (time/avg_time_per_process) * 100
	
	print("\t\t\t\t\t\t----------")
	print("\tTotal \t\t\t\t\t  " + str(round(sum_percen, 3)) + "%\n")
	if sum_percen > 100 :
		print("Percentages don't add upto 100%")

	print("Avg time per gate per process : " + str(round(sum_time, 6)) + " s")	
	if avg_cz_time:
		print("Avg phase 1 time per process : " + str(round(avg_cz_time, 6)) + " s = " +\
			str(round((avg_cz_time/avg_time_per_process) * 100, 3)) + "%")
	if avg_dfs_time:
		print("Avg phase 2 time per process : " + str(round(avg_dfs_time, 6)) + " s = " +\
			str(round((avg_dfs_time/avg_time_per_process) * 100, 3)) + "%")
	if avg_CPU_uti_per_p:
		print("Avg CPU utilization per process : " + \
			str(round(avg_CPU_uti_per_p/num_CZ_paths, 6)) + " % ")

	print("\n¯\_(ツ)_/¯ \n")

if __name__ == "__main__":
    main()
