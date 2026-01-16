#!/usr/bin/env python3

import argparse
import subprocess
import time
import statistics
import re

def parse_power(line):
  match = re.search(r'Power\s*:\s*(\d+)\s*Watts', line)
  if match:
    return int(match.group(1))
  return None

def main(bdf, duration):
  powers = []
  end_time = time.time() + duration

  while time.time() < end_time:
    result = subprocess.run(
      f'xbutil examine -r electrical -d {bdf} | grep Power',
      shell=True,
      capture_output=True,
      text=True
    )

    for line in result.stdout.splitlines():
      power = parse_power(line)
      if power is not None:
        print(line)
        powers.append(power)
    
    time.sleep(1)

  if not powers:
    print('warning: no power readings captured.')
    return

  print(f'samples : {len(powers)}')
  print(f'min     : {min(powers)} watts')
  print(f'median  : {statistics.median(powers)} watts')
  print(f'avg     : {statistics.mean(powers):.2f} watts')
  print(f'max     : {max(powers)} watts')

if __name__ == '__main__':
  parser = argparse.ArgumentParser()
  parser.add_argument('-d', '--bdf', required=True, help='device bdf')
  parser.add_argument('-t', '--time', type=int, default=180, help='duration to monitor in seconds')
  args = parser.parse_args()

  main(args.bdf, args.time)

