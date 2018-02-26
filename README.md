# Rollright - A Quantum Simulator

Rollright is a Schrondinger-style quantum simulator that is particularly efficient on Google quantum-supremacy circuits. In addition, to a full state-vector representation, it also uses sum-of-tensors representations. 

## Run the Simulator

### Command line options
* **--CZ_path, -c**
	* Must be followed by either a bit string of 0s and 1s or two integers separated by a comma - length of CZ bitstring and value of the bitstring. 
	* Defines the CZ path for CZ gates that cross partitions for distributed simulation.
* **--depth, -d** 
	* Must be followed by an integer to specify the number of cycles to simulate
	* The default setting simulates all cycles specified in the input. 
	* This option is only relevant when reading from input files.
* **--google_spec, -g**
	* Must be followed by two arguments - number of qubits and total depth of the circuit.
	* It is used to generate Google-style circuits.
* **--google_input, -p**
	* Must be followed by a filename of a circuit in the Google format.
	* The file is assumed to be in `input\random_circuits_google`.
* **--hcut, --b**
	* Must be followed by an integer that specifies one side of a horizontal cut in a circuit.
	* The program calculates the size of the other partition given number of qubits in the first partition.
	* If not specified, then the sum-of-tensors simulation calculates the best horizontal cut. 
* **--help, --h**
	* Help option for command line options.
* **--inputfile, -i**
	* Must be followed by a filename (currently only reads the file in the Rollright format).
	* The file is assumed to be in `input\random_circuits_rollright`.
* **--idx, -x**
	* Must be followed by either a filename or two integers separated by a comma - seed and number of indices.
	* Optionally, end the two integers argument with a '+' to print the generated indices to a file.
	* The program prints the values of the amplitudes in the specified indices to a file in `output\amp_vectors\<dirname>\output_<pid>.amps` or `output\amp_vectors\output_<pid>.amps`. `<dirname>` can be specified using the -o option. Providing a `<dirname>` is recommended for distributed simulation to simplify recollection of files for computations.
	* Generated indices are printed in `output\amp_vectors\<dirname>\output_<pid>.idx` or `output\amp_vectors\output_<pid>.idx`
* **--num_threads, -t**
	* Must be followed by an integer.
	* Sets a maximum limit on the number of threads to be used 
* **--outfile, -o**
	* Must be followed by a filename.
	* The file specified is always created in the `output` directory, except when creating new random cicuits that are written to `input\random_circuits_rollright`. 
	* The argument only includes the name of a file, not a path. The filename is used to write to predetermined locations pertinent to the type of output. (Namely, `output\amp_vectors`, `output\probabilities`, `output\qpro_scripts`, and `output\reports`.)
* **--sim_type, -s**
	* Must be followed by an integer between 0 and 8 that specifies the type of simulation.
	* The simulation types supported (in order): LosslessH, LosslessV, Approx1CutH, Approx1CutV, Approx2Cuts, FullState, Approx2011, Approx1_101, Approx1110. Default simulation type is FullState. 
* **--FTthreshold, -f**
	* Must be followed by an integer that is smaller than the total number of qubits (default is 14). It controls the base case of the Recursive Transform algorithm for X and Y gates.
* **--vcut, -a**
	* Must be followed by an integer that specifies one side of a vertical cut in a circuit.
	* The program calculates the size of the other partition given number of qubits in the first partition.
	* If not specified, then the sum-of-tensors simulation calculates the best vertical cut. 
* **--verbose, -v**
	* Must be followed by an integer between 0 and 4 that specifies the level of verbosity of the final report (default is 3).
	* The verbosity levels are (in order) :
	```
	(0) None : no report
	(1) NCCV : no hardware information, no compilation information, and no correctness verification 
	(2) NCC : no hardware information and no compilation information
	(3) Default 
	(4) Cycles : monitor doubled cycles and output it in report. May significantly increase runtime.
	```

**Mandatory option** : You must either specify the input file (-h or -i) or specify that the simulator creates its own random circuit (-g).

### Building a binary

* The simulator needs c++17 and g++-7 to build successfully.
* The provided Makefile takes care of creating new directories if needed.
* Preprocessor macros specified in the Makefile can be modified for specific simulation types.

Navigate to the working directory (QuantumSim)
```shellsession
$ make all
```

### Quick runs

Navigate to the working directory (QuantumSim).

#### Example runs:
1. Create and simulate an entire circuit of 6 qubits and 100 cycles using full state-vector simulation and with default report:
```shellsession
$ ./bin/rr -g 6 100
```

2. Create and simulate an entire circuit of 6 qubits and 100 cycles using full state-vector simulation and with default report. Output the circuit to a file and create the Quiddpro script:
```shellsession
$ ./bin/rr -g 6 100 -o test
```

This creates two files, test0.txt and test0.qpro in the folders `input\random_circuits_aneeqa` and `output\qpro_scripts`, respectively.

3. Read Rollright's circuit file test0.txt from `input\random_circuits_aneeqa` and simulate to full depth:
```shellsession
$ ./bin/rr -i test0.txt
```

4. Read Google's circuit file inst_5_5_100_5.txt from `input\random_circuits_google` and simulate to depth 26:
```shellsession
$ ./bin/rr -p inst_5_5_100_5 -d 26
```

5. This command is an example of distributed simulation. Read Google's circuit file inst_5_5_100_5.txt, simulate to depth 10 using sum-of-tensors simulation with a vertical cut. Print 5 randomly generated indices with seed 7 to a file. The CZ path is a lenth 8 bit string of value 31. The amplitudes are printed to `output\amp_vectors\test\output_<pid>.amps`.
```shellsession
$ ./bin/rr -p inst_5_5_100_5 -d 10 -s 1 -x 7,5 -c 8,31 -o test
```

6. Read Google's circuit file inst_5_5_100_5.txt and using only 10 threads simulate to full depth using full state-vector simulation.
```shellsession
$ ./bin/rr -p inst_5_5_100_5 -s 5 -t 10 
```

7. Read Google's circuit file inst_6_5_100_5.txt. Simulate using sum-of-tensors with a horizontal cut of size 15 + 15. Set the XY Fast Transform threshold to 15.
```
$ ./bin/rr -p inst_6_5_100_5 -s 0 -f 15 -b 15
```

### Macros (TODO)
