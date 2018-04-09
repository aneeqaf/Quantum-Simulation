import os 
import matplotlib.pyplot as plt
import numpy as np
from scipy.stats import norm
from scipy.stats import gaussian_kde
import click
from math import log

# Define a function for a density plot
def plotModel(xdata, ydata, title, inputfiles):
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
@click.argument("norm_perc", nargs=1)
@click.argument("norm_depth", nargs=1)
def main(norm_perc, norm_depth):
