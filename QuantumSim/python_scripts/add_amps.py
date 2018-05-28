#!/usr/bin/env python3
"""Script for adding amplitudes from the output files of the simulator"""

import os
import click
import numpy as np
import mmap
import time

@click.command()
@click.argument("cir_dir", nargs=1)
@click.argument("num_idx", nargs=1)
@click.option("--binary_vectors_only", nargs=1, required=False, is_flag=True)
@click.option("--not_final_amps", nargs=1, required=False, is_flag=True)
@click.option("--add_partial_res_amps", nargs=1, required=False, is_flag=True)
def main(cir_dir, num_idx, binary_vectors_only, not_final_amps, add_partial_res_amps):

	outdir = os.path.join("output", "amp_vectors", cir_dir)
	files = os.listdir(outdir)

	amps = np.zeros(int(num_idx) + 5, dtype=complex)
	
	for filename in files:
		if filename.endswith("_ascii.amps") or binary_vectors_only or "result" in filename:
			if binary_vectors_only and (filename.endswith("_ascii.amps")
			 or "result" in filename):
				continue
			if add_partial_res_amps and "result" not in filename:
				continue

			full_filename = os.path.join(outdir, filename)
			print(full_filename)

			if binary_vectors_only:
				lines = b''
				with open(full_filename, "r+b") as f:
					mm = mmap.mmap(f.fileno(), 0)
					for l in mm:
						lines += mm.readline()
					temp = np.fromstring(lines, dtype='c8')
			else:
				with open(full_filename, "r") as f:
					lines = f.readlines()
					temp = np.loadtxt(lines, dtype=complex)

			amps += temp;

	results_file = "result" + ".amps"
	if not_final_amps:
		results_file = "result" + str(time.clock()) + ".amps"
	# print(len(amps))
	# if binary_vectors_only:
	# 	with open(os.path.join(outdir, results_file), "wb") as f:
	# 		f.write(amps)
	# else : 
	with open(os.path.join(outdir, results_file), "w") as f:
		for a in amps:
			f.write(str(a).replace("(", "").replace(")","") + "\n")

if __name__ == "__main__":
    main()	