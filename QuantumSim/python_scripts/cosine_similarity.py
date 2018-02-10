import os 
import numpy as np
from scipy import spatial, stats
from scipy.stats import linregress
import click

@click.command()
@click.argument("qubits", nargs=1)
@click.argument("input_files", nargs=2)
def main(qubits, input_files):

	arrs = []
	for file in input_files:
		with open(file, "r") as f:
			lines = f.readlines()
			arrs.append(np.loadtxt(lines, dtype=complex))

	# slope, intercept, r_value, p_value, std_err = stats.linregress(arrs[0], arrs[1])
	# print ("slope: " + str(round(slope, 3)) + ", intercept: " + str(round(intercept, 3)) \
	# 	+ ", r_value: " + str(round(r_value, 3)))
	# print ((float)((1 - spatial.distance.cosine(arrs[0], arrs[1]) - (1/pow(2,int(qubits))))))
	print((np.corrcoef(arrs[0], arrs[1])))
	print("")
	# print(linregress(arrs[0], arrs[1]))
	
if __name__ == "__main__":
    main()
