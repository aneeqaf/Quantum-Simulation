#!/usr/bin/env python3
"""Script for adding amplitudes from the output files of the simulator"""

import os
import click
import numpy as np

@click.command()
@click.argument("cir_dir", nargs=1)
@click.argument("num_idx", nargs=1)
def main(cir_dir, num_idx):

	outdir = os.path.join("output", "amp_vectors", cir_dir)
	files = os.listdir(outdir)

	amps = np.zeros(int(num_idx), dtype=complex)

	for filename in files:
		if filename.endswith("_ascii.amps"):
			full_filename = os.path.join(outdir, filename)
			print(full_filename)
			with open(full_filename, "r") as f:
				lines = f.readlines()
				temp = np.loadtxt(lines, dtype=complex)

			for i, amp in enumerate(temp):
				amps[i] += amp;

	with open(os.path.join(outdir, "result.amps"), "w") as f:
		for a in amps:
			f.write(str(a).replace("(", "").replace(")","") + "\n")

if __name__ == "__main__":
    main()	