#!/usr/bin/env python3

import matplotlib.pyplot as plt
import numpy as np
import os
import colorsys

fout = os.path.abspath('bar.pdf')
fontsize = 26

plt.rcParams['mathtext.fontset'] = 'cm'
plt.rcParams['font.family'] = 'STIXGeneral'
plt.rcParams['font.size'] = fontsize

plt.figure(figsize=(12, 6))

names = ['Stall', 'VMAC', 'Zero', 'Forward', 'Flush', 'Other']
## fp32
#cfgs = {
#  'Traditional SA': [16.22434287, 61.67687776, 0.120462652, 8.131229001, 0.385386375, 13.46170135],
#  'MCSA':           [9.527622098, 76.5996739, 0.149608738, 0, 0.47863108, 13.24446418],
#  'Ideal MCSA':     [9.10987735, 79.2703873, 0.154824975, 0, 0, 11.46491037],
#}

## int32
cfgs = {
  'Traditional SA': [16.63488992, 62.16480275, 0.12141563, 8.19555505, 0.388435161, 12.49490149],
  'MCSA':           [9.904242059, 77.42865413, 0.15122784, 0, 0.483810942, 12.03206503],
  'Ideal MCSA':     [4.352034198, 88.09727048, 0.172064981, 0, 0, 7.378630336],
}

vals = np.array(cfgs['Traditional SA'])
sort_idxs = np.argsort(vals)[::-1]

names = [names[i] for i in sort_idxs]
for cfg in cfgs:
  cfgs[cfg] = [cfgs[cfg][i] for i in sort_idxs]

config_names = list(cfgs.keys())
values_array = np.array(list(cfgs.values()))

n_configs = len(config_names)
n_names = len(names)

bar_width = 0.15
x = np.arange(n_configs)

def get_distinct_pastels(n, lrange=(0.65, 0.85), s=0.6):
  colors = []
  for i in np.linspace(0, 1, n, endpoint=False):
    lightness = lrange[0] if (len(colors) % 2 == 0) else lrange[1]
    rgb = colorsys.hls_to_rgb(i, lightness, s)
    colors.append(rgb)
  return colors

colors = get_distinct_pastels(len(names))
cycle_colors = {cycle: colors[i % len(colors)] for i, cycle in enumerate(names)}

for i, cycle in enumerate(names):
  rects = plt.bar(x + i * bar_width, values_array[:, i], width=bar_width,
                  color=cycle_colors[cycle], label=cycle,
                  edgecolor='gray', linewidth=0.8)

  plt.bar_label(rects, padding=5, fmt='%.1f', fontsize=fontsize*0.7)

plt.xticks(x + bar_width*(n_names-1)/2, config_names, fontsize=fontsize)
plt.yticks(fontsize=fontsize*0.8)
plt.ylabel('Percentage (%)', fontsize=fontsize)
plt.legend(loc='best', fontsize=fontsize*0.9)
plt.grid(axis='y', linestyle='--', alpha=0.6)
plt.gca().set_axisbelow(True)

plt.ylim(0, 100)
plt.tight_layout()
plt.savefig(fout, bbox_inches="tight", pad_inches=0)
print(f'wrote: {fout}')
