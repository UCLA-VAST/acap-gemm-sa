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

Instructions for reproducing Tables III-VII and Figures 6-9 are available in [REPRODUCING.md](REPRODUCING.md).

