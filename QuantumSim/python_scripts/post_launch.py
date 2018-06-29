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
@click.option("--no_checkpoint_with_ranges", nargs=1, required=False, is_flag=True)
@click.option("--binary_vectors_only", nargs=1, required=False, is_flag=True)
@click.option("--not_final_amps", nargs=1, required=False, is_flag=True)
@click.option("--batch_range", nargs=2, required=False, default=[-1, -1], type=int)
@click.option("--save_checkpoint_to_file", nargs=1, required=False, default=0)
def main(cir_dir, num_procs, num_batches, est_time, t_time,
 num_idx, max_procs, test_fid, no_checkpoint_with_ranges, 
 binary_vectors_only, not_final_amps, batch_range, save_checkpoint_to_file):
	# TODO: fix proc_per_script
	proc_per_script = ceil(float(num_procs)/ float(num_batches)) if num_procs > 1 else 1
	per_proc = num_procs if max_procs == 0 else max_procs
	est_time = round((float(t_time) * int(per_proc))/int(num_batches), 3)
	
	log_dir = os.path.join("output", "log", cir_dir)
	changing = True
	logs_prev_mem = np.zeros(num_batches)
	log_files = []
	range_b = [0, int(num_batches)]
	
	if batch_range[0] != -1:
		range_b[0] = batch_range[0]
		range_b[1] = batch_range[1]

	for i in range(range_b[0], range_b[1]):
		log_files.append(os.path.join(log_dir, "log_script_" + str(i) + ".txt"))

	changing = True
	while changing:
		if t_time < 500:
			time.sleep(5 * float(t_time))
		else:
			time.sleep(2 * float(t_time))
			 # Put script to sleep for substantial changes to take place
		any_log_changed = False
		for i, lf in enumerate(log_files):
			try:
				if os.stat(lf).st_size > logs_prev_mem[i]:
					any_log_changed = True
					logs_prev_mem[i] = os.stat(lf).st_size
			except:
				print ("Expected log " + str(i) + " not found")
				exit()
		if not any_log_changed:
			changing = False

	# Check logs for completion
	if range_b[1] == num_batches:
		range_b[1] = int(range_b[1]) - 1

	log_file_count = 0
	for i in range(range_b[0], range_b[1]):
		with open(log_files[log_file_count], "r") as lf:
			log_content = lf.read()
			count_runtimes = sum(1 for _ in re.finditer(r'\b%s\b' % re.escape("Runtime"), log_content))
			if count_runtimes != proc_per_script:
				print ("The simulations in script_" + str(i) + " did not complete")
				exit()
		log_file_count += 1
		
	if range_b[1] == num_batches - 1:
		with open(log_files[-1], "r") as lf:
			log_content = lf.read()
			proc_per_script = num_procs - ((num_batches - 1) * proc_per_script)
			count_runtimes = sum(1 for _ in re.finditer(r'\b%s\b' % re.escape("Runtime"), log_content))
			if count_runtimes != proc_per_script:
				print ("The simulations in script_" + str(num_batches - 1) + " did not complete")
				exit()

	add_amps_cmd = "./python_scripts/add_amps.py " + str(cir_dir) + " " + str(num_idx)
	if binary_vectors_only:
		add_amps_cmd += " --binary_vectors_only"
	if not_final_amps:
		add_amps_cmd += " --not_final_amps"		
	print(add_amps_cmd) 
	os.system(add_amps_cmd)

	report_cmd = "./python_scripts/dist_sim_report_gen.py " + cir_dir + " --est_time " + str(est_time)\
	 + " --max_procs " + str(max_procs)
	if test_fid:
		report_cmd += " --test_fid"
	if no_checkpoint_with_ranges:
		report_cmd += " --no_checkpoint_with_ranges"
	if save_checkpoint_to_file:
		report_cmd += " --save_checkpoint_to_file " + str(save_checkpoint_to_file)
	
	print(report_cmd)
	os.system(report_cmd)

	
if __name__ == "__main__":
    main()	