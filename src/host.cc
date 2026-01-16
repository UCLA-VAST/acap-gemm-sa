#include <xrt/xrt_device.h>
#include <xrt/xrt_kernel.h>
#include <xrt/xrt_bo.h>
#include <xrt/xrt_graph.h>
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <chrono>
#include <typeinfo>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <memory>

#include "parameters.hh"

#define CONCAT(a, b) a##b
#define ARGS(n, name) CONCAT(ARGS_, n)(name)
#define ARGS_1(name) name[0]
#define ARGS_2(name) ARGS_1(name), name[1]

template <typename T>
void gemm(std::vector<T> const &A, std::vector<T> const &B, std::vector<T> &C,
          int const M, int const K, int const N);

enum class Direction {
  Row,
  Col,
};

template <int P, int S, Direction DS, Direction DB, int PY, int PX, int AY, int AX, int YY, int XX, int PACK, typename T>
std::vector<std::vector<T>>
transform_in(std::vector<T> const &V, int const Y, int const X, int &Y_pad, int &X_pad);

template <int P, int S, int SS, Direction DS, Direction DB, int PY, int PX, int AY, int AX, int YY, int XX, int PACK, typename T>
std::vector<T>
transform_out(std::vector<std::vector<T>> const &V, int const Y_pad, int const X_pad, int const Y, int const X);

int main(int argc, char *argv[]) {

  if (argc != 2 && argc != 3) {
    printf("usage: %s xclbin [device_idx]\n", argv[0]);
    return 1;
  }

  char *xclbin = argv[1];
  //unsigned M = std::stoul(argv[2]);
  //unsigned K = std::stoul(argv[3]);
  //unsigned N = std::stoul(argv[4]);
  int device_idx{0};
  if (argc == 3) { device_idx = std::stoi(argv[2]); }

  //if ((M % AIE_ROWS != 0) || (N % AIE_COLS != 0)) {
  //  printf("error: dim MxN=%dx%d is not divisible by AIE array RxC=%dx%d\n", M, N, AIE_ROWS, AIE_COLS);
  //  return 3;
  //}

  char *s = std::getenv("XCL_EMULATION_MODE");
  std::string emu_mode = s ? s : "";
  setbuf(stdout, nullptr);

  auto device = xrt::device(device_idx);
  std::cout << "Device " << device_idx << ":\n";
  std::cout << "  name: " << device.get_info<xrt::info::device::name>() << "\n";
  std::cout << "  bdf: " << device.get_info<xrt::info::device::bdf>() << "\n";

  auto xclbin_uuid = device.load_xclbin(xclbin);

  printf("Creating kernel object ...\n");
  auto dma = xrt::kernel(device, xclbin_uuid, "dma");

  printf("Generating data with STL ...\n");
  std::random_device rd;
  std::mt19937 rne{rd()};
  std::uniform_int_distribution<> dist{1, 9};

  std::vector<DT> A(M*K, -1); 
  std::vector<DT> B(K*N, -2);
  std::vector<DT> C_ref(M*N, -4);

  //std::generate(A.begin(), A.end(), [&dist, &rne]() { return dist(rne); });
  //std::generate(B.begin(), B.end(), [&dist, &rne]() { return dist(rne); });

  //std::generate(A.begin(), A.end(), []() { return 1; });
  //std::generate(B.begin(), B.end(), []() { return 1; });

  //std::generate(A.begin(), A.end(), []() { static int i{0}; return i++; });
  //std::generate(B.begin(), B.end(), []() { static int i{0}; return i++; });

  std::generate(A.begin(), A.end(), []() { static int i{0}; return i++; });
  for (int k=0; k<K; k++) {
    for (int n=0; n<N; n++) {
      if (k == n) { B[k*N+n] = 1; }
      else        { B[k*N+n] = 0; }
    }
  }

  //for (int m=0; m<M; m++) {
  //  for (int k=0; k<K; k++) {
  //    if (m == k) { A[m*K+k] = 1; }
  //    else        { A[m*K+k] = 0; }
  //  }
  //}
  //std::generate(B.begin(), B.end(), []() { static int i{0}; return i++; });

  printf("  A: shape=(%d, %d) size=%lu dtype=%s [[%.1f, %.1f, ...], [%.1f, %.1f ...], ...]\n",
    M, K, A.size(), typeid(DT).name(),
    static_cast<float>(A[0]), static_cast<float>(A[1]), 
    static_cast<float>(A[K]), static_cast<float>(A[K+1]));
  //for (int r=0; r<M; r++) {
  //  for (int c=0; c<K; c++) {
  //    printf("%d ", A[r*K+c]);
  //  }
  //  printf("\n");
  //}

  printf("  B: shape=(%d, %d) size=%lu dtype=%s [[%.1f, %.1f, ...], [%.1f, %.1f ...], ...]\n",
    K, N, B.size(), typeid(DT).name(),
    static_cast<float>(B[0]), static_cast<float>(B[1]), 
    static_cast<float>(B[N]), static_cast<float>(B[N+1]));
  //for (int r=0; r<K; r++) {
  //  for (int c=0; c<N; c++) {
  //    printf("%d ", B[r*N+c]);
  //  }
  //  printf("\n");
  //}

  //{
  //  std::ofstream ofs("A.txt"); 
  //  for (int m=0; m<M; m++) {
  //    for (int k=0; k<K; k++) {
  //      ofs << A[m*K+k] << " ";
  //    }
  //    ofs << "\n";
  //  }
  //}

  //{
  //  std::ofstream ofs("B.txt");
  //  for (int k=0; k<K; k++) {
  //    for (int n=0; n<N; n++) {
  //      ofs << B[k*N+n] << " ";
  //    }
  //    ofs << "\n";
  //  }
  //}

  int M_pad, K_pad;
  auto const A_in = transform_in<PARTS, AIE_ROWS, Direction::Row, Direction::Col,
    PL_M, PL_K, AIE_M, AIE_K, AIE_MM, AIE_KK, PLIO_PACK>(A, M, K, M_pad, K_pad);

  {
    std::ofstream ofs("A_in.txt"); 
    for (auto const &part : A_in) {
      for (decltype(part.size()) i=0; i<part.size(); i++) {
        ofs << part[i] << " ";
        if (i % PL_K == PL_K-1) { ofs << "\n"; }
      }
      ofs << "--------------------------------------------------------------------------------\n";
    }
  }

  int K_pad2, N_pad;
  auto const B_in = transform_in<PARTS, AIE_COLS, Direction::Col, Direction::Row,
    PL_K, PL_N, AIE_K, AIE_N, AIE_KK, AIE_NN, PLIO_PACK>(B, K, N, K_pad2, N_pad);

  if (K_pad != K_pad2) {
    throw std::runtime_error("expected same padded size for K=" + std::to_string(K_pad)
                           + ", got: " + std::to_string(K_pad2));
  }

  {
    std::ofstream ofs("B_in.txt"); 
    for (auto const &part : B_in) {
      for (decltype(part.size()) i=0; i<part.size(); i++) {
        ofs << part[i] << " ";
        if (i % PL_N == PL_N-1) { ofs << "\n"; }
      }
      ofs << "--------------------------------------------------------------------------------\n";
    }
  }

  std::vector<std::vector<DT>> C_pad(PARTS);
  for (auto &part : C_pad) { part.resize(M_pad*N_pad/PARTS); }

  using duration_t = std::chrono::duration<double, std::ratio<1>>;
  decltype(std::chrono::steady_clock::now()) t0, t1;

  printf("Parameters:\n");
  printf("  R=%d C=%d M=%d(%d) K=%d(%d) N=%d(%d) PL_M=%d PL_K=%d PL_N=%d AIE_M=%d AIE_K=%d AIE_N=%d\n"
         "  BM1=%d BK1=%d BN1=%d BM2=%d BK2=%d BN2=%d BM3=%d BK3=%d BN3=%d\n"
         "  DATA_WIDTH=%d DRAM_WIDTH=%d PLIO_WIDTH=%d DRAM_PACK=%d PLIO_PACK=%d\n"
         "  AIE_ARRAY=%dx%d\n",
         AIE_ROWS, AIE_COLS, M, M_pad, K, K_pad, N, N_pad, PL_M, PL_K, PL_N, AIE_M, AIE_K, AIE_N,
         BM1, BK1, BN1, BM2, BK2, BN2, BM3, BK3, BN3, 
         DATA_WIDTH, DRAM_WIDTH, PLIO_WIDTH, DRAM_PACK, PLIO_PACK,
         AIE_M * AIE_ROWS, AIE_N * AIE_COLS);

  auto const A_bytes = M_pad*K_pad*sizeof(DT);
  auto const B_bytes = K_pad*N_pad*sizeof(DT);
  auto const C_bytes = M_pad*N_pad*sizeof(DT);

  printf("Writing A buffer ... ");
  t0 = std::chrono::steady_clock::now();
  xrt::bo A_bo[PARTS];
  for (int p=0; p<PARTS; p++) {
    A_bo[p] = xrt::bo(device, A_bytes/PARTS, xrt::bo::flags::normal, dma.group_id(0*PARTS+p));
    A_bo[p].write(A_in[p].data());
    A_bo[p].sync(XCL_BO_SYNC_BO_TO_DEVICE);
  }
  t1 = std::chrono::steady_clock::now();
  duration_t const t_A_bo{t1-t0};
  printf("wrote %lu bytes in %lf seconds.\n", A_bytes, t_A_bo.count());

  printf("Writing B buffer ... ");
  t0 = std::chrono::steady_clock::now();
  xrt::bo B_bo[PARTS];
  for (int p=0; p<PARTS; p++) {
    B_bo[p] = xrt::bo(device, B_bytes/PARTS, xrt::bo::flags::normal, dma.group_id(1*PARTS+p));
    B_bo[p].write(B_in[p].data());
    B_bo[p].sync(XCL_BO_SYNC_BO_TO_DEVICE);
  }
  t1 = std::chrono::steady_clock::now();
  duration_t const t_B_bo{t1-t0};
  printf("wrote %lu bytes in %lf seconds.\n", B_bytes, t_B_bo.count());

  printf("Creating C buffer ... ");
  xrt::bo C_bo[PARTS];
  for (int p=0; p<PARTS; p++) {
    C_bo[p] = xrt::bo(device, C_bytes/PARTS, xrt::bo::flags::normal, dma.group_id(2*PARTS+p));
  }
  printf("allocated %lu bytes.\n", C_bytes);

  printf("Creating aie graph ...\n");
  std::unique_ptr<xrt::graph> graph;
  if (emu_mode == "sw_emu") {
    graph = std::make_unique<xrt::graph>(device, xclbin_uuid, "graph");
  }

  printf("Running dma kernel for gemm %dx%dx%d ...\n", M, K, N);
  int const iters = (emu_mode.empty()) ? 100 : 1;
  int const warmup = (emu_mode.empty()) ? 10 : 0;
  duration_t t_dma;
  t0 = std::chrono::steady_clock::now();
  for (int i=0; i<iters+warmup; i++) {
    if (graph) {
      printf("Running aie graph ...\n");
      graph->run(ADF_ITERS);
    }

    auto const t2 = std::chrono::steady_clock::now();
    auto run = dma(
      ARGS(DEF_PARTS, A_bo),
      ARGS(DEF_PARTS, B_bo),
      ARGS(DEF_PARTS, C_bo)
    );

    run.wait();
    auto const t3 = std::chrono::steady_clock::now();

    if (i >= warmup) { t_dma += t3-t2; }
  }
  t1 = std::chrono::steady_clock::now();
  if (emu_mode == "sw_emu") {
    graph->end();
    printf("Finished aie graph.\n");
    t_dma = t1 - t0;
  }
  printf("Finished %d dma kernel iterations in %lf seconds.\n", 
         iters+warmup, duration_t{t1-t0}.count());

  printf("Reading C buffer ... ");
  t0 = std::chrono::steady_clock::now();
  for (int p=0; p<PARTS; p++) {
    C_bo[p].sync(XCL_BO_SYNC_BO_FROM_DEVICE);
    C_bo[p].read(&C_pad[p][0]);
  }
  t1 = std::chrono::steady_clock::now();
  duration_t const t_C_bo{t1-t0};
  printf("read %lu bytes in %lf seconds.\n", C_bytes, t_C_bo.count());

  unsigned long long ops{static_cast<unsigned long long>(2)*M*K*N*iters};
  unsigned long long ops_pad{static_cast<unsigned long long>(2)*M_pad*K_pad*N_pad*iters};
  auto const t_transfer{t_A_bo + t_B_bo + t_C_bo};
  auto const t_dma_transfer{t_dma + t_transfer};

  printf("Metrics:\n");
  printf("  iters=%d warmup=%d ops=%llu dtype=%s transfer=%lfs compute=%lfs\n",
         iters, warmup, ops, typeid(DT).name(),
         t_transfer.count(), t_dma.count());

  printf("  gop/s: %lf\n", ops / t_dma.count() / 1e9);
  printf("  gop/s w/ pad: %lf\n", ops_pad / t_dma.count() / 1e9);
  printf("  gop/s w/ transfer: %lf\n", ops / t_dma_transfer.count() / 1e9);

  {
    std::ofstream ofs("C_pad.txt"); 
    for (auto const &vec : C_pad) {
      for (decltype(vec.size()) i=0; i<vec.size(); i++) {
        ofs << vec[i] << " ";
        if (i % PL_N == PL_N-1) { ofs << "\n"; }
      }
      ofs << "--------------------------------------------------------------------------------\n";
    }
  }

  auto const C = transform_out<PARTS, AIE_COLS, AIE_ROWS, Direction::Col, Direction::Col,
    PL_M, PL_N, AIE_M, AIE_N, AIE_MM, AIE_NN, PLIO_PACK>(C_pad, M_pad, N_pad, M, N);

  {
    std::ofstream ofs("C_out.txt"); 
    for (decltype(C.size()) i=0; i<C.size(); i++) {
      ofs << C[i] << " ";
      if (i % PL_N == PL_N-1) { ofs << "\n"; }
    }
  }

  printf("Calculating reference data...\n");
  gemm(A, B, C_ref, M, K, N);

  //{
  //  std::ofstream ofs("C_ref.txt");
  //  for (int m=0; m<M; m++) {
  //    for (int n=0; n<N; n++) {
  //      ofs << C_ref[m*N+n] << " ";
  //    }
  //    ofs << "\n";
  //  }
  //}

  printf("Checking results ...\n");
  bool error = false;
  float tol = 1e-3;
  for (decltype(C.size()) i=0; i<C.size(); i++) {
    if (std::fabs(static_cast<float>(C[i]) - C_ref[i]) >= tol) {
      printf("error: C[%d](%f) != C_ref[%d](%f)\n", 
        i, static_cast<float>(C[i]), i, static_cast<float>(C_ref[i]));
      error = true;
    }
  }
  
  if (error) {
    printf("FAIL.\n");
    return 2;
  } else {
    printf("PASS.\n");
    return 0;
  }
}

template <typename T>
void gemm(std::vector<T> const &A, std::vector<T> const &B, std::vector<T> &C,
          int const M, int const K, int const N)
{
  for (int m=0; m<M; m++) {
    for (int n=0; n<N; n++) {
      C.at(m*N+n) = static_cast<T>(0);
    }
  }

  for (int m=0; m<M; m++) {
    for (int n=0; n<N; n++) {
      for (int k=0; k<K; k++) {
        C.at(m*N+n) += A.at(m*K+k) * B.at(k*N+n);
      }
    }
  }
}

template <int P, int S, Direction DS, Direction DB, int PY, int PX, int AY, int AX, int YY, int XX, int PACK, typename T>
std::vector<std::vector<T>>
transform_in(std::vector<T> const &V, int const Y, int const X, 
             int &Y_pad, int &X_pad)
{
  Y_pad = (Y % PY == 0) ? Y : Y + PY - (Y%PY);
  X_pad = (X % PX == 0) ? X : X + PX - (X%PX);

  if constexpr (DS == Direction::Row) {
    static_assert(PY % (AY*S) == 0, "invalid PL row dimension");
    static_assert(PX % AX     == 0, "invalid PL col dimension");
  } else {
    static_assert(PY % AY     == 0, "invalid PL row dimension");
    static_assert(PX % (AX*S) == 0, "invalid PL col dimension");
  }

  int const size_per_stream = Y_pad * X_pad / S;
  if (size_per_stream % PLIO_PACK != 0) {
    throw std::runtime_error("expected size_per_stream % PLIO_PACK == 0, got: " 
                             + std::to_string(size_per_stream % PLIO_PACK));
  }

  std::vector<std::vector<T>> vecs(S);
  for (auto &vec : vecs) { vec.reserve(size_per_stream); }

  constexpr int SY = (DS == Direction::Row) ? S*AY : AY;
  constexpr int SX = (DS == Direction::Col) ? S*AX : AX;
  auto sy = [=](int const s) {
    if constexpr (DS == Direction::Row) { return s*AY; }
    else                                { return 0; }
  };
  auto sx = [=](int const s) {
    if constexpr (DS == Direction::Col) { return s*AX; }
    else                                { return 0; }
  };

  int const BY1 = Y_pad / PY;
  int const BX1 = X_pad / PX;
  constexpr int BY2 = PY / SY;
  constexpr int BX2 = PX / SX;
  constexpr int BY3 = AY / YY;
  constexpr int BX3 = AX / XX;

  auto traverse_block = [=, &V, &vecs](int const s, int const base_y, int const base_x) {
    for (int by3=0; by3<BY3; by3++) {
      for (int bx3=0; bx3<BX3; bx3++) {
        for (int yy=0; yy<YY; yy++) {
          for (int xx=0; xx<XX; xx++) {
            int const y = base_y + by3*YY + yy;
            int const x = base_x + bx3*XX + xx;

            if (y < Y && x < X) {
              vecs[s].push_back(V[y*X+x]);
            } else {
              vecs[s].push_back(0);
            }
          }
        }
      }
    }
  };

  if constexpr (DB == Direction::Col) {
    for (int by1=0; by1<BY1; by1++) {
      for (int bx1=0; bx1<BX1; bx1++) {
        for (int by2=0; by2<BY2; by2++) {
          for (int bx2=0; bx2<BX2; bx2++) {
            for (int s=0; s<S; s++) {
              int const base_y = by1*PY + by2*SY + sy(s);
              int const base_x = bx1*PX + bx2*SX + sx(s);
              traverse_block(s, base_y, base_x);
            }
          }
        }
      }
    }
  } else {
    for (int bx1=0; bx1<BX1; bx1++) {
      for (int by1=0; by1<BY1; by1++) {
        for (int bx2=0; bx2<BX2; bx2++) {
          for (int by2=0; by2<BY2; by2++) {
            for (int s=0; s<S; s++) {
              int const base_y = by1*PY + by2*SY + sy(s);
              int const base_x = bx1*PX + bx2*SX + sx(s);
              traverse_block(s, base_y, base_x);
            }
          }
        }
      }
    }
  }

  //{
  //  std::ofstream ofs((DS == Direction::Row) ?  "A_vecs.txt" : "B_vecs.txt");
  //  for (auto const &vec : vecs) {
  //    for (int y=0; y<((DS == Direction::Row) ? Y_pad/S : Y_pad); y++) {
  //      for (int x=0; x<((DS == Direction::Col) ? X_pad/S : X_pad); x++) {
  //        ofs << vec.at(y*((DS == Direction::Col) ? X_pad/S : X_pad)+x) << " ";
  //      }
  //      ofs << "\n";
  //    }
  //    ofs << "--------------------------------------------------------------------------------\n";
  //  }
  //}

  std::vector<decltype(vecs[0].begin())> iters(S);
  for (int s=0; s<S; s++) { iters[s] = vecs[s].begin(); }

  std::vector<std::vector<T>> ret(P);
  for (auto &vec : ret) { vec.reserve(Y_pad*X_pad/P); }

  constexpr int STREAMS_PER_PART = S / P;
  for (int i=0; i<size_per_stream; i+=PLIO_PACK) {
    for (int s=0; s<S; s++) {
      int const p = s / STREAMS_PER_PART;
      ret[p].insert(ret[p].end(), iters[s], iters[s]+PLIO_PACK);
      iters[s] += PLIO_PACK;
    }
  }

  return ret;
}

template <int P, int S, int SS, Direction DS, Direction DB, int PY, int PX, int AY, int AX, int YY, int XX, int PACK, typename T>
std::vector<T>
transform_out(std::vector<std::vector<T>> const &V, int const Y_pad, int const X_pad, int const Y, int const X)
{
  if (V.size() != P) {
    throw std::runtime_error("expected " + std::to_string(P) + "parts, got: " 
                             + std::to_string(V.size()));
  }

  int const size_per_stream = Y_pad * X_pad / S;
  if (size_per_stream % PLIO_PACK != 0) {
    throw std::runtime_error("expected size_per_stream % PLIO_PACK == 0, got: " 
                             + std::to_string(size_per_stream % PLIO_PACK));
  }

  int const size_pad = Y_pad * X_pad;
  for (auto const &vec : V) {
    if (vec.size() != size_pad/P) {
      throw std::runtime_error("expected " + std::to_string(size_pad/P) + " elements per padded part, got:"
                               + std::to_string(vec.size()));
    }
  }

  std::vector<std::vector<T>> vecs(S);
  for (auto &vec : vecs) { vec.reserve(size_per_stream); }

  std::vector<decltype(V[0].begin())> piters(P);
  for (int p=0; p<P; p++) { piters[p] = V[p].begin(); }

  constexpr int STREAMS_PER_PART = S / P;
  for (int i=0; i<size_per_stream; i+=PLIO_PACK) {
    for (int s=0; s<S; s++) {
      int const p = s / STREAMS_PER_PART;
      vecs[s].insert(vecs[s].end(), piters[p], piters[p]+PLIO_PACK);
      piters[p] += PLIO_PACK;
    }
  }

  //{
  //  std::ofstream ofs("C_vecs.txt"); 
  //  for (auto const &vec : vecs) {
  //    for (int y=0; y<((DS == Direction::Row) ? Y_pad/S : Y_pad); y++) {
  //      for (int x=0; x<((DS == Direction::Col) ? X_pad/S : X_pad); x++) {
  //        ofs << vec.at(y*((DS == Direction::Col) ? X_pad/S : X_pad)+x) << " ";
  //      }
  //      ofs << "\n";
  //    }
  //    ofs << "--------------------------------------------------------------------------------\n";
  //  }
  //}

  std::vector<decltype(vecs[0].begin())> siters(S);
  for (int s=0; s<S; s++) { siters[s] = vecs[s].begin(); }

  int const ret_size = Y*X;
  std::vector<T> ret(ret_size);

  constexpr int SY = (DS == Direction::Row) ? S*AY : SS*AY;
  constexpr int SX = (DS == Direction::Col) ? S*AX : SS*AX;
  auto sy = [=](int const s) {
    if constexpr (DS == Direction::Row) { return s*AY; }
    else                                { return 0; }
  };
  auto sx = [=](int const s) {
    if constexpr (DS == Direction::Col) { return s*AX; }
    else                                { return 0; }
  };

  auto ssy = [=](int const ss) {
    if constexpr (DS == Direction::Row) { return 0; }
    else                                { return ss*AY; }
  };
  auto ssx = [=](int const ss) {
    if constexpr (DS == Direction::Col) { return 0; }
    else                                { return ss*AX; }
  };

  int const BY1 = Y_pad / PY;
  int const BX1 = X_pad / PX;
  constexpr int BY2 = PY / SY;
  constexpr int BX2 = PX / SX;
  constexpr int BY3 = AY / YY;
  constexpr int BX3 = AX / XX;

  auto traverse_block = [=, &ret, &siters](int const s, int const base_y, int const base_x) {
    for (int by3=0; by3<BY3; by3++) {
      for (int bx3=0; bx3<BX3; bx3++) {
        for (int yy=0; yy<YY; yy++) {
          for (int xx=0; xx<XX; xx++) {
            int const y = base_y + by3*YY + yy;
            int const x = base_x + bx3*XX + xx;

            if (y < Y && x < X) {
              ret[y*X+x] = *siters[s];
            }

            siters[s]++;
          }
        }
      }
    }
  };

  if constexpr (DB == Direction::Col) {
    for (int by1=0; by1<BY1; by1++) {
      for (int bx1=0; bx1<BX1; bx1++) {
        for (int by2=0; by2<BY2; by2++) {
          for (int bx2=0; bx2<BX2; bx2++) {
            for (int s=0; s<S; s++) {
              for (int ss=0; ss<SS; ss++) {
                int const base_y = by1*PY + by2*SY + sy(s) + ssy(ss);
                int const base_x = bx1*PX + bx2*SX + sx(s) + ssx(ss);
                traverse_block(s, base_y, base_x);
              }
            }
          }
        }
      }
    }
  } else {
    for (int bx1=0; bx1<BX1; bx1++) {
      for (int by1=0; by1<BY1; by1++) {
        for (int bx2=0; bx2<BX2; bx2++) {
          for (int by2=0; by2<BY2; by2++) {
            for (int s=0; s<S; s++) {
              for (int ss=0; ss<SS; ss++) {
                int const base_y = by1*PY + by2*SY + sy(s) + ssy(ss);
                int const base_x = bx1*PX + bx2*SX + sx(s) + ssx(ss);
                traverse_block(s, base_y, base_x);
              }
            }
          }
        }
      }
    }
  }

  return ret;
}
