#ifndef GEMM_HH
#define GEMM_HH

#include <adf.h>

#include "parameters.hh"

#if defined(__X86SIM__)
#define SIM_ONLY(...) __VA_ARGS__
#else
#define SIM_ONLY(...)
#endif

template <typename DT, int R, int TM, int TK, int TN, int MM, int KK, int NN, bool RFWD, bool CFWD>
class Gemm {
public:
  Gemm(int row, int col);

  static void registerKernelClass() {
    if constexpr (RFWD && CFWD) {
      REGISTER_FUNCTION(Gemm::in3out3); 
    } else if constexpr (RFWD) {
      REGISTER_FUNCTION(Gemm::in2out2); 
    } else if constexpr (CFWD) {
      REGISTER_FUNCTION(Gemm::in3out2); 
    } else {
      REGISTER_FUNCTION(Gemm::in2out1); 
    }
  }

  void in3out3( // RFWD && CFWD
    adf::input_async_buffer<DT> & __restrict rin,
    adf::input_async_buffer<DT> & __restrict cin,
    adf::input_async_buffer<DT> & __restrict oin,
    adf::output_async_buffer<DT> & __restrict oout,
    adf::output_async_buffer<DT> & __restrict rout,
    adf::output_async_buffer<DT> & __restrict cout
  ) { impl(rin, cin, oin, rout, cout, oout); }

  void in2out2( // RFWD && !CFWD
    adf::input_async_buffer<DT> & __restrict rin,
    adf::input_async_buffer<DT> & __restrict cin,
    adf::output_async_buffer<DT> & __restrict oout,
    adf::output_async_buffer<DT> & __restrict rout
  ) { impl(rin, cin, rin /*unused*/, rout, rout /*unused*/, oout); }

  void in3out2( // !RFWD && CFWD
    adf::input_async_buffer<DT> & __restrict rin,
    adf::input_async_buffer<DT> & __restrict cin,
    adf::input_async_buffer<DT> & __restrict oin,
    adf::output_async_buffer<DT> & __restrict oout,
    adf::output_async_buffer<DT> & __restrict cout
  ) { impl(rin, cin, oin, cout /*unused*/, cout, oout); }

  void in2out1( // !RFWD && !CFWD
    adf::input_async_buffer<DT> & __restrict rin,
    adf::input_async_buffer<DT> & __restrict cin,
    adf::output_async_buffer<DT> & __restrict oout
  ) { impl(rin, cin, rin /*unused*/, oout /*unused*/, oout /*unused*/, oout); }

private:

  //__attribute__ ((noinline))
  void impl(
    adf::input_async_buffer<DT> & __restrict rin,
    adf::input_async_buffer<DT> & __restrict cin,
    adf::input_async_buffer<DT> & __restrict oin,
    adf::output_async_buffer<DT> & __restrict rout,
    adf::output_async_buffer<DT> & __restrict cout,
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
