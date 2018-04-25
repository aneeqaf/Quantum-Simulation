#!/usr/bin/env python3
"""Script for adding amplitudes from the output files of the simulator"""

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
	log_dir = os.path.join("output", "log")

	if not os.path.isdir(log_dir):
		try:
			os.makedirs(log_dir)
		except OSError as e:
			if e.errno != errno.EEXIST:
				raise

	if os.path.isdir(log_dir):
		log_dir = os.path.join(log_dir, script_dir)
		if not os.path.isdir(log_dir):
			try:
				os.makedirs(log_dir)
			except OSError as e:
				if e.errno != errno.EEXIST:
					raise
		else:
			shutil.rmtree(log_dir, ignore_errors=True)
			try:
				os.makedirs(log_dir)
			except OSError as e:
				if e.errno != errno.EEXIST:
					raise

	for filename in files:
		if filename.endswith(".sh"):
			script_count = int(re.sub('[^0-9]', '', filename))
			if script_count >= int(start_range) and script_count < int(end_range):
				print("./" + os.path.join(bindir, filename) + \
				  " > " + os.path.join(log_dir, "log_" + filename.replace(".sh", ".txt")) + " 2>&1 &")
				os.system("./" + os.path.join(bindir, filename) + \
				  " > " + os.path.join(log_dir, "log_" + filename.replace(".sh", ".txt")) + " 2>&1 &")


if __name__ == "__main__":
    main()	