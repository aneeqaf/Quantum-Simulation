#!/usr/bin/env python3
"""Script for adding amplitudes from the output files of the simulator"""

import os
import click
import numpy as np
import mmap
import time
import dist_util

@click.command()
@click.argument("exact_file", nargs=1)
@click.argument("approx_file", nargs=1)
def main(exact_file, approx_file):

	print("Fidelity: " + str(dist_util.CalculateFidelity(exact_file, approx_file)));
	

if __name__ == "__main__":
    main()	