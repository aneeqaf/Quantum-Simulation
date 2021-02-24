#!/usr/bin/env python3
"""Script for adding amplitudes from the output files of the simulator"""

import os
import click
import numpy as np
import mmap
import time

def read_1D_amps_file(approx_file):
	approx_amps = list()
	with open(approx_file, "r") as f:
		line_num = 0
		first_float = 0
		for line in f:
			if line_num % 2 == 0:
				first_float = float(line)
			if line_num % 2 == 1:
				# print(complex(first_float, float(line)))
				approx_amps.append(complex(first_float, float(line)))
			line_num += 1
	return approx_amps

def read_2D_amps_file(approx_file, size):
	approx_amps_temp = list()
	approx_amps = list()

	with open(approx_file, "r") as f:
		lines = f.readlines()
		approx_amps_temp = np.loadtxt(lines, dtype=complex)

	for i in range(0, size):
		approx_amps.append(complex(approx_amps_temp[i], approx_amps_temp[size + i]))

	print(len(approx_amps))
	return approx_amps


@click.command()
@click.argument("exact_file", nargs=1)
@click.argument("approx_file", nargs=1)
@click.argument("file_type", default="c")
def main(exact_file, approx_file, file_type):

	if not os.path.isfile(exact_file) or not os.path.isfile(approx_file):
		return float('nan');

	exact_amps = []
	with open(exact_file, "r") as f:
		lines = f.readlines()
		exact_amps = np.loadtxt(lines, dtype=complex)

	approx_amps = []
	if file_type == "c":
		with open(approx_file, "r") as f:
			lines = f.readlines()
			approx_amps = np.loadtxt(lines, dtype=complex)
	elif file_type == "1d":
		approx_amps = read_1D_amps_file(approx_file)
	elif file_type == "2d":
		approx_amps = read_2D_amps_file(approx_file, len(exact_amps))
	else:
		print("Incorrect reading format")
		exit()
	
	dotp_exact_approx = np.vdot(exact_amps, approx_amps) / (np.linalg.norm(exact_amps) * np.linalg.norm(approx_amps))
	fidelity = round(pow(np.linalg.norm(dotp_exact_approx), 2), 5)

	print("Fidelity: " + str(fidelity));
	

if __name__ == "__main__":
    main()	