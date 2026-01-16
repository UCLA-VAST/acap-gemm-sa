# acap-gemm-sa

A systolic array-based GEMM implementation for AMD Versal ACAP.

## Requirements

- CMake 3.23+
- GNU Make
- Vitis 2023.1
- C++17
- (For solver) ``amplpy`` with Gurobi, ``numpy``
- (For figures) ``matplotlib``, ``seaborn``

## Build and Run

Files to edit in `$SRC_DIR`:
- `parameters.hh`: Set design configuration
- `xsa.cfg`: Comment/uncomment connectivity based on array dimensions
- `gemm.hh`: Comment/uncomment inlining for AIE simulation
- `CMakeLists.txt`: Set desired frequency

```console
mkdir build && cd build
cmake .. [-DVPP_JOBS=<n>] [-DVPP_OPTIMIZE=0..3] [-DXILINX_TARGET=hw|hw_emu|sw_emu] # cmake/xilinx-setup.cmake
make -j [VERBOSE=1] gemm
[XCL_EMULATION_MODE=sw_emu|hw_emu] ./bin/gemm ./<xclbin> [DEV_IDX]
```

## Run AIE Simulation

Generate data:

```console
cd $SRC_DIR
./generate_gemm_data.py [--help] -d <PM,PK,PN> -t <AM,AK,AN> -a <R,C>
```

Run simulation:

```console
cd $BUILD_DIR
make -j [VERBOSE=1] gemm-x86sim|gemm-aiesim
```

## Run Solver

```console
./scripts/model.py ./scripts/model.mod [--help] -s <M,K,N>
```

## Run Scripts

```console
./scripts/parse_profile.py [--help] -i <profile_instr> [-f <func>] [--no-stalls]
./scripts/monitor_power.py [--help] -d <bdf>
./scripts/heatmap.py [--help] -d <aiesimulator_output,...> -f <func,...> [-o heatmap.pdf]
./scripts/plot_bar.py
./scripts/plot_misc.py
./scripts/plot_scaling.py
```

## Reproducing Results

### Table III

We use a $(1, 1, 1)$ L1 and $(4, 4, 4)$ L2 iteration space for single-core simulations.
For example, for $(M_a, K_a, N_a) = (16, 64, 16)$, we use a $(64, 256, 64)$ problem and PL tile size:

```console
# Estimated Time: ~5 minutes
cd src
vim parameters.hh # DT=int32_t, M=64, K=256, N=64, PL_M=64, PL_K=256, PL_N=64, AIE_M=16, AIE_K=64, AIE_N=16,
                  # DEF_AIE_ROWS=1, DEF_AIE_COLS=1, DEF_PARTS=1
./generate_gemm_data.py -d 64,256,64 -t 16,64,16 -a 1,1
vim gemm.hh # apply (uncomment) `noinline` for ONLY `compute`
cd ..
mkdir build
cd build
cmake .. -DVPP_JOBS=16
make -j VERBOSE=1 gemm-aiesim
../scripts/parse_profile.py -f compute -i src/aiesimulator_output/profile_instr_0_0.txt
```

### Figure 6

Same steps as with Table III (can reuse build), but with the parameters listed in the paper.
For each method, use the corresponding `src*` directory and `gemm*-aiesim` target, e.g.:

```console
# Estimated Time: ~4 hours
cd ../src-trad
vim parameters.hh # apply config
./generate_gemm_data.py -d 1024,512,800 -t 16,64,16 -a 8,50 --identity
vim gemm.hh # apply (uncomment) `noinline` for ONLY `impl`
cd ../build
make -j VERBOSE=1 gemm-trad-aiesim
```

- VMAC: Reported cycles from `parse_profile.py` on `profile_instr_25_4.txt`
- Stall: Total cycle difference from `parse_profile.py` using `--no-stalls`
- Zero: Manually parsed `VST` pipeline block
- Forward: Manually parsed `VLD` and `VST` pipeline block after `ACQ` on $A$ and $B$
- Flush: Manually parsed `VLD` and `VST` pipeline block after `VMAC` pipeline and `ACQ` on $C_{in}$ and $C_{out}$
- Other: Remaining cycles

### Figure 7

Same steps as with Figure 6 (7c already done), but with different targets and inlining.
Save `aiesimulator_output` directories to somewhere safe and with a unique name, e.g. `sim-comp`.

- 7a: use `src`, `gemm-aiesim`, and `compute`
- 7b: use `src`, `gemm-aiesim`, and `impl`
- 7c: use `src-trad`, `gemm-trad-aiesim`, and `impl`
- 7d: use `src-ideal`, `gemm-ideal-aiesim`, and `impl`

To generate the heatmap (requires `seaborn` and `numpy`):

```console
$TOP_DIR/scripts/heatmap.py -d sim-comp,sim-mcsa,sim-trad,sim-ideal -f compute,impl,impl,impl -o heatmap.png
```

### Figure 8

Use an initial configuration of:
- `DT = int32_t`
- `(M, K, N) = (PL_M, PL_K, PL_N) = (128,512,16)`
- `(AIE_M, AIE_K, AIE_N) = (16, 64, 16)`
- `(DEF_AIE_ROWS, DEF_AIE_COLS) = (1, 1)`
- `noinline => impl`

As `DEF_AIE_ROWS`/`DEF_AIE_COLS` increases, scale `M = PL_M`/`N = PL_N` by the same factor,
e.g., for a `(6, 32)` SA, use `(128*6=768, 512, 16*32=512)`.

Run simulations for all configurations, then collect average efficiencies.
For example, if each `aiesimulator_output` is saved as `<R>x<C>` in the current directory
and are the only directories present (alternatively, use symlinks):

```console
$TOP_DIR/scripts/heatmap.py -d $(fd -td -d1 | sort -V | paste -sd',' -) | awk '{print $1,$3}'
vim $TOP_DIR/scripts/plot_scaling.py # transfer averages to script
$TOP_DIR/scripts/plot_scaling.py
```

### Table IV

Apply any of the configurations using ``parameters.hh`` and ``xsa.cfg``,
and examine ``src/work.hls/hls/syn/report/dma_csynth.rpt``.

```console
# Estimated Time: ~20 minutes
vim src/parameters.hh # apply config (use DEF_PARTS=2 for 2x2+ configs)
vim src/xsa.cfg # comment out unused streams (in0 = rows, in1 = cols, out0 = cols)
mkdir build && cd build
cmake .. -DVPP_JOBS=16 -DVPP_OPTIMIZE=3
make -j VERBOSE=1 gemm-xo
```

### Table V

Same steps as with Table IV (can reuse build), but build the full target and execute.

```console
# Estimated Time: up to ~12 hours depending on SA size
make -j VERBOSE=1 gemm
./bin/gemm ./lib/gemm.hw.xilinx_vck5000_gen4x8_qdma_2_202220_1.xclbin <dev_idx>
```

### Table VI

Increase ``iters`` in ``host.cc`` to a very large value or make the loop infinite.
(Can reuse build from Table V.)

```console
# Estimated Time: 3 minutes
make -j VERBOSE=1 gemm-host
./bin/gemm ./lib/gemm.hw.xilinx_vck5000_gen4x8_qdma_2_202220_1.xclbin <dev_idx> &
../scripts/monitor_power.py -d <bdf>
```

### Table VII

Builds from Table V also output padded performance.

### Figure 9

- Baseline: reuse from Table V
- 250 MHz: build and run after changing `src/CMakeLists.txt` to use `PL_FREQ_MHZ 250`
- No DRAM: swap out commented lines in `load_A`, `load_B`, and `store_C` to avoid writing to the `dram_t *` in `dma.cc`

Transfer throughputs to `plot_misc.py` and run it.
