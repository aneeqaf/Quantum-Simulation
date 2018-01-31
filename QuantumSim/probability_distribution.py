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
    ax.plot(x_data, density_est(x_data), color = '#539caf', lw = 2)
    plt.axvline(np.mean((x_data - 1)/pow(2, qubits)), color='red', linestyle='dashed', linewidth=2)
    plt.yscale('log')
    ax.set_ylabel(y_label)
    ax.set_xlabel(x_label)
    ax.set_title(title)
    plt.savefig(inputfile + '.png')
    plt.close()

@click.command()
@click.argument("input_files", nargs=-1)
@click.argument("qubits", nargs=1)
def main(input_files, qubits):

	arr = []
	for file in input_files:
		print(file)
		with open(file, "r") as f:
			lines = f.readlines()
			arr = np.loadtxt(lines, dtype=float)

	for i, a in enumerate(arr):
		if int(a) < 1.0:
			arr[i] = log10(a)

	x_size = pow(2, int(qubits))

	density_est = gaussian_kde(arr)
	# Control the 'smoothness'of the estimate. Higher values give
	# smoother estimates.
	# density_est.covariance_factor = lambda : .0001
	# density_est._compute_covariance()
	x_data = np.arange(-10, 10, 1)

	# Call the function to create plot
	densityplot( x_data = x_data
	            , density_est = density_est
	            , x_label = 'Np'
	            , y_label = 'Frequency'
	            , title = 'Distribution function of ' + str(qubits) + " qubits circuit"
	            , inputfile = input_files[0]
	            , qubits = int(qubits))

if __name__ == "__main__":
    main()
