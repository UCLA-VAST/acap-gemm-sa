#!/usr/bin/env python3

import numpy as np
import argparse
import sys
import os
import math

parser = argparse.ArgumentParser(description='generate data for matmul(MxK, KxN)', 
                                 formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument('-d', '--dim', required=True, metavar='M,K,N', help='MxKxN problem size')
parser.add_argument('-t', '--tile', required=True, metavar='TM,TK,TN', help='TMxTKxTN tile size')
parser.add_argument('-m', '--mmul', default='1,1,1', metavar='MM,KK,NN', help='MMxKKxNN mmul size')
parser.add_argument('-a', '--array', required=True, metavar='R,C', help='RxC AIE array')
parser.add_argument('-l', '--lower', type=int, default=1, help='inclusive lower bound')
parser.add_argument('-u', '--upper', type=int, default=10, help='exclusive upper bound')
parser.add_argument('-p', '--plio', type=int, default='128', help='plio bitwidth')
parser.add_argument('-o', '--out-dir', default='data', help='output dir')
parser.add_argument('--dtype', default='int32', help='numpy dtype')
parser.add_argument('--mem', type=int, default=32, help='memory (KB) per tile')
parser.add_argument('--write', action=argparse.BooleanOptionalAction, default=True, help='write data')
group = parser.add_mutually_exclusive_group()
group.add_argument('--indices', action=argparse.BooleanOptionalAction, default=False, help='use indices as values')
group.add_argument('--ones', action=argparse.BooleanOptionalAction, default=False, help='use ones as values')
group.add_argument('--rows', action=argparse.BooleanOptionalAction, default=False, help='use row indices as values')
group.add_argument('--cols', action=argparse.BooleanOptionalAction, default=False, help='use col indices as values')
group.add_argument('--identity', action=argparse.BooleanOptionalAction, default=False, help='use identity of indices as values')
args = parser.parse_args()

M, K, N = map(int, args.dim.split(','))
TM, TK, TN = map(int, args.tile.split(','))
MM, KK, NN = map(int, args.mmul.split(','))
ROWS, COLS = map(int, args.array.split(','))

#if M % (ROWS * TM) != 0:
#  print(f'error: M({M}) % (ROWS({ROWS}) * TM({TM})) != 0')
#  sys.exit(1)
#
#if N % (COLS * TN) != 0:
#  print(f'error: N({N}) % (COLS({COLS}) * TN({TN})) != 0')
#  sys.exit(1)

if TM % MM != 0:
  print(f'error: TM({TM}) % MM({MM}) != 0')
  sys.exit(1)

if TK % KK != 0:
  print(f'error: TK({TK}) % KK({KK}) != 0')
  sys.exit(1)

if TN % NN != 0:
  print(f'error: TN({TN}) % NN({NN}) != 0')
  sys.exit(1)

#if TK*TN != TM*TN:
#  print(f'error: TK({TK})*TN({TN}) != TM({TM})*TN({TN})')
#  sys.exit(1)

dbytes = eval(f'np.{args.dtype}().nbytes')
dbits = dbytes * 8
if args.plio % dbits != 0:
  print(f'error: plio({args.plio}) % dbits({dbits}) != 0')
  sys.exit(1)
n_cols = args.plio // dbits
print(f'generating data using dtype={args.dtype} dbytes={dbytes} plio={args.plio}b')

if args.indices:
  A = np.reshape(np.indices((M*K,)), (M, K))
  B = np.reshape(np.indices((K*N,)), (K, N))
elif args.ones:
  A = np.ones((M, K))
  B = np.ones((K, N))
elif args.rows:
  A = np.arange(M).reshape(M, 1).repeat(K, axis=1)
  B = np.arange(K).reshape(K, 1).repeat(N, axis=1)
elif args.cols:
  A = np.arange(K).reshape(1, K).repeat(M, axis=0)
  B = np.arange(N).reshape(1, N).repeat(K, axis=0)
elif args.identity:
  A = np.reshape(np.indices((M*K,)), (M, K))
  B = np.eye(K, N)
else:
  A = np.random.randint(args.lower, args.upper, (M, K))
  B = np.random.randint(args.lower, args.upper, (K, N))

A = eval(f'A.astype(np.{args.dtype})')
B = eval(f'B.astype(np.{args.dtype})')
C = np.matmul(A, B)

print(f'A: shape={A.shape} type={A.dtype} mem={A.nbytes}B\n{A}')
print(f'B: shape={B.shape} type={B.dtype} mem={B.nbytes}B\n{B}')
print(f'C: shape={C.shape} type={C.dtype} mem={C.nbytes}B\n{C}')

os.makedirs(args.out_dir, exist_ok=True)

def update_symlink(lname, fname):
  bname = os.path.basename(fname)
  dname = os.path.dirname(fname)
  dst = os.path.join(dname, lname)
  os.symlink(bname, lname)
  os.replace(lname, dst)
  print(f'wrote {dst} -> {fname}')

class SimDataWriter:
  def __init__(self, fname, n_cols, fmt='%s'):
    self.f = open(fname, 'w')
    self.n_cols = n_cols
    self.i = 0
    self.fmt = fmt

  def __enter__(self):
    return self

  def __exit__(self, exc_type, exc_value, traceback):
    self.f.close()

  def write(self, x):
    self.f.write(self.fmt % x)
    if self.i % self.n_cols == self.n_cols - 1:
      self.f.write('\n')
    else:
      self.f.write(' ')
    self.i += 1

  def write_skip(self, x):
    self.f.write(f'SKIP {x}\n')

fmt_A = f'%{len(str(A.max()))}s'
fmt_B = f'%{len(str(B.max()))}s'
fmt_C = f'%{len(str(C.max()))}s'

qual = f'{M}x{K}x{N}-{TM}x{TK}x{TN}-{MM}x{KK}x{NN}-{ROWS}x{COLS}-w{args.plio}'

if args.write:
  fname = os.path.join(args.out_dir, f'A-{qual}.txt')
  np.savetxt(fname, A, fmt=fmt_A)
  update_symlink('A.txt', fname)

  fname = os.path.join(args.out_dir, f'B-{qual}.txt')
  np.savetxt(fname, B, fmt=fmt_B)
  update_symlink('B.txt', fname)

  fname = os.path.join(args.out_dir, f'C-{qual}.txt')
  np.savetxt(fname, C, fmt=fmt_C)
  update_symlink('C.txt', fname)

BM2 = math.ceil(M / TM / ROWS)
BK2 = math.ceil(K / TK)
BN2 = math.ceil(N / TN / COLS)
BM3 = TM // MM
BK3 = TK // KK
BN3 = TN // NN

# note: avoid slices to facilitate porting to C++
if args.write:
  for r in range(ROWS):
    fname = os.path.join(args.out_dir, f'in0_{r}-{qual}.txt')

    with SimDataWriter(fname, n_cols, fmt_A) as writer:
      for bm2 in range(BM2):
        for bn2 in range(BN2):
          for bk2 in range(BK2):
            for bm3 in range(BM3):
              for bk3 in range(BK3):
                for mm in range(MM):
                  y = bm2*TM*ROWS + r*TM + bm3*MM + mm
                  for kk in range(KK):
                    x = bk2*TK + bk3*KK + kk
                    if y < A.shape[0] and x < A.shape[1]:
                      writer.write(f'{A[y,x]}')
                    else:
                      writer.write('0')

    update_symlink(f'in0_{r}.txt', fname)

  for c in range(COLS):
    fname = os.path.join(args.out_dir, f'in1_{c}-{qual}.txt')

    with SimDataWriter(fname, n_cols, fmt_B) as writer:
      for bm2 in range(BM2):
        for bn2 in range(BN2):
          for bk2 in range(BK2):
            for bk3 in range(BK3):
              for bn3 in range(BN3):
                for kk in range(KK):
                  y = bk2*TK + bk3*KK + kk
                  for nn in range(NN):
                    x = bn2*TN*COLS + c*TN + bn3*NN + nn
                    if y < B.shape[0] and x < B.shape[1]:
                      writer.write(f'{B[y,x]}')
                    else:
                      writer.write('0')

    update_symlink(f'in1_{c}.txt', fname)

  for c in range(COLS):
    fname = os.path.join(args.out_dir, f'out0_{c}-{qual}.txt')

    with SimDataWriter(fname, n_cols, fmt_C) as writer:
      for bm2 in range(BM2):
        for bn2 in range(BN2):
          #writer.write_skip((BK2-1)*TM*TN)

          for r in range(ROWS):
            for bm3 in range(BM3):
              for bn3 in range(BN3):
                for mm in range(MM):
                  y = bm2*TM*ROWS + r*TM + bm3*MM + mm
                  for nn in range(NN):
                    x = bn2*TN*COLS + c*TN + bn3*NN + nn
                    if y < C.shape[0] and x < C.shape[1]:
                      writer.write(f'{C[y,x]}')
                    else:
                      writer.write('0')

    update_symlink(f'out0_{c}.txt', fname)

def serialize_mmul(X, mr, mc):
  flat = X.copy().flatten()
  idx = 0
  
  for r in range(0, X.shape[0], mr):
    for c in range(0, X.shape[1], mc):
      block = X[r:r+mr, c:c+mc].flatten()
      flat[idx:idx + block.size] = block
      idx += block.size
  
  return flat.reshape(X.shape)

if args.write:
  At = serialize_mmul(A, MM, KK)
  fname = os.path.join(args.out_dir, f'At-{qual}.txt')
  np.savetxt(fname, At, fmt=fmt_A)
  update_symlink('At.txt', fname)

  Bt = serialize_mmul(B, KK, NN)
  fname = os.path.join(args.out_dir, f'Bt-{qual}.txt')
  np.savetxt(fname, Bt, fmt=fmt_B)
  update_symlink('Bt.txt', fname)

  Ct = serialize_mmul(C, MM, NN)
  fname = os.path.join(args.out_dir, f'Ct-{qual}.txt')
  np.savetxt(fname, Ct, fmt=fmt_C)
  update_symlink('Ct.txt', fname)

in0 = TM*TK
in1 = TK*TN
in2 = TM*TN
out0 = TM*TN
buf0 = TM*TN
mem = (2*(in0 + in1 + in2 + out0) + buf0)*dbytes
print(f'BM2={BM2} BK2={BK2} BN2={BN2} iters={BM2*BK2*BN2 + ROWS-1} mem={mem}')

if mem > args.mem*1024:
  print(f'!! warning: tile size {TM}x{TK}x{TN} exceeds tile memory ({args.mem} KB) !!')
