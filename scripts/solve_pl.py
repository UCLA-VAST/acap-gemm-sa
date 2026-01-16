#!/usr/bin/env python3

import argparse
parser = argparse.ArgumentParser()
parser.add_argument('-a', '--array', default='1,1',
                    metavar='R,C', help='RxC AIE array')
parser.add_argument('-t', '--tile', default='16,64,16',
                    metavar='TM,TK,TN', help='set TMxTKxTN tile size')
parser.add_argument('-s', '--size', 
                    metavar='M,K,N', help='set any value in MxKxN')
parser.add_argument('-c', '--check',
                    metavar='PM,PK,PN', help='set any value for PMxPKxPN')
parser.add_argument('-d', '--dram-width', default=512, type=int,
                    metavar='D', help='dram bitwidth')
parser.add_argument('-p', '--plio-width', default=128, type=int,
                    metavar='P', help='plio bitwidth')
parser.add_argument('-w', '--data-width', default=32, type=int,
                    metavar='W', help='data bitwidth')
parser.add_argument('-u', '--upper', default=800, type=int,
                    metavar='U', help='PL dimension upper limit')
parser.add_argument('-m', '--macs', default=8, type=int,
                    help='AIE macs per cycle')
parser.add_argument('--multibank', default=False, action=argparse.BooleanOptionalAction,
                    help='enables multiple banks')
args = parser.parse_args()

TM, TK, TN = map(int, args.tile.split(','))
R, C = map(int, args.array.split(','))
W = args.data_width
D = args.dram_width
P = args.plio_width
U = args.upper
macs = args.macs
multibank = args.multibank
SM = TM*R
SK = TK
SN = TN*C

if args.size:
  MM, MK, MN = map(lambda x: int(x) if x else None, args.size.split(','))
else:
  MM = MK = MN = None

if args.check:
  CM, CK, CN = map(lambda x: int(x) if x else None, args.check.split(','))
else:
  CM = CK = CN = None

from z3 import *
set_param(proof=True)

PM, PK, PN = Ints('PM PK PN')
F, FM, FK, FN = Ints('F FM FK FN')

if MM: ZM = Int('ZM')
if MK: ZK = Int('ZK')
if MN: ZN = Int('ZN')

def add_constraints(s):
  s.add(PM>0, PK>0, PN>0, F>0, FM>0, FK>0, FN>0)
  s.add(PM % SM == 0, PK % SK == 0, PN % SN == 0)
  s.add(PM == SM*FM, PK == SK*FK, PN == SN*FN)
  s.add(F == FM*FK*FN)
  s.add(PM <= U, PK <= U, PN <= U)
  s.add(PK / SK >= R)

  if MM: s.add(ZM == MM, ZM % PM == 0)
  if MK: s.add(ZK == MK, ZK % PK == 0)
  if MN: s.add(ZN == MN, ZN % PN == 0)

  if CM: s.add(PM == CM)
  if CK: s.add(PK == CK)
  if CN: s.add(PN == CN)

def solve(s, d):
  if s.check() == sat:
    m = s.model()

    vF, vFM, vFK, vFN = map(lambda x: x.as_long(), (m[F], m[FM], m[FK], m[FN]))
    vPM, vPK, vPN = map(lambda x: x.as_long(), (m[PM], m[PK], m[PN]))

    AIE_comm = max(TM*TK, TK*TN) * W / P * vF
    AIE_comp = TM*TK*TN / macs / (P/W) * vF
    AIE = max(AIE_comm, AIE_comp)
    if multibank:
      PL = max(vPM*vPK, vPK*vPN) * W / D
    else:
      PL = (vPM*vPK + vPK*vPN) * W / D

    t = ((vPM, vPK, vPN), (vFM, vFK, vFN))
    v = PL - AIE
    if v not in d:
      d[v] = []
    d[v].append(t)

    return t[0]
  else:
    return (None, None, None)

s = Solver()
add_constraints(s)

print(f'R={R} C={C} TM={TM} TK={TK} TN={TN} SM={SM} SK={SK} SN={SN} W={W} D={D} P={P}')
print(f'MM={MM} MK={MK} MN={MN} CM={CM} CK={CK} CN={CN}')
print('='*80)

d = {}
i = 0
while True:
  print(f'num solutions: {i}', end='\r')
  vPM, vPK, vPN = solve(s, d)

  if not all((vPM, vPK, vPN)):
    print()
    print('='*80)
    break

  i += 1
  s.add(Not(And(PM == vPM, PK == vPK, PN == vPN)))

if i == 0:
  print('UNSAT proof:')
  print(s.proof())

for k, v in sorted(d.items()):
  print(k, sorted(v))

if d:
  print('='*80)

  from functools import reduce
  from operator import mul

  k_min = min((abs(k) for k in d))
  p_min = 1e9
  opt = None
  for v in d[k_min]:
    p = reduce(mul, v[0])
    if p < p_min:
      p_min = p
      opt = v

  print('Optimal:')
  print('  min(abs(PL-AIE))')
  print('  min(PM*PK*PN)')
  print(f'PL-AIE={k_min} solution={opt}')

