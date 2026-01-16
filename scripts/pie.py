#!/usr/bin/env python3

import matplotlib.pyplot as plt
import os

fout = os.path.abspath('pie.pdf')

plt.rcParams['mathtext.fontset'] = 'cm'
plt.rcParams['font.family'] = 'STIXGeneral'
plt.rcParams['font.size'] = 14

cycles = [
  ('Stalls', 34479),
  ('VMAC', 131072),
  ('Zero', 256),
  ('FWDA', 8640),
  ('FWDB', 8640),
  ('Flush', 819),
  ('Other', 28608),
]
cycles = sorted(cycles, key=lambda x: x[1], reverse=True)

explode = [0]*len(cycles)
explode[0] = 0.1

def autopct(pct):
    return f'{pct:.1f}%' if pct >= 1.0 else ''

plt.pie(
  list(map(lambda x: x[1], cycles)),
  autopct=autopct,
  #startangle=90,
  #explode=explode,
  #shadow=True,
)

plt.legend(list(map(lambda x: x[0], cycles)), loc='upper right')
plt.axis('equal')
#plt.title('Cycle Distribution')
plt.tight_layout()
plt.savefig(fout)
print(f'wrote: {fout}')
