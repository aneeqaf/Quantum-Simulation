# Rollright - A Quantum Simulator

Rollright is a hybrid Schrondinger-Feynman quantum simulator that is particularly efficient on Google quantum-supremacy circuits. In addition to a full state-vector representation, it uses sum-of-tensors representations. Distributed simulation with no interprocess communication is performed using a meta-simulation script. 

Tensor partitions are currently specified by the choice of a vertical or horizontal cut with a particular size breakdown (3q + 4q versus 4q + 3q). Non-straightline cuts are supported internally but would require additional command-line controls. Straightline cuts are preferrable for deeper circuits.

In order to simulate each two-qubit gate acting across tensor partitions, Rollright branches on SVD components of the gate. For example, each CZ gate implies a branching factor of two. Optional command-line arguments specify a branch for each crossing CZ gate (xCZ), which cumulatively form a "CZ path". Command-line arguments may request a range of such CZ paths to be simulated in the same process with resulting amplitudes added up and saved in an output file, or added up to a previously existing file (saved by another process). 

The indices of the amplitudes to save can be specified either explicitly or using a pseudo-random number generator by providing a seed and the number of indices (produced indices can be optionally saved into a file). To minimize I/O effort, Rollright reads and saves the amplitudes in binary format using memory mapped I/O by default. ASCII output can be requested for the last process in a batch. 

Rollright prints many details about its hardware and software environment, as well as runtime statistics, with requested verbosity. 

Rollright supports multithreaded execution, however the number of threads requested must be carefully balanced against the number of parallel processes launched.

**Important: depth 41 is 1 + 40, where 1 accounts for cycle 0 of H gates.**

## Running the Simulator

### Command line options
* **--CZ_path, -c**
	* CZ path is usually specified for distributed simulation.
	* CZ path has four components, the last two of which are optional.
	* The first two integers separated by a comma specify the length of the CZ path and the value of the CZ path. 
	* The third argument is the number of bits that specify the range of CZ paths that the simulator goes over in a single process.
	* The fourth argument is the number of bits to branch on using depth first search.
	* Initially, simulations consumes the process prefix and subsequent CZ bits from the specified range. Then, a branching simulation is performed for specified number of bits. 
	* Defines the CZ path for CZ gates that cross partitions for distributed simulation.
* **--depth, -d** 
	* Must be followed by an integer to specify the number of cycles to simulate
	* The default setting simulates all cycles specified in the input.
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
* **--num_threads, -t**
	* Must be followed by an integer.
	* Sets a maximum limit on the number of threads to be used 
* **--outfile, -o**
	* Must be followed by a filename.
	* The file specified is always created in the `output` directory, except when creating new random cicuits that are written to `input\random_circuits_rollright`. 
	* The argument only includes the name of a file, not a path. The filename is used to write to predetermined locations pertinent to the type of output. (Namely, `output\amp_vectors`, `output\probabilities`, `output\qpro_scripts`, and `output\reports`.)
	* If the amplitudes are requested to be printed, then the simulator creates a separate directory in `output\amp_vectors` pertinent to the name of the circuit file and prints the vectors in a file with the `.amp` extension. The default is binary. The name of the output file followed by `@` prints the amplitudes in the ASCII format.
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

**Mandatory argument** : You must either specify the input file (-i).

### Directory requirements

* All google circuit files must be in `input\random_circuits_google`.
* The following directories should be present  : `output\amp_vectors` , `output\log` and `bin`.

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

## Python Scripts

The directory requirements outlined in, 'Running the Simulator'  section must be followed.

### Getting started

If you do not already have python3, please install it. Use the commands below to install the python3 virtual environment, developer tools, and required libraries.
```shellsession
$ sudo apt-get install build-essential libssl-dev libffi-dev python3-dev
$ sudo apt-get install -y python3-venv
$ python3 -m venv env
$ source env/bin/activate # Use everytime to activate the virtual env
$ pip install click
$ pip install numpy
$ pip install psutil
```
### Running important scripts

1. #### dist_sim.py

Launches the multiprocess simulation only if the simulations are being run on a single node. Only creates the bash scripts if the script is being run on multiple nodes in the `bin\<circuit_filename>_<depth>_<prefix_bits>_<num_threads>` or if performing approximate simulation `bin\<circuit_filename>_<depth>_<prefix_bits>_<num_threads>_approx_<eps>`.

Logs, amplitude files and scripts are outputted in the directory `<circuit_filename>_<depth>_<prefix_bits>_<num_threads>` or `<circuit_filename>_<depth>_<prefix_bits>_<num_threads>_approx_<eps>`  within the directory `log\`,  `amp_vectors\`,  or  `bin\` respectively.

##### Important command line options

**Mandatory argument** : circuit filename

**Optional arguments** with functionality similar to that described in, 'Running the Simulator'  section:
By default the script chooses optimal or near optimal choices for each of the options below:
* **--depth**
* **--v_cut**
* **--h_cut**
* **--proc_prefix_bits**
* **--ranges_bits** : for approximation this is by default set to zero
* **--branch_bits**
* **--num_batches**

These options should be modified if default value is not desired:
* **--num_threads**: default = 4
* **--num_idx**: default = 1000
* **--trial** : this is a flag and if specified then a single trial run is performed. Not recommended for simulations with long processes. Default is false.
* **--max_procs** : this is to be used for trial batches to get a good estimate of how long a single process will take when multiple processes are running in parallel. The value should ideally be set to number of expected batches. Default value is 0.
* **--approx** : the value of this option is the denominator of the fidelity; e.g For fidelity = 0.125 , --approx=8.
* **--multiple_nodes**: this is a flag and must be specified if the simulation is going to be carried on multiple nodes. It prevents the launch of multiple batches on the same node, producing bash scripts instead.

##### Example runs:

1. Runs a multiprocess simulation of a 30q circuit with depth 26 and 0.125 fidelity. Stores only a 100 amps in each run.
```shellsession
$ ./python_scripts/dist_sim.py inst_6_5_100_5 --num_idx=100 --depth=26 --approx=8
```
2. Produces 100 bash scripts in the directory `bin\inst_7_7_100_5_41_<prefix_bits>_4`. Does not launch simulations.
```shellsession
$ ./python_scripts/dist_sim.py inst_7_7_100_5 --num_idx=10000 --depth=41 --approx=100 --multiple_nodes --num_batches=100
```

2. #### post_launch.py

After the simulation has completed, use this script to add all the amplitudes of the different batches (the result is outputted in, `result.amps`) and then generate the multiprocess simulation report. The script also verifies whether each batch has succesfully completed.

(dist_sim.py outputs the command line to be used for this script before exiting.)

##### Important command line options

**Mandatory argument** :  `<circuit_filename>_<depth>_<prefix_bits>_<num_threads>` or `<circuit_filename>_<depth>_<prefix_bits>_<num_threads>_approx_<eps>`

prefix_bits = proc_prefix_bits + ranges_bits

**Required options**

* **--num_procs** : total number of processes in the simulation.
* **--num_batches** :  total number of batches in the simulation.
* **--num_idx** : same as the value set in dist_sim.py.

dist_sim.py prints the total number of processes  and batches on the terminal.

**Optional options**

* **--max_procs** : same as the value set in dist_sim.py. (Must be set if running trial batches )
* **--cloud_services** : if a monetary value is associated with the simulation, set this flag.

##### Example runs:

1. The command below would be run after the simulation of a 30q circuit, depth 26 with fidelity 0.125 is completed.
```shellsession
$ ./python_scripts/post_launch.py inst_6_5_100_5_26_12_4_approx_8 --num_procs 512 --num_batches 15  --num_idx 1000
```

3. #### execute_scripts.py

This script is used to run bash scripts for batches within a range. The purpose of this is to launch simulations on multiple nodes.

##### Important command line options

**Mandatory arguments** :
* `<circuit_filename>_<depth>_<prefix_bits>_<num_threads>` or `<circuit_filename>_<depth>_<prefix_bits>_<num_threads>_approx_<eps>`
* Starting range of batches to be simulated on a node (inclusive).
* Ending range of batches to be simulated on a node (exclusive).

##### Example runs:

1. The command below would launch batches 10 to 14 for the simulation of a 30q circuit, depth 26 with fidelity 0.125.
```shellsession
$ ./python_scripts/execute_scripts.py inst_6_5_100_5_26_12_4_approx_8 10 15
```

4. #### resume_batches.py

This script is used to resume the simulation for batches that were stopped because of external issues like an instance on the cloud getting terminated.

It is crucial to have the logs of a terminated batch to resume the simulations. In case the log of a batch has been lost, the simulation cannot be resumed.

##### Important command line options

**Mandatory arguments** :
* `<circuit_filename>_<depth>_<prefix_bits>_<num_threads>` or `<circuit_filename>_<depth>_<prefix_bits>_<num_threads>_approx_<eps>`
* Start of range of batches to be resumed (inclusive).
* End of range of batches to be resumed (exclusive).

##### Example runs:

1. The command below would resumed batches 10 to 14 for the simulation of a 30q circuit, depth 26 with fidelity 0.125.
```shellsession
$ ./python_scripts/resume_batches.py inst_6_5_100_5_26_12_4_approx_8 10 15
```

5. #### add_amps.py

This script adds the amplitudes from all the batches. The result is output in the file `result.amps`  in the same directory.

##### Important command line options

**Mandatory arguments** :
* `<circuit_filename>_<depth>_<prefix_bits>_<num_threads>` or `<circuit_filename>_<depth>_<prefix_bits>_<num_threads>_approx_<eps>`
* Number of idxs

##### Example runs:

1. The command below adds up all the amplitudes produced after the simulations of a  30q circuit, depth 26 with fidelity 0.125. `result.amps` would have a 1000 amps.
```shellsession
$ ./python_scripts/add_amps.py inst_6_5_100_5_26_12_4_approx_8 1000
```

6. #### dist_sim_report_gen.py

Generates the reports after a simulation has been completed. Uses simulation logs of the batches.

##### Important command line options

**Mandatory arguments** :  `<circuit_filename>_<depth>_<prefix_bits>_<num_threads>` or `<circuit_filename>_<depth>_<prefix_bits>_<num_threads>_approx_<eps>`

**Optional arguments**

* **--max_procs** : same as the value set in dist_sim.py. (Must be set if running trial batches )
* **--cloud_services** : if a monetary value is associated with the simulation, set this flag.


##### Example runs:

1. The command below prints the multiprocess simulation report after the simulations of a  30q circuit, depth 26 with fidelity 0.125.
```shellsession
$ ./python_scripts/dist_sim_report_gen.py inst_6_5_100_5_26_12_4_approx_8
```


