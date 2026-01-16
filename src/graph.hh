#ifndef GRAPH_HH
#define GRAPH_HH

#include <string>

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

  template <bool FLUSH> struct Kernel {
    using type = Gemm<DT, R, TM, TK, TN, MM, KK, NN, FLUSH>;
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

    for (int r=R-1; r>=0; r--) { // top-to-bottom (for r+1 connection)
      for (int c=0; c<C; c++) { // left-to-right
        if (r == R-1) {
          kernel[r][c] = adf::kernel::create_object<typename Kernel<false>::type>(r, c);
        } else if constexpr (R > 1) {
          kernel[r][c] = adf::kernel::create_object<typename Kernel<true>::type>(r, c);
        }

        adf::source(kernel[r][c]) = "gemm.cc";
        adf::runtime<adf::ratio>(kernel[r][c]) = 1.0;

        adf::location<adf::kernel>(kernel[r][c]) = adf::tile(c, r);
        //adf::location<adf::stack>(kernel[r][c]) = adf::location<adf::kernel>(kernel[r][c]);
        //adf::location<adf::buffer>(kernel[r][c].in[0]) = adf::location<adf::kernel>(kernel[r][c]);
        //adf::location<adf::buffer>(kernel[r][c].in[1]) = adf::location<adf::kernel>(kernel[r][c]);
        //adf::location<adf::buffer>(kernel[r][c].out[0]) = adf::location<adf::kernel>(kernel[r][c]);
        //if (!is_sink) { adf::location<adf::buffer>(kernel[r][c].out[1]) = adf::location<adf::kernel>(kernel[r][c]); }

        adf::dimensions(kernel[r][c].in[0]) = {dimR};
        adf::dimensions(kernel[r][c].in[1]) = {dimC};
        if (r != R-1) { adf::dimensions(kernel[r][c].in[2]) = {dimO}; }
        adf::dimensions(kernel[r][c].out[0]) = {dimO};

        adf::connect(in0[r].out[0], kernel[r][c].in[0]);
        adf::connect(in1[c].out[0], kernel[r][c].in[1]);

        if (r != R-1) {
          adf::connect(kernel[r+1][c].out[0], kernel[r][c].in[2]);
        }

        if (r == 0) {
          adf::connect(kernel[r][c].out[0], out0[c].in[0]);
        }

      }
    }

  }
};

#endif // GRAPH_HH
