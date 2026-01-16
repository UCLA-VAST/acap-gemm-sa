#include "graph.hh"

GemmGraph<DT, AIE_ROWS, AIE_COLS, AIE_M, AIE_K, AIE_N, AIE_MM, AIE_KK, AIE_NN> graph;

#if defined(__AIESIM__) || defined(__X86SIM__)
int main(int argc, char *argv[]) {
  graph.init();

  auto ret = graph.run(ADF_ITERS);
  if (ret != adf::ok) {
    printf("run failed\n");
    return ret;
  }

  ret = graph.end();
  if (ret != adf::ok) {
    printf("end failed\n");
    return ret;
  }

  return 0;
}
#endif
