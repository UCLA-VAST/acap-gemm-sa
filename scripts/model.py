#!/usr/bin/env python3

import argparse
parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument('model', help='ampl model file')
parser.add_argument('-s', '--size', metavar='m,k,n', required=True, help='mxkxn problem size')
parser.add_argument('-a', '--array', metavar='r,c', help='rxc AIE array')
parser.add_argument('--pl-size', metavar='pm,pk,pn', help='pmxpkxpn PL tile size')
parser.add_argument('--aie-size', metavar='am,ak,an', help='amxakxan AIE tile size')
parser.add_argument('--freq-compute', type=float, default=312.5, help='frequency for ideal compute (MHz)')
parser.add_argument('--freq-memory', type=float, default=250, help='frequency for ideal memory (MHz)')
parser.add_argument('-f', '--freq', type=float, help='custom frequency (MHz)')
parser.add_argument('-l', '--log', default='ampl.log', help='log file name')
parser.add_argument('-p', '--pool', type=int, default=1, help='solution pool size')
parser.add_argument('-c', '--compute-efficiency', type=float, default=1.0, help='scale AIE compute cycles')
parser.add_argument('-m', '--memory-efficiency', type=float, default=1.0, help='scale PL load/store cycles')
parser.add_argument('-b', '--bram', metavar='a,b,c', help='bind a,b,c to bram')
parser.add_argument('-u', '--memory-util', type=float, default=0.9, help='max memory unit utilization')
parser.add_argument('--enable-split', action='store_true', help='enable SA splitting')
parser.add_argument('--stub', default='solution', help='solution stub name')
parser.add_argument('--dtype', default='int32', help='data type')
parser.add_argument('-i', '--iis', action='store_true', help='compute irreducible inconsistent subsystem')
parser.add_argument('-t', '--time-limit', type=int, default=60, help='solver time limit')
args = parser.parse_args()

import amplpy
import numpy as np
import os, sys
from collections import ChainMap
from contextlib import contextmanager
from device import *
import sys
import time

m, k, n = map(int, args.size.split(','))

dbytes = eval(f'np.{args.dtype}().nbytes')
dbits = dbytes * 8

begin = ' BEGIN '.center(80, '=')
end = ' END '.center(80, '=')

print(begin)
print(f'Reading model {args.model} ...', flush=True)
ampl = amplpy.AMPL()
ampl.read(args.model)
print(end)

print(begin)
print(f'Configuring ampl ...', flush=True)
params = {
  # dimensions
  'm': m,
  'k': k,
  'n': n,
  'max_r': VCK5000.AIE_ROWS,
  'max_c': VCK5000.AIE_COLS,

  # memory
  'parts_a': 2,
  'parts_b': 2,
  'parts_c': 2,
  'bitwidth_dram': 512,
  'bitwidth_plio': 128,
  'bitwidth_word': 128,
  'bitwidth_data':  32,
  'bitwidth_aie':   32,

  'bram_bits': VCK5000.PL_BRAM.BITS,
  'bram_width': VCK5000.PL_BRAM.WIDTH,
  'bram_count': VCK5000.PL_BRAM.count(),
  'bram_util': args.memory_util,

  'uram_bits': VCK5000.PL_URAM.BITS,
  'uram_width': VCK5000.PL_URAM.WIDTH,
  'uram_count': VCK5000.PL_URAM.count(),
  'uram_util': args.memory_util,

  # aie
  'aie_pl_clock_ratio': 4,
  'aie_store_bits_per_cycle': VCK5000.AIE_STORE_BITS_PER_CYCLE,
  'aie_mac_per_cycle': VCK5000.AIE_MAC_PER_CYCLE[dbits],
  'aie_vlen': VCK5000.AIE_VLEN[dbits],
  'aie_alen': VCK5000.AIE_ALEN,
  'aie_mem_bytes': VCK5000.AIE_MEM_BYTES,

  # efficiency
  'comp_eff': args.compute_efficiency,
  'mem_eff': args.memory_efficiency,

  'enable_split': args.enable_split,
}

if args.array:
  r, c = map(int, args.array.split(','))
  params.update({
    'fixed_rc': 1,
    'r': r,
    'c': c,
  })

if args.pl_size:
  pm, pk, pn = map(int, args.pl_size.split(','))
  params.update({
    'fixed_pl': 1,
    'pm': pm,
    'pk': pk,
    'pn': pn,
  })

if args.aie_size:
  am, ak, an = map(int, args.aie_size.split(','))
  params.update({
    'fixed_aie': 1,
    'am': am,
    'ak': ak,
    'an': an,
  })

if args.bram:
  a, b, c = map(int, args.bram.split(','))
  params.update({
    'fixed_bram': 1,
    'bram_a': a,
    'bram_b': b,
    'bram_c': c,
  })

mp_options = [
  #'obj:multi=1',
  'tech:outlev=1',
  f'tech:logfile={args.log}',
  f'sol:poolmode=2',
  f'sol:poollimit={args.pool}',
  f'sol:stub={args.stub}',
  'mip:inttol=1e-9',
  'mip:gap=1e-9',
  f'iisfind={int(bool(args.iis))}',
  f'lim:time={args.time_limit}',
]

threads = os.environ.get("SLURM_CPUS_ON_NODE", None)
if threads:
  mp_options.append(f'threads={threads}')

options = {
  'solver': 'gurobi',
  'mp_options': ' '.join(mp_options),
}

for key, val in params.items():
  ampl.param[key] = val

for key, val in options.items():
  ampl.option[key] = val
print(end)

print(begin)
print(f'Solving model ...', flush=True)
t0 = time.time()
ampl.solve(verbose=True)
solving_time = time.time() - t0
print(end)

if ampl.solve_result == 'solved':
  pass
else:
  if args.iis:
    vars, cons = ampl.get_iis()
    print(vars, cons)
   
  print(f'warning: no feasible solution')
  sys.exit(0)

@contextmanager
def inject_globals(d):
  _globals = globals()
  try:
    globals().update(d)
    yield
  finally:
    for k in set(globals().keys()) - set(_globals.keys()):
      del globals()[k]
    globals().update(_globals)

def print_solution(sol):
  with inject_globals(sol):
    Ops = 2*m*k*n
    OpsPad = 2*PadM*PadK*PadN
    PLBRAMUnitsUtil = PLBRAMUnits / VCK5000.PL_BRAM.count() * 100
    PLURAMUnitsUtil = PLURAMUnits / VCK5000.PL_URAM.count() * 100

    PLFreqMHzCompute = args.freq_compute
    PLSecondsCompute = PLCycles / PLFreqMHzCompute / 1e6
    PLBufferSecondsCompute = PLBufferCycles / PLFreqMHzCompute / 1e6
    GOPsCompute = Ops / PLSecondsCompute / 1e9
    GOPsPadCompute = OpsPad / PLSecondsCompute / 1e9

    PLFreqMHzMemory = args.freq_memory
    PLSecondsMemory = PLCycles / PLFreqMHzMemory / 1e6
    PLBufferSecondsMemory = PLBufferCycles / PLFreqMHzMemory / 1e6
    GOPsMemory = Ops / PLSecondsMemory / 1e9
    GOPsPadMemory = OpsPad / PLSecondsMemory / 1e9

    #print(sol, flush=True)
    print(f'''
  Rows={Rows} Cols={Cols} PhysRows={PhysRows} PhysCols={PhysCols} SplitRows={SplitRows}
  m={m}({PadM}) k={k}({PadK}) n={n}({PadN}) PM={PM} PK={PK} PN={PN} AM={AM} AK={AK} AN={AN}
  BM1={BM1} BK1={BK1} BN1={BN1} BM2={BM2} BK2={BK2} BN2={BN2} BM3={BM3} BK3={BK3} BN3={BN3}
  HostBytes={HostBytes} PLBytes={PLBytes} AIEBytes={AIEBytes} AOnBRAM={AOnBRAM} BOnBRAM={BOnBRAM} COnBRAM={COnBRAM}
    PLBRAMUnits={PLBRAMUnits}({PLBRAMUnitsUtil:.1f}%) PLBRAMUnitsA={PLBRAMUnitsA} PLBRAMUnitsB={PLBRAMUnitsB} PLBRAMUnitsC={PLBRAMUnitsC}
    PLURAMUnits={PLURAMUnits}({PLURAMUnitsUtil:.1f}%) PLURAMUnitsA={PLURAMUnitsA} PLURAMUnitsB={PLURAMUnitsB} PLURAMUnitsC={PLURAMUnitsC}
  AIEBufferReadCycles={AIEBufferReadCycles} AIECompCycles={AIECompCycles} AIEFlushCycles={AIEFlushCycles} AIECycles={AIECycles} AIECyclesPL={AIECyclesPL}
  PLBufferReadACycles={PLBufferReadACycles} PLBufferReadBCycles={PLBufferReadBCycles} PLBufferWriteCycles={PLBufferWriteCycles} 
  PLBufferCompCycles={PLBufferCompCycles} PLBufferReuse={PLBufferReuse} SACompCycles={SACompCycles} SABufferSendCycles={SABufferSendCycles}
  PLBufferCycles={PLBufferCycles} PLCycles={PLCycles}

  Ops={2*m*k*n} OpsPad={2*PadM*PadK*PadN}
  @{PLFreqMHzCompute:.1f}MHz (compute frequency):
    PLSeconds={PLSecondsCompute:.3g} PLBufferSeconds={PLBufferSecondsCompute:.3g}
    Estimated GOP/s: {GOPsCompute:.3f}
    Estimated Padded GOP/s: {GOPsPadCompute:.3f}

  @{PLFreqMHzMemory:.1f}MHz (memory frequency):
    PLSeconds={PLSecondsMemory:.3g} PLBufferSeconds={PLBufferSecondsMemory:.3g}
    Estimated GOP/s: {GOPsMemory:.3f}
    Estimated Padded GOP/s: {GOPsPadMemory:.3f}
''', flush=True)

    if args.freq:
      PLFreqMHzRequested = args.freq
      PLSecondsRequested = PLCycles / PLFreqMHzRequested / 1e6
      PLBufferSecondsRequested = PLBufferCycles / PLFreqMHzRequested / 1e6
      GOPsRequested = Ops / PLSecondsRequested / 1e9
      GOPsPadRequested = OpsPad / PLSecondsRequested / 1e9

      print(f'''  @{PLFreqMHzRequested:.1f}MHz (requested frequency):
    PLSeconds={PLSecondsRequested:.3g} PLBufferSeconds={PLBufferSecondsRequested:.3g}
    Estimated GOP/s: {GOPsRequested:.3f}
    Estimated Padded GOP/s: {GOPsPadRequested:.3f}
''', flush=True)

print(begin)
for i in range(1, args.pool+1):
  print('Solution {i}:\n{border}'.format(border='-'*80, i=i))
  ampl.eval(f'solution {args.stub}{i}.sol;')
  sol = ampl.get_solution(flat=False, zeros=True)
  print_solution(sol)
print(end)

print(f'Solving time: {solving_time:.2f} seconds')

