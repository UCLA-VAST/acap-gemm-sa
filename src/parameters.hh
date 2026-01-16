#ifndef PARAMETERS_HH
#define PARAMETERS_HH

#include <cstdint>
#include <algorithm>

namespace {
  // BEGIN user config
  using DT = float;
  constexpr int DATA_WIDTH = sizeof(DT) * 8;
  constexpr int DRAM_WIDTH = 512;
  constexpr int PLIO_WIDTH = 128;
  // for macro expansion
  #define DEF_AIE_ROWS 8
  #define DEF_AIE_COLS 50
  #define DEF_PARTS 2
  constexpr int AIE_ROWS = DEF_AIE_ROWS;
  constexpr int AIE_COLS = DEF_AIE_COLS;
  constexpr int PARTS = DEF_PARTS;
  constexpr int M = 8192;
  constexpr int K = 8192;
  constexpr int N = 8192;
  constexpr int AIE_M = 16;
  constexpr int AIE_K = 64;
  constexpr int AIE_N = 16;
  constexpr int AIE_MM = 1;
  constexpr int AIE_KK = 1;
  constexpr int AIE_NN = 1;
  constexpr int PL_M = 1024;
  constexpr int PL_K = 512;
  constexpr int PL_N = 800;
  // END user config

  template <typename T>
  constexpr auto ceil(T x) { return x == static_cast<int>(x) ? x : static_cast<int>(x) + 1; }

  constexpr int BM1 = ceil(static_cast<float>(M) / PL_M);
  constexpr int BK1 = ceil(static_cast<float>(K) / PL_K);
  constexpr int BN1 = ceil(static_cast<float>(N) / PL_N);
  constexpr int BM2 = PL_M / AIE_M / AIE_ROWS;
  constexpr int BK2 = PL_K / AIE_K;
  constexpr int BN2 = PL_N / AIE_N / AIE_COLS;
  constexpr int BM3 = AIE_M / AIE_MM;
  constexpr int BK3 = AIE_K / AIE_KK;
  constexpr int BN3 = AIE_N / AIE_NN;
  constexpr int ADF_ITERS = 1; //BM2*BK2*BN2*BM1*BK1*BN1 + ((AIE_ROWS > 1) ? 1 : 0);

  constexpr int DRAM_PACK = DRAM_WIDTH / DATA_WIDTH;
  constexpr int PLIO_PACK = PLIO_WIDTH / DATA_WIDTH;;
  constexpr int DRAM_PLIO_PACK = DRAM_PACK / PLIO_PACK;
  constexpr int PACK_PER_ROW_STREAM = std::max(1, DRAM_PLIO_PACK / (AIE_ROWS / PARTS));
  constexpr int PACK_PER_COL_STREAM = std::max(1, DRAM_PLIO_PACK / (AIE_COLS / PARTS));

  static_assert(PL_M  % AIE_M*AIE_ROWS == 0);
  static_assert(PL_M >= AIE_M*AIE_ROWS);
  static_assert(PL_K  % AIE_K == 0);
  static_assert(PL_K >= AIE_K);
  static_assert(PL_N  % AIE_N*AIE_COLS == 0);
  static_assert(PL_N >= AIE_N*AIE_COLS);
  static_assert(AIE_M  % AIE_MM == 0);
  static_assert(AIE_M >= AIE_MM);
  static_assert(AIE_K  % AIE_KK == 0); 
  static_assert(AIE_K >= AIE_KK); 
  static_assert(AIE_K  % AIE_KK == 0); 
  static_assert(AIE_N >= AIE_NN);
  static_assert(DRAM_WIDTH  % PLIO_WIDTH == 0);
  static_assert(DRAM_WIDTH >= PLIO_WIDTH);
  static_assert(PLIO_WIDTH  % DATA_WIDTH == 0);
  static_assert(PLIO_WIDTH >= DATA_WIDTH);
  static_assert(M / PLIO_PACK / DRAM_PLIO_PACK > 0);
  static_assert(K / PLIO_PACK / DRAM_PLIO_PACK > 0);
  static_assert(N / PLIO_PACK / DRAM_PLIO_PACK > 0);
  static_assert(PL_M / PLIO_PACK / DRAM_PLIO_PACK > 0);
  static_assert(PL_K / PLIO_PACK / DRAM_PLIO_PACK > 0);
  static_assert(PL_N / PLIO_PACK / DRAM_PLIO_PACK > 0);
  static_assert(AIE_K % PLIO_PACK == 0, "load_buf_A");
  static_assert(AIE_N % PLIO_PACK == 0, "load_buf_B");
  static_assert(DRAM_PACK % PLIO_PACK == 0);
  static_assert(AIE_ROWS/PARTS > 0 && AIE_COLS/PARTS > 0);
  static_assert(AIE_ROWS % PARTS == 0);
  static_assert(AIE_COLS % PARTS == 0);

}

#endif // PARAMETERS_HH
