#!/usr/bin/env python3
""" Script for generating Qiskit code.
"""

import click
import os

@click.command()
@click.argument("circuit_file", nargs=1)
@click.option("--depth", nargs=1, required=False, default=0)
@click.option("--backend", nargs=1, required=False, default="state_vector")
@click.option("--num_threads", nargs=1, required=False, default=8)
def main(circuit_file, depth, backend, num_threads):

	input_file = circuit_file
	output_file = circuit_file.replace(".txt", "") + ".py"
	print(output_file)
	
	with open(output_file, "w") as outfile:
		outfile.write("#!/usr/bin/python3\n\n")
		outfile.write("\nfrom qiskit import QuantumCircuit, QuantumRegister, ClassicalRegister, Aer")
		outfile.write("\nfrom qiskit import *")
		outfile.write("\nfrom qiskit.providers.aer import *")
		outfile.write("\nimport math\n\n")
		
		with open(input_file, "r") as circuit_in:
			for line in circuit_in:
				if int(depth) != 0 and len(line.split()) > 1 and int(line.split()[0]) >= int(depth):
					break
				if "cz" in line:
					qubit1 = line.split()[2]
					qubit2 = line.split()[3]
					outfile.write("\ncircuit.cz(quantum_r[" + qubit1 + "], quantum_r[" + qubit2 + "])")
				elif "h" in line:
					qubit1 = line.split()[2]
					outfile.write("\ncircuit.h(quantum_r[" + qubit1 + "])")
				elif "t" in line:
					qubit1 = line.split()[2]
					outfile.write("\ncircuit.t(quantum_r[" + qubit1 + "])")
				elif "x" in line:
					qubit1 = line.split()[2]
					outfile.write("\ncircuit.rx(math.pi/4.0 , quantum_r[" + qubit1 + "])")
				elif "y" in line:
					qubit1 = line.split()[2]
					outfile.write("\ncircuit.ry(math.pi/4.0 , quantum_r[" + qubit1 + "])")
				else:
					num_q = line.replace("\n", "")
					outfile.write("quantum_r = QuantumRegister(" + num_q + ")")
					outfile.write("\nclassical_r = ClassicalRegister(" + num_q + ")")
					outfile.write("\ncircuit = QuantumCircuit(quantum_r, classical_r)\n")

					if backend == "ibmq_qasm":
						outfile.write("\nIBMQ.save_account('')\n")
		
			outfile.write("\n\n# Get backend")
			outfile.write("\nbackend_options = {'precision': 'single', 'max_parallel_threads':" + str(num_threads) + ", 'fusion_enable':True}")
			if backend == "state_vector":
				outfile.write("\nbackend = StatevectorSimulator()")
			elif backend == "local_qasm":
				outfile.write("\nbackend = QasmSimulator()")
			elif backend == "ibmq_qasm" :
				outfile.write("\nprovider = IBMQ.load_account()")
				outfile.write("\nbackend =  provider.get_backend('ibmq_qasm_simulator')")
			else:
				print("Incorrect backend")
				exit(1)
	
			outfile.write("\n\n# Transpile circuit and assemble QOBJ")
			outfile.write("\nqobj = assemble(transpile(circuit, backend), shots=1, **backend_options)")

			outfile.write("\n\n# Run simulation")
			if backend == "ibmq_qasm":
				outfile.write("\ntry:")
				outfile.write("\n\tjob_sim = backend.run(qobj)")
				outfile.write("\n\tprint(job_sim.status())")
				outfile.write("\n\tresult_sim = job_sim.result()")
				outfile.write("\n\tprint(\"Simulation: \", result_sim.status)")
				outfile.write("\n\tprint(\"Time Taken (s): \", result_sim.time_taken)")
				outfile.write("\nexcept:\n\tprint(\"All devices are currently unavailable.\")")
			else:
				outfile.write("\nresult_sim = backend.run(qobj).result()")
				outfile.write("\nprint(\"Simulation: \", result_sim.status)")
				outfile.write("\nprint(\"Time Taken (s): \", result_sim.time_taken)")

			if backend == "state_vector" :
				outfile.write("\n\nstate_v = job_sim.result().get_statevector(circuit)")
				outfile.write("\nprint(\"amp[3] = \" + str(state_v[3]))")
				outfile.write("\nprint(\"amp[1/4] = \" + str(state_v[int(math.pow(2, " + num_q + ")/4)]) )")
				outfile.write("\nprint(\"amp[1/2] = \" + str(state_v[int(math.pow(2, " + num_q + ")/2)]) )")
				outfile.write("\nprint(\"amp[3/4] = \" + str(state_v[int(3 * math.pow(2, " + num_q + ")/4)]) )")
				outfile.write("\nprint(\"amp[-3] = \" + str(state_v[int(math.pow(2, " + num_q + ") - 3)]) )")


		
if __name__ == "__main__":
    main()


