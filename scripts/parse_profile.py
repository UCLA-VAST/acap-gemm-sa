#!/usr/bin/env python3

import argparse
import re
import sys

def parse_profile(inp, func, stalls=True, verbose=True):
  with open(inp, 'r') as f:
    in_compute = False
    in_instructions = False
    it = iter(f)
    mac_cycles = 0
    total_cycles = 0

    try:
      while True:
        line = next(it)
        if in_instructions:
          while line.strip():
            count = int(line[idx_count[0]:idx_count[1]])
            if stalls:
              cycles = int(line[idx_cycles[0]:idx_cycles[1]])
            else:
              cycles = count
            total_cycles += cycles
            if 'MAC' in line:
              mac_cycles += count
              mark = 'x'
            else:
              mark = ' '
            prog = f'{mac_cycles}/{total_cycles}'
            if verbose:
              print(f'{prog:>{cycles_len*2+1}}: {mark} {line.strip()[:idx_cycles[1]]}')
            line = next(it)
          break
        elif in_compute:
          while True:
            line = next(it)
            if line.strip().startswith('PC '):
              line = next(it)
              idx = line.index('-') # PC
              idx = line.index(' ', idx+1) # Instruction
              idx = line.index(' ', idx+1) # Assembly
              idx = i = line.index(' ', idx+1) # Exe-count
              idx = line.index(' ', idx+1) # Cycles
              idx_count = (i+1, idx)
              idx_cycles = (idx+1, line.index(' ', idx+1))
              in_instructions = True
              break
        elif line.startswith(f'Function summary:'):
          while True:
            line = next(it)
            if func in line:
              cycles_len = len(re.search(r'\d+', line).group())
              break
        elif line.startswith(f'Function detail: {func}'):
          in_compute = True
    except StopIteration:
      pass

  if not in_compute:
    raise Exception(f'could not find function {func}')

  if not total_cycles:
    raise Exception(f'could not find any cycles in {func}')

  if not mac_cycles:
    raise Exception(f'could not find any MAC cycles in {func}')
  else:
    eff = mac_cycles / total_cycles
    if verbose:
      print(f'Efficiency: {eff:.3f}')

  return eff

def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('-i', '--input', required=True, help='profile_instr*.txt to parse')
  parser.add_argument('-f', '--func', default='main', help='compute function name')
  parser.add_argument('-q', '--quiet', action='store_true', help='silence output')
  parser.add_argument('--stalls', default=True, action=argparse.BooleanOptionalAction, help='include/exclude stall cycles')
  args = parser.parse_args()

  parse_profile(args.input, args.func, args.stalls, not args.quiet)

if __name__ == '__main__':
  main()
