# Rollright - A Quantum Simulator

Rollright is a hybrid Schrondinger-Feynman quantum simulator that is particularly efficient on Google quantum-supremacy circuits. In addition to a full state-vector representation, it uses sum-of-tensors representations. Distributed simulation with no interprocess communication is performed using a meta-simulation script. 

Tensor partitions are currently specified by the choice of a vertical or horizontal cut with a particular size breakdown (3q + 4q versus 4q + 3q). Non-straightline cuts are supported internally but would require additional command-line controls. Straightline cuts are preferrable for deeper circuits.

In order to simulate each two-qubit gate acting across tensor partitions, Rollright branches on SVD components of the gate. For example, each CZ gate implies a branching factor of two. Optional command-line arguments specify a branch for each crossing CZ gate (xCZ), which cumulatively form a "CZ path". Command-line arguments may request a range of such CZ paths to be simulated in the same process with resulting amplitudes added up and saved in an output file, or added up to a previously existing file (saved by another process). 

The indices of the amplitudes to save can be specified either explicitly or using a pseudo-random number generator by providing a seed and the number of indices (produced indices can be optionally saved into a file). To minimize I/O effort, Rollright reads and saves the amplitudes in binary format using memory mapped I/O by default. ASCII output can be requested for the last process in a batch. 

Rollright prints many details about its hardware and software environment, as well as runtime statistics, with requested verbosity. 

Rollright supports multithreaded execution, however the number of threads requested must be carefully balanced against the number of parallel processes launched.

Rollright was developed in c++17 and uses g++7.

**Important: depth 41 is 1 + 40, where 1 accounts for cycle 0 of H gates.**

## Running the Simulator

### Command line options

**Mandatory argument** : You must specify the input file (-i).

* **--CZ_path, -c**
	* CZ path is usually specified for distributed simulation.
	* CZ path has four components, the last two of which are optional.
	* The first two integers separated by a comma specify the length of the CZ path and the value of the CZ path. 
	* The third argument is the number of bits that specify the range of CZ paths that the simulator goes over in a single process.
	* The fourth argument is the number of bits to branch on using depth first search.
	* Initially, simulations consumes the process prefix and subsequent CZ bits from the specified range. Then, a branching simulation is performed for specified number of bits. 
	* Defines the CZ path for CZ gates that cross partitions for distributed simulation.
* **--depth, -d** 
	* Must be followed by an integer to specify the number of cycles to simulate.
	* The default setting simulates all cycles specified in the input.
* **--first_partition_smaller, -f**
        * This is a flag and if specified it chooses a cut where the first partition is smaller than the second.
        * The default behavior is that the first partition is bigger than the second.
* **--grid_type, -m**
        * Must be followed by a string specifying the type of 2D circuit : "c" or "column_major", "r" or "row_major".
        * Default is row major.
* **--hcut, -_**
	* Must be followed by an integer that specifies one side of a horizontal cut in a circuit.
	* The program calculates the size of the other partition given number of qubits in the first partition.
	* If not specified, then the sum-of-tensors simulation calculates the best horizontal cut. 
* **--help, --h**
	* Lists available command line options.
* **--inputfile, -i**
	* Must be followed by a filename (currently only reads the file in the Rollright or in the Google circuit format).
	* The file is assumed to be in `input\random_circuits_rollright` or in `input\random_circuits_google`.
* **--idx, -x**
	* Must be followed by either a filename or two comma separated integers, seed and number of indices.
	* Optionally, end the two integers argument with a '+' to print the generated indices to a file.
	* The program prints the values of the amplitudes in the specified indices to a file in `output\amp_vectors\<dirname pertaining to circuit>\<filename>.amps` or `output\amp_vectors\<filename>.amps`. `<filename>` can be specified using the -o option. Providing a `<filename>` is recommended for distributed simulation to simplify recollection of files for computations. 
	* Generated indices are printed in `output\amp_vectors\<dirname>\<filename>.idx` or `output\amp_vectors\<filename>.idx`
* **--layers_Hgates_b4_meas, -H**
        * Must be followed by the number of layers of H gates to add at the specified depth.
        * Default is zero.
        * Don't need to specify this flag if the circuit file already has the required layer of H gates.
* **--no_checkpoint_ranges, -p**
        * This is a flag and if specified, the state vector after simulation of the process prefix bits is not saved.
        * Default is false.
        * The purpose of this flag is to save memory with possible deprecation in performance.
* **--no_nearest_neighbors, -n**
        * If specified then the simulator does not check to make sure all two qubits gates are acting on nearest neighbors.
* **--num_threads, -t**
	* Must be followed by an integer.
	* Sets a maximum limit on the number of threads to be used 
* **--outfile, -o**
	* Must be followed by a filename.
	* The file specified is always created in the `output` directory, except when creating new random cicuits that are written to `input\random_circuits_rollright`. 
	* The argument only includes the name of a file, not a path. The filename is used to write to predetermined locations pertinent to the type of output. (Namely, `output\amp_vectors`, `output\probabilities`, `output\qpro_scripts`, and `output\reports`.)
	* If the amplitudes are requested to be printed, then the simulator creates a separate directory in `output\amp_vectors` pertinent to the name of the circuit file and prints the vectors in a file with the `.amp` extension. The default is binary. The name of the output file followed by `@` prints the amplitudes in the ASCII format.
* **--save_checkpoint_to_file** :
    * Takes as argument an integer between 0 and 2. Each integer implies the following :
        * 0 : do not save any checkpoint to file
        * 1 : save only the checkpoint before ranges to file 
        * 2 : save both the checkpoints to file
    * Default value is 0. 
* **--sim_type, -s**
	* Must be followed by an integer between 0 and 8 that specifies the type of simulation.
	* The simulation types supported (in order): LosslessH, LosslessV, Approx1CutH, Approx1CutV, Approx2Cuts, FullState, Approx2011, Approx1_101, Approx1110. Default simulation type is FullState. 
	* The default is FullState for up to 32 qubits and LosslessH for circuits with greater than 32 qubits. 
* **--vcut, -|**
	* Must be followed by an integer that specifies one side of a vertical cut in a circuit.
	* The program calculates the size of the other partition given number of qubits in the first partition.
	* If not specified, then the sum-of-tensors simulation calculates the best vertical cut. 
* **--verbose, -v**
	* Must be followed by an integer between 0 and 3 that specifies the level of verbosity of the final report (default is 3).
	* The verbosity levels are (in order) :
	```
	(0) None : no report
	(1) NCCV : no hardware information, no compilation information, and no correctness verification 
	(2) NCC : no hardware information and no compilation information
	(3) Default 
	```

### Directory requirements

* All google circuit files must be in `input\random_circuits_google`.
* The following directories should be present  : `output\amp_vectors` , `output\log` and `bin`.

### Quick runs

Navigate to the working directory (QuantumSim).

#### Example runs:

1. Read Rollright circuit file test0.txt from `input\random_circuits_aneeqa` and simulate to full depth:
```shellsession
$ ./bin/rr -i test0.txt
```

2. Read Google circuit file inst_5_5_100_5 from `input\random_circuits_google` and simulate to depth 26:
```shellsession
$ ./bin/rr -i inst_5_5_100_5 -d 26
```

3. This command is an example of distributed simulation. Read Google's circuit file inst_5_5_100_5.txt, simulate to depth 10 using sum-of-tensors simulation with a vertical cut. Print 5 randomly generated indices with seed 7 to a file. The CZ path is a length-8 bitstring of value 31 (00011111). The amplitudes are printed to `output\amp_vectors\inst_5_5_100_5_10_8_8\test.amps`.
```shellsession
$ ./bin/rr -i inst_5_5_100_5 -d 10 -s 1 -x 7,5 -c 8,31 -o test
```

4. Read Google circuit file inst_5_5_100_5 and using only 10 threads simulate to full depth using full state-vector simulation.
```shellsession
$ ./bin/rr -i inst_5_5_100_5 -s 5 -t 10 
```

5. Read Google circuit file inst_6_5_100_5. Simulate using sum-of-tensors with a horizontal cut of size 15 + 15. 
```shellsession
$ ./bin/rr -i inst_6_5_100_5 -s 0 -b 15
```

6. Read Google circuit file inst_6_5_100_5. Simulate upto depth 16 (15 excluding the H gate application) using sum-of-tensors with a horizontal cut and a maximum of 4 threads. Print 20 amplitudes in ASCII format to `output\amp_vectors\inst_6_5_100_5_16_10_4\output_1.amps` from randomly generated indices with a seed of 7. 
```shellsession
$ ./bin/rr -i inst_6_5_100_5 -d 16 --num_threads 4 --sim_type 0 --idx 7,20 --CZ_path 8,0,2,2 --outfile output_1@
```

