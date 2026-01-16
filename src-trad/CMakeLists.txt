vitis_flow(NAME gemm-trad
  ADF_SOURCE  graph.cc
  ADF_HEADERS graph.hh

  AIE_SOURCES gemm.cc
  AIE_HEADERS gemm.hh parameters.hh

  STACK_BYTES 2048
  HEAP_BYTES 2048

  DATA_DIR data
  DATA_GLOB out0*

  PL_SOURCE dma.cc
  PL_HEADERS parameters.hh
  PL_FREQ_MHZ 1250/4
  #PL_SIM_NAME dma_1
  XSA_CONFIG xsa.cfg
  #PLACE_DESIGN_PRE place_design_pre.tcl

  HOST_SOURCE host.cc
)

