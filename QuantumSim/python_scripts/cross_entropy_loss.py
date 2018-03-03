import os 
import numpy as np
from scipy import spatial
import click
from math import log2
from sklearn.metrics import log_loss

@click.command()
@click.argument("qubits", nargs=1)
@click.argument("input_files", nargs=2)
def main(qubits, input_files):

	amp_size = pow(2, int(qubits))
	arrs = []
	for file in input_files:
		with open(file, "r") as f:
			lines = f.readlines()
			arrs.append(np.loadtxt(lines, dtype=complex))

	xe = 0.0
	for i, _ in enumerate(arrs[0]):
		arrs[1][i] = arrs[1][i] / amp_size
		arrs[0][i] = arrs[0][i] / amp_size
		if (arrs[1][i]) != 0:
			xe += arrs[0][i] * log2(arrs[1][i])

	print((float)(-xe))

	# xel = 0.0
	# for i, _ in enumerate(arrs[0]):
	# 	if (arrs[1][i] * np.conj(arrs[1][i])) != 0:
	# 		xel += (arrs[0][i] * np.conj(arrs[0][i])) * log2((arrs[1][i] * np.conj(arrs[1][i]))) 

	# print("Cross entropy (" + input_files[0].split("/")[2] + ", " +
	# 	input_files[1].split("/")[2] + ") : " + str(-xel / len(arrs[0])))
	# print("")

if __name__ == "__main__":
    main()