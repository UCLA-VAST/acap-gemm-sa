#!/usr/bin/env python3

from parse_profile import parse_profile
import argparse
import numpy as np
import subprocess as sp
import os
import glob
from typing import Iterable, Optional, Tuple, List

def comma_separated_list(s: str) -> List[str]:
  return s.split(',')

def float_range(s: str) -> Tuple[float, float]:
  parts = s.split(',')
  if len(parts) != 2:
    raise argparse.ArgumentTypeError('range must be two comma-separated numbers: vmin,vmax')
  return float(parts[0]), float(parts[1])

def heatmap(
  simouts: Iterable[Tuple[str, str]],
  fout: str = '',
  vminmax: Optional[Tuple[float, float]] = None,
  height_per_plot: float = 12.0,
) -> None:

  simouts_list = list(simouts)
  n = len(simouts_list)

  eff_matrices = []
  auto_vmin, auto_vmax = float('inf'), float('-inf')

  for simout, func in simouts_list:
    #print(f'processing: {simout}')
    fnames = glob.glob(os.path.join(simout, 'profile_instr_*.txt'))

    parsed_files = []
    min_c, min_r = float('inf'), float('inf')
    max_c, max_r = float('-inf'), float('-inf')
    for fname in fnames:
      stem = os.path.splitext(os.path.basename(fname))[0]
      c, r = map(int, stem.split('_')[2:4])
      min_c, min_r = min(min_c, c), min(min_r, r)
      max_c, max_r = max(max_c, c), max(max_r, r)
      parsed_files.append((c, r, fname))

    if not parsed_files:
      continue

    C, R = (max_c - min_c) + 1, (max_r - min_r) + 1
    matrix = np.zeros((R, C))

    for c, r, fpath in parsed_files:
      eff = parse_profile(fpath, func, verbose=False)
      if eff >= 0:
        matrix[r - min_r, c - min_c] = eff
        auto_vmin, auto_vmax = min(auto_vmin, eff), max(auto_vmax, eff)

    eff_matrices.append(matrix)
    print(f'{simout}: {matrix.min():.3f} {matrix.mean():.3f} {matrix.max():.3f}')

  if fout:
    import seaborn as sns
    import matplotlib as mpl
    import matplotlib.pyplot as plt
    import matplotlib.ticker as ticker

    vmin, vmax = vminmax if vminmax else (auto_vmin, auto_vmax)

    R, C = eff_matrices[0].shape
    fig_width = (C / R) * height_per_plot
    fig_height = height_per_plot * n

    sns.set_theme(context='notebook', style='white')

    base = height_per_plot * 12
    mpl.rcParams.update({
      'font.size': base,
      #'xtick.labelsize': base,
      #'ytick.labelsize': base,
    })

    #sns.set_theme(context='notebook', font_scale=height_per_plot * 0.4, style='white')
    plt.rcParams['mathtext.fontset'] = 'cm'
    plt.rcParams['font.family'] = 'STIXGeneral'

    fig, axes = plt.subplots(
      nrows=n,
      ncols=1,
      figsize=(fig_width, fig_height),
      constrained_layout=True,
    )
    #plt.subplots_adjust(left=0.2, bottom=0.2, hspace=0.5)

    fig.set_constrained_layout_pads(h_pad=0.5)
    axes = [axes] if n == 1 else axes

    for i, (ax, matrix) in enumerate(zip(axes, eff_matrices)):
      data_min = matrix.min().min()
      data_max = matrix.max().max()
      padding = (data_max - data_min) * 0.1
      ticks = np.linspace(data_min + padding, data_max - padding, 3)

      hm = sns.heatmap(
        matrix, ax=ax, #vmin=vmin, vmax=vmax,
        square=True, cbar=True,
        xticklabels=False, 
        yticklabels=False,
        cbar_kws={
          'pad': 0.02, 
          'aspect': 10, 
          'ticks': ticks,
          'format': ' %.3f',
        },
      )
      ax.invert_yaxis()

      hm.collections[0].colorbar.ax.tick_params(length=base*0.2, labelsize=base*0.8)

      #ax.set_xticks(np.arange(matrix.shape[1]) + 0.5)
      #ax.set_yticks(np.arange(matrix.shape[0]) + 0.5)

      #ax.set_xticklabels([str(i) for i in range(matrix.shape[1])], rotation=90, ha='center')
      #ax.set_yticklabels([str(i) for i in range(matrix.shape[0])], rotation=90, ha='right')

      #ax.tick_params(left=True, bottom=True, direction='out', length=base * 0.2)

      if n > 1:
        ax.text(
          0.5, -0.2, f'({chr(97+i)})',
          transform=ax.transAxes, ha='center', va='center'
        )

    #cbar = fig.colorbar(
    #  hm.collections[0], ax=axes,
    #  fraction=0.1,
    #  pad=0.02,
    #  #aspect=12 * n
    #)
    #cbar.ax.tick_params(length=base * 0.2, labelsize=base*1.2)

    out_path = os.path.abspath(fout)
    print(f'writing: {out_path}')
    plt.savefig(out_path, bbox_inches='tight')
    plt.close(fig)

def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('-d', '--dirs', type=comma_separated_list, required=True, help='aiesimulator_output directories')
  parser.add_argument('-f', '--funcs', type=comma_separated_list, help='profile function names')
  parser.add_argument('-r', '--range', type=float_range, metavar='vmin,vmax', help='set vmin and vmax')
  parser.add_argument('-o', '--out', help='set output filename')
  parser.add_argument('--height', type=float, default=12.0, help='set height per plot')
  args = parser.parse_args()

  if args.out:
    import seaborn as sns
    import matplotlib as mpl
    import matplotlib.pyplot as plt
    import matplotlib.ticker as ticker

  heatmap(zip(args.dirs, args.funcs or ['impl']*len(args.dirs)), fout=args.out, vminmax=args.range, height_per_plot=args.height)

if __name__ == '__main__':
  main()
