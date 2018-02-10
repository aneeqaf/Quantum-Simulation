import os 
import numpy as np
from scipy import spatial
import click
import csv
from mpl_toolkits.mplot3d import Axes3D
import matplotlib.pyplot as plt

def approxModel(xdata, ydata, zdata, title):
	_, ax = plt.subplots(projection='3d')

	for cs in ydata:
  		ax.plot(xdata, cs, zdata, label='parametric curve')
  		plt.yscale("log")

	ax.set_ylabel("cosine similarity")
	ax.set_xlabel("cycles")
	ax.set_zlabel("gates count")
	ax.set_title(title)
	handles, labels = ax.get_legend_handles_labels()
	ax.legend(handles[::-1], labels[::-1])
	plt.savefig(title + '.png')
	plt.close()

def main():

	cycles = []
	CZ = []
	T = []
	XY = []
	xCZH = []
	xCZV = []
	approx_gate_data = [] #list of list of dictionaries.
	approx_CS_data = []
	approx_XE_data = []
	CS_input_files = ["output/misc/CS_approx2cuts.txt",  "output/misc/CS_approx1cutH.txt", \
	"output/misc/CS_approx1cutV.txt", "output/misc/CS_approx2011.txt"]
	csv_files = ["output/misc/g_30q_41d_approx2cuts.txt", "output/misc/g_30q_41d_approx1cutH.txt", \
	"output/misc/g_30q_41d_approx1cutV.txt", "output/misc/g_30q_41d_approx2011.txt"]
	XE_input_files = ["output/misc/XE_approx2cuts.txt", "output/misc/XE_approx1cutH.txt", \
	 "output/misc/XE_approx1cutV.txt",  "output/misc/XE_approx2011.txt"]

	for czv in czv_files:
		cz = [], t = [], xy = [], xczh = [], xczv = [], c = []
		with open(csv) as csvfile:
			readCSV = csv.reader(csvfile, delimiter=',')
			for row in readCSV:
				c.append(row[0])
				cz.append(row[1])
				t.append(row[2])
				xy.append(row[3])
				xczh.append(row[4])
				xczv.append(row[5])
		cycles.append(c)
		CZ.append(cz)
		T.append(t)
		XY.append(xy)
		xCZH.append(xczh)
		xCZV.append(xczv)
	
	for file in CS_input_files:
		with open(file, "r") as f:
			lines = f.readlines()
			approx_CS_data.append(np.loadtxt(lines, dtype=complex))

	for file in CS_input_files:
		with open(file, "r") as f:
			lines = f.readlines()
			approx_XE_data.append(np.loadtxt(lines, dtype=complex))
	
	
	approxModel(cycles[0], approx_CS_data, CZ[0], "Gate: CZ");
	approxModel(cycles[0], approx_CS_data, T[0], "Gate: T");
	approxModel(cycles[0], approx_CS_data, XY[0], "Gate: XY");
	approxModel(cycles[0], approx_CS_data, xCZH[0], "Gate: xCZH");
	approxModel(cycles[0], approx_CS_data, xCZV[0], "Gate: xCZV");

	for i, x in enumerate(cycles):
 		print("Cycles\tCZ\tT\tXY\txCZH\txCZV\tCos sim\tXE")
 		for j, c in enumerate(cycles[i]):
 			print(str(cycles[i][j]) + "\t" + str(CZ[i][j]) + "\t" + str(T[i][j]) + "\t" + str(XY[i][j])\
 			+ "\t" + str(xCZH[i][j]) + "\t" + str(xCZV[i][j]) + "\t" + str(approx_CS_data[i][j]) + \
 			str(approx_XE_data[i][j]))

if __name__ == "__main__":
    main()