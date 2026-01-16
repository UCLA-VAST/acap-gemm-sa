#include <hls_stream.h>
#include <hls_task.h>
#include <ap_axi_sdata.h>
#include <algorithm> // std::max

#include "parameters.hh"

using data_t = ap_uint<DATA_WIDTH>;
using dram_t = ap_uint<DRAM_WIDTH>;
using plio_t = ap_uint<PLIO_WIDTH>;
using axis_t = hls::axis<plio_t, 0, 0, 0>;
using dram_stream_t = hls::stream<dram_t>;
using plio_stream_t = hls::stream<plio_t>;
using axis_stream_t = hls::stream<axis_t>;

template <int W> struct Union {};
template <> struct Union<8>  { union type {  uint8_t uint; DT val; }; };
template <> struct Union<16> { union type { uint16_t uint; DT val; }; };
template <> struct Union<32> { union type { uint32_t uint; DT val; }; };
using union_t = Union<DATA_WIDTH>::type;

#ifndef __SYNTHESIS__
#include <cstdio>
#include <sstream>

template <int WIDTH>
auto ap2str(ap_uint<WIDTH> const &x) {
  constexpr auto PACK = WIDTH/DATA_WIDTH;
  std::stringstream ss;
  for (int p=0; p<PACK; p++) {
    union_t u; u.uint = x((p+1)*DATA_WIDTH-1, p*DATA_WIDTH).to_uint();
    ss << u.val;
    if (p != PACK-1) { ss << " "; }
  }
  return ss.str();
};

#define STDERR(...) fprintf(stderr, __VA_ARGS__)
#define SIM_ONLY(...) __VA_ARGS__
#else
#define STDERR(...)
#define SIM_ONLY(...)
#endif

#define STR(x) #x
#define XSTR(x) STR(x)
#define CONCAT(a, b) a##b

#define ARGS(n, type, name) CONCAT(ARGS_, n)(type, name)
/* {{{ */
#define ARGS_1(type, name) type name##0
#define ARGS_2(type, name) ARGS_1(type, name), type name##1
#define ARGS_3(type, name) ARGS_2(type, name), type name##2
#define ARGS_4(type, name) ARGS_3(type, name), type name##3
#define ARGS_5(type, name) ARGS_4(type, name), type name##4
#define ARGS_6(type, name) ARGS_5(type, name), type name##5
#define ARGS_7(type, name) ARGS_6(type, name), type name##6
#define ARGS_8(type, name) ARGS_7(type, name), type name##7
#define ARGS_9(type, name) ARGS_8(type, name), type name##8
#define ARGS_10(type, name) ARGS_9(type, name), type name##9
#define ARGS_11(type, name) ARGS_10(type, name), type name##10
#define ARGS_12(type, name) ARGS_11(type, name), type name##11
#define ARGS_13(type, name) ARGS_12(type, name), type name##12
#define ARGS_14(type, name) ARGS_13(type, name), type name##13
#define ARGS_15(type, name) ARGS_14(type, name), type name##14
#define ARGS_16(type, name) ARGS_15(type, name), type name##15
#define ARGS_17(type, name) ARGS_16(type, name), type name##16
#define ARGS_18(type, name) ARGS_17(type, name), type name##17
#define ARGS_19(type, name) ARGS_18(type, name), type name##18
#define ARGS_20(type, name) ARGS_19(type, name), type name##19
#define ARGS_21(type, name) ARGS_20(type, name), type name##20
#define ARGS_22(type, name) ARGS_21(type, name), type name##21
#define ARGS_23(type, name) ARGS_22(type, name), type name##22
#define ARGS_24(type, name) ARGS_23(type, name), type name##23
#define ARGS_25(type, name) ARGS_24(type, name), type name##24
#define ARGS_26(type, name) ARGS_25(type, name), type name##25
#define ARGS_27(type, name) ARGS_26(type, name), type name##26
#define ARGS_28(type, name) ARGS_27(type, name), type name##27
#define ARGS_29(type, name) ARGS_28(type, name), type name##28
#define ARGS_30(type, name) ARGS_29(type, name), type name##29
#define ARGS_31(type, name) ARGS_30(type, name), type name##30
#define ARGS_32(type, name) ARGS_31(type, name), type name##31
#define ARGS_33(type, name) ARGS_32(type, name), type name##32
#define ARGS_34(type, name) ARGS_33(type, name), type name##33
#define ARGS_35(type, name) ARGS_34(type, name), type name##34
#define ARGS_36(type, name) ARGS_35(type, name), type name##35
#define ARGS_37(type, name) ARGS_36(type, name), type name##36
#define ARGS_38(type, name) ARGS_37(type, name), type name##37
#define ARGS_39(type, name) ARGS_38(type, name), type name##38
#define ARGS_40(type, name) ARGS_39(type, name), type name##39
#define ARGS_41(type, name) ARGS_40(type, name), type name##40
#define ARGS_42(type, name) ARGS_41(type, name), type name##41
#define ARGS_43(type, name) ARGS_42(type, name), type name##42
#define ARGS_44(type, name) ARGS_43(type, name), type name##43
#define ARGS_45(type, name) ARGS_44(type, name), type name##44
#define ARGS_46(type, name) ARGS_45(type, name), type name##45
#define ARGS_47(type, name) ARGS_46(type, name), type name##46
#define ARGS_48(type, name) ARGS_47(type, name), type name##47
#define ARGS_49(type, name) ARGS_48(type, name), type name##48
/* }}} */
#define ARGS_50(type, name) ARGS_49(type, name), type name##49

//#define CALL_BUF(n, p, func, buf, s) CONCAT(CALL_BUF_, n)(n, p, func, buf, s)
///* {{{ */
//#define CALL_BUF_1(n, p, func, buf, s) func(buf[0/(n/p)][0%(n/p)], s##0);
//#define CALL_BUF_2(n, p, func, buf, s) CALL_BUF_1(n, p, func, buf, s) func(buf[1/(n/p)][1%(n/p)], s##1);
//#define CALL_BUF_3(n, p, func, buf, s) CALL_BUF_2(n, p, func, buf, s) func(buf[2/(n/p)][2%(n/p)], s##2);
//#define CALL_BUF_4(n, p, func, buf, s) CALL_BUF_3(n, p, func, buf, s) func(buf[3/(n/p)][3%(n/p)], s##3);
//#define CALL_BUF_5(n, p, func, buf, s) CALL_BUF_4(n, p, func, buf, s) func(buf[4/(n/p)][4%(n/p)], s##4);
//#define CALL_BUF_6(n, p, func, buf, s) CALL_BUF_5(n, p, func, buf, s) func(buf[5/(n/p)][5%(n/p)], s##5);
//#define CALL_BUF_7(n, p, func, buf, s) CALL_BUF_6(n, p, func, buf, s) func(buf[6/(n/p)][6%(n/p)], s##6);
//#define CALL_BUF_8(n, p, func, buf, s) CALL_BUF_7(n, p, func, buf, s) func(buf[7/(n/p)][7%(n/p)], s##7);
//#define CALL_BUF_9(n, p, func, buf, s) CALL_BUF_8(n, p, func, buf, s) func(buf[8/(n/p)][8%(n/p)], s##8);
//#define CALL_BUF_10(n, p, func, buf, s) CALL_BUF_9(n, p, func, buf, s) func(buf[9/(n/p)][9%(n/p)], s##9);
//#define CALL_BUF_11(n, p, func, buf, s) CALL_BUF_10(n, p, func, buf, s) func(buf[10/(n/p)][10%(n/p)], s##10);
//#define CALL_BUF_12(n, p, func, buf, s) CALL_BUF_11(n, p, func, buf, s) func(buf[11/(n/p)][11%(n/p)], s##11);
//#define CALL_BUF_13(n, p, func, buf, s) CALL_BUF_12(n, p, func, buf, s) func(buf[12/(n/p)][12%(n/p)], s##12);
//#define CALL_BUF_14(n, p, func, buf, s) CALL_BUF_13(n, p, func, buf, s) func(buf[13/(n/p)][13%(n/p)], s##13);
//#define CALL_BUF_15(n, p, func, buf, s) CALL_BUF_14(n, p, func, buf, s) func(buf[14/(n/p)][14%(n/p)], s##14);
//#define CALL_BUF_16(n, p, func, buf, s) CALL_BUF_15(n, p, func, buf, s) func(buf[15/(n/p)][15%(n/p)], s##15);
//#define CALL_BUF_17(n, p, func, buf, s) CALL_BUF_16(n, p, func, buf, s) func(buf[16/(n/p)][16%(n/p)], s##16);
//#define CALL_BUF_18(n, p, func, buf, s) CALL_BUF_17(n, p, func, buf, s) func(buf[17/(n/p)][17%(n/p)], s##17);
//#define CALL_BUF_19(n, p, func, buf, s) CALL_BUF_18(n, p, func, buf, s) func(buf[18/(n/p)][18%(n/p)], s##18);
//#define CALL_BUF_20(n, p, func, buf, s) CALL_BUF_19(n, p, func, buf, s) func(buf[19/(n/p)][19%(n/p)], s##19);
//#define CALL_BUF_21(n, p, func, buf, s) CALL_BUF_20(n, p, func, buf, s) func(buf[20/(n/p)][20%(n/p)], s##20);
//#define CALL_BUF_22(n, p, func, buf, s) CALL_BUF_21(n, p, func, buf, s) func(buf[21/(n/p)][21%(n/p)], s##21);
//#define CALL_BUF_23(n, p, func, buf, s) CALL_BUF_22(n, p, func, buf, s) func(buf[22/(n/p)][22%(n/p)], s##22);
//#define CALL_BUF_24(n, p, func, buf, s) CALL_BUF_23(n, p, func, buf, s) func(buf[23/(n/p)][23%(n/p)], s##23);
//#define CALL_BUF_25(n, p, func, buf, s) CALL_BUF_24(n, p, func, buf, s) func(buf[24/(n/p)][24%(n/p)], s##24);
//#define CALL_BUF_26(n, p, func, buf, s) CALL_BUF_25(n, p, func, buf, s) func(buf[25/(n/p)][25%(n/p)], s##25);
//#define CALL_BUF_27(n, p, func, buf, s) CALL_BUF_26(n, p, func, buf, s) func(buf[26/(n/p)][26%(n/p)], s##26);
//#define CALL_BUF_28(n, p, func, buf, s) CALL_BUF_27(n, p, func, buf, s) func(buf[27/(n/p)][27%(n/p)], s##27);
//#define CALL_BUF_29(n, p, func, buf, s) CALL_BUF_28(n, p, func, buf, s) func(buf[28/(n/p)][28%(n/p)], s##28);
//#define CALL_BUF_30(n, p, func, buf, s) CALL_BUF_29(n, p, func, buf, s) func(buf[29/(n/p)][29%(n/p)], s##29);
//#define CALL_BUF_31(n, p, func, buf, s) CALL_BUF_30(n, p, func, buf, s) func(buf[30/(n/p)][30%(n/p)], s##30);
//#define CALL_BUF_32(n, p, func, buf, s) CALL_BUF_31(n, p, func, buf, s) func(buf[31/(n/p)][31%(n/p)], s##31);
//#define CALL_BUF_33(n, p, func, buf, s) CALL_BUF_32(n, p, func, buf, s) func(buf[32/(n/p)][32%(n/p)], s##32);
//#define CALL_BUF_34(n, p, func, buf, s) CALL_BUF_33(n, p, func, buf, s) func(buf[33/(n/p)][33%(n/p)], s##33);
//#define CALL_BUF_35(n, p, func, buf, s) CALL_BUF_34(n, p, func, buf, s) func(buf[34/(n/p)][34%(n/p)], s##34);
//#define CALL_BUF_36(n, p, func, buf, s) CALL_BUF_35(n, p, func, buf, s) func(buf[35/(n/p)][35%(n/p)], s##35);
//#define CALL_BUF_37(n, p, func, buf, s) CALL_BUF_36(n, p, func, buf, s) func(buf[36/(n/p)][36%(n/p)], s##36);
//#define CALL_BUF_38(n, p, func, buf, s) CALL_BUF_37(n, p, func, buf, s) func(buf[37/(n/p)][37%(n/p)], s##37);
//#define CALL_BUF_39(n, p, func, buf, s) CALL_BUF_38(n, p, func, buf, s) func(buf[38/(n/p)][38%(n/p)], s##38);
//#define CALL_BUF_40(n, p, func, buf, s) CALL_BUF_39(n, p, func, buf, s) func(buf[39/(n/p)][39%(n/p)], s##39);
//#define CALL_BUF_41(n, p, func, buf, s) CALL_BUF_40(n, p, func, buf, s) func(buf[40/(n/p)][40%(n/p)], s##40);
//#define CALL_BUF_42(n, p, func, buf, s) CALL_BUF_41(n, p, func, buf, s) func(buf[41/(n/p)][41%(n/p)], s##41);
//#define CALL_BUF_43(n, p, func, buf, s) CALL_BUF_42(n, p, func, buf, s) func(buf[42/(n/p)][42%(n/p)], s##42);
//#define CALL_BUF_44(n, p, func, buf, s) CALL_BUF_43(n, p, func, buf, s) func(buf[43/(n/p)][43%(n/p)], s##43);
//#define CALL_BUF_45(n, p, func, buf, s) CALL_BUF_44(n, p, func, buf, s) func(buf[44/(n/p)][44%(n/p)], s##44);
//#define CALL_BUF_46(n, p, func, buf, s) CALL_BUF_45(n, p, func, buf, s) func(buf[45/(n/p)][45%(n/p)], s##45);
//#define CALL_BUF_47(n, p, func, buf, s) CALL_BUF_46(n, p, func, buf, s) func(buf[46/(n/p)][46%(n/p)], s##46);
//#define CALL_BUF_48(n, p, func, buf, s) CALL_BUF_47(n, p, func, buf, s) func(buf[47/(n/p)][47%(n/p)], s##47);
//#define CALL_BUF_49(n, p, func, buf, s) CALL_BUF_48(n, p, func, buf, s) func(buf[48/(n/p)][48%(n/p)], s##48);
///* }}} */
//#define CALL_BUF_50(n, p, func, buf, s) CALL_BUF_49(n, p, func, buf, s) func(buf[49/(n/p)][49%(n/p)], s##49);

//#define LOAD_BUF(n, p, func, split, buf, b) CONCAT(LOAD_BUF_, n)(n, p, func, split, buf, b)
///* {{{ */
//#define LOAD_BUF_1(n, p, func, split, buf, b) func(split[0/(n/p)][0%(n/p)], buf[0/(n/p)][0%(n/p)], b);
//#define LOAD_BUF_2(n, p, func, split, buf, b) LOAD_BUF_1(n, p, func, split, buf, b) func(split[1/(n/p)][1%(n/p)], buf[1/(n/p)][1%(n/p)], b);
//#define LOAD_BUF_3(n, p, func, split, buf, b) LOAD_BUF_2(n, p, func, split, buf, b) func(split[2/(n/p)][2%(n/p)], buf[2/(n/p)][2%(n/p)], b);
//#define LOAD_BUF_4(n, p, func, split, buf, b) LOAD_BUF_3(n, p, func, split, buf, b) func(split[3/(n/p)][3%(n/p)], buf[3/(n/p)][3%(n/p)], b);
//#define LOAD_BUF_5(n, p, func, split, buf, b) LOAD_BUF_4(n, p, func, split, buf, b) func(split[4/(n/p)][4%(n/p)], buf[4/(n/p)][4%(n/p)], b);
//#define LOAD_BUF_6(n, p, func, split, buf, b) LOAD_BUF_5(n, p, func, split, buf, b) func(split[5/(n/p)][5%(n/p)], buf[5/(n/p)][5%(n/p)], b);
//#define LOAD_BUF_7(n, p, func, split, buf, b) LOAD_BUF_6(n, p, func, split, buf, b) func(split[6/(n/p)][6%(n/p)], buf[6/(n/p)][6%(n/p)], b);
//#define LOAD_BUF_8(n, p, func, split, buf, b) LOAD_BUF_7(n, p, func, split, buf, b) func(split[7/(n/p)][7%(n/p)], buf[7/(n/p)][7%(n/p)], b);
//#define LOAD_BUF_9(n, p, func, split, buf, b) LOAD_BUF_8(n, p, func, split, buf, b) func(split[8/(n/p)][8%(n/p)], buf[8/(n/p)][8%(n/p)], b);
//#define LOAD_BUF_10(n, p, func, split, buf, b) LOAD_BUF_9(n, p, func, split, buf, b) func(split[9/(n/p)][9%(n/p)], buf[9/(n/p)][9%(n/p)], b);
//#define LOAD_BUF_11(n, p, func, split, buf, b) LOAD_BUF_10(n, p, func, split, buf, b) func(split[10/(n/p)][10%(n/p)], buf[10/(n/p)][10%(n/p)], b);
//#define LOAD_BUF_12(n, p, func, split, buf, b) LOAD_BUF_11(n, p, func, split, buf, b) func(split[11/(n/p)][11%(n/p)], buf[11/(n/p)][11%(n/p)], b);
//#define LOAD_BUF_13(n, p, func, split, buf, b) LOAD_BUF_12(n, p, func, split, buf, b) func(split[12/(n/p)][12%(n/p)], buf[12/(n/p)][12%(n/p)], b);
//#define LOAD_BUF_14(n, p, func, split, buf, b) LOAD_BUF_13(n, p, func, split, buf, b) func(split[13/(n/p)][13%(n/p)], buf[13/(n/p)][13%(n/p)], b);
//#define LOAD_BUF_15(n, p, func, split, buf, b) LOAD_BUF_14(n, p, func, split, buf, b) func(split[14/(n/p)][14%(n/p)], buf[14/(n/p)][14%(n/p)], b);
//#define LOAD_BUF_16(n, p, func, split, buf, b) LOAD_BUF_15(n, p, func, split, buf, b) func(split[15/(n/p)][15%(n/p)], buf[15/(n/p)][15%(n/p)], b);
//#define LOAD_BUF_17(n, p, func, split, buf, b) LOAD_BUF_16(n, p, func, split, buf, b) func(split[16/(n/p)][16%(n/p)], buf[16/(n/p)][16%(n/p)], b);
//#define LOAD_BUF_18(n, p, func, split, buf, b) LOAD_BUF_17(n, p, func, split, buf, b) func(split[17/(n/p)][17%(n/p)], buf[17/(n/p)][17%(n/p)], b);
//#define LOAD_BUF_19(n, p, func, split, buf, b) LOAD_BUF_18(n, p, func, split, buf, b) func(split[18/(n/p)][18%(n/p)], buf[18/(n/p)][18%(n/p)], b);
//#define LOAD_BUF_20(n, p, func, split, buf, b) LOAD_BUF_19(n, p, func, split, buf, b) func(split[19/(n/p)][19%(n/p)], buf[19/(n/p)][19%(n/p)], b);
//#define LOAD_BUF_21(n, p, func, split, buf, b) LOAD_BUF_20(n, p, func, split, buf, b) func(split[20/(n/p)][20%(n/p)], buf[20/(n/p)][20%(n/p)], b);
//#define LOAD_BUF_22(n, p, func, split, buf, b) LOAD_BUF_21(n, p, func, split, buf, b) func(split[21/(n/p)][21%(n/p)], buf[21/(n/p)][21%(n/p)], b);
//#define LOAD_BUF_23(n, p, func, split, buf, b) LOAD_BUF_22(n, p, func, split, buf, b) func(split[22/(n/p)][22%(n/p)], buf[22/(n/p)][22%(n/p)], b);
//#define LOAD_BUF_24(n, p, func, split, buf, b) LOAD_BUF_23(n, p, func, split, buf, b) func(split[23/(n/p)][23%(n/p)], buf[23/(n/p)][23%(n/p)], b);
//#define LOAD_BUF_25(n, p, func, split, buf, b) LOAD_BUF_24(n, p, func, split, buf, b) func(split[24/(n/p)][24%(n/p)], buf[24/(n/p)][24%(n/p)], b);
//#define LOAD_BUF_26(n, p, func, split, buf, b) LOAD_BUF_25(n, p, func, split, buf, b) func(split[25/(n/p)][25%(n/p)], buf[25/(n/p)][25%(n/p)], b);
//#define LOAD_BUF_27(n, p, func, split, buf, b) LOAD_BUF_26(n, p, func, split, buf, b) func(split[26/(n/p)][26%(n/p)], buf[26/(n/p)][26%(n/p)], b);
//#define LOAD_BUF_28(n, p, func, split, buf, b) LOAD_BUF_27(n, p, func, split, buf, b) func(split[27/(n/p)][27%(n/p)], buf[27/(n/p)][27%(n/p)], b);
//#define LOAD_BUF_29(n, p, func, split, buf, b) LOAD_BUF_28(n, p, func, split, buf, b) func(split[28/(n/p)][28%(n/p)], buf[28/(n/p)][28%(n/p)], b);
//#define LOAD_BUF_30(n, p, func, split, buf, b) LOAD_BUF_29(n, p, func, split, buf, b) func(split[29/(n/p)][29%(n/p)], buf[29/(n/p)][29%(n/p)], b);
//#define LOAD_BUF_31(n, p, func, split, buf, b) LOAD_BUF_30(n, p, func, split, buf, b) func(split[30/(n/p)][30%(n/p)], buf[30/(n/p)][30%(n/p)], b);
//#define LOAD_BUF_32(n, p, func, split, buf, b) LOAD_BUF_31(n, p, func, split, buf, b) func(split[31/(n/p)][31%(n/p)], buf[31/(n/p)][31%(n/p)], b);
//#define LOAD_BUF_33(n, p, func, split, buf, b) LOAD_BUF_32(n, p, func, split, buf, b) func(split[32/(n/p)][32%(n/p)], buf[32/(n/p)][32%(n/p)], b);
//#define LOAD_BUF_34(n, p, func, split, buf, b) LOAD_BUF_33(n, p, func, split, buf, b) func(split[33/(n/p)][33%(n/p)], buf[33/(n/p)][33%(n/p)], b);
//#define LOAD_BUF_35(n, p, func, split, buf, b) LOAD_BUF_34(n, p, func, split, buf, b) func(split[34/(n/p)][34%(n/p)], buf[34/(n/p)][34%(n/p)], b);
//#define LOAD_BUF_36(n, p, func, split, buf, b) LOAD_BUF_35(n, p, func, split, buf, b) func(split[35/(n/p)][35%(n/p)], buf[35/(n/p)][35%(n/p)], b);
//#define LOAD_BUF_37(n, p, func, split, buf, b) LOAD_BUF_36(n, p, func, split, buf, b) func(split[36/(n/p)][36%(n/p)], buf[36/(n/p)][36%(n/p)], b);
//#define LOAD_BUF_38(n, p, func, split, buf, b) LOAD_BUF_37(n, p, func, split, buf, b) func(split[37/(n/p)][37%(n/p)], buf[37/(n/p)][37%(n/p)], b);
//#define LOAD_BUF_39(n, p, func, split, buf, b) LOAD_BUF_38(n, p, func, split, buf, b) func(split[38/(n/p)][38%(n/p)], buf[38/(n/p)][38%(n/p)], b);
//#define LOAD_BUF_40(n, p, func, split, buf, b) LOAD_BUF_39(n, p, func, split, buf, b) func(split[39/(n/p)][39%(n/p)], buf[39/(n/p)][39%(n/p)], b);
//#define LOAD_BUF_41(n, p, func, split, buf, b) LOAD_BUF_40(n, p, func, split, buf, b) func(split[40/(n/p)][40%(n/p)], buf[40/(n/p)][40%(n/p)], b);
//#define LOAD_BUF_42(n, p, func, split, buf, b) LOAD_BUF_41(n, p, func, split, buf, b) func(split[41/(n/p)][41%(n/p)], buf[41/(n/p)][41%(n/p)], b);
//#define LOAD_BUF_43(n, p, func, split, buf, b) LOAD_BUF_42(n, p, func, split, buf, b) func(split[42/(n/p)][42%(n/p)], buf[42/(n/p)][42%(n/p)], b);
//#define LOAD_BUF_44(n, p, func, split, buf, b) LOAD_BUF_43(n, p, func, split, buf, b) func(split[43/(n/p)][43%(n/p)], buf[43/(n/p)][43%(n/p)], b);
//#define LOAD_BUF_45(n, p, func, split, buf, b) LOAD_BUF_44(n, p, func, split, buf, b) func(split[44/(n/p)][44%(n/p)], buf[44/(n/p)][44%(n/p)], b);
//#define LOAD_BUF_46(n, p, func, split, buf, b) LOAD_BUF_45(n, p, func, split, buf, b) func(split[45/(n/p)][45%(n/p)], buf[45/(n/p)][45%(n/p)], b);
//#define LOAD_BUF_47(n, p, func, split, buf, b) LOAD_BUF_46(n, p, func, split, buf, b) func(split[46/(n/p)][46%(n/p)], buf[46/(n/p)][46%(n/p)], b);
//#define LOAD_BUF_48(n, p, func, split, buf, b) LOAD_BUF_47(n, p, func, split, buf, b) func(split[47/(n/p)][47%(n/p)], buf[47/(n/p)][47%(n/p)], b);
//#define LOAD_BUF_49(n, p, func, split, buf, b) LOAD_BUF_48(n, p, func, split, buf, b) func(split[48/(n/p)][48%(n/p)], buf[48/(n/p)][48%(n/p)], b);
///* }}} */
//#define LOAD_BUF_50(n, p, func, split, buf, b) LOAD_BUF_49(n, p, func, split, buf, b) func(split[49/(n/p)][49%(n/p)], buf[49/(n/p)][49%(n/p)], b);

#define CALL_STREAMS(n, p, func, split, buf, s) CONCAT(CALL_STREAMS_, n)(n, p, func, split, buf, s)
/* {{{ */
#define CALL_STREAMS_1(n, p, func, split, buf, s) \
  func(split[0/(n/p)][0%(n/p)], buf[0/(n/p)][0%(n/p)], s##0);
#define CALL_STREAMS_2(n, p, func, split, buf, s) CALL_STREAMS_1(n, p, func, split, buf, s) \
  func(split[1/(n/p)][1%(n/p)], buf[1/(n/p)][1%(n/p)], s##1);
#define CALL_STREAMS_3(n, p, func, split, buf, s) CALL_STREAMS_2(n, p, func, split, buf, s) \
  func(split[2/(n/p)][2%(n/p)], buf[2/(n/p)][2%(n/p)], s##2);
#define CALL_STREAMS_4(n, p, func, split, buf, s) CALL_STREAMS_3(n, p, func, split, buf, s) \
  func(split[3/(n/p)][3%(n/p)], buf[3/(n/p)][3%(n/p)], s##3);
#define CALL_STREAMS_5(n, p, func, split, buf, s) CALL_STREAMS_4(n, p, func, split, buf, s) \
  func(split[4/(n/p)][4%(n/p)], buf[4/(n/p)][4%(n/p)], s##4);
#define CALL_STREAMS_6(n, p, func, split, buf, s) CALL_STREAMS_5(n, p, func, split, buf, s) \
  func(split[5/(n/p)][5%(n/p)], buf[5/(n/p)][5%(n/p)], s##5);
#define CALL_STREAMS_7(n, p, func, split, buf, s) CALL_STREAMS_6(n, p, func, split, buf, s) \
  func(split[6/(n/p)][6%(n/p)], buf[6/(n/p)][6%(n/p)], s##6);
#define CALL_STREAMS_8(n, p, func, split, buf, s) CALL_STREAMS_7(n, p, func, split, buf, s) \
  func(split[7/(n/p)][7%(n/p)], buf[7/(n/p)][7%(n/p)], s##7);
#define CALL_STREAMS_9(n, p, func, split, buf, s) CALL_STREAMS_8(n, p, func, split, buf, s) \
  func(split[8/(n/p)][8%(n/p)], buf[8/(n/p)][8%(n/p)], s##8);
#define CALL_STREAMS_10(n, p, func, split, buf, s) CALL_STREAMS_9(n, p, func, split, buf, s) \
  func(split[9/(n/p)][9%(n/p)], buf[9/(n/p)][9%(n/p)], s##9);
#define CALL_STREAMS_11(n, p, func, split, buf, s) CALL_STREAMS_10(n, p, func, split, buf, s) \
  func(split[10/(n/p)][10%(n/p)], buf[10/(n/p)][10%(n/p)], s##10);
#define CALL_STREAMS_12(n, p, func, split, buf, s) CALL_STREAMS_11(n, p, func, split, buf, s) \
  func(split[11/(n/p)][11%(n/p)], buf[11/(n/p)][11%(n/p)], s##11);
#define CALL_STREAMS_13(n, p, func, split, buf, s) CALL_STREAMS_12(n, p, func, split, buf, s) \
  func(split[12/(n/p)][12%(n/p)], buf[12/(n/p)][12%(n/p)], s##12);
#define CALL_STREAMS_14(n, p, func, split, buf, s) CALL_STREAMS_13(n, p, func, split, buf, s) \
  func(split[13/(n/p)][13%(n/p)], buf[13/(n/p)][13%(n/p)], s##13);
#define CALL_STREAMS_15(n, p, func, split, buf, s) CALL_STREAMS_14(n, p, func, split, buf, s) \
  func(split[14/(n/p)][14%(n/p)], buf[14/(n/p)][14%(n/p)], s##14);
#define CALL_STREAMS_16(n, p, func, split, buf, s) CALL_STREAMS_15(n, p, func, split, buf, s) \
  func(split[15/(n/p)][15%(n/p)], buf[15/(n/p)][15%(n/p)], s##15);
#define CALL_STREAMS_17(n, p, func, split, buf, s) CALL_STREAMS_16(n, p, func, split, buf, s) \
  func(split[16/(n/p)][16%(n/p)], buf[16/(n/p)][16%(n/p)], s##16);
#define CALL_STREAMS_18(n, p, func, split, buf, s) CALL_STREAMS_17(n, p, func, split, buf, s) \
  func(split[17/(n/p)][17%(n/p)], buf[17/(n/p)][17%(n/p)], s##17);
#define CALL_STREAMS_19(n, p, func, split, buf, s) CALL_STREAMS_18(n, p, func, split, buf, s) \
  func(split[18/(n/p)][18%(n/p)], buf[18/(n/p)][18%(n/p)], s##18);
#define CALL_STREAMS_20(n, p, func, split, buf, s) CALL_STREAMS_19(n, p, func, split, buf, s) \
  func(split[19/(n/p)][19%(n/p)], buf[19/(n/p)][19%(n/p)], s##19);
#define CALL_STREAMS_21(n, p, func, split, buf, s) CALL_STREAMS_20(n, p, func, split, buf, s) \
  func(split[20/(n/p)][20%(n/p)], buf[20/(n/p)][20%(n/p)], s##20);
#define CALL_STREAMS_22(n, p, func, split, buf, s) CALL_STREAMS_21(n, p, func, split, buf, s) \
  func(split[21/(n/p)][21%(n/p)], buf[21/(n/p)][21%(n/p)], s##21);
#define CALL_STREAMS_23(n, p, func, split, buf, s) CALL_STREAMS_22(n, p, func, split, buf, s) \
  func(split[22/(n/p)][22%(n/p)], buf[22/(n/p)][22%(n/p)], s##22);
#define CALL_STREAMS_24(n, p, func, split, buf, s) CALL_STREAMS_23(n, p, func, split, buf, s) \
  func(split[23/(n/p)][23%(n/p)], buf[23/(n/p)][23%(n/p)], s##23);
#define CALL_STREAMS_25(n, p, func, split, buf, s) CALL_STREAMS_24(n, p, func, split, buf, s) \
  func(split[24/(n/p)][24%(n/p)], buf[24/(n/p)][24%(n/p)], s##24);
#define CALL_STREAMS_26(n, p, func, split, buf, s) CALL_STREAMS_25(n, p, func, split, buf, s) \
  func(split[25/(n/p)][25%(n/p)], buf[25/(n/p)][25%(n/p)], s##25);
#define CALL_STREAMS_27(n, p, func, split, buf, s) CALL_STREAMS_26(n, p, func, split, buf, s) \
  func(split[26/(n/p)][26%(n/p)], buf[26/(n/p)][26%(n/p)], s##26);
#define CALL_STREAMS_28(n, p, func, split, buf, s) CALL_STREAMS_27(n, p, func, split, buf, s) \
  func(split[27/(n/p)][27%(n/p)], buf[27/(n/p)][27%(n/p)], s##27);
#define CALL_STREAMS_29(n, p, func, split, buf, s) CALL_STREAMS_28(n, p, func, split, buf, s) \
  func(split[28/(n/p)][28%(n/p)], buf[28/(n/p)][28%(n/p)], s##28);
#define CALL_STREAMS_30(n, p, func, split, buf, s) CALL_STREAMS_29(n, p, func, split, buf, s) \
  func(split[29/(n/p)][29%(n/p)], buf[29/(n/p)][29%(n/p)], s##29);
#define CALL_STREAMS_31(n, p, func, split, buf, s) CALL_STREAMS_30(n, p, func, split, buf, s) \
  func(split[30/(n/p)][30%(n/p)], buf[30/(n/p)][30%(n/p)], s##30);
#define CALL_STREAMS_32(n, p, func, split, buf, s) CALL_STREAMS_31(n, p, func, split, buf, s) \
  func(split[31/(n/p)][31%(n/p)], buf[31/(n/p)][31%(n/p)], s##31);
#define CALL_STREAMS_33(n, p, func, split, buf, s) CALL_STREAMS_32(n, p, func, split, buf, s) \
  func(split[32/(n/p)][32%(n/p)], buf[32/(n/p)][32%(n/p)], s##32);
#define CALL_STREAMS_34(n, p, func, split, buf, s) CALL_STREAMS_33(n, p, func, split, buf, s) \
  func(split[33/(n/p)][33%(n/p)], buf[33/(n/p)][33%(n/p)], s##33);
#define CALL_STREAMS_35(n, p, func, split, buf, s) CALL_STREAMS_34(n, p, func, split, buf, s) \
  func(split[34/(n/p)][34%(n/p)], buf[34/(n/p)][34%(n/p)], s##34);
#define CALL_STREAMS_36(n, p, func, split, buf, s) CALL_STREAMS_35(n, p, func, split, buf, s) \
  func(split[35/(n/p)][35%(n/p)], buf[35/(n/p)][35%(n/p)], s##35);
#define CALL_STREAMS_37(n, p, func, split, buf, s) CALL_STREAMS_36(n, p, func, split, buf, s) \
  func(split[36/(n/p)][36%(n/p)], buf[36/(n/p)][36%(n/p)], s##36);
#define CALL_STREAMS_38(n, p, func, split, buf, s) CALL_STREAMS_37(n, p, func, split, buf, s) \
  func(split[37/(n/p)][37%(n/p)], buf[37/(n/p)][37%(n/p)], s##37);
#define CALL_STREAMS_39(n, p, func, split, buf, s) CALL_STREAMS_38(n, p, func, split, buf, s) \
  func(split[38/(n/p)][38%(n/p)], buf[38/(n/p)][38%(n/p)], s##38);
#define CALL_STREAMS_40(n, p, func, split, buf, s) CALL_STREAMS_39(n, p, func, split, buf, s) \
  func(split[39/(n/p)][39%(n/p)], buf[39/(n/p)][39%(n/p)], s##39);
#define CALL_STREAMS_41(n, p, func, split, buf, s) CALL_STREAMS_40(n, p, func, split, buf, s) \
  func(split[40/(n/p)][40%(n/p)], buf[40/(n/p)][40%(n/p)], s##40);
#define CALL_STREAMS_42(n, p, func, split, buf, s) CALL_STREAMS_41(n, p, func, split, buf, s) \
  func(split[41/(n/p)][41%(n/p)], buf[41/(n/p)][41%(n/p)], s##41);
#define CALL_STREAMS_43(n, p, func, split, buf, s) CALL_STREAMS_42(n, p, func, split, buf, s) \
  func(split[42/(n/p)][42%(n/p)], buf[42/(n/p)][42%(n/p)], s##42);
#define CALL_STREAMS_44(n, p, func, split, buf, s) CALL_STREAMS_43(n, p, func, split, buf, s) \
  func(split[43/(n/p)][43%(n/p)], buf[43/(n/p)][43%(n/p)], s##43);
#define CALL_STREAMS_45(n, p, func, split, buf, s) CALL_STREAMS_44(n, p, func, split, buf, s) \
  func(split[44/(n/p)][44%(n/p)], buf[44/(n/p)][44%(n/p)], s##44);
#define CALL_STREAMS_46(n, p, func, split, buf, s) CALL_STREAMS_45(n, p, func, split, buf, s) \
  func(split[45/(n/p)][45%(n/p)], buf[45/(n/p)][45%(n/p)], s##45);
#define CALL_STREAMS_47(n, p, func, split, buf, s) CALL_STREAMS_46(n, p, func, split, buf, s) \
  func(split[46/(n/p)][46%(n/p)], buf[46/(n/p)][46%(n/p)], s##46);
#define CALL_STREAMS_48(n, p, func, split, buf, s) CALL_STREAMS_47(n, p, func, split, buf, s) \
  func(split[47/(n/p)][47%(n/p)], buf[47/(n/p)][47%(n/p)], s##47);
#define CALL_STREAMS_49(n, p, func, split, buf, s) CALL_STREAMS_48(n, p, func, split, buf, s) \
  func(split[48/(n/p)][48%(n/p)], buf[48/(n/p)][48%(n/p)], s##48);
/* }}} */
#define CALL_STREAMS_50(n, p, func, split, buf, s) CALL_STREAMS_49(n, p, func, split, buf, s) \
  func(split[49/(n/p)][49%(n/p)], buf[49/(n/p)][49%(n/p)], s##49);

#define CALL_STREAMS_TASK(n, p, name, func, split, buf, s) \
        CONCAT(CALL_STREAMS_TASK_, n)(n, p, name, func, split, buf, s)
/* {{{ */
#define CALL_STREAMS_TASK_1(n, p, name, func, split, buf, s) \
  hls_thread_local hls::task name##0( \
    [&b = buf[0/(n/p)][0%(n/p)], &ps = split[0/(n/p)][0%(n/p)], &as = s##0]() \
    { func(ps, b, as); });
#define CALL_STREAMS_TASK_2(n, p, name, func, split, buf, s) \
        CALL_STREAMS_TASK_1(n, p, name, func, split, buf, s) \
  hls_thread_local hls::task name##1( \
    [&b = buf[1/(n/p)][1%(n/p)], &ps = split[1/(n/p)][1%(n/p)], &as = s##1]() \
    { func(ps, b, as); });
#define CALL_STREAMS_TASK_3(n, p, name, func, split, buf, s) \
        CALL_STREAMS_TASK_2(n, p, name, func, split, buf, s) \
  hls_thread_local hls::task name##2( \
    [&b = buf[2/(n/p)][2%(n/p)], &ps = split[2/(n/p)][2%(n/p)], &as = s##2]() \
    { func(ps, b, as); });
#define CALL_STREAMS_TASK_4(n, p, name, func, split, buf, s) \
        CALL_STREAMS_TASK_3(n, p, name, func, split, buf, s) \
  hls_thread_local hls::task name##3( \
    [&b = buf[3/(n/p)][3%(n/p)], &ps = split[3/(n/p)][3%(n/p)], &as = s##3]() \
    { func(ps, b, as); });
#define CALL_STREAMS_TASK_5(n, p, name, func, split, buf, s) \
        CALL_STREAMS_TASK_4(n, p, name, func, split, buf, s) \
  hls_thread_local hls::task name##4( \
    [&b = buf[4/(n/p)][4%(n/p)], &ps = split[4/(n/p)][4%(n/p)], &as = s##4]() \
    { func(ps, b, as); });
#define CALL_STREAMS_TASK_6(n, p, name, func, split, buf, s) \
        CALL_STREAMS_TASK_5(n, p, name, func, split, buf, s) \
  hls_thread_local hls::task name##5( \
    [&b = buf[5/(n/p)][5%(n/p)], &ps = split[5/(n/p)][5%(n/p)], &as = s##5]() \
    { func(ps, b, as); });
#define CALL_STREAMS_TASK_7(n, p, name, func, split, buf, s) \
        CALL_STREAMS_TASK_6(n, p, name, func, split, buf, s) \
  hls_thread_local hls::task name##6( \
    [&b = buf[6/(n/p)][6%(n/p)], &ps = split[6/(n/p)][6%(n/p)], &as = s##6]() \
    { func(ps, b, as); });
#define CALL_STREAMS_TASK_8(n, p, name, func, split, buf, s) \
        CALL_STREAMS_TASK_7(n, p, name, func, split, buf, s) \
  hls_thread_local hls::task name##7( \
    [&b = buf[7/(n/p)][7%(n/p)], &ps = split[7/(n/p)][7%(n/p)], &as = s##7]() \
    { func(ps, b, as); });
#define CALL_STREAMS_TASK_9(n, p, name, func, split, buf, s) \
        CALL_STREAMS_TASK_8(n, p, name, func, split, buf, s) \
  hls_thread_local hls::task name##8( \
    [&b = buf[8/(n/p)][8%(n/p)], &ps = split[8/(n/p)][8%(n/p)], &as = s##8]() \
    { func(ps, b, as); });
#define CALL_STREAMS_TASK_10(n, p, name, func, split, buf, s) \
        CALL_STREAMS_TASK_9(n, p, name, func, split, buf, s) \
  hls_thread_local hls::task name##9( \
    [&b = buf[9/(n/p)][9%(n/p)], &ps = split[9/(n/p)][9%(n/p)], &as = s##9]() \
    { func(ps, b, as); });
#define CALL_STREAMS_TASK_11(n, p, name, func, split, buf, s) \
        CALL_STREAMS_TASK_10(n, p, name, func, split, buf, s) \
  hls_thread_local hls::task name##10( \
    [&b = buf[10/(n/p)][10%(n/p)], &ps = split[10/(n/p)][10%(n/p)], &as = s##10]() \
    { func(ps, b, as); });
#define CALL_STREAMS_TASK_12(n, p, name, func, split, buf, s) \
        CALL_STREAMS_TASK_11(n, p, name, func, split, buf, s) \
  hls_thread_local hls::task name##11( \
    [&b = buf[11/(n/p)][11%(n/p)], &ps = split[11/(n/p)][11%(n/p)], &as = s##11]() \
    { func(ps, b, as); });
#define CALL_STREAMS_TASK_13(n, p, name, func, split, buf, s) \
        CALL_STREAMS_TASK_12(n, p, name, func, split, buf, s) \
  hls_thread_local hls::task name##12( \
    [&b = buf[12/(n/p)][12%(n/p)], &ps = split[12/(n/p)][12%(n/p)], &as = s##12]() \
    { func(ps, b, as); });
#define CALL_STREAMS_TASK_14(n, p, name, func, split, buf, s) \
        CALL_STREAMS_TASK_13(n, p, name, func, split, buf, s) \
  hls_thread_local hls::task name##13( \
    [&b = buf[13/(n/p)][13%(n/p)], &ps = split[13/(n/p)][13%(n/p)], &as = s##13]() \
    { func(ps, b, as); });
#define CALL_STREAMS_TASK_15(n, p, name, func, split, buf, s) \
        CALL_STREAMS_TASK_14(n, p, name, func, split, buf, s) \
  hls_thread_local hls::task name##14( \
    [&b = buf[14/(n/p)][14%(n/p)], &ps = split[14/(n/p)][14%(n/p)], &as = s##14]() \
    { func(ps, b, as); });
#define CALL_STREAMS_TASK_16(n, p, name, func, split, buf, s) \
        CALL_STREAMS_TASK_15(n, p, name, func, split, buf, s) \
  hls_thread_local hls::task name##15( \
    [&b = buf[15/(n/p)][15%(n/p)], &ps = split[15/(n/p)][15%(n/p)], &as = s##15]() \
    { func(ps, b, as); });
#define CALL_STREAMS_TASK_17(n, p, name, func, split, buf, s) \
        CALL_STREAMS_TASK_16(n, p, name, func, split, buf, s) \
  hls_thread_local hls::task name##16( \
    [&b = buf[16/(n/p)][16%(n/p)], &ps = split[16/(n/p)][16%(n/p)], &as = s##16]() \
    { func(ps, b, as); });
#define CALL_STREAMS_TASK_18(n, p, name, func, split, buf, s) \
        CALL_STREAMS_TASK_17(n, p, name, func, split, buf, s) \
  hls_thread_local hls::task name##17( \
    [&b = buf[17/(n/p)][17%(n/p)], &ps = split[17/(n/p)][17%(n/p)], &as = s##17]() \
    { func(ps, b, as); });
#define CALL_STREAMS_TASK_19(n, p, name, func, split, buf, s) \
        CALL_STREAMS_TASK_18(n, p, name, func, split, buf, s) \
  hls_thread_local hls::task name##18( \
    [&b = buf[18/(n/p)][18%(n/p)], &ps = split[18/(n/p)][18%(n/p)], &as = s##18]() \
    { func(ps, b, as); });
#define CALL_STREAMS_TASK_20(n, p, name, func, split, buf, s) \
        CALL_STREAMS_TASK_19(n, p, name, func, split, buf, s) \
  hls_thread_local hls::task name##19( \
    [&b = buf[19/(n/p)][19%(n/p)], &ps = split[19/(n/p)][19%(n/p)], &as = s##19]() \
    { func(ps, b, as); });
#define CALL_STREAMS_TASK_21(n, p, name, func, split, buf, s) \
        CALL_STREAMS_TASK_20(n, p, name, func, split, buf, s) \
  hls_thread_local hls::task name##20( \
    [&b = buf[20/(n/p)][20%(n/p)], &ps = split[20/(n/p)][20%(n/p)], &as = s##20]() \
    { func(ps, b, as); });
#define CALL_STREAMS_TASK_22(n, p, name, func, split, buf, s) \
        CALL_STREAMS_TASK_21(n, p, name, func, split, buf, s) \
  hls_thread_local hls::task name##21( \
    [&b = buf[21/(n/p)][21%(n/p)], &ps = split[21/(n/p)][21%(n/p)], &as = s##21]() \
    { func(ps, b, as); });
#define CALL_STREAMS_TASK_23(n, p, name, func, split, buf, s) \
        CALL_STREAMS_TASK_22(n, p, name, func, split, buf, s) \
  hls_thread_local hls::task name##22( \
    [&b = buf[22/(n/p)][22%(n/p)], &ps = split[22/(n/p)][22%(n/p)], &as = s##22]() \
    { func(ps, b, as); });
#define CALL_STREAMS_TASK_24(n, p, name, func, split, buf, s) \
        CALL_STREAMS_TASK_23(n, p, name, func, split, buf, s) \
  hls_thread_local hls::task name##23( \
    [&b = buf[23/(n/p)][23%(n/p)], &ps = split[23/(n/p)][23%(n/p)], &as = s##23]() \
    { func(ps, b, as); });
#define CALL_STREAMS_TASK_25(n, p, name, func, split, buf, s) \
        CALL_STREAMS_TASK_24(n, p, name, func, split, buf, s) \
  hls_thread_local hls::task name##24( \
    [&b = buf[24/(n/p)][24%(n/p)], &ps = split[24/(n/p)][24%(n/p)], &as = s##24]() \
    { func(ps, b, as); });
#define CALL_STREAMS_TASK_26(n, p, name, func, split, buf, s) \
        CALL_STREAMS_TASK_25(n, p, name, func, split, buf, s) \
  hls_thread_local hls::task name##25( \
    [&b = buf[25/(n/p)][25%(n/p)], &ps = split[25/(n/p)][25%(n/p)], &as = s##25]() \
    { func(ps, b, as); });
#define CALL_STREAMS_TASK_27(n, p, name, func, split, buf, s) \
        CALL_STREAMS_TASK_26(n, p, name, func, split, buf, s) \
  hls_thread_local hls::task name##26( \
    [&b = buf[26/(n/p)][26%(n/p)], &ps = split[26/(n/p)][26%(n/p)], &as = s##26]() \
    { func(ps, b, as); });
#define CALL_STREAMS_TASK_28(n, p, name, func, split, buf, s) \
        CALL_STREAMS_TASK_27(n, p, name, func, split, buf, s) \
  hls_thread_local hls::task name##27( \
    [&b = buf[27/(n/p)][27%(n/p)], &ps = split[27/(n/p)][27%(n/p)], &as = s##27]() \
    { func(ps, b, as); });
#define CALL_STREAMS_TASK_29(n, p, name, func, split, buf, s) \
        CALL_STREAMS_TASK_28(n, p, name, func, split, buf, s) \
  hls_thread_local hls::task name##28( \
    [&b = buf[28/(n/p)][28%(n/p)], &ps = split[28/(n/p)][28%(n/p)], &as = s##28]() \
    { func(ps, b, as); });
#define CALL_STREAMS_TASK_30(n, p, name, func, split, buf, s) \
        CALL_STREAMS_TASK_29(n, p, name, func, split, buf, s) \
  hls_thread_local hls::task name##29( \
    [&b = buf[29/(n/p)][29%(n/p)], &ps = split[29/(n/p)][29%(n/p)], &as = s##29]() \
    { func(ps, b, as); });
#define CALL_STREAMS_TASK_31(n, p, name, func, split, buf, s) \
        CALL_STREAMS_TASK_30(n, p, name, func, split, buf, s) \
  hls_thread_local hls::task name##30( \
    [&b = buf[30/(n/p)][30%(n/p)], &ps = split[30/(n/p)][30%(n/p)], &as = s##30]() \
    { func(ps, b, as); });
#define CALL_STREAMS_TASK_32(n, p, name, func, split, buf, s) \
        CALL_STREAMS_TASK_31(n, p, name, func, split, buf, s) \
  hls_thread_local hls::task name##31( \
    [&b = buf[31/(n/p)][31%(n/p)], &ps = split[31/(n/p)][31%(n/p)], &as = s##31]() \
    { func(ps, b, as); });
#define CALL_STREAMS_TASK_33(n, p, name, func, split, buf, s) \
        CALL_STREAMS_TASK_32(n, p, name, func, split, buf, s) \
  hls_thread_local hls::task name##32( \
    [&b = buf[32/(n/p)][32%(n/p)], &ps = split[32/(n/p)][32%(n/p)], &as = s##32]() \
    { func(ps, b, as); });
#define CALL_STREAMS_TASK_34(n, p, name, func, split, buf, s) \
        CALL_STREAMS_TASK_33(n, p, name, func, split, buf, s) \
  hls_thread_local hls::task name##33( \
    [&b = buf[33/(n/p)][33%(n/p)], &ps = split[33/(n/p)][33%(n/p)], &as = s##33]() \
    { func(ps, b, as); });
#define CALL_STREAMS_TASK_35(n, p, name, func, split, buf, s) \
        CALL_STREAMS_TASK_34(n, p, name, func, split, buf, s) \
  hls_thread_local hls::task name##34( \
    [&b = buf[34/(n/p)][34%(n/p)], &ps = split[34/(n/p)][34%(n/p)], &as = s##34]() \
    { func(ps, b, as); });
#define CALL_STREAMS_TASK_36(n, p, name, func, split, buf, s) \
        CALL_STREAMS_TASK_35(n, p, name, func, split, buf, s) \
  hls_thread_local hls::task name##35( \
    [&b = buf[35/(n/p)][35%(n/p)], &ps = split[35/(n/p)][35%(n/p)], &as = s##35]() \
    { func(ps, b, as); });
#define CALL_STREAMS_TASK_37(n, p, name, func, split, buf, s) \
        CALL_STREAMS_TASK_36(n, p, name, func, split, buf, s) \
  hls_thread_local hls::task name##36( \
    [&b = buf[36/(n/p)][36%(n/p)], &ps = split[36/(n/p)][36%(n/p)], &as = s##36]() \
    { func(ps, b, as); });
#define CALL_STREAMS_TASK_38(n, p, name, func, split, buf, s) \
        CALL_STREAMS_TASK_37(n, p, name, func, split, buf, s) \
  hls_thread_local hls::task name##37( \
    [&b = buf[37/(n/p)][37%(n/p)], &ps = split[37/(n/p)][37%(n/p)], &as = s##37]() \
    { func(ps, b, as); });
#define CALL_STREAMS_TASK_39(n, p, name, func, split, buf, s) \
        CALL_STREAMS_TASK_38(n, p, name, func, split, buf, s) \
  hls_thread_local hls::task name##38( \
    [&b = buf[38/(n/p)][38%(n/p)], &ps = split[38/(n/p)][38%(n/p)], &as = s##38]() \
    { func(ps, b, as); });
#define CALL_STREAMS_TASK_40(n, p, name, func, split, buf, s) \
        CALL_STREAMS_TASK_39(n, p, name, func, split, buf, s) \
  hls_thread_local hls::task name##39( \
    [&b = buf[39/(n/p)][39%(n/p)], &ps = split[39/(n/p)][39%(n/p)], &as = s##39]() \
    { func(ps, b, as); });
#define CALL_STREAMS_TASK_41(n, p, name, func, split, buf, s) \
        CALL_STREAMS_TASK_40(n, p, name, func, split, buf, s) \
  hls_thread_local hls::task name##40( \
    [&b = buf[40/(n/p)][40%(n/p)], &ps = split[40/(n/p)][40%(n/p)], &as = s##40]() \
    { func(ps, b, as); });
#define CALL_STREAMS_TASK_42(n, p, name, func, split, buf, s) \
        CALL_STREAMS_TASK_41(n, p, name, func, split, buf, s) \
  hls_thread_local hls::task name##41( \
    [&b = buf[41/(n/p)][41%(n/p)], &ps = split[41/(n/p)][41%(n/p)], &as = s##41]() \
    { func(ps, b, as); });
#define CALL_STREAMS_TASK_43(n, p, name, func, split, buf, s) \
        CALL_STREAMS_TASK_42(n, p, name, func, split, buf, s) \
  hls_thread_local hls::task name##42( \
    [&b = buf[42/(n/p)][42%(n/p)], &ps = split[42/(n/p)][42%(n/p)], &as = s##42]() \
    { func(ps, b, as); });
#define CALL_STREAMS_TASK_44(n, p, name, func, split, buf, s) \
        CALL_STREAMS_TASK_43(n, p, name, func, split, buf, s) \
  hls_thread_local hls::task name##43( \
    [&b = buf[43/(n/p)][43%(n/p)], &ps = split[43/(n/p)][43%(n/p)], &as = s##43]() \
    { func(ps, b, as); });
#define CALL_STREAMS_TASK_45(n, p, name, func, split, buf, s) \
        CALL_STREAMS_TASK_44(n, p, name, func, split, buf, s) \
  hls_thread_local hls::task name##44( \
    [&b = buf[44/(n/p)][44%(n/p)], &ps = split[44/(n/p)][44%(n/p)], &as = s##44]() \
    { func(ps, b, as); });
#define CALL_STREAMS_TASK_46(n, p, name, func, split, buf, s) \
        CALL_STREAMS_TASK_45(n, p, name, func, split, buf, s) \
  hls_thread_local hls::task name##45( \
    [&b = buf[45/(n/p)][45%(n/p)], &ps = split[45/(n/p)][45%(n/p)], &as = s##45]() \
    { func(ps, b, as); });
#define CALL_STREAMS_TASK_47(n, p, name, func, split, buf, s) \
        CALL_STREAMS_TASK_46(n, p, name, func, split, buf, s) \
  hls_thread_local hls::task name##46( \
    [&b = buf[46/(n/p)][46%(n/p)], &ps = split[46/(n/p)][46%(n/p)], &as = s##46]() \
    { func(ps, b, as); });
#define CALL_STREAMS_TASK_48(n, p, name, func, split, buf, s) \
        CALL_STREAMS_TASK_47(n, p, name, func, split, buf, s) \
  hls_thread_local hls::task name##47( \
    [&b = buf[47/(n/p)][47%(n/p)], &ps = split[47/(n/p)][47%(n/p)], &as = s##47]() \
    { func(ps, b, as); });
#define CALL_STREAMS_TASK_49(n, p, name, func, split, buf, s) \
        CALL_STREAMS_TASK_48(n, p, name, func, split, buf, s) \
  hls_thread_local hls::task name##48( \
    [&b = buf[48/(n/p)][48%(n/p)], &ps = split[48/(n/p)][48%(n/p)], &as = s##48]() \
    { func(ps, b, as); });
/* }}} */
#define CALL_STREAMS_TASK_50(n, p, name, func, split, buf, s) \
        CALL_STREAMS_TASK_49(n, p, name, func, split, buf, s) \
  hls_thread_local hls::task name##49( \
    [&b = buf[49/(n/p)][49%(n/p)], &ps = split[49/(n/p)][49%(n/p)], &as = s##49]() \
    { func(ps, b, as); });

#define CALL_PARTS1(p, func, arg0, arg1) CONCAT(CALL_PARTS1_, p)(func, arg0, arg1)
/* {{{ */
#define CALL_PARTS1_1(func, arg0, arg1) func(arg0##0, arg1[0]);
#define CALL_PARTS1_2(func, arg0, arg1) CALL_PARTS1_1(func, arg0, arg1) func(arg0##1, arg1[1]);
#define CALL_PARTS1_3(func, arg0, arg1) CALL_PARTS1_2(func, arg0, arg1) func(arg0##2, arg1[2]);
/* }}} */
#define CALL_PARTS1_4(func, arg0, arg1) CALL_PARTS1_3(func, arg0, arg1) func(arg0##3, arg1[3]);

#define CALL_PARTS2(p, func, arg0, arg1) CONCAT(CALL_PARTS2_, p)(func, arg0, arg1)
/* {{{ */
#define CALL_PARTS2_1(func, arg0, arg1) func(arg0[0], arg1[0]);
#define CALL_PARTS2_2(func, arg0, arg1) CALL_PARTS2_1(func, arg0, arg1) func(arg0[1], arg1[1]);
#define CALL_PARTS2_3(func, arg0, arg1) CALL_PARTS2_2(func, arg0, arg1) func(arg0[2], arg1[2]);
/* }}} */
#define CALL_PARTS2_4(func, arg0, arg1) CALL_PARTS2_3(func, arg0, arg1) func(arg0[3], arg1[3]);

#define CALL_PARTS3(p, func, arg0, arg1, arg2) CONCAT(CALL_PARTS3_, p)(func, arg0, arg1, arg2)
/* {{{ */
#define CALL_PARTS3_1(func, arg0, arg1, arg2) func(arg0[0], arg1[0], arg2);
#define CALL_PARTS3_2(func, arg0, arg1, arg2) CALL_PARTS3_1(func, arg0, arg1, arg2) func(arg0[1], arg1[1], arg2);
#define CALL_PARTS3_3(func, arg0, arg1, arg2) CALL_PARTS3_2(func, arg0, arg1, arg2) func(arg0[2], arg1[2], arg2);
/* }}} */
#define CALL_PARTS3_4(func, arg0, arg1, arg2) CALL_PARTS3_3(func, arg0, arg1, arg2) func(arg0[3], arg1[3], arg2);

template <int P>
void load_A(dram_t *pl, dram_stream_t &s)
{
  STDERR("load_A begin %p\n", pl);
  constexpr int ELEMS_PER_PART_TILE = PL_M * PL_K / P;

  loop0: for (int bm1=0; bm1<BM1; bm1++) {
    loop1: for (int bn1=0; bn1<BN1; bn1++) {
      loop2: for (int bk1=0; bk1<BK1; bk1++) {
        loop3: for (int i=0; i<ELEMS_PER_PART_TILE/DRAM_PACK; i++) {
          #pragma HLS pipeline II=1
          int idx = (bm1*BK1 + bk1)*ELEMS_PER_PART_TILE/DRAM_PACK + i;
          s.write(pl[idx]);
          //s.write(1);
          //STDERR("bm1=%d bn1=%d bk1=%d i=%d idx=%d pl=%s\n",
          //       bm1, bn1, bk1, i, idx, ap2str(pl[idx]).c_str());
        }
      }
    }
  }

  STDERR("load_A end %p\n", pl);
}

template <int P>
void load_B(dram_t *pl, dram_stream_t &s)
{
  STDERR("load_B begin %p\n", pl);
  constexpr int ELEMS_PER_PART_TILE = PL_K * PL_N / P;

  loop4: for (int bm1=0; bm1<BM1; bm1++) {
    loop5: for (int bn1=0; bn1<BN1; bn1++) {
      loop6: for (int bk1=0; bk1<BK1; bk1++) {
        loop7: for (int i=0; i<ELEMS_PER_PART_TILE/DRAM_PACK; i++) {
          #pragma HLS pipeline II=1
          int idx = (bn1*BK1 + bk1)*ELEMS_PER_PART_TILE/DRAM_PACK + i;
          s.write(pl[idx]);
          //s.write(1);
          //STDERR("bm1=%d bn1=%d bk1=%d i=%d idx=%d pl=%s\n",
          //       bm1, bn1, bk1, i, idx, ap2str(pl[idx]).c_str());
        }
      }
    }
  }

  STDERR("load_B end %p\n", pl);
}

template <int P>
void store_C(dram_t *pl, dram_stream_t &s)
{
  STDERR("store_C begin %p\n", pl);
  constexpr int ELEMS_PER_PART_TILE = PL_M * PL_N / P;

  loop8: for (int bm1=0; bm1<BM1; bm1++) {
    loop9: for (int bn1=0; bn1<BN1; bn1++) {
      loop10: for (int i=0; i<ELEMS_PER_PART_TILE/DRAM_PACK; i++) {
        #pragma HLS pipeline II=1
        int idx = (bm1*BN1 + bn1)*ELEMS_PER_PART_TILE/DRAM_PACK + i;
        pl[idx] = s.read();
        //s.read();
        //STDERR("bm1=%d bn1=%d i=%d idx=%d pl=%s\n",
        //       bm1, bn1, i, idx, ap2str(pl[idx]).c_str());
      }
    }
  }

  STDERR("store_C end %p\n", pl);
}

template <int P, int S, int PY, int PX, int Z>
void split_stream(dram_stream_t &ds,
                  plio_stream_t (&ps)[S/P][Z])
{
  STDERR("split_stream begin %p\n", &ps[0][0]);
  constexpr int ELEMS_PER_PART_TILE = PY * PX / P;

  loop11: for (int bm1=0; bm1<BM1; bm1++) {
    loop12: for (int bn1=0; bn1<BN1; bn1++) {
      loop13: for (int bk1=0; bk1<BK1; bk1++) {
        STDERR("split_stream bm1=%d bn1=%d bk1=%d\n", bm1, bn1, bk1);
        loop14: for (int i=0; i<ELEMS_PER_PART_TILE; i+=DRAM_PACK) {
          #pragma HLS pipeline II=1
          dram_t d = ds.read();

          int j = i / PLIO_PACK;
          loop15: for (int p=0; p<DRAM_PLIO_PACK; p++) {
            int s = (j+p) % (S / P);
            int z = p / (S / P);
            ps[s][z].write(d((p+1)*PLIO_WIDTH-1, p*PLIO_WIDTH));
            //STDERR("split_stream i=%d j=%d p=%d s=%d z=%d ps=%p data=%s\n",
            //       i, j, p, s, z, &ps[s][z], ap2str(plio_t{d((p+1)*PLIO_WIDTH-1, p*PLIO_WIDTH)}).c_str());
          }
        }
      }
    }
  }

  STDERR("split_stream end %p\n", &ps[0]);
}

template <int P, int S, int PY, int PX, int Z>
void merge_stream(dram_stream_t &ds,
                  plio_stream_t (&ps)[S/P][Z])
{
  STDERR("merge_stream begin %p\n", &ps[0][0]);
  constexpr int ELEMS_PER_PART_TILE = PY * PX / P;

  loop16: for (int bm1=0; bm1<BM1; bm1++) {
    loop17: for (int bn1=0; bn1<BN1; bn1++) {
      STDERR("merge_stream bm1=%d bn1=%d\n", bm1, bn1);
      loop18: for (int i=0; i<ELEMS_PER_PART_TILE; i+=DRAM_PACK) {
        #pragma HLS pipeline II=1
        dram_t d;

        int j = i / PLIO_PACK;
        loop19: for (int p=0; p<DRAM_PLIO_PACK; p++) {
          int s = (j+p) % (S / P);
          int z = p / (S / P);
          d((p+1)*PLIO_WIDTH-1, p*PLIO_WIDTH) = ps[s][z].read();
          //STDERR("merge_stream i=%d j=%d p=%d s=%d z=%d ps=%p data=%s\n",
          //       i, j, p, s, z, &ps[s][z], ap2str(plio_t{d((p+1)*PLIO_WIDTH-1, p*PLIO_WIDTH)}).c_str());
        }

        ds.write(d);
      }
    }
  }

  STDERR("merge_stream end %p\n", &ps[0]);
}

template <int S, int PY, int PX, int Z>
void load_buf(plio_stream_t (&split)[Z],
              plio_t (&buf)[PY*PX/S/PLIO_PACK],
              bool last)
{
  if (last) { return; }
  STDERR("load_buf begin %p\n", &buf[0]);

  loop20: for (int i=0; i<PY*PX/S/PLIO_PACK; i+=Z) {
    #pragma HLS pipeline II=1
    loop21: for (int z=0; z<Z; z++) {
      buf[i+z] = split[z].read();
      //STDERR("load_buf i=%d split=%p buf=%p data=%s\n", i, &split[z], &buf[i], ap2str(buf[i]).c_str());
    }
  }

  STDERR("load_buf end %p\n", &buf[0]);
}

//void load_par(plio_stream_t (&split_A)[PARTS][AIE_ROWS/PARTS],
// /* {{{ */
//              plio_stream_t (&split_B)[PARTS][AIE_COLS/PARTS],
//              plio_t (&buf_A)[PARTS][AIE_ROWS/PARTS][PL_M*PL_K/AIE_ROWS/PLIO_PACK],
//              plio_t (&buf_B)[PARTS][AIE_COLS/PARTS][PL_K*PL_N/AIE_COLS/PLIO_PACK],
//              bool last)
//{
//  #pragma HLS dataflow
//
//  CALL_PARTS3(DEF_PARTS, (load_buf<PARTS, AIE_ROWS, PL_M, PL_K>), split_A, buf_A, last);
//  CALL_PARTS3(DEF_PARTS, (load_buf<PARTS, AIE_COLS, PL_K, PL_N>), split_B, buf_B, last);
//
////  SIM_ONLY(
////    if (!last) {
////      STDERR("--------------------------------------------------------------------------------\n");
////      STDERR("buf_A: %p\n", &buf_A[0][0]);
////      for (int r=0; r<AIE_ROWS; r++) {
////        for (int i=0; i<PL_M*PL_K/AIE_ROWS/PLIO_PACK; i++) {
////          STDERR("%s ", ap2str(buf_A[r][i]).c_str());
////          if (i % PL_K == PL_K-1) { STDERR("\n"); }
////        }
////      }
////      STDERR("--------------------------------------------------------------------------------\n");
////    }
////  );
//
////  SIM_ONLY(
////    if (!last) {
////      STDERR("--------------------------------------------------------------------------------\n");
////      STDERR("buf_B: %p\n", &buf_B[0][0]);
////      for (int c=0; c<AIE_COLS; c++) {
////        for (int i=0; i<PL_K*PL_N/AIE_COLS/PLIO_PACK; i++) {
////          STDERR("%s ", ap2str(buf_B[c][i]).c_str());
////          if (i % PL_N == PL_N-1) { STDERR("\n"); }
////        }
////      }
////      STDERR("--------------------------------------------------------------------------------\n");
////    }
////  );
//
//}
// /* }}} */

void send_buf_A(plio_t (&buf)[PL_M*PL_K/AIE_ROWS/PLIO_PACK], axis_stream_t &s)
{
  STDERR("send_buf_A begin %p\n", &buf[0]);

  loop22: for (int bm2=0; bm2<BM2; bm2++) {
    loop23: for (int bn2=0; bn2<BN2; bn2++) {
      loop24: for (int bk2=0; bk2<BK2; bk2++) {
        int idx = (bm2*BK2 + bk2)*AIE_M*AIE_K/PLIO_PACK;
        loop25: for (int i=0; i<AIE_M*AIE_K/PLIO_PACK; i++) {
          #pragma HLS pipeline II=1
          axis_t d;

          d.data = buf[idx++];
          s.write(d);

          //STDERR("send_buf_A p=%p bm2=%d bn2=%d bk2=%d i=%d idx=%d buf=%s\n",
          //       &buf[0], bm2, bn2, bk2, i, idx, ap2str(buf[idx]).c_str());
        }
      }
    }
  }

  STDERR("send_buf_A end %p\n", &buf[0]);
}

void send_buf_B(plio_t (&buf)[PL_K*PL_N/AIE_COLS/PLIO_PACK], axis_stream_t &s)
{
  STDERR("send_buf_B begin %p\n", &buf[0]);

  loop26: for (int bm2=0; bm2<BM2; bm2++) {
    loop27: for (int bn2=0; bn2<BN2; bn2++) {
      loop28: for (int bk2=0; bk2<BK2; bk2++) {
        int idx = (bn2*BK2 + bk2)*AIE_K*AIE_N/PLIO_PACK;
        loop29: for (int i=0; i<AIE_K*AIE_N/PLIO_PACK; i++) {
          #pragma HLS pipeline II=1
          axis_t d;

          d.data = buf[idx++];
          s.write(d);

          //STDERR("send_buf_B p=%p bm2=%d bn2=%d bk2=%d i=%d idx=%d buf=%s\n",
          //       &buf[0], bm2, bn2, bk2, i, idx, ap2str(buf[idx]).c_str());
        }
      }
    }
  }

  STDERR("send_buf_B end %p\n", &buf[0]);
}

//void send_iter(dram_stream_t (&stream_A)[PARTS],
// /* {{{ */
//               dram_stream_t (&stream_B)[PARTS],
//               plio_t (&buf_A_0)[PARTS][AIE_ROWS/PARTS][PL_M*PL_K/AIE_ROWS/PLIO_PACK],
//               plio_t (&buf_B_0)[PARTS][AIE_COLS/PARTS][PL_K*PL_N/AIE_COLS/PLIO_PACK],
//               plio_t (&buf_A_1)[PARTS][AIE_ROWS/PARTS][PL_M*PL_K/AIE_ROWS/PLIO_PACK],
//               plio_t (&buf_B_1)[PARTS][AIE_COLS/PARTS][PL_K*PL_N/AIE_COLS/PLIO_PACK],
//               ARGS(DEF_AIE_ROWS, axis_stream_t &, aie_in0_),
//               ARGS(DEF_AIE_COLS, axis_stream_t &, aie_in1_),
//               bool last)
//{
//  #pragma HLS dataflow
//
//  CALL_BUF(DEF_AIE_ROWS, DEF_PARTS, send_buf_A, buf_A_0, aie_in0_);
//  CALL_BUF(DEF_AIE_COLS, DEF_PARTS, send_buf_B, buf_B_0, aie_in1_);
//  load_par(stream_A, stream_B, buf_A_1, buf_B_1, last);
//}
// /* }}} */

template <int S, int PY, int PX, int Z>
void send_inner_A(plio_stream_t (&split)[Z],
                  plio_t (&buf_0)[PY*PX/S/PLIO_PACK],
                  plio_t (&buf_1)[PY*PX/S/PLIO_PACK],
                  axis_stream_t &s,
                  bool last)
{
  #pragma HLS dataflow
  send_buf_A(buf_0, s);
  load_buf<S, PY, PX>(split, buf_1, last);
}

template <int S, int PY, int PX, int Z>
void send_A(plio_stream_t (&split)[Z],
            plio_t (&buf)[2][PY*PX/S/PLIO_PACK],
            axis_stream_t &s)
{
  STDERR("send_A begin s=%p\n", &s);

  load_buf<S, PY, PX>(split, buf[0], false);

  int iter=0;
  loop30: for (int bm1=0; bm1<BM1; bm1++) {
    loop31: for (int bn1=0; bn1<BN1; bn1++) {
      loop32: for (int bk1=0; bk1<BK1; bk1++) {
        STDERR("send_A bm1=%d bn1=%d bk1=%d\n", bm1, bn1, bk1);
        bool last { bm1 == BM1-1 && bn1 == BN1-1 && bk1 == BK1-1 };
        if (iter == 0) {
          send_inner_A<S, PY, PX>(split, buf[0], buf[1], s, last);
        } else {
          send_inner_A<S, PY, PX>(split, buf[1], buf[0], s, last);
        }
        iter = !iter;
      }
    }
  }

  STDERR("send_A end s=%p\n", &s);
}

template <int S, int PY, int PX, int Z>
void send_inner_B(plio_stream_t (&split)[Z],
                  plio_t (&buf_0)[PY*PX/S/PLIO_PACK],
                  plio_t (&buf_1)[PY*PX/S/PLIO_PACK],
                  axis_stream_t &s,
                  bool last)
{
  #pragma HLS dataflow
  send_buf_B(buf_0, s);
  load_buf<S, PY, PX>(split, buf_1, last);
}

template <int S, int PY, int PX, int Z>
void send_B(plio_stream_t (&split)[Z],
            plio_t (&buf)[2][PY*PX/S/PLIO_PACK],
            axis_stream_t &s)
{
  STDERR("send_B begin s=%p\n", &s);

  load_buf<S, PY, PX>(split, buf[0], false);

  int iter=0;
  loop33: for (int bm1=0; bm1<BM1; bm1++) {
    loop34: for (int bn1=0; bn1<BN1; bn1++) {
      loop35: for (int bk1=0; bk1<BK1; bk1++) {
        STDERR("send_B bm1=%d bn1=%d bk1=%d\n", bm1, bn1, bk1);
        bool last { bm1 == BM1-1 && bn1 == BN1-1 && bk1 == BK1-1 };
        if (iter == 0) {
          send_inner_B<S, PY, PX>(split, buf[0], buf[1], s, last);
        } else {
          send_inner_B<S, PY, PX>(split, buf[1], buf[0], s, last);
        }
        iter = !iter;
      }
    }
  }

  STDERR("send_B end s=%p\n", &s);
}

void recv_buf(plio_t (&buf)[PL_M*PL_N/AIE_COLS/PLIO_PACK],
              axis_stream_t &s)
{
  //STDERR("recv_buf begin %p\n", &buf[0]);

  loop36: for (int bk1=0; bk1<BK1; bk1++) {
    loop37: for (int bm2=0; bm2<BM2; bm2++) {
      loop38: for (int bn2=0; bn2<BN2; bn2++) {
        int idx = (bm2*BN2 + bn2)*AIE_M*AIE_N*AIE_ROWS/PLIO_PACK;
        loop39: for (int i=0; i<AIE_M*AIE_N*AIE_ROWS/PLIO_PACK; i++) {
          #pragma HLS pipeline II=1
          #pragma HLS dependence variable=buf type=inter false
          axis_t d = s.read();

          if (bk1 > 0) {
            loop40: for (int p=0; p<PLIO_PACK; p++) {
              union_t u1; u1.uint = d.data((p+1)*DATA_WIDTH-1, p*DATA_WIDTH).to_uint();
              union_t u2; u2.uint = buf[idx]((p+1)*DATA_WIDTH-1, p*DATA_WIDTH).to_uint();
              u1.val += u2.val;
              d.data((p+1)*DATA_WIDTH-1, p*DATA_WIDTH) = u1.uint;
            }
          }
          buf[idx++] = d.data;
          //STDERR("recv_buf bk1=%d bm2=%d bn2=%d i=%d idx=%d buf=%p data=%s\n",
          //       bk1, bm2, bn2, i, idx-1, &buf[idx-1], ap2str(buf[idx-1]).c_str());
        }
      }
    }
  }

  //STDERR("recv_buf end %p\n", &buf[0]);
}

template <int S, int PY, int PX, int Z>
void store_buf(plio_stream_t (&split)[Z], 
               plio_t (&buf)[PY*PX/S/PLIO_PACK],
               bool first)
{
  if (first) { return; }
  //STDERR("store_buf begin %p\n", &buf[0]);

  loop41: for (int i=0; i<PY*PX/S/PLIO_PACK; i+=Z) {
    #pragma HLS pipeline II=1
    loop42: for (int z=0; z<Z; z++) {
      split[z].write(buf[i+z]);
    }
    //STDERR("store_buf i=%d split=%p buf=%p data=%s\n", i, &split, &buf[i], ap2str(buf[i]).c_str());
  }

  //STDERR("store_buf end %p\n", &buf[0]);
}

//void store_par(dram_stream_t (&stream_C)[PARTS],
// /* {{{ */
//               plio_t (&buf_C)[PARTS][AIE_COLS/PARTS][PL_M*PL_N/AIE_COLS/PLIO_PACK],
//               bool first)
//{
//  #pragma HLS dataflow
//
//  CALL_PARTS3(DEF_PARTS, (store_buf<PARTS, AIE_COLS, PL_M, PL_N>), stream_C, buf_C, first);
//
//  //SIM_ONLY(
//  //  if (!first) {
//  //    STDERR("--------------------------------------------------------------------------------\n");
//  //    for (int p=0; p<PARTS; p++) {
//  //      STDERR("buf_C[%d]: %p\n", p, &buf_C[0][0]);
//  //      for (int c=0; c<AIE_COLS/PARTS; c++) {
//  //        for (int i=0; i<PL_M*PL_N/AIE_COLS/PLIO_PACK; i++) {
//  //          STDERR("%s ", ap2str(buf_C[p][c][i]).c_str());
//  //          if (i % PL_N == PL_N-1) { STDERR("\n"); }
//  //        }
//  //      }
//  //    }
//  //    STDERR("--------------------------------------------------------------------------------\n");
//  //  }
//  //);
//
//}
// /* }}} */

//void recv_iter(dram_stream_t (&stream_C)[PARTS],
// /* {{{ */
//               plio_t (&buf_C_0)[PARTS][AIE_COLS/PARTS][PL_M*PL_N/AIE_COLS/PLIO_PACK],
//               plio_t (&buf_C_1)[PARTS][AIE_COLS/PARTS][PL_M*PL_N/AIE_COLS/PLIO_PACK],
//               ARGS(DEF_AIE_COLS, axis_stream_t &, aie_out0_),
//               bool first)
//{
//  #pragma HLS dataflow
//
//  CALL_BUF(DEF_AIE_COLS, DEF_PARTS, recv_buf, buf_C_0, aie_out0_);
//  store_par(stream_C, buf_C_1, first);
//}
// /* }}} */

template <int S, int PY, int PX, int Z>
void recv_inner_C(plio_stream_t (&split)[Z],
                  plio_t (&buf_0)[PY*PX/S/PLIO_PACK],
                  plio_t (&buf_1)[PY*PX/S/PLIO_PACK],
                  axis_stream_t &s,
                  bool first)
{
  #pragma HLS dataflow
  recv_buf(buf_0, s);
  store_buf<S, PY, PX>(split, buf_1, first);
}

template <int S, int PY, int PX, int Z>
void recv_C(plio_stream_t (&split)[Z],
            plio_t (&buf)[2][PY*PX/S/PLIO_PACK],
            axis_stream_t &s)
{
  STDERR("recv_C begin\n");

  int iter=0;
  loop43: for (int bm1=0; bm1<BM1; bm1++) {
    loop44: for (int bn1=0; bn1<BN1; bn1++) {
      STDERR("recv_C bm1=%d bn1=%d\n", bm1, bn1);
      bool first { bm1 == 0 && bn1 == 0 };
      if (iter == 0) {
        recv_inner_C<S, PY, PX>(split, buf[0], buf[1], s, first);
      } else {
        recv_inner_C<S, PY, PX>(split, buf[1], buf[0], s, first);
      }
      iter = !iter;
    }
  }

  if (iter == 0) {
    store_buf<S, PY, PX>(split, buf[1], false);
  } else {
    store_buf<S, PY, PX>(split, buf[0], false);
  }

  STDERR("recv_C end\n");
}

extern "C" 
void dma(ARGS(DEF_PARTS, dram_t *, pl_in0_),
         ARGS(DEF_PARTS, dram_t *, pl_in1_),
         ARGS(DEF_PARTS, dram_t *, pl_out0_),
         ARGS(DEF_AIE_ROWS, axis_stream_t &, aie_in0_),
         ARGS(DEF_AIE_COLS, axis_stream_t &, aie_in1_),
         ARGS(DEF_AIE_COLS, axis_stream_t &, aie_out0_))
{
  #pragma HLS dataflow

  static plio_t buf_A[PARTS][AIE_ROWS/PARTS][2][PL_M*PL_K/AIE_ROWS/PLIO_PACK];
  #pragma HLS bind_storage variable=buf_A type=ram_t2p impl=uram
  #pragma HLS array_partition variable=buf_A dim=1 type=complete
  #pragma HLS array_partition variable=buf_A dim=2 type=complete
  #pragma HLS array_partition variable=buf_A dim=3 type=complete
  #pragma HLS array_partition variable=buf_A dim=4 type=cyclic \
                              factor=std::max(1, DRAM_PLIO_PACK/(AIE_ROWS/PARTS)/2)

  static plio_t buf_B[PARTS][AIE_COLS/PARTS][2][PL_K*PL_N/AIE_COLS/PLIO_PACK];
  #pragma HLS bind_storage variable=buf_B type=ram_t2p impl=bram
  #pragma HLS array_partition variable=buf_B dim=1 type=complete
  #pragma HLS array_partition variable=buf_B dim=2 type=complete
  #pragma HLS array_partition variable=buf_B dim=3 type=complete
  #pragma HLS array_partition variable=buf_B dim=4 type=cyclic \
                              factor=std::max(1, DRAM_PLIO_PACK/(AIE_COLS/PARTS)/2)

  static plio_t buf_C[PARTS][AIE_COLS/PARTS][2][PL_M*PL_N/AIE_COLS/PLIO_PACK];
  #pragma HLS bind_storage variable=buf_C type=ram_t2p impl=uram
  #pragma HLS array_partition variable=buf_C dim=1 type=complete
  #pragma HLS array_partition variable=buf_C dim=2 type=complete
  #pragma HLS array_partition variable=buf_C dim=3 type=complete
  #pragma HLS array_partition variable=buf_C dim=4 type=cyclic \
                              factor=std::max(1, DRAM_PLIO_PACK/(AIE_COLS/PARTS)/2)

  dram_stream_t stream_A[PARTS];
  dram_stream_t stream_B[PARTS];
  dram_stream_t stream_C[PARTS];
  hls_thread_local plio_stream_t split_A[PARTS][AIE_ROWS/PARTS][PACK_PER_ROW_STREAM];
  hls_thread_local plio_stream_t split_B[PARTS][AIE_COLS/PARTS][PACK_PER_COL_STREAM];
  hls_thread_local plio_stream_t split_C[PARTS][AIE_COLS/PARTS][PACK_PER_COL_STREAM];

  CALL_PARTS1(DEF_PARTS, (load_A<PARTS>), pl_in0_, stream_A);
  CALL_PARTS1(DEF_PARTS, (load_B<PARTS>), pl_in1_, stream_B);

  CALL_PARTS2(DEF_PARTS, (split_stream<PARTS, AIE_ROWS, PL_M, PL_K>), stream_A, split_A);
  CALL_PARTS2(DEF_PARTS, (split_stream<PARTS, AIE_COLS, PL_K, PL_N>), stream_B, split_B);

#ifdef XILINX_TARGET_IS_HW
  CALL_STREAMS(DEF_AIE_ROWS, PARTS, 
      (send_A<AIE_ROWS, PL_M, PL_K>), split_A, buf_A, aie_in0_);
  CALL_STREAMS(DEF_AIE_COLS, PARTS, 
      (send_B<AIE_COLS, PL_K, PL_N>), split_B, buf_B, aie_in1_);
  CALL_STREAMS(DEF_AIE_COLS, PARTS, 
      (recv_C<AIE_COLS, PL_M, PL_N>), split_C, buf_C, aie_out0_);
#else
  CALL_STREAMS_TASK(DEF_AIE_ROWS, PARTS, task_send_A,
      (send_A<AIE_ROWS, PL_M, PL_K>), split_A, buf_A, aie_in0_);
  CALL_STREAMS_TASK(DEF_AIE_COLS, PARTS, task_send_B,
      (send_B<AIE_COLS, PL_K, PL_N>), split_B, buf_B, aie_in1_);
  CALL_STREAMS_TASK(DEF_AIE_COLS, PARTS, task_recv_C,
      (recv_C<AIE_COLS, PL_M, PL_N>), split_C, buf_C, aie_out0_);
#endif

  CALL_PARTS2(DEF_PARTS, (merge_stream<PARTS, AIE_COLS, PL_M, PL_N>), stream_C, split_C);

  CALL_PARTS1(DEF_PARTS, store_C<PARTS>, pl_out0_, stream_C);

}
