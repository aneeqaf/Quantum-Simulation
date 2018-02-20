import os 
import matplotlib.pyplot as plt
import numpy as np
from scipy.stats import norm
from scipy.stats import gaussian_kde
import click
from math import log

# Define a function for a density plot
def approxModel(xdata, ydata, title, inputfiles):
	_, ax = plt.subplots()

	for i, f in enumerate(ydata):
		ax.plot(xdata, f, lw = 2, label=inputfiles[i].split("/")[2])

	ax.set_ylabel("log fidelity")
	ax.set_xlabel("simulated cycles")
	ax.set_title(title)
	handles, labels = ax.get_legend_handles_labels()
	ax.legend(handles[::-1], labels[::-1])
	plt.savefig(str("output/misc/") + title + '.pdf')
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
			fid.append(np.loadtxt(lines,  delimiter=',', usecols=(1), unpack=True, dtype=float))

	with open(input_files[0], "r") as f:
		lines = f.readlines()
		cycle = np.loadtxt(lines,  delimiter=',', usecols=(0), unpack=True, dtype=float)

	print(len(cycle))

	for i,f in enumerate(fid):
		for j, prob in enumerate(f):
			fid[i][j] = log(prob)/log(2)

	approxModel(cycle, fid, title, input_files)

if __name__ == "__main__":
    main()