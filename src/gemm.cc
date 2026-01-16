#include "gemm.hh"
#include <aie_api/aie.hpp>
#include <type_traits>

// https://docs.amd.com/r/2023.1-English/ug1079-ai-engine-kernel-coding/Vector-Data-Types
template <int W> struct aie_vlen {};
template <> struct aie_vlen<8>  { static constexpr int value = 64; };
template <> struct aie_vlen<16> { static constexpr int value = 32; };
template <> struct aie_vlen<32> { static constexpr int value = 16; };
constexpr int AIE_VLEN = aie_vlen<DATA_WIDTH>::value;

// https://docs.amd.com/r/en-US/am009-versal-ai-engine/Functional-Overview
template <int W> struct aie_alen {};
template <> struct aie_alen<8>  { static constexpr int value = 128; };
template <> struct aie_alen<16> { static constexpr int value = 32; };
template <> struct aie_alen<32> { static constexpr int value = 8; };
constexpr int AIE_ALEN = aie_alen<DATA_WIDTH>::value;


#if defined(__X86SIM__) || defined(__AIESIM__)

#include <cstdio>
#define STDERR(...) fprintf(stderr, __VA_ARGS__);

#include <sstream>
template <typename T, unsigned Elems>
auto vec2str(aie::vector<T, Elems> const &r) {
  T const *p = (T const *) &r;
  std::stringstream ss;
  for (int i=0; i<Elems; i++) {
    ss << p[i];
    if (i != Elems-1) { ss << " "; }
  }
  return ss.str();
};

#else
#define STDERR(...)
#endif

namespace {
  template <typename T> void template_type(T&&);
  constexpr auto VLEN = aie::detail::native_vector_length<DT>::value;
  constexpr auto ALEN = 8;
}

template <typename DT, int R, int TM, int TK, int TN, int MM, int KK, int NN, bool FWD>
Gemm<DT, R, TM, TK, TN, MM, KK, NN, FWD>::
Gemm(int row_, int col_)
  : row{row_}, col{col_}, 
    stop_iter{BK2},
    stop_lap{BM1*BN1*BK1*BM2*BN2}
{
  //STDERR("id=(%d,%d) this=%p stop_lap=%d stop_iter=%d\n", row, col, this, stop_lap, stop_iter);
}

template <typename DT, int R, int TM, int TK, int TN, int MM, int KK, int NN, bool FWD>
void
Gemm<DT, R, TM, TK, TN, MM, KK, NN, FWD>::
impl(adf::input_async_buffer<DT> & __restrict rin,
     adf::input_async_buffer<DT> & __restrict cin,
     adf::input_async_buffer<DT> & __restrict oin,
     adf::output_async_buffer<DT> & __restrict oout)
{

  int lap{0};
  int iter{0};
  int flush{0};
  
  auto flush_step = [&]() {
    oin.acquire();
    oout.acquire();

    auto in = aie::begin_vector<VLEN>(oin);
    auto out = aie::begin_vector<VLEN>(oout);
    for (int i=0; i<TM*TN/VLEN; i++) {
      //STDERR("id=(%d,%d) lap=%d/%d iter=%d/%d flush=%d i=%d v=%s\n",
      //        row, col, lap+1, stop_lap, iter+1, stop_iter, flush, i, vec2str(*in).c_str());
      *out++ = *in++;
    }

    oin.release();
    oout.release();

    flush--;
  };

  for (lap=0; lap<stop_lap; lap++) {
    auto p = aie::begin_vector<VLEN>(&buf0[0][0]);
    for (int i=0; i<TM*TN/VLEN; i++) { *p++ = aie::zeros<DT, VLEN>(); }

    for (iter=0; iter<stop_iter; iter++) {
      //STDERR("id=(%d,%d) lap=%d/%d iter=%d/%d begin\n",
      //       row, col, lap+1, stop_lap, iter+1, stop_iter);

      //STDERR("id=(%d,%d) lap=%d/%d iter=%d/%d rin acq %p\n",
      //       row, col, lap+1, stop_lap, iter+1, stop_iter, rin.data());
      rin.acquire();

      //{
      //  auto p = aie::begin_vector<VLEN>(rin);
      //  for (int i=0; i<TK*TN/VLEN; i++) {
      //    STDERR("id=(%d,%d) lap=%d/%d iter=%d/%d rin i=%d v=%s\n", 
      //           row, col, lap+1, stop_lap, iter+1, stop_iter, i, vec2str(*p++).c_str());
      //  }
      //}

      //STDERR("id=(%d,%d) lap=%d/%d iter=%d/%d cin acq %p\n",
      //       row, col, lap+1, stop_lap, iter+1, stop_iter, cin.data());
      cin.acquire();

      //{
      //  auto p = aie::begin_vector<VLEN>(cin);
      //  for (int i=0; i<TM*TN/VLEN; i++) {
      //    STDERR("id=(%d,%d) lap=%d/%d iter=%d/%d cin i=%d v=%s\n", 
      //           row, col, lap+1, stop_lap, iter+1, stop_iter, i, vec2str(*p++).c_str());
      //  }
      //}

      compute(rin, cin);

      //{
      //  auto in = aie::begin_vector<VLEN>(&buf0[0][0]);
      //  for (int i=0; i<TM*TN/VLEN; i++) {
      //    STDERR("id=(%d,%d) lap=%d/%d iter=%d/%d state i=%d v=%s\n", 
      //           row, col, lap+1, stop_lap, iter+1, stop_iter, i, vec2str(*in++).c_str());
      //  }
      //}

      //STDERR("id=(%d,%d) lap=%d/%d iter=%d/%d rin rel %p\n",
      //       row, col, lap+1, stop_lap, iter+1, stop_iter, rin.data());
      rin.release();

      //STDERR("id=(%d,%d) lap=%d/%d iter=%d/%d cin rel %p\n",
      //       row, col, lap+1, stop_lap, iter+1, stop_iter, cin.data());
      cin.release();

      if (flush > 0) { flush_step(); }

      //STDERR("id=(%d,%d) lap=%d/%d iter=%d/%d end\n", row, col, lap+1, stop_lap, iter+1, stop_iter);
    }

    while (flush > 0) { flush_step(); }

    //STDERR("id=(%d,%d) lap=%d/%d iter=%d/%d oout acq %p\n", 
    //       row, col, lap+1, stop_lap, iter+1, stop_iter, oout.data());
    oout.acquire();

    auto in = aie::begin_vector<VLEN>(&buf0[0][0]);
    auto out = aie::begin_vector<VLEN>(oout);
    for (int i=0; i<TM*TN/VLEN; i++) {
      //STDERR("id=(%d,%d) lap=%d/%d iter=%d/%d stop i=%d v=%s\n",
      //        row, col, lap+1, stop_lap, iter+1, stop_iter, i, vec2str(*in).c_str());
      *out++ = *in++;
    }

    //STDERR("id=(%d,%d) lap=%d/%d iter=%d/%d oout rel %p\n",
    //       row, col, lap+1, stop_lap, iter+1, stop_iter, oout.data());
    oout.release();

    flush = R - row - 1;

    //STDERR("id=(%d,%d) lap=%d/%d iter=%d/%d end\n", row, col, lap+1, stop_lap, iter+1, stop_iter);

  }

  while (flush > 0) { flush_step(); }

  //STDERR("id=(%d,%d) lap=%d/%d iter=%d/%d end RESET\n", row, col, lap+1, stop_lap, iter+1, stop_iter);
}

template <typename DT, int R, int TM, int TK, int TN, int MM, int KK, int NN, bool FWD>
void
Gemm<DT, R, TM, TK, TN, MM, KK, NN, FWD>::
compute(adf::input_async_buffer<DT> & __restrict rin,
        adf::input_async_buffer<DT> & __restrict cin)
{
  //STDERR("id=(%d,%d) lap=%d/%d iter=%d/%d compute\n", row, col, lap, stop_lap, iter, stop_iter);

  using accum_t = decltype(aie::mul(aie::vector<DT, ALEN>(), aie::vector<DT, ALEN>()));

  static constexpr bool f = std::is_same_v<DT, float>;
  static constexpr int M_STEP = 2;
  static constexpr int N_STEP = f ? 1 : 2;
  static constexpr int PACK_A = 2;
  static constexpr int SIZE_A = ALEN / PACK_A;
  static constexpr int SIZE_B = ALEN;
  static_assert(M_STEP == 1 || M_STEP == 2);
  static_assert(N_STEP == 1 || N_STEP == 2);

  DT const * __restrict pA = rin.data();
  DT const * __restrict pB = cin.data();
  DT * __restrict pC = &buf0[0][0];

  aie::vector<DT, SIZE_A> vA0;
  aie::vector<DT, SIZE_A> vA1;
  aie::vector<DT, SIZE_B> vB0;
  aie::vector<DT, SIZE_B> vB1;
  decltype(aie::mul(vB0, vA0[0])) aC00;
  decltype(aie::mul(vB0, vA1[0])) aC10;
  decltype(aie::mul(vB1, vA0[0])) aC01;
  decltype(aie::mul(vB1, vA1[0])) aC11;

  for (int m=0; m<TM; m+=M_STEP) {
    for (int n=0; n<TN; n+=N_STEP*ALEN) {
      aC00.from_vector(aie::load_v<ALEN>(pC));
      if constexpr (M_STEP == 2) { aC10.from_vector(aie::load_v<ALEN>(pC+TN)); }
      if constexpr (N_STEP == 2) {
        aC01.from_vector(aie::load_v<ALEN>(pC+ALEN));
        if constexpr (M_STEP == 2) { aC11.from_vector(aie::load_v<ALEN>(pC+TN+ALEN)); }
      }

      pA = rin.data() + m*TK;
      pB = cin.data() + n;

      for (int kp=0; kp<(TK/ALEN)*PACK_A; kp++) {
        vA0 = aie::load_v<SIZE_A>(pA);
        if constexpr (M_STEP == 2) { vA1 = aie::load_v<SIZE_A>(pA+TK); }
        pA += SIZE_A;

        for (int kk=0; kk<ALEN/PACK_A; kk++) {
          vB0 = aie::load_v<ALEN>(pB);
          if constexpr (N_STEP == 2) { vB1 = aie::load_v<ALEN>(pB+ALEN); }
          pB += TN;

          aC00 = aie::mac(aC00, vB0, vA0[kk]);
          if constexpr (M_STEP == 2) { aC10 = aie::mac(aC10, vB0, vA1[kk]); }
          if constexpr (N_STEP == 2) {
            aC01 = aie::mac(aC01, vB1, vA0[kk]);
            if constexpr (M_STEP == 2) { aC11 = aie::mac(aC11, vB1, vA1[kk]); }
          }
        }
      }

      aie::store_v(pC, aC00.template to_vector<DT>());
      if constexpr (M_STEP == 2) { aie::store_v(pC+TN, aC10.template to_vector<DT>()); }
      if constexpr (N_STEP == 2) {
        aie::store_v(pC+ALEN, aC01.template to_vector<DT>());
        if constexpr (M_STEP == 2) { aie::store_v(pC+TN+ALEN, aC11.template to_vector<DT>()); }
      }

      pC += N_STEP*ALEN;
    }

    pC += (M_STEP-1)*TN;
  }

}

