#!/usr/bin/env python3

import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import os

fout = os.path.abspath('misc.pdf')

plt.rcParams['mathtext.fontset'] = 'cm'
plt.rcParams['font.family'] = 'STIXGeneral'
plt.rcParams['font.size'] = 18

problem_sizes = [512, 1024, 2048, 3072, 4096, 6144, 8192]

# normal build
baseline = [1063.724, 2720.774, 4297.282, 5279.402, 4744.339, 5413.673, 5309.325]
# change PL_FREQ_MHZ in CMakeLists.txt
freq_250MHz = [1029.645, 2123.872, 4291.142, 5232.982, 4746.716, 5442.575, 5298.115]
# change DRAM access in dma.cc to generate data on-chip & sink output data
no_dram = [1215.203, 3021.115, 4509.360, 5349.898, 4785.075, 5475.339, 5316.964]

plt.figure(figsize=(10, 3.6))
plt.plot(problem_sizes, baseline, marker='o', label='Baseline')
plt.plot(problem_sizes, freq_250MHz, marker='s', label='250 MHz')
plt.plot(problem_sizes, no_dram, marker='^', label='No DRAM')

plt.xlabel('Problem Size')
plt.ylabel('GOP/s')
plt.xticks(problem_sizes)
plt.gca().yaxis.set_major_locator(ticker.MaxNLocator(nbins=5))
plt.grid(True)
plt.legend()
plt.tight_layout()

plt.savefig(fout, bbox_inches="tight", pad_inches=0)
print(f'wrote: {fout}')
