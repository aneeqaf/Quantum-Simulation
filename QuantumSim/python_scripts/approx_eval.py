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
@click.argument("fid_file", nargs=1)
def main(qubits, prob_files, amp_files, fid_file):

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

	med_exact = (float)(median(probs[0]))
	med_approx = (float)(median(probs[1]))
	amp_size = len(amps[0])

	confusion_matrix = [[0.0,0.0], [0.0,0.0]]

	for i, p in enumerate(probs[0]):
		if probs[0][i] >= med_exact and probs[1][i] >= med_approx:
			confusion_matrix[0][0] += 1.0;
		elif probs[0][i] >= med_exact and probs[1][i] < med_approx:
			confusion_matrix[0][1] += 1.0;
		
		elif probs[0][i] < med_exact and probs[1][i] >= med_approx:
			confusion_matrix[1][0] += 1.0;
		elif probs[0][i] < med_exact and probs[1][i] < med_approx:
			confusion_matrix[1][1] += 1.0;

	
	cos_sim = (float)(1.0 - spatial.distance.cosine(probs[0], probs[1]) - (1.0/pow(2,int(qubits))))
	dotp_exact_approx = np.vdot(amps[0], amps[1]) / sqrt(np.linalg.norm(amps[0]) * np.linalg.norm(amps[1]))
	fidelity = round(np.linalg.norm(dotp_exact_approx), 5)

	if not os.path.isfile(fid_file):
		with open(fid_file, "w") as file:
			file.write(prob_files[0].split("_")[3].split(".")[0] + "," + str(fidelity) + "\n")
	else:
		with open(fid_file, "a") as file:
			file.write(prob_files[0].split("_")[3].split(".")[0] + "," + str(fidelity)+ "\n")

	print("Cycle : " + prob_files[0].split("_")[3].split(".")[0])
	print("Fidelity : " + str(fidelity))
	print("Corr : " + str(np.corrcoef(probs[0], probs[1])))
	print ("Cos sim: " + str(round(cos_sim, 5)))
	print ("Approx median : " + str(med_approx));
	print ("Exact median : " + str(med_exact));
	print ("						Approx >= approx_median		Approx < approx_median")
	print ("Exact >= exact_median	   " + str (round((confusion_matrix[0][0]/amp_size)*100, 5)) + "%\t\t\t\t\t\t" + \
		str (round((confusion_matrix[0][1]/amp_size)*100, 5)) + "%")

	print ("Exact < exact_median	   " + str (round((confusion_matrix[1][0]/amp_size)*100, 5)) + "%\t\t\t\t\t\t" + \
		str (round((confusion_matrix[1][1]/amp_size)*100, 5)) + "%")
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

