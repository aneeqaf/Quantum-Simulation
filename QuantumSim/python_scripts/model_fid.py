import os 
import matplotlib.pyplot as plt
import numpy as np
from scipy.stats import norm
from scipy.stats import gaussian_kde
import click
from math import log

# Define a function for a density plot
def approxModel(xdata, ydata, title):
	_, ax = plt.subplots()
	ax.plot(xdata, ydata, "o-" ,label='parametric curve', markevery=1000)

	ax.set_ylabel("fidelity")
	ax.set_xlabel("cycles")
	ax.set_title(title)
	handles, labels = ax.get_legend_handles_labels()
	ax.legend(handles[::-1], labels[::-1])
	plt.savefig(title + '.png')
	plt.close()

@click.command()
@click.argument("title", nargs=1)
@click.argument("input_files", nargs=-1)
def main(title, input_files):

	cycle = []
	fid = []
	for file in input_files:
		print(file)
		with open(file, "r") as f:
			lines = f.readlines()
			cycles, fid = np.loadtxt(lines,  delimiter=',', usecols=(0, 1), unpack=True, dtype=float)

	for i,f in enumerate(fid):
		fid[i] = log(f)/log(2)

	approxModel(cycles, fid, title)

if __name__ == "__main__":
    main()