#!/usr/bin/env python3
"""Script for distributed execution"""

import os
import click
from itertools import product
import numpy as np

@click.command()
@click.argument("num_cz", nargs=1)
@click.argument("num_idx", nargs=1)
@click.option("--command", nargs=1, required=True)
@click.option("--print_all", nargs=1, required=False, default=-1)
@click.option("--seed", nargs=1, required=False, default=-1)
@click.option("--idx_file", nargs=1, required=False, default="")
@click.option("--p_idx", nargs=1, required=False, default=-1)
def main(num_cz, num_idx, command, print_all, seed, idx_file, p_idx):

	cz_bits_strings = []
	for bit_comb in product(range(2), repeat=int(num_cz)):
		bit_str = ""
		for char in bit_comb:
			bit_str = bit_str + str(char)
		cz_bits_strings.append(bit_str)

	if int(print_all) != -1:
		with open(idx_file, "w") as f:
			for q in range(1 << print_all):
				f.write(str(q) + "\n")

	temp_dir = "test_" + str(os.getpid())
	command = command + " -o " + temp_dir
	if int(seed) != -1:
		command += " -x " + seed + "," + num_idx
		if p_idx != -1:
			command += "+"
		command += " -c "
	elif idx_file != "":
		command += " -x " + idx_file + " -c "
	else:
		command += " -x " + 7 + "," + num_idx
		if int(p_idx) != -1:
			command += "+"
		command += " -c "

	for cz_bits_str in cz_bits_strings:
		print(command + cz_bits_str)
		os.system(command + cz_bits_str)

	res_amps = []
	for i in range(int(num_idx)):
		res_amps.append(0);
	
	outdir = "output/amp_vectors/" + temp_dir;
	for filename in os.listdir(outdir):
		if filename.endswith(".amps"):
			with open(os.path.join(outdir, filename), "r") as f:
				lines = f.readlines()
				temp = np.loadtxt(lines, dtype=complex)
			for i, amp in enumerate(temp):
				res_amps[i] += amp;

	with open("output/amp_vectors/res_output_" + str(os.getpid()) + ".txt", "w") as f:
		for amp in res_amps:
			f.write(str(amp) + "\n")

if __name__ == "__main__":
    main()