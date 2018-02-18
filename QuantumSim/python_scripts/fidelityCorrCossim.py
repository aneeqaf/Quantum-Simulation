import os 
import numpy as np
from scipy import spatial, stats
from scipy.stats import linregress
from statistics import median
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

	amps[0] = amps[0]/np.linalg.norm(amps[0])
	amps[1] = amps[1]/np.linalg.norm(amps[1])

	med_exact = median(probs[0])
	med_approx = median(probs[1])
	amp_size = len(amps[0])

	confusion_matrix = [[[0, 0],[0,0]], [[0, 0],[0,0]]]

	for i, p in enumerate(probs[0]):
		if probs[0][i] >= med_exact && probs[0][i] >= med_approx:
			confusion_matrix[0][0][0]++;
		else if probs[0][i] >= med_exact && probs[0][i] < med_approx:
			confusion_matrix[0][1][0]++;
		else if probs[0][i] < med_exact && probs[0][i] >= med_approx:
			confusion_matrix[1][0][0]++;
		else if probs[0][i] < med_exact && probs[0][i] < med_approx:
			confusion_matrix[1][1][0]++;

		if probs[1][i] >= med_exact && probs[1][i] >= med_approx:
			confusion_matrix[0][0][1]++;
		else if probs[1][i] >= med_exact && probs[1][i] < med_approx:
			confusion_matrix[0][1][1]++;
		else if probs[1][i] < med_exact && probs[1][i] >= med_approx:
			confusion_matrix[1][0][1]++;
		else if probs[1][i] < med_exact && probs[1][i] < med_approx:
			confusion_matrix[1][1][1]++;


	dotp_exact_approx = np.vdot(amps[0], amps[1]) / sqrt(np.linalg.norm(amps[0]) * np.linalg.norm(amps[1]))
	print("Cycle : " + prob_files[0].split("_")[3])
	print("Fidelity : " + str(np.linalg.norm(dotp_exact_approx)))
	print("Corr : " + str(np.corrcoef(probs[0], probs[1])))
	print ("Cos sim: " + str((float)((1 - spatial.distance.cosine(probs[0], probs[1]) - (1/pow(2,int(qubits)))))))
	print ("				Approx >= median		Approx < median")
	print ("Exact >= median	   " + str (confusion_matrix[0][0][0]/amp_size*100) + "%, " + \
		str ((confusion_matrix[0][0][1]/amp_size)*100) + "%		" + \
		str ((confusion_matrix[0][1][0]/amp_size)*100) + "%, " + \
		str ((confusion_matrix[0][1][1]/amp_size)*100) + "%")

	print ("Exact < median	   " + str (confusion_matrix[1][0][0]/amp_size*100) + "%, " + \
		str ((confusion_matrix[1][0][1]/amp_size)*100) + "%		" + \
		str ((confusion_matrix[1][1][0]/amp_size)*100) + "%, " + \
		str ((confusion_matrix[1][1][1]/amp_size)*100) + "%")
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

