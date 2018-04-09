#!/usr/bin/env python3

import os 
import matplotlib.pyplot as plt
import numpy as np
from scipy.stats import norm
from scipy.stats import gaussian_kde
import click
from math import log10

# Define a function for a density plot
def densityplot(x_data, density_est, x_label, y_label, title, inputfile, qubits):
    _, ax = plt.subplots()
    plt.axvline(np.mean((x_data - 1)/pow(2, qubits)), color='red', linestyle='dashed', linewidth=2)

    for i, d in enumerate(density_est):
    	ax.plot(x_data, d(x_data), lw = 2)#, label=inputfile[i])
    	# plt.yscale('log')
    	# plt.xscale('log')

    ax.set_ylabel(y_label)
    ax.set_xlabel(x_label)
    ax.set_title(title)
    handles, labels = ax.get_legend_handles_labels()
    ax.legend(handles[::-1], labels[::-1])
    plt.savefig(inputfile[0] + '.png')
    plt.close()

@click.command()
@click.argument("title", nargs=1)
@click.argument("qubits", nargs=1)
@click.argument("input_files", nargs=-1)
@click.option("--fid", nargs=1, default=1.0)
def main(title, input_files, qubits, fid):

	arrs = []
	for file in input_files:
		print(file)
		with open(file, "r") as f:
			lines = f.readlines()
			arrs.append(np.loadtxt(lines, dtype=complex))

	for arr in arrs:
		norm_amps = np.linalg.norm(arr)
		for i, a in enumerate(arr):
			arr[i] = ((a * a.conjugate())) *  pow(2, int(qubits)) * float(fid)
			if int(arr[i]) < 1.0 and int(arr[i]) != 0:
				arr[i] = log10(arr[i])

	density_est = []
	
	for i, arr in enumerate(arrs):
		density_est.append(gaussian_kde(arr))
	# Control the 'smoothness'of the estimate. Higher values give
	# smoother estimates.
		# density_est[i].covariance_factor = lambda : 0.01
		# density_est[i]._compute_covariance()
	x_data = np.linspace(-10,5,200)

	# Call the function to create plot
	densityplot( x_data = x_data
	            , density_est = density_est
	            , x_label = 'Np'
	            , y_label = 'Frequency'
	            , title = "PDF after simulating a " + title + " circuit"
	            , inputfile = input_files
	            , qubits = int(qubits))

if __name__ == "__main__":
    main()