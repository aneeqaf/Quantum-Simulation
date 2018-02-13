import os 
import numpy as np
from scipy import spatial, stats
from scipy.stats import linregress
import click
from math import sqrt

@click.command()
@click.argument("qubits", nargs=1)
@click.argument("prob_files", nargs=2)
@click.argument("amp_files", nargs=2)
def main(qubits, prob_files, amp_files):

	probs = []
	for file in prob_files:
		with open(file, "r") as f:
			lines = f.readlines()
			probs.append(np.loadtxt(lines, dtype=complex))

	amps = []
	for file in amp_files:
		with open(file, "r") as f:
			lines = f.readlines()
			amps.append(np.loadtxt(lines, dtype=complex))

	dotp_exact_approx = np.dot(amps[0], amps[1]) / sqrt(np.linalg.norm(amps[0]) * np.linalg.norm(amps[1]))
	print("Fidelity : " + str(np.linalg.norm(dotp_exact_approx)))
	print("Corr : " + str(np.corrcoef(probs[0], probs[1])))
	print ("Cos sim: " + str((float)((1 - spatial.distance.cosine(probs[0], probs[1]) - (1/pow(2,int(qubits)))))))
	# slope, intercept, r_value, p_value, std_err = stats.linregress(arrs[0], arrs[1])
	# print ("slope: " + str(round(slope, 3)) + ", intercept: " + str(round(intercept, 3)) \
	# 	+ ", r_value: " + str(round(r_value, 3)))
	# print ((float)((1 - spatial.distance.cosine(arrs[0], arrs[1]) - (1/pow(2,int(qubits))))))
	print("")
	# print(linregress(arrs[0], arrs[1]))
	
if __name__ == "__main__":
    main()

# for i in range(1000):
# 		amps[0].append(1)
# 		if i % 2 == 0:
# 			amps[1].append(1)
# 		else:
# 			amps[1].append(0)

# 		probs[0].append(amps[0][i] * amps[0][i])
# 		probs[1].append(amps[1][i] * amps[1][i])
	
# 	amps[0] = amps[0]/np.linalg.norm(amps[0])
# 	amps[1] = amps[1]/np.linalg.norm(amps[1])

