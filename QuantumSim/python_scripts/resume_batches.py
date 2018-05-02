#!/usr/bin/env python3
"""Script for resuming simulation for batches within the specified range incase a batch was
stopped in the middle"""

import os
import click
import re
import shutil

@click.command()
@click.argument("script_dir", nargs=1)
@click.argument("start_range", nargs=1)
@click.argument("end_range", nargs=1)
def main(script_dir, start_range, end_range):

	bindir = os.path.join("bin", script_dir)
	files = os.listdir(bindir)

	log_dir = os.path.join("output", "log", script_dir)

	if not os.path.isdir(log_dir):
		print("Log directory not present")
		exit()

	for filename in files:
		if filename.endswith(".sh"):
			if "temp_" in filename:
				continue
			script_count = int(re.sub('[^0-9]', '', filename))
			elif script_count >= int(start_range) and script_count < int(end_range):
				log_file = os.path.join(log_dir, "log_" + filename.replace(".sh", ".txt"))
				temp_script_dir = os.path.join(bindir, "temp_" + filename)
				script_dir = os.path.join(bindir, filename)
				num_procs_completed = 0
				with open(log_file, "r") as lf :
					for line in lf:
						if "processes completed" in line:
							num_procs_completed += 1
				with open(script_dir, "r") as sh :
					start_writing = False
					with open(temp_script_dir, "w") as tsh:
						tsh.write("#!/bin/bash\nset -e\nexport OMP_DISPLAY_ENV=true\n\n")
						for line in sh:
							if "PROCS=" in line:
								tsh.write(line + "\n")
							
							if not start_writing and "processes completed" in line:
								if int(line.split()[0]) == num_procs_completed:
									tsh.write("echo \"\n")
									start_writing = True
							if start_writing:
								tsh.write(line)
									
				print("./" + temp_script_dir +  " >> " + log_file + " 2>&1 &")
				os.system("chmod +x " + temp_script_dir)
				os.system("./" + temp_script_dir + " >> " + log_file + " 2>&1 &")


if __name__ == "__main__":
    main()	