#!/usr/bin/env python

import click
import os
import random

from pathlib import Path

@click.command()
@click.argument("num_idxs", nargs=1, required=True, default=0)
@click.argument("num_q", nargs=1, required=True, default=0)
def main(num_idxs, num_q):
    max_idx = 1 << int(num_q)
    p = Path(os.path.dirname(os.path.realpath(__file__)))
    print(str(p))
    with open(os.path.join(str(p.parent), "qsim", "circuits", "idxs_" + str(num_idxs) + "_" + str(num_q)), "w+") as f:
        for i in range(num_idxs):
            idx = random.randrange(max_idx)
            f.write(format(idx, '#0' + str(num_q + 2) + 'b')[2 : num_q + 2]+ "\n")


if __name__ == "__main__":
    main()