#!/usr/bin/env python3
""" Script for generating Q# code.
"""

import click
import os

@click.command()
@click.argument("circuit_file", nargs=1)
@click.option("--depth", nargs=1, required=False, default=0)
def main(circuit_file, depth):

	input_file = os.path.join("input", "random_circuits_google", circuit_file)
	output_file = os.path.join("Quantum", "msftBM", circuit_file.replace(".txt", "") + ".qs")
	print(output_file)
	num_q = ""

	with open(output_file, "w") as outfile:
		outfile.write("namespace Microsoft.Quantum.msftBM {\n")
		outfile.write("\topen Microsoft.Quantum.Primitive;\n\topen Microsoft.Quantum.Canon;")
		outfile.write("\n\topen Microsoft.Quantum.Extensions.Math;")
		outfile.write("\n\n\toperation simulate_" + circuit_file.replace(".txt", "") + " () : () {\n\t\tbody {")

		with open(input_file, "r") as circuit_in:
			for line in circuit_in:
				if int(depth) != 0 and len(line.split()) > 1 and int(line.split()[0]) > int(depth):
					break
				if "cz" in line:
					qubit1 = line.split()[2]
					qubit2 = line.split()[3]
					outfile.write("\n\t\t\t\tCZ(qubits[" + qubit1 + "], qubits[" + qubit2 + "]);")
				elif "h" in line:
					qubit1 = line.split()[2]
					outfile.write("\n\t\t\t\tH(qubits[" + qubit1 + "]);")
				elif "t" in line:
					qubit1 = line.split()[2]
					outfile.write("\n\t\t\t\tT(qubits[" + qubit1 + "]);")
				elif "x" in line:
					qubit1 = line.split()[2]
					outfile.write("\n\t\t\t\tRx(PI()/2.0 , qubits[" + qubit1 + "]);")
				elif "y" in line:
					qubit1 = line.split()[2]
					outfile.write("\n\t\t\t\tRy(PI()/2.0 , qubits[" + qubit1 + "]);")
				else:
					num_q = line.replace("\n", "")
					outfile.write("\n\t\t\tlet num_qubits = " + num_q + ";")
					outfile.write("\n\t\t\tmutable results = new Result[num_qubits];")
					outfile.write("\n\n\t\t\tusing (qubits = Qubit[num_qubits]) {")

		outfile.write("\n\n\t\t\t\tset results = MultiM(qubits);")
		outfile.write("\n\n\t\t\t\tMessage($\"Measurement for {num_qubits} qubits simulation :\");")
		outfile.write("\n\t\t\t\tfor (q in 0..(num_qubits - 1)) { ")
		outfile.write("\n\t\t\t\t\tMessage($\"{q} : {results[q]}\");\n\t\t\t\t}")
		outfile.write("\n\t\t\t\tfor (q in 0..(num_qubits - 1)) { ")
		outfile.write("\n\t\t\t\t\tif (results[q] == One) {\n\t\t\t\t\t\tX(qubits[q]);\n\t\t\t\t\t} ")
		outfile.write("\n\t\t\t\t}\n\t\t\t}\n\t\t}\n\t}\n}")

if __name__ == "__main__":
    main()

# set results = MultiM(qubits);
# for (q in 0..num_qubits) {
# 	if (results[q] == One) {
#         X(qubits[q]);
#     }
# }

