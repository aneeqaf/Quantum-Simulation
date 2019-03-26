#!/usr/bin/env python3
"""Script for finding heavy outputs"""

import os
import click
import numpy as np
import time
from math import pow, log

@click.command()
@click.argument("num_q", nargs=1, default=0)
@click.option("--amps_file", nargs=1, required=True, default="")
@click.option("--idx_file", nargs=1, required=True, default="")
@click.option("--approx_amps_file", nargs=1, required=True, default="")
@click.option("--approx_idx_file", nargs=1, required=True, default="")
@click.option("--print_heavy_idx", required=False, default=0)
def main(num_q, amps_file, idx_file, approx_amps_file, approx_idx_file, print_heavy_idx):

	analytical_median = (1.0/pow(2.0, num_q)) * log(2.0)

	amps = []
	with open(os.path.join(amps_file), "r") as f:
		lines = f.readlines()
		amps = np.loadtxt(lines, dtype=complex)

	idxs = []
	if idx_file != "":
		with open(idx_file, "r") as f:
			lines = f.readlines()
			idxs = np.loadtxt(lines, dtype=int)

	if (approx_idx_file == "") ^ (approx_amps_file == ""):
		print("Please specify both the idx and amp approx files")
		exit()

	approx_idxs = []
	approx_amps = []
	approx_median = 0
	if approx_idx_file != ""  and approx_amps_file != "":
		with open(approx_idx_file, "r") as f:
			lines = f.readlines()
			approx_idxs = np.loadtxt(lines, dtype=int)

		with open(approx_amps_file, "r") as f:
			lines = f.readlines()
			approx_amps = np.loadtxt(lines, dtype=complex)

		approx_amps = approx_amps * np.conjugate(approx_amps)
		print(approx_amps)
		approx_median = np.nanpercentile(approx_amps, q = 75).real
		
	heavy_exact = 0
	heavy_approx_n_heavy_exact = 0
	heavy_approx = 0
	idx = 0
	heavy_idxs = {}
	for a_exact in amps:
		if (a_exact * complex(a_exact.real, -a_exact.imag)).real > analytical_median:
			heavy_idxs[idxs[idx]] = 0
			
			if print_heavy_idx == 1 or print_heavy_idx == 3:
				with open(os.path.join(os.path.dirname(amps_file), ""), "w+") as f:
					f.write(str(idxs[idx]) + "\n")
			
			heavy_exact = heavy_exact + 1
		idx = idx + 1
	
	idx = 0
	if approx_idx_file != "":
		for a_approx in approx_amps:
			if a_approx.real > approx_median:
				if approx_idxs[idx] in heavy_idxs:
					heavy_approx_n_heavy_exact = heavy_approx_n_heavy_exact + 1
				
				if print_heavy_idx == 2 or print_heavy_idx == 3:
					with open(os.path.join(os.path.dirname(approx_amps_file), ""), "w+") as f:
						f.write(str(approx_idxs[idx]) + "\n")
				
				heavy_approx = heavy_approx + 1
				
			idx = idx + 1

	
	print ("Anayltical median: " + str(analytical_median))
	if approx_amps_file != "" :
		print ("Approx simulation median: " + str(approx_median))
	print ("|H_exact|: " + str(heavy_exact))
	print ("|H_approx|: " + str(heavy_approx))
	print ("|H_exact| / num amps: " + str(round(heavy_exact/len(amps), 5)))
	if approx_amps_file != "" :
		print ("|H_approx n H_exact| / num amps: " + str(round(heavy_approx_n_heavy_exact/len(approx_amps), 5)))
		print ("|H_approx n H_exact|/|H_approx|: " + str(round(heavy_approx_n_heavy_exact/heavy_approx, 5)))
		print ("|H_approx n H_exact|/|H_exact|: " + str(round(heavy_approx_n_heavy_exact/heavy_exact, 5)))

if __name__ == "__main__":
    main()	