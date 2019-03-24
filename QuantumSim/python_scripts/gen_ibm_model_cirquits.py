#!/usr/bin/env python3
"""Script for generating optimized IBM model circuits"""

import os
import click
import numpy as np
import time
import random

Gates = {1:"h", 2:"x_1_2", 3:"y_1_2", 4:"i", 5:"t", 6:"cz"}

def add_v_2q_gates(num_q, h):
	layer = []
	i = 0
	while i < num_q:
		g = Gates[6], i, i + h
		layer.append(g)

		i = i + 1
		if i and i % h == 0:
			i = i + h 

	return layer

def add_h_2q_gates(num_q, h):
	layer = []
	i = 0
	while i < num_q:
		g = Gates[6], i, i + 1
		layer.append(g)

		i = i + 2
		if (i + 1) % h == 0:
			i = i + 1
	
	return layer

def print_circuit(circuit, num_q):

	with open(os.path.join("input", \
		"random_circuits_ibm", str(num_q) + "_" + str(len(circuit)) + "_simple.txt"), "w") as f:
		f.write(str(num_q) + "\n")
		for x in range(0, len(circuit)):
			for i in range(0, len(circuit[x])):
				if len(circuit[x][i]) == 2:
					if circuit[x][i][0] != "i":
						f.write(str(x) + " " + circuit[x][i][0] + " " + str(circuit[x][i][1]) + "\n")
				else :
					f.write(str(x) + " " + circuit[x][i][0] + " " \
						+ str(circuit[x][i][1]) + " " + str(circuit[x][i][2]) + "\n")


@click.command()
@click.argument("num_q", nargs=1, default=0)
@click.argument("depth", nargs=1, default=0)
@click.option("--h", nargs=1, required=True, default=0)
@click.option("--v", nargs=1, required=True, default=0)
@click.option("--h_cut", required=False, is_flag=True)
@click.option("--v_cut", required=False, is_flag=True)
def main(num_q, depth, h, v, h_cut, v_cut):

	circuit = [];
	layer0 = []
	for i in range(0, num_q):
		g = Gates[1], i
		layer0.append(g)
	circuit.append(layer0)

	for x in range(0, depth):
		if h_cut:
			if x + 1 % 10 == 0:
				circuit.append(add_v_2q_gates(num_q, h))
			else: 
				circuit.append(add_h_2q_gates(num_q, h))
		else:
			if x + 1 % 10 == 0:
				circuit.append(add_h_2q_gates(num_q, h))
			else: 
				circuit.append(add_v_2q_gates(num_q, h))

		layer1 = []
		layer2 = []
		for i in range(0, num_q):
			g1 = Gates[random.randrange(2,6)], i
			g2 = Gates[random.randrange(2,6)], i
			layer1.append(g1)
			# layer2.append(g2)

		circuit.append(layer1)
		# circuit.append(layer2)

		
	print_circuit(circuit, num_q)
	

if __name__ == "__main__":
    main()	