### General Information

For parameters in this section:
- We use the source code name for simplicity, e.g., `PL_M` instead of $M_p$.

For simulations:
- Related files: `parameters.hh`, `gemm.hh`
- We only support single PL tile data generation and simulation,
  i.e., `(M, K, N) = (PL_M, PL_K, PL_N)`.
  Otherwise, stalling behavior might be observed during simulation.
- The parameter `DEF_PARTS` determines how many NMUs the PL design uses per argument.
  While it does not affect simulation, it must divide `DEF_AIE_ROWS` and `DEF_AIE_COLS`.
  In practice, this means setting `DEF_PARTS=1` when running single-core simulations.
- For large simulations, we can generate data using `--identity` to avoid overflow.

### Table III

We use a $(1, 1, 1)$ L1 and $(4, 4, 4)$ L2 iteration space for single-core simulations.
For example, for `(AIE_M, AIE_K, AIE_N) = (16, 64, 16)`, 
we use `(PL_M, PL_K, PL_N) = (4*16, 4*64, 4*16) = (64, 256, 64)`.

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
../scripts/parse_profile.py -f compute -i src/aiesimulator_output/profile_instr_0_0.txt | grep Efficiency
```

### Figure 6

Same steps as with Table III, but with the parameters listed in the paper.
For each method, use the corresponding `src*` directory and `gemm*-aiesim` target,
and save each `aiesimulator_output` directory as they will be used for Figure 7.

- Parameters:
  - `DT = int32_t`
  - `(M, K, N) = (PL_M, PL_K, PL_N) = (1024, 512, 800)`
  - `(AIE_M, AIE_K, AIE_N) = (16, 64, 16)`
  - `DEF_AIE_ROWS=8`
  - `DEF_AIE_COLS=50`
- Format: `<src_dir>`, `<build_target>`, `<noinline>`, `<save_dir>`
- Traditional SA: `src-trad`, `gemm-trad-aiesim`, `impl`, `sim-trad`
- MCSA: `src`, `gemm-aiesim`, `impl`, `sim-mcsa`
- Ideal MCSA: `src-ideal`, `gemm-ideal-aiesim`, `impl`, `sim-ideal`

The `parse_profile.py` script finds the `impl` section (only created for non-inlined functions) 
and prints the `PC`, `Instruction`, `Assembly`, `Exe-count`, and `Cycles` columns
while keeping track of VMAC instructions (marked by `x`),
executed cycle count (given by `Exe-count`),
and total cycle count including stalls (given by `Cycles`).

- VMAC: Reported cycles from `parse_profile.py` on `profile_instr_25_4.txt`
- Stall: Total cycle difference from `parse_profile.py` using `--no-stalls`
- Zero: Manually parsed `VST` pipeline block
- Forward: Manually parsed `VLD` and `VST` pipeline block after `ACQ` on $A$ and $B$
- Flush: Manually parsed `VLD` and `VST` pipeline block after `VMAC` pipeline and `ACQ` on $C_{in}$ and $C_{out}$
- Other: Remaining cycles

We show an example using Traditional SA:

```console
# Estimated Time: ~4 hours
cd ../src-trad
vim parameters.hh # apply config
./generate_gemm_data.py -d 1024,512,800 -t 16,64,16 -a 8,50 --identity
vim gemm.hh # apply (uncomment) `noinline` for ONLY `impl`
cd ../build
make -j VERBOSE=1 gemm-trad-aiesim
../scripts/parse_profile.py -f impl -i src-trad/aiesimulator_output/profile_instr_25_4.txt
```

We can calculate each of the reported values as follows:
- VMAC: the last entry gives `131072/210846`, or $62.2%$.
- Stall: the last entry gives `131072/175772`, or $(210846 - 175772) / 210846 = 16.6%$.
- Zero: we search for the first `VST` block corresponding to line 92 in `src-trad/gemm.cc` 
  and count the `Exe-count` cycles.  The VLIW instruction looks like: `NOP; VST wr3, [p6], #32`.
  There are 32 lines of 8 cycles, or $(32\*8)/210846 = 0.1%$.
- Forward: in `src-trad/gemm.cc` we can see that we acquire `rin`, `rout`, `cin`, and `cout`,
  so we search for the first two `ACQ` instructions.
  The following `VLD` and `VST` pipeline block corresponds to the row-wise forwarding of $A$,
  and contain cycle count values of $64$ (prologue/epilogue) and $448$ (core pipeline) for a total of $8640$ cycles.
  We find the next two `ACQ` instructions for `cin` and `cout` to locate the next pipeline block for col-wise forwarding of $B$.
  Both cycle counts for $A$ and $B$ are the same in this case, so our percentage is $(2\*8640)/210846=8.2%$.
- Flush: following the logic in `gemm.cc`, the next pipeline block corresponds to `compute` with VMAC instructions.
  The `flush_step` occurs after this block and can be located by the next two `ACQ` instructions;
  there are 39 lines of 21 cycles. 
  The draining while-loop (find next two `ACQ`) has cycle counts of $0$ due to our flushing constraint $TK_2 \ge R$.
  The next single `ACQ` corresponds to writing the local buffer to `oout`; there are 40 lines of 8 cycles.
  The last two `ACQ` corresponds to the final drain at the end of execution; there are 39 lines of 3 cycles.
  The final calculation is: $(39\*21 + 40\*8 + 39\*3) / 210846 = 0.6%$.
- Other: remaining cycle count/percentage.

### Figure 7

Figure 7 reuses the output from Figure 6 but with one addition for 7a.

- 7a: `src`, `gemm-aiesim`, `compute`, `sim-comp`
- 7b-d: from Figure 6

To generate the heatmap (requires `seaborn` and `numpy`):

```console
$TOP_DIR/scripts/heatmap.py -d sim-comp,sim-mcsa,sim-trad,sim-ideal -f compute,impl,impl,impl -o heatmap.png
```

Note: any `mpl`-compatible output format can be used.

### Figure 8

Use an initial configuration of:
- `DT = int32_t`
- `(M, K, N) = (PL_M, PL_K, PL_N) = (128,512,16)`
- `(AIE_M, AIE_K, AIE_N) = (16, 64, 16)`
- `(DEF_AIE_ROWS, DEF_AIE_COLS) = (1, 1)`
- `noinline => impl`

As `DEF_AIE_ROWS`/`DEF_AIE_COLS` increases, scale `M = PL_M`/`N = PL_N` by the same factor,
e.g., for a `(6, 32)` SA, use `(128*6=768, 512, 16*32=512)`.

Configurations:
| $R \backslash C$ | $1$ | $2$ | $4$ | $8$ | $16$ | $32$ | $50$ |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| $1$ | $(128, 16)$ | $(128, 32)$ | $(128, 64)$ | $(128, 128)$ | $(128, 256)$ | $(128, 512)$ | $(128, 800)$ |
| $2$ | $(256, 16)$ | $(256, 32)$ | $(256, 64)$ | $(256, 128)$ | $(256, 256)$ | $(256, 512)$ | $(256, 800)$ |
| $4$ | $(512, 16)$ | $(512, 32)$ | $(512, 64)$ | $(512, 128)$ | $(512, 256)$ | $(512, 512)$ | $(512, 800)$ |
| $6$ | $(768, 16)$ | $(768, 32)$ | $(768, 64)$ | $(768, 128)$ | $(768, 256)$ | $(768, 512)$ | $(768, 800)$ |
| $8$ | $(1024, 16)$ | $(1024, 32)$ | $(1024, 64)$ | $(1024, 128)$ | $(1024, 256)$ | $(1024, 512)$ | $(1024, 800)$ |

Run simulations for all configurations, save each output as `<R>x<C>` in a clean directory,
and then collect average efficiencies.

```console
cd $SAVE_DIR
$TOP_DIR/scripts/heatmap.py -d $(find . -maxdepth 1 -type d | sort -V | paste -sd',' -) | awk '{print $1,$3}'
vim $TOP_DIR/scripts/plot_scaling.py # transfer averages to script
$TOP_DIR/scripts/plot_scaling.py
```

- `find` finds all `<R>x<C>` directories
- `sort` sorts these by rows then columns
- `paste` creates a comma-separated list for `heatmap.py`
- `awk` extracts the first and third columns (cols: name, min, avg, max)

### Table IV

Apply any of the configurations using `parameters.hh` and `xsa.cfg`,
and examine ``src/work.hls/hls/syn/report/dma_csynth.rpt``.
It is recommended to *use a different CMake _source_ directory for each configuration*
to avoid recompilation for later Tables and Figures.

Configurations:
- `DT = float`
- `M = K = N` = "Size"
- `(PL_M, PL_K, PL_N)` = $(M_p, K_p, N_p)$
- `(AIE_M, AIE_K, AIE_N)` = $(16, 64, 16)$
- `(DEF_AIE_ROWS, DEF_AIE_COLS)` = $(R, C)$
- `DEF_PARTS` = $2$
- `xsa.cfg` specifies 8x50 stream connections. Comment out unused streams for smaller $(R, C)$,
  where `in0` are rows and `(in1, out0)` are columns.
  E.g., for 6x32, comment out `in0_[6-7]`, `in1_[32-49]`, and `out0_[32-49]`.

```console
# Estimated Time: ~20 minutes
vim src/parameters.hh # apply config
vim src/xsa.cfg # comment out unused streams
mkdir build && cd build
cmake .. -DVPP_JOBS=16 -DVPP_OPTIMIZE=3
make -j VERBOSE=1 gemm-xo
```

### Table V

Same steps as with Table IV, but build the full target and execute.

```console
# Estimated Time: up to ~12 hours depending on SA size
make -j VERBOSE=1 gemm
./bin/gemm ./lib/gemm.hw.xilinx_vck5000_gen4x8_qdma_2_202220_1.xclbin <dev_idx>
```

### Table VI

Increase ``iters`` in ``host.cc`` to a very large value or make the loop infinite.
(Can reuse builds from Table V.)

```console
# Estimated Time: 3 minutes
make -j VERBOSE=1 gemm-host
./bin/gemm ./lib/gemm.hw.xilinx_vck5000_gen4x8_qdma_2_202220_1.xclbin <dev_idx> &
../scripts/monitor_power.py -d <bdf>
```

### Table VII

Runs from Table V also output padded performance.

### Figure 9

- Baseline: reuse from Table V
- 250 MHz: build and run after changing `src/CMakeLists.txt` to use `PL_FREQ_MHZ 250`
- No DRAM: swap out commented lines in `load_A`, `load_B`, and `store_C` to avoid writing to the `dram_t *` in `dma.cc`

Transfer throughputs to `plot_misc.py` and run it.
