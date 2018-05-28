#!/usr/bin/env python3
"""Script for report generation after a multinode simulation has completed"""

import os
import click
import numpy as np
import time
import re
from math import ceil

@click.command()
@click.argument("cir_dir", nargs=1)
@click.option("--num_idx", nargs=1, required=True, default= 0)
@click.option("--max_procs", nargs=1, required=False, default= 0)
@click.option("--test_fid", nargs=1, required=False, is_flag=True)
def main(cir_dir, num_idx, max_procs, test_fid):
	
	add_amps_cmd = "./python_scripts/add_amps_dist.py " + str(cir_dir) + " " + str(num_idx) + \
					" --add_partial_res_amps"
	print(add_amps_cmd) 
	os.system(add_amps_cmd)

	report_cmd = "./python_scripts/multinode_report_gen.py " + str(cir_dir) + \
	  " --max_procs " + str(max_procs)
	if test_fid:
		report_cmd += " --test_fid"
	print(report_cmd)
	os.system(report_cmd)

	
if __name__ == "__main__":
    main()	