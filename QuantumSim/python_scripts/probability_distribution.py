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
    	ax.plot(x_data, d(x_data), lw = 2, label=inputfile[i].split("/")[2])
    	plt.yscale('log')

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
def main(title, input_files, qubits):

	arrs = []
	for file in input_files:
		print(file)
		with open(file, "r") as f:
			lines = f.readlines()
			arrs.append(np.loadtxt(lines, dtype=float))

	for arr in arrs:
		for i, a in enumerate(arr):
			if int(a) < 1.0 and int(a) != 0:
				arr[i] = log10(a)

	density_est = []
	
	for arr in arrs:
		density_est.append(gaussian_kde(arr))
	# Control the 'smoothness'of the estimate. Higher values give
	# smoother estimates.
	# density_est.covariance_factor = lambda : .0001
	# density_est._compute_covariance()
	x_data = np.arange(0, 10, 1)

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