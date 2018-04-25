#!/usr/bin/env python3
"""Script for error checking and report generation after launching a distributed simulation"""

import os
import click
import numpy as np
import time
import re
from math import ceil

@click.command()
@click.argument("cir_dir", nargs=1)
@click.option("--num_procs", nargs=1, required=True, default= 0)
@click.option("--num_batches", nargs=1, required=True, default= 0)
@click.option("--est_time", nargs=1, required=False, default= 0.0)
@click.option("--t_time", nargs=1, required=False, default= 0.0)
@click.option("--num_idx", nargs=1, required=True, default= 0)
@click.option("--max_procs", nargs=1, required=False, default= 0)
@click.option("--test_fid", nargs=1, required=False, is_flag=True)
@click.option("--cloud_services", nargs=1, required=False, is_flag=True)
def main(cir_dir, num_procs, num_batches, est_time, t_time,
 num_idx, max_procs, test_fid, cloud_services):

	# TODO: fix proc_per_script
	proc_per_script = ceil(float(num_procs)/ float(num_batches)) if num_procs > 1 else 1
	per_proc = num_procs if max_procs == 0 else max_procs
	est_time = round((float(t_time) * int(per_proc))/int(num_batches), 3)
	
	log_dir = os.path.join("output", "log", cir_dir)
	changing = True
	logs_prev_mem = np.zeros(num_batches)
	log_files = []
	for i in range(num_batches):
		log_files.append(os.path.join(log_dir, "log_script_" + str(i) + ".txt"))

	# if not t_time:
	# 	t_time = 5
	# changing = True
	# while changing:
	# 	if t_time < 500:
	# 		time.sleep(5 * float(t_time))
	# 	else:
	# 		time.sleep(2 * float(t_time))
	# 		 # Put script to sleep for substantial changes to take place
	# 	any_log_changed = False
	# 	for i, lf in enumerate(log_files):
	# 		if os.stat(lf).st_size > logs_prev_mem[i]:
	# 			any_log_changed = True
	# 			logs_prev_mem[i] = os.stat(lf).st_size
	# 	if not any_log_changed:
	# 		changing = False

	# Check logs for completion
	for i in range(int(num_batches - 1)):
		with open(log_files[i], "r") as lf:
			log_content = lf.read()
			count_runtimes = sum(1 for _ in re.finditer(r'\b%s\b' % re.escape("Runtime"), log_content))
			if count_runtimes != proc_per_script:
				print ("The simulations in script_" + str(i) + " did not complete")
				exit()
		
	with open(log_files[num_batches - 1], "r") as lf:
		log_content = lf.read()
		proc_per_script = num_procs - ((num_batches - 1) * proc_per_script)
		count_runtimes = sum(1 for _ in re.finditer(r'\b%s\b' % re.escape("Runtime"), log_content))
		if count_runtimes != proc_per_script:
			print ("The simulations in script_" + str(num_batches - 1) + " did not complete")
			exit()

	os.system("./python_scripts/add_amps.py " + str(cir_dir) + " " + str(num_idx))

	report_cmd = "./python_scripts/dist_sim_report_gen.py " + cir_dir + " --est_time " + str(est_time)\
	 + " --max_procs " + str(max_procs)
	if cloud_services:
		report_cmd += " --cloud_services"
	if test_fid:
		report_cmd += " --test_fid"
	print(report_cmd)
	os.system(report_cmd)

	
if __name__ == "__main__":
    main()	