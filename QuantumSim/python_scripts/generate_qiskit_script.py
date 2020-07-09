#!/usr/bin/env python3
""" Script for generating Qiskit code.
"""

import click
import os

@click.command()
@click.argument("circuit_file", nargs=1)
@click.option("--depth", nargs=1, required=False, default=0)
@click.option("--backend", nargs=1, required=False, default="local_qasm")
@click.option("--num_threads", nargs=1, required=False, default=0)
@click.option("--max_memory", nargs=1, required=False, default=16)
def main(circuit_file, depth, backend, num_threads, max_memory):

	input_file = os.path.join("input", "random_circuits_google", circuit_file)
	output_file = os.path.join("qiskit-terra", "ibm-bm", circuit_file.replace(".txt", "") + ".py")
	print(output_file)
	num_q = ""

	with open(output_file, "w") as outfile:
		outfile.write("#!/usr/bin/python3\n\n")
		outfile.write("\nfrom qiskit import QuantumCircuit, QuantumRegister, ClassicalRegister")
		outfile.write("\nfrom qiskit import *")
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
					# outfile.write("\ncircuit.rz(math.pi/4.0 , quantum_r[" + qubit1 + "])")
					outfile.write("\ncircuit.rx(math.pi/2.0 , quantum_r[" + qubit1 + "])")
					# outfile.write("\ncircuit.rz(math.pi/4.0 , quantum_r[" + qubit1 + "])")
				elif "y" in line:
					qubit1 = line.split()[2]
					#outfile.write("\ncircuit.rz(math.pi/4.0 , quantum_r[" + qubit1 + "])")
					outfile.write("\ncircuit.ry(math.pi/2.0 , quantum_r[" + qubit1 + "])")
					# outfile.write("\ncircuit.rz(math.pi/4.0 , quantum_r[" + qubit1 + "])")				
				else:
					num_q = line.replace("\n", "")
					outfile.write("quantum_r = QuantumRegister(" + num_q + ")")
					outfile.write("\nclassical_r = ClassicalRegister(" + num_q + ")")
					outfile.write("\ncircuit = QuantumCircuit(quantum_r, classical_r)")

					if backend == "ibmq_qasm":
						outfile.write("\nIBMQ.save_account('ce1bbfc4d3a0dbd544d79a42905b3dbd691e56bfba14803643dc0f5c25db37a92cddebae0274a0dea2546cc5ce6906ec7e0198398f01369d64030309d01aa909')\n")
						
			if num_threads:
				outfile.write("\n\nhpc_dict = {}\nhpc_dict['omp_num_threads'] = " + str(num_threads) +  
				"\nhpc_dict['multi_shot_optimization'] = True")

			if max_memory:
				outfile.write("\n\nconfig_dict = {}\nconfig_dict['max_memory'] = " + str(max_memory))
				
			if backend == "local_qasm" :
				outfile.write("\n\nsimulator = BasicAer.get_backend('qasm_simulator')\njob_sim = execute(circuit, simulator)")
				outfile.write("\nprint(job_sim.status)")
				outfile.write("\nsim_result = job_sim.result()")
				outfile.write("\nprint(sim_result.get_counts(circuit))")
				outfile.write("\nprint(\"simulation: \", sim_result)")
				outfile.write("\nprint(job_sim.status)")
			elif backend == "ibmq_qasm" :
				outfile.write("\n\nprovider = IBMQ.load_account()")
				outfile.write("\ntry:\n\tsimulator =  provider.get_backend('ibmq_qasm_simulator')")
				outfile.write("\n\tjob_sim = execute(circuit, simulator)")
				outfile.write("\n\tprint(job_sim.status)")
				outfile.write("\n\tsim_result = job_sim.result()")
				outfile.write("\n\tprint(sim_result.get_counts(circuit))")
				outfile.write("\n\tprint(\"simulation: \", sim_result)")
				outfile.write("\n\tprint(job_sim.status)")
				outfile.write("\nexcept:\n\tprint(\"All devices are currently unavailable.\")")
			else :
				print("Incorrect backend")
				exit(1)

			if backend == "state_vector" :
				outfile.write("state_v = sim_result.get_statevector(circuit)\n")
				outfile.write("\nprint(\"amp[3] = \" + str(state_v[3]))")
				outfile.write("\nprint(\"amp[1/4] = \" + str(state_v[int(math.pow(2, " + num_q + ")/4)]) )")
				outfile.write("\nprint(\"amp[1/2] = \" + str(state_v[int(math.pow(2, " + num_q + ")/2)]) )")
				outfile.write("\nprint(\"amp[3/4] = \" + str(state_v[int(3 * math.pow(2, " + num_q + ")/4)]) )")
				outfile.write("\nprint(\"amp[-3] = \" + str(state_v[int(math.pow(2, " + num_q + ") - 3)]) )")


		
if __name__ == "__main__":
    main()

# set results = MultiM(qubits);
# for (q in 0..num_qubits) {
# 	if (results[q] == One) {
#         X(qubits[q]);
#     }
# }

