#ifndef GEMM_HH
#define GEMM_HH

#include <adf.h>

#include "parameters.hh"

#if defined(__X86SIM__)
#define SIM_ONLY(...) __VA_ARGS__
#else
#define SIM_ONLY(...)
#endif

template <typename DT, int R, int TM, int TK, int TN, int MM, int KK, int NN, bool FWD>
class Gemm {
public:
  Gemm(int row, int col);

  static void registerKernelClass() {
    if constexpr (FWD) {
      REGISTER_FUNCTION(Gemm::in3out1); 
    } else {
      REGISTER_FUNCTION(Gemm::in2out1); 
    }
  }

  void in2out1(
    adf::input_async_buffer<DT> & __restrict in0,
    adf::input_async_buffer<DT> & __restrict in1,
    adf::output_async_buffer<DT> & __restrict out0
  ) { impl(in0, in1, in0 /*unused*/, out0); }

  void in3out1(
    adf::input_async_buffer<DT> & __restrict in0,
    adf::input_async_buffer<DT> & __restrict in1,
    adf::input_async_buffer<DT> & __restrict in2,
    adf::output_async_buffer<DT> & __restrict out0
  ) { impl(in0, in1, in2, out0); }

private:

  //__attribute__ ((noinline))
  void impl(
    adf::input_async_buffer<DT> & __restrict rin,
    adf::input_async_buffer<DT> & __restrict cin,
    adf::input_async_buffer<DT> & __restrict oin,
    adf::output_async_buffer<DT> & __restrict oout
  );

  //__attribute__ ((noinline))
  void compute(adf::input_async_buffer<DT> & __restrict rin,
               adf::input_async_buffer<DT> & __restrict cin);

  DT buf0[TM][TN];

  int const row;
  int const col;

  int const stop_iter;
  int const stop_lap;
};

#endif // GEMM_HH
