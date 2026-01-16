#!/usr/bin/env python3

import matplotlib.pyplot as plt
import numpy as np
import os
from collections import defaultdict

fout = os.path.abspath('scaling.pdf')

fontsize = 22
plt.rcParams['mathtext.fontset'] = 'cm'
plt.rcParams['font.family'] = 'STIXGeneral'
plt.rcParams['font.size'] = fontsize
plt.figure(figsize=(12, 6))

# simulate each config then pass to heatmap.py:
# heatmap.py -d $(fd -tl -d1 | sort -V | paste -sd',' -) | awk '{print $1,$3}'
data = {
  '1x1': 0.869, '1x2': 0.869, '1x4': 0.869, '1x8': 0.869, '1x16': 0.869, '1x32': 0.869, '1x50': 0.869,
  '2x1': 0.863, '2x2': 0.863, '2x4': 0.862, '2x8': 0.863, '2x16': 0.862, '2x32': 0.861, '2x50': 0.862,
  '4x1': 0.853, '4x2': 0.853, '4x4': 0.853, '4x8': 0.852, '4x16': 0.851, '4x32': 0.852, '4x50': 0.851,
  '6x1': 0.841, '6x2': 0.840, '6x4': 0.839, '6x8': 0.840, '6x16': 0.838, '6x32': 0.842, '6x50': 0.840,
  '8x1': 0.828, '8x2': 0.829, '8x4': 0.827, '8x8': 0.828, '8x16': 0.828, '8x32': 0.828, '8x50': 0.829,
}

grouped = defaultdict(list)
for k, v in data.items():
  r, c = map(int, k.split('x'))
  grouped[r].append((c, v))

rows = sorted(grouped)
colors = plt.cm.tab10(np.linspace(0, 1, len(rows)))

styles = [
  dict(linestyle='-',  marker='o', fillstyle='none', markersize=12),
  dict(linestyle='--', marker='x', fillstyle='none', markersize=12),
  dict(linestyle=':',  marker='^', fillstyle='none', markersize=12),
  dict(linestyle='-.', marker='s', fillstyle='none', markersize=12),
  dict(linestyle='-',  marker='D', fillstyle='none', markersize=12)
]
style = iter(styles)

for r in rows:
  grouped[r].sort()
  cs, eff = zip(*grouped[r])
  cores = [r * c for c in cs]
  s = next(style)
  plt.plot(cores, eff, label=f'{r}-row', color='black', **s)

#for color, r in zip(colors, rows):
#  grouped[r].sort()
#  cs, eff = zip(*grouped[r])
#  cores = [r * c for c in cs]
#  plt.plot(cores, eff, marker='o', label=f'{r} rows', color=color)

plt.xscale('log', base=2)
#plt.xlim(1, 512)
#plt.ylim(0.88, 0.94)

xticks = [2**x for x in range(10)]
plt.xticks(xticks, [str(x) for x in xticks], fontsize=fontsize*0.8)
plt.yticks(fontsize=fontsize*0.8)

plt.xlabel('Cores')
plt.ylabel('Efficiency')
plt.grid(True, which='both', linestyle='--', alpha=0.6)
plt.legend(loc='best', fontsize=fontsize*0.8)#, ncols=len(grouped))

plt.tight_layout()
plt.savefig(fout, bbox_inches='tight', pad_inches=0)
print(f'wrote: {fout}')
