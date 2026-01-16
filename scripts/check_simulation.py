#!/usr/bin/env python3

import argparse

parser = argparse.ArgumentParser(description='check reference data against simulation output')
parser.add_argument('ref_dir', help='path to reference data directory')
parser.add_argument('sim_dir', help='path to simulation output directory')
parser.add_argument('glob', help='glob to match output files')
args = parser.parse_args()

import sys
import math
import glob
import os

def value_generator(f):
  try:
    for line in f:
      if line.startswith('T'):
        continue
      elif line.startswith('SKIP'):
        skip = int(line.split()[1])
        yield None, skip
      else:
        for val in line.split():
          yield float(val), None
  except ValueError:
    sys.stderr.write(f'unexpected value in {f.name}: {val}\n')
    sys.exit(1)

files = glob.glob(args.glob, root_dir=args.sim_dir)
for file in files:
  with open(os.path.join(args.ref_dir, file), 'r') as ref, \
       open(os.path.join(args.sim_dir, file), 'r') as sim:
    ref_vals = value_generator(ref)
    sim_vals = value_generator(sim)
    rel_tol = 1e-9
    idx = 0

    print(f'check_simulation.py: {os.path.abspath(ref.name)} {os.path.abspath(sim.name)}')
    sys.stdout.flush()

    while True:
      try:
        ref_val, skip = next(ref_vals)
      except StopIteration:
        break

      if skip is not None:
        stop = False
        for i in range(skip):
          try:
            sim_val, _ = next(sim_vals)
          except StopIteration:
            stop = True
            break

        if stop:
          break

        continue

      try:
        sim_val, _ = next(sim_vals)
      except StopIteration:
        break

      if not math.isclose(ref_val, sim_val, rel_tol=rel_tol):
        sys.stderr.write(f'error: file={file} idx={idx} reference={ref_val} simulation={sim_val} tolerance={rel_tol}\n')
        sys.exit(2)

      idx += 1

    sim_rem = sum(1 for _ in sim_vals)
    ref_rem = sum(1 for _ in ref_vals)

    if sim_rem:
      sys.stderr.write(f'error: found {sim_rem} remaining simulation values in {sim.name}\n')
      sys.exit(3)

    if ref_rem:
      sys.stderr.write(f'error: found {ref_rem} remaining reference values in {ref.name}\n')
      sys.exit(4)
