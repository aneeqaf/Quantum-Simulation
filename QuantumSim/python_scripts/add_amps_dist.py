#!/usr/bin/env python3
"""Script for adding amplitudes from the output files of the simulator"""

import os
import traceback
import click
from multiprocessing import Process
from multiprocessing import Queue
import multiprocessing
import numpy as np
import mmap
import time


# Shortcut to multiprocessing's logger
def error(msg, *args):
    return multiprocessing.get_logger().error(msg, *args)


def ProcessorWrapper(process_id, queue_in, queue_out, binary_vectors_only, outdir, save_ascii):
  try:
    keep_processing = True
    while keep_processing:
      process_data = queue_in.get()
      print(process_id, 'got data:', process_data)
      # check if we are done
      if process_data == 'shut_down':
        print('shutting down', process_id)
        keep_processing = False
        break

      filename = process_data

      full_filename = os.path.join(outdir, filename)
      print(full_filename)

      if binary_vectors_only:
        lines = b''
        with open(full_filename, "r+b") as f:
          mm = mmap.mmap(f.fileno(), 0)
          for l in mm:
            lines += mm.readline()
          temp = np.fromstring(lines, dtype='c8')
      else:
        with open(full_filename, "r") as f:
          lines = f.readlines()
          temp = np.loadtxt(lines, dtype=complex)

      if save_ascii:
        fn, ext = os.path.splitext(filename)
        np.savetxt(os.path.join(outdir, fn + '_ascii' + ext), temp, fmt = '%.18e%+.18ej')
      queue_out.put((filename, temp))

  except Exception as e:
    error(traceback.format_exc())
    raise


def InitializeProcesses(binary_vectors_only, outdir, save_ascii):
  queue_in = Queue()
  queue_out = Queue()
  for counter in range(multiprocessing.cpu_count()):
    process = Process(target=ProcessorWrapper,
                      args=(counter, queue_in, queue_out, binary_vectors_only, outdir, save_ascii))
    process.start()

  return queue_in, queue_out

@click.command()
@click.argument("cir_dir", nargs=1)
@click.argument("num_idx", nargs=1)
@click.option("--binary_vectors_only", nargs=1, required=False, is_flag=True)
@click.option("--not_final_amps", nargs=1, required=False, is_flag=True)
@click.option("--add_partial_res_amps", nargs=1, required=False, is_flag=True)
@click.option("--save_ascii", nargs=1, required=False, is_flag=True)
def main(cir_dir, num_idx, binary_vectors_only, not_final_amps, add_partial_res_amps, save_ascii):

  outdir = os.path.join("output", "amp_vectors", cir_dir)
  files = os.listdir(outdir)

  amps = np.zeros(int(num_idx) + 5, dtype=complex)

  queue_in, queue_out = InitializeProcesses(binary_vectors_only, outdir, save_ascii)

  to_process = {}

  for filename in files:
    if filename.endswith("_ascii.amps") or binary_vectors_only or "result" in filename:
      if binary_vectors_only and (filename.endswith("_ascii.amps")
       or "result" in filename):
        continue
      if add_partial_res_amps and "result" not in filename:
        continue
      to_process[filename] = False
      queue_in.put(filename)

  print('Filled queue')

  n_files = len(to_process)
  for j in range(n_files):
    print('Waiting for output', n_files - j)
    filename, temp = queue_out.get()
    to_process[filename] = True
    amps += temp

  print('Shutting down')
  for counter in range(multiprocessing.cpu_count()):
    queue_in.put('shut_down')

  for k, v in to_process.items():
    if not v:
      print('NOT PROCESSED:', k)

  print('Writting results.')
  results_file = "result" + ".amps"
  if not_final_amps:
    results_file = "result" + str(time.clock()) + ".amps"
  # print(len(amps))
  # if binary_vectors_only:
  #   with open(os.path.join(outdir, results_file), "wb") as f:
  #     f.write(amps)
  # else :
  with open(os.path.join(outdir, results_file), "w") as f:
    for a in amps:
      f.write(str(a).replace("(", "").replace(")","") + "\n")

if __name__ == "__main__":
    main()
