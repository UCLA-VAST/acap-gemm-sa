#ifndef GRAPH_HH
#define GRAPH_HH

#include <string>
#include <array>

#include "gemm.hh"

template <int W> struct plio_bits {};
template <> struct plio_bits<32>  { static constexpr auto value = adf::plio_32_bits; };
template <> struct plio_bits<64>  { static constexpr auto value = adf::plio_64_bits; };
template <> struct plio_bits<128> { static constexpr auto value = adf::plio_128_bits; };

template <typename DT, int R, int C, int TM, int TK, int TN, int MM, int KK, int NN>
class GemmGraph : public adf::graph {

private:
  //std::array<std::array<adf::kernel, C>, R> kernel;
  //std::array<adf::input_plio, C>            in0;
  //std::array<adf::input_plio, R>            in1;
  //std::array<adf::output_plio, C>           out0;
  adf::kernel      kernel[R][C];
  adf::input_plio  in0[R];
  adf::input_plio  in1[C];
  adf::output_plio out0[C];

  template <bool RFWD, bool CFWD> struct Kernel {
    using type = Gemm<DT, R, TM, TK, TN, MM, KK, NN, RFWD, CFWD>;
  };

public:
  GemmGraph() {
    constexpr int dimR = TM*TK;
    constexpr int dimC = TK*TN;
    constexpr int dimO = TM*TN;

    for (int r=0; r<R; r++) {
      in0[r] = adf::input_plio::create("in0_" + std::to_string(r), plio_bits<PLIO_WIDTH>::value,
                                       "data/in0_" + std::to_string(r) + ".txt");
    }

    for (int c=0; c<C; c++) {
      in1[c] = adf::input_plio::create("in1_" + std::to_string(c), plio_bits<PLIO_WIDTH>::value,
                                       "data/in1_" + std::to_string(c) + ".txt");
      out0[c] = adf::output_plio::create("out0_" + std::to_string(c), plio_bits<PLIO_WIDTH>::value,
                                         "data/out0_" + std::to_string(c) + ".txt");
    }

    for (int r=0; r<R; r++) {
      for (int c=0; c<C; c++) {
        if (r == R-1 && c == C-1) { // !RFWD && !CFWD
          kernel[r][c] = adf::kernel::create_object<typename Kernel<false, false>::type>(r, c);
        } else if (r == R-1) { // RFWD && !CFWD
          if constexpr (C > 1) {
            kernel[r][c] = adf::kernel::create_object<typename Kernel<true, false>::type>(r, c);
            adf::dimensions(kernel[r][c].out[1]) = {dimR};
          }
        } else if (c == C-1) { // !RFWD && CFWD
          if constexpr (R > 1) { 
            kernel[r][c] = adf::kernel::create_object<typename Kernel<false, true>::type>(r, c);
            adf::dimensions(kernel[r][c].in[2]) = {dimO};
            adf::dimensions(kernel[r][c].out[1]) = {dimC};
          }
        } else { // RFWD && CFWD
          if constexpr (R > 1 && C > 1) {
            kernel[r][c] = adf::kernel::create_object<typename Kernel<true, true>::type>(r, c);
            adf::dimensions(kernel[r][c].in[2]) = {dimO};
            adf::dimensions(kernel[r][c].out[1]) = {dimR};
            adf::dimensions(kernel[r][c].out[2]) = {dimC};
          }
        }

        adf::source(kernel[r][c]) = "gemm.cc";
        adf::runtime<adf::ratio>(kernel[r][c]) = 1.0;
        adf::location<adf::kernel>(kernel[r][c]) = adf::tile(c, r);

        adf::dimensions(kernel[r][c].in[0]) = {dimR};
        adf::dimensions(kernel[r][c].in[1]) = {dimC};
        adf::dimensions(kernel[r][c].out[0]) = {dimO};
      }
    }

    for (int r=0; r<R; r++) {
      for (int c=0; c<C; c++) {
        if (c == 0) { adf::connect(in0[r].out[0], kernel[r][c].in[0]); } // in0 -> rin
        if (r == 0) {
          adf::connect(in1[c].out[0], kernel[r][c].in[1]); // in1 -> cin
          adf::connect(kernel[r][c].out[0], out0[c].in[0]); // oout -> out0
        }

        if (r > 0) { adf::connect(kernel[r][c].out[0], kernel[r-1][c].in[2]); } // oout -> oin

        if (r == R-1 && c == C-1) { // !RFWD && !CFWD
          // pass
        } else if (r == R-1) { // RFWD && !CFWD
          adf::connect(kernel[r][c].out[1], kernel[r][c+1].in[0]); // rout -> rin
        } else if (c == C-1) { // !RFWD && CFWD
          adf::connect(kernel[r][c].out[1], kernel[r+1][c].in[1]); // cout -> cin
        } else { // RFWD && CFWD
          adf::connect(kernel[r][c].out[1], kernel[r][c+1].in[0]); // rout -> rin
          adf::connect(kernel[r][c].out[2], kernel[r+1][c].in[1]); // cout -> cin
        }
      }
    }

  }
};

#endif // GRAPH_HH
