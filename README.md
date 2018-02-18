# Rollright - A Quantum Simulator

## Run the Simulator

### Command line options
* **-depth, -d** 
	* Required argument : integer to specify the number of cycles to simulate
	* If flag is not used then the default is set to simulating all the cycles. 
	* This option is not used when creating a random circuit. It is only relevant when reading from inputfiles.
* **-cutSizes, -c**
	* Required argument : integer to specify the size of one side of a horizontal or vertical cut in a 2D circuit.
	* This flag is only relevant when the simulation involves sum of tensors. 
	* If flag is not used then, the program decides the best cut for sum of tensors simulation.
* **-google, -g**
	* Required arguments : this flag takes in two arguments - number of qubits and total depth of the circuits.
	* It is responsible for creating a google circuit.
* **-googleInput, -h**
	* Required argument : name of a Google circuit input file. The file needs to be in the Google circuit file's format.
	* Input files need to be in `input\random_circuits_google`.
* **-inputfile, -i**
	* Required argument : name of input file (currently only reads the file the Rollright format).
	* Input files need to be in `input\random_circuits_aneeqa`.
* **-outfile, -o**
	* Required argument : name of the file to write output to.
	* The file specified is always created in a directory in the `output` directory, except for the circuit files that the simulator produces upon creating a new random circuit. Newly generated random cicuits are written to `input\random_circuits_aneeqa`. 
	* The argument only takes the name of a file not path. The file name is used to write to predetermined locations pertinent to the type of output. (Namely, `output\amp_vectors`, `output\probabilities`, `output\qpro_scripts`, and `output\reports`. Please create these folders if not already present.)
* **-threshold, -t**
	* Required argument : integer that is smaller than the total number of qubits. This is used to control the base case of the Recursive Transform algorithm.
	* If flag is not used than the default threshold is 14.
* **-simType, -s**
	* Required argument : integer between 0 and 8 that specify the type of simulation.
	* The simulator supports 9 different types of simulations : LosslessH, LosslessV, Approx1CutH, Approx1CutV, Approx2Cuts, FullState, Approx2011, Approx1_101, Approx1110. The order reflects the integer applicable to the simulation type. 
	* Default simulation type is FullState. 
* **-verbose, -v**
	* Required argument : integer between 0 and 4 that specify the level of verbosity of the final report that is output by the simulator.
	* Default verbosity is set at level 3.
	* The verbosity levels are: None, NCCV, NCC, Default, Cycles, where the order reflects the numerical level of verbosity. 
		1. None : no report
		2. NCCV : no hardware information, no compilation information, and no correctness verification 
		3. NCC : no hardware information and no compilation information
		4. Default 
		5. Cycles : monitor doubled cycles and output it in report

**Mandatory option** : You must either specify the input file (-h or -i) or tell the simulator to create its own random circuit (-g).

### Quick runs
#### Notes:
* The simulator needs c++17 and g++-7 to build successfully.
* A Makefile is provided with the simulator that takes care of creating directories that do not already exist in your directory structure but are required by the program.
* Modifications to the Makefile to alter the macros for compilation will be needed for detailed experiments.
* Using the -o flag when reading input files is used for outputting other types of files, like the probabilities of the entire state vector.

Navigate to the working directory (QuantumSim).

#### Examples runs:
1. Create and simulate entire circuit of 6 qubits and 100 cycles using full state-vector simulation and with default report:
```shellsession
$ make all
$ ./bin/rollright_sim -g 6 100
```

2. Create and simulate entire circuit of 6 qubits and 100 cycles using full state-vector simulation and with default report. Output the circuit to a file and create the Quiddpro script:
```shellsession
$ make all
$ ./bin/rollright_sim -g 6 100 -o test
```

This creates two files, test0.txt and test0.qpro in the folders `input\random_circuits_aneeqa` and `output\qpro_scripts`, respectively.

3. Read Rollright's circuit file test0.txt from `input\random_circuits_aneeqa` and simulate to full depth:
```shellsession
$ make all
$ ./bin/rollright_sim -i test0.txt
```

4. Read Google's circuit file inst_5_5_100_5.txt from `input\random_circuits_google` and simulate to depth 26:
```shellsession
$ make all
$ ./bin/rollright_sim -h inst_5_5_100_5 -d 26
```
### Macros (TODO)
