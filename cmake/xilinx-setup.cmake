################################
# Cache variables
################################

set(XILINX_VITIS "" CACHE STRING "Path to Vitis install")
set(XILINX_TARGET "hw" CACHE STRING "Xilinx target")
set(XILINX_PLATFORM "xilinx_vck5000_gen4x8_qdma_2_202220_1" CACHE STRING "Xilinx platform")
set(XILINX_PART "" CACHE STRING "Manually set corresponding platform part")

set(VPP_AIE_XLOPT "2" CACHE STRING "Set --aie.xlopt=0-2")
set(VPP_AIE_VERBOSE OFF CACHE BOOL "Set --aie.verbose")
set(VPP_AIE_SCHEDULE OFF CACHE BOOL "Generate aie scheduling report")

set(AIESIM_PROFILE ON CACHE BOOL "Set aiesimulator --profile")
set(AIESIM_DUMP_VCD "" CACHE STRING "Set aiesimulator --dump-vcd")

set(VPP_OPTIMIZE "" CACHE STRING "Set --optimize=0-3,s,quick")
set(VPP_REPORT_LEVEL "" CACHE STRING "Set --report_level=0-2,estimate")
set(VPP_SAVE_TEMPS ON CACHE BOOL "Set --save-temps")
set(VPP_DEBUG OFF CACHE BOOL "Set --debug")
set(VPP_PROFILE "" CACHE STRING 
  "v++ profiling for (all|OPT...) where OPT=data, memory, stall, exec, aie")
set(VPP_PROFILE_FLAGS "" CACHE STRING "Manually set --profile flags")

set(VPP_JOBS "1" CACHE STRING "Set minimum jobs for each step")
set(VPP_HLS_JOBS "1" CACHE STRING "Set --hls.jobs=N")
set(VPP_AIE_JOBS "1" CACHE STRING "Set --aie.Xelfgen=-jN")
set(VPP_VIVADO_IMPL_JOBS "1" CACHE STRING "Set --vivado.impl.jobs=N")
set(VPP_VIVADO_SYNTH_JOBS "1" CACHE STRING "Set --vivado.synth.jobs=N")

set(XRT_USER "" CACHE STRING "Path to user-provided xrt.ini")

################################
# Apply configuration
################################

if(NOT XILINX_VITIS)
  set(XILINX_VITIS "$ENV{XILINX_VITIS}" CACHE STRING "Forced by empty XILINX_VITIS" FORCE)
  if(NOT EXISTS "${XILINX_VITIS}")
    message(FATAL_ERROR "directory does not exist: XILINX_VITIS=${XILINX_VITIS}")
  endif()
endif()

string(REGEX MATCH "[0-9]+\\.[0-9]+" XILINX_VERSION "${XILINX_VITIS}")
if(XILINX_VERSION VERSION_LESS "2022.2")
  message(FATAL_ERROR "XILINX_VERSION=${XILINX_VERSION} does not support Unified API (<2022.2)")
endif()

find_program(VITIS_VPP    v++          NO_DEFAULT_PATH HINTS "${XILINX_VITIS}/bin" REQUIRED)
find_program(EMCONFIGUTIL emconfigutil NO_DEFAULT_PATH HINTS "${XILINX_VITIS}/bin" REQUIRED)
find_program(X86SIMULATOR x86simulator NO_DEFAULT_PATH HINTS "${XILINX_VITIS}/aietools/bin" REQUIRED)
find_program(AIESIMULATOR aiesimulator NO_DEFAULT_PATH HINTS "${XILINX_VITIS}/aietools/bin" REQUIRED)

find_program(PLATFORMINFO platforminfo NO_DEFAULT_PATH HINTS "${XILINX_VITIS}/bin" REQUIRED)

if(NOT XILINX_PART)
  execute_process(
    COMMAND ${PLATFORMINFO} ${XILINX_PLATFORM} 
      --json=hardwarePlatforms.reconfigurablePartitions[0].hardwarePlatform.board.part
    OUTPUT_VARIABLE XILINX_PART
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_VARIABLE _err
  )
  if(_err)
    message(FATAL_ERROR "could not automatically set XILINX_PART: ${_err}")
  endif()
  unset(_err)
endif()

message(STATUS "Found Vitis: ${XILINX_VITIS}")
message(STATUS "  Target: ${XILINX_TARGET}")
message(STATUS "  Platform: ${XILINX_PLATFORM}")
message(STATUS "  Part: ${XILINX_PART}")

find_package(XRT REQUIRED)
find_package(Python3 REQUIRED COMPONENTS Interpreter)

# XILINX_*
if(NOT "${XILINX_TARGET}" MATCHES "sw_emu|hw_emu|hw")
  message(FATAL_ERROR "expected XILINX_TARGET=sw_emu|hw_emu|hw, got: ${XILINX_TARGET}")
endif()

if("${XILINX_TARGET}" STREQUAL "hw_emu")
  set(VPP_DEBUG ON CACHE BOOL "Forced by XILINX_TARGET=${XILINX_TARGET}" FORCE)
  message(NOTICE "!! VPP_DEBUG=${VPP_DEBUG} forced by XILINX_TARGET=${XILINX_TARGET}")
endif()

define_property(TARGET
  PROPERTY XCLBIN
  BRIEF_DOCS "Path to XCLBIN"
  FULL_DOCS  "Path to XCLBIN"
)

# VPP_*
if(VPP_PROFILE OR VPP_PROFILE_FLAGS)
  set(VPP_DEBUG ON CACHE BOOL "Forced by VPP_PROFILE*" FORCE)
  message(NOTICE "!! VPP_DEBUG=${VPP_DEBUG} forced by VPP_PROFILE*")
endif()

if("${VPP_PROFILE}" STREQUAL "all")
  set(_opts data memory stall exec aie)
else()
  string(REPLACE "," ";" _opts "${VPP_PROFILE}")
endif()

foreach(_opt ${_opts})
  if(_opt MATCHES data)
    list(APPEND VPP_PROFILE_FLAGS "--profile.${_opt}=all:all:all")
  elseif(_opt MATCHES stall|exec)
    list(APPEND VPP_PROFILE_FLAGS "--profile.${_opt}=all:all")
  elseif(_opt MATCHES memory|aie)
    list(APPEND VPP_PROFILE_FLAGS "--profile.${_opt}=all")
  else()
    message(FATAL_ERROR "unrecognized profile option: ${_opt}")
  endif()
endforeach()
unset(_opts)

foreach(_job AIE HLS VIVADO_IMPL VIVADO_SYNTH)
  if(VPP_JOBS GREATER VPP_${_job}_JOBS)
    set(VPP_${_job}_JOBS ${VPP_JOBS} CACHE STRING "Forced by VPP_JOBS=${VPP_JOBS}" FORCE)
    message(NOTICE "!! VPP_${_job}_JOBS=${VPP_${_job}_JOBS} forced by VPP_JOBS=${VPP_JOBS}")
  endif()
endforeach()
unset(_job)

# XRT_*
if(XRT_USER)
  file(COPY "${XRT_USER}"
       DESTINATION "${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}")
elseif("${XILINX_TARGET}" MATCHES "hw_emu|hw")
  file(COPY "${PROJECT_SOURCE_DIR}/cmake/xrt.ini"
       DESTINATION "${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}")
endif()

# Rule for emconfig.json
add_custom_command(OUTPUT "${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}/emconfig.json" VERBATIM
  COMMAND ${EMCONFIGUTIL} --platform=${XILINX_PLATFORM}
                          --od=${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}
)
add_custom_target(emconfig-json
  DEPENDS "${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}/emconfig.json"
)

################################
# Functions
################################

function(vitis_flow)
  set(one_args         NAME PL_SOURCE HOST_SOURCE)
  set(one_opt_args     XSA_CONFIG PL_SIM_NAME PLACE_DESIGN_PRE)
  set(one_aie_args     ADF_SOURCE)
  set(one_aie_opt_args DATA_DIR DATA_GLOB STACK_BYTES HEAP_BYTES)
  set(one_value_args ${one_args} ${one_opt_args} ${one_aie_args} ${one_aie_opt_args})

  set(multi_args)
  set(multi_opt_args     PL_HEADERS PL_FREQ_MHZ)
  set(multi_aie_args     AIE_SOURCES)
  set(multi_aie_opt_args AIE_HEADERS ADF_HEADERS)
  set(multi_value_args ${multi_args} ${multi_opt_args} ${multi_aie_args} ${multi_aie_opt_args})

  cmake_parse_arguments(ARGS "" "${one_value_args}" "${multi_value_args}" ${ARGN})

  foreach(_arg ${one_args} ${multi_args})
    if(NOT DEFINED ARGS_${_arg})
      message(FATAL_ERROR "vitis_flow: missing required argument: ${_arg}")
    endif()
  endforeach()

  set(aie_mode OFF)
  foreach(_arg ${one_aie_args} ${multi_aie_args})
    if(DEFINED ARGS_${_arg})
      set(aie_mode ON)
      list(APPEND one_aie_args XSA_CONFIG)
      break()
    endif()
  endforeach()

  if(aie_mode)
    foreach(_arg ${one_aie_args} ${multi_aie_args})
      if(NOT DEFINED ARGS_${_arg})
        message(FATAL_ERROR "vitis_flow: missing required aie argument: ${_arg}")
      endif()
    endforeach()
  else()
    foreach(_arg ${one_aie_opt_args} ${multi_aie_opt_args})
      if(DEFINED ARGS_${_arg})
        message(WARNING "vitis_flow: aie disabled, ignoring aie argument: ${_arg}")
      endif()
    endforeach()
  endif()

  foreach(_arg ADF_SOURCE ADF_HEADERS AIE_SOURCES AIE_HEADERS
               PL_SOURCE  PL_HEADERS  XSA_CONFIG  HOST_SOURCE)
    if(DEFINED ARGS_${_arg})
      list(TRANSFORM ARGS_${_arg} PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
    endif()
  endforeach()

  if(aie_mode)
    set(_libadf_x86 "libadf.${ARGS_NAME}.x86sim.${XILINX_PLATFORM}.a")
    set(_libadf_hw  "libadf.${ARGS_NAME}.hw.${XILINX_PLATFORM}.a")
    if("${XILINX_TARGET}" STREQUAL "sw_emu")
      set(_libadf "${_libadf_x86}")
    else()
      set(_libadf "${_libadf_hw}")
    endif()
  else()
    unset(_libadf)
  endif()

  set(_work_aie_x86   "work.aie.x86sim")
  set(_work_aie_hw    "work.aie.hw")
  set(_work_hls       "work.hls")
  set(_work_link      "work.link")
  set(_work_package   "work.package")
  set(_hls_cfg        "hls.cfg")
  set(_pre_sim_tcl    "pre_sim.tcl")

  get_filename_component(_pl_name "${ARGS_PL_SOURCE}" NAME_WLE)
  set(_qualifier "${XILINX_TARGET}.${XILINX_PLATFORM}")
  set(_xo        "${ARGS_NAME}.${_qualifier}.xo")
  set(_xsa       "${ARGS_NAME}.${_qualifier}.xsa")
  set(_xclbin    "${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}/${ARGS_NAME}.${_qualifier}.xclbin")

  if(DEFINED ARGS_PL_SIM_NAME)
    set(_pl_sim_name "${ARGS_PL_SIM_NAME}")
  else()
    set(_pl_sim_name "${_pl_name}")
  endif()

  if(DEFINED ARGS_PL_FREQ_MHZ)
    execute_process(
      COMMAND "${Python3_EXECUTABLE}" -c "print(${ARGS_PL_FREQ_MHZ})"
      OUTPUT_VARIABLE _freq_mhz
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
      COMMAND "${Python3_EXECUTABLE}" -c "print(int(${ARGS_PL_FREQ_MHZ}*1e6))"
      OUTPUT_VARIABLE _freq_hz
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(_freq_hz_qual          "${_freq_hz}:${_pl_name}")
    set(_aie_pl_freq           "--aie.pl-freq=${_freq_mhz}")
    #set(_hls_clock             "--hls.clock=${_freq_hz_qual}")
    #set(_hls_clock             "--hls.clock=${_freq_mhz}MHz")
    set(_hls_cfg_clock         "clock=${_freq_mhz}MHz")
    set(_clock_freq_hz         "--clock.freqHz=${_freq_hz_qual}")
    set(_clock_default_freq_hz "--clock.defaultFreqHz=${_freq_hz}")
    set(_kernel_freq           "--kernel_frequency=${_freq_mhz}")
  endif()

  if(DEFINED ARGS_STACK_BYTES)
    math(EXPR ARGS_STACK_BYTES "${ARGS_STACK_BYTES}")
  endif()

  if(DEFINED ARGS_HEAP_BYTES)
    math(EXPR ARGS_HEAP_BYTES "${ARGS_HEAP_BYTES}")
  endif()

  if(DEFINED ARGS_PLACE_DESIGN_PRE)
    get_filename_component(_abspath "${ARGS_PLACE_DESIGN_PRE}" ABSOLUTE)
    set(_place_design_pre "--vivado.prop=run.impl_1.STEPS.PLACE_DESIGN.TCL.PRE={${_abspath}}")
  endif()

  if(aie_mode)
    add_custom_command(OUTPUT "${_libadf_x86}" VERBATIM COMMAND_EXPAND_LISTS
      COMMAND ${VITIS_VPP} --compile --mode aie --work_dir=${_work_aie_x86}
        --target=x86sim --platform=${XILINX_PLATFORM}
        --include=${CMAKE_CURRENT_SOURCE_DIR}
        $<IF:$<BOOL:${VPP_AIE_VERBOSE}>,--aie.verbose,>
        $<IF:$<BOOL:${VPP_AIE_SCHEDULE}>,--aie.Xchess=main:backend.mist2.xargs=-ggraph,>
        $<IF:$<BOOL:${ARGS_STACK_BYTES}>,--aie.stacksize=${ARGS_STACK_BYTES},>
        $<IF:$<BOOL:${ARGS_HEAP_BYTES}>,--aie.heapsize=${ARGS_HEAP_BYTES},>
        --aie.xlopt=${VPP_AIE_XLOPT} 
        --aie.Xelfgen=-j${VPP_AIE_JOBS}
        ${_aie_pl_freq}
        ${ARGS_ADF_SOURCE}
        --output=libadf.a # v2022.2 ignores --output; explicitly set default for others
      COMMAND ${CMAKE_COMMAND} -E rename libadf.a "${_libadf_x86}"
      DEPENDS "${ARGS_ADF_SOURCE}" ${ARGS_AIE_SOURCES} 
               ${ARGS_ADF_HEADERS} ${ARGS_AIE_HEADERS}
      BYPRODUCTS "${_work_aie_x86}"
      WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    )

    add_custom_command(OUTPUT "${_libadf_hw}" VERBATIM COMMAND_EXPAND_LISTS
      COMMAND ${VITIS_VPP} --compile --mode aie --work_dir=${_work_aie_hw}
        --target=hw --platform=${XILINX_PLATFORM}
        --include=${CMAKE_CURRENT_SOURCE_DIR}
        $<IF:$<BOOL:${VPP_AIE_VERBOSE}>,--aie.verbose,>
        $<IF:$<BOOL:${VPP_AIE_SCHEDULE}>,--aie.Xchess=main:backend.mist2.xargs=-ggraph,>
        $<IF:$<BOOL:${ARGS_STACK_BYTES}>,--aie.stacksize=${ARGS_STACK_BYTES},>
        $<IF:$<BOOL:${ARGS_HEAP_BYTES}>,--aie.heapsize=${ARGS_HEAP_BYTES},>
        --aie.xlopt=${VPP_AIE_XLOPT} 
        --aie.Xelfgen=-j${VPP_AIE_JOBS}
        ${_aie_pl_freq}
        ${ARGS_ADF_SOURCE}
        --output=libadf.a # v2022.2 ignores --output; explicitly set default for others
      COMMAND ${CMAKE_COMMAND} -E rename libadf.a "${_libadf_hw}"
      DEPENDS "${ARGS_ADF_SOURCE}" ${ARGS_AIE_SOURCES}
               ${ARGS_ADF_HEADERS} ${ARGS_AIE_HEADERS}
      BYPRODUCTS "${_work_aie_hw}"
      WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    )

    add_custom_target(${ARGS_NAME}-adf-x86 DEPENDS "${_libadf_x86}")
    add_custom_target(${ARGS_NAME}-adf-hw  DEPENDS "${_libadf_hw}")
    add_custom_target(${ARGS_NAME}-adf DEPENDS "${_libadf}")

    add_custom_command(OUTPUT "${ARGS_DATA_DIR}" VERBATIM
      COMMAND ${CMAKE_COMMAND} -E create_symlink 
        ${CMAKE_CURRENT_SOURCE_DIR}/${ARGS_DATA_DIR} ${ARGS_DATA_DIR}
      COMMENT "Creating symlink ${ARGS_DATA_DIR}"
      WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    )
    add_custom_target(${ARGS_NAME}-data DEPENDS "${ARGS_DATA_DIR}")

    add_custom_target(${ARGS_NAME}-x86sim VERBATIM
      COMMAND ${CMAKE_COMMAND} -E rm -rf x86simulator_output/${ARGS_DATA_DIR}
      COMMAND ${X86SIMULATOR} --pkg-dir=${_work_aie_x86}
      COMMAND ${Python3_EXECUTABLE} ${PROJECT_SOURCE_DIR}/scripts/check_simulation.py
        ${CMAKE_CURRENT_SOURCE_DIR}/${ARGS_DATA_DIR}
        x86simulator_output/${ARGS_DATA_DIR}
        ${ARGS_DATA_GLOB}
      COMMAND ${CMAKE_COMMAND} -E echo "==== x86simulator PASS ===="
      DEPENDS ${ARGS_NAME}-adf-x86 ${ARGS_NAME}-data
      BYPRODUCTS x86simulator_output
      WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    )
    add_custom_target(${ARGS_NAME}-aiesim VERBATIM COMMAND_EXPAND_LISTS
      COMMAND ${CMAKE_COMMAND} -E rm -rf aiesimulator_output/${ARGS_DATA_DIR}
      COMMAND ${AIESIMULATOR} --pkg-dir=${_work_aie_hw} 
        $<IF:$<BOOL:${AIESIM_PROFILE}>,--profile,>
        $<IF:$<BOOL:${AIESIM_DUMP_VCD}>,--dump-vcd=${AIESIM_DUMP_VCD},>
      COMMAND ${Python3_EXECUTABLE} ${PROJECT_SOURCE_DIR}/scripts/check_simulation.py
        ${CMAKE_CURRENT_SOURCE_DIR}/${ARGS_DATA_DIR}
        aiesimulator_output/${ARGS_DATA_DIR}
        ${ARGS_DATA_GLOB}
      COMMAND ${CMAKE_COMMAND} -E echo "==== aiesimulator PASS ===="
      DEPENDS ${ARGS_NAME}-adf-hw ${ARGS_NAME}-data
      BYPRODUCTS aiesimulator_output
      WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    )
  endif(aie_mode)

  string(TOUPPER "${XILINX_TARGET}" XILINX_TARGET_UPPER)
  configure_file("${PROJECT_SOURCE_DIR}/cmake/hls.cfg.in" "${_hls_cfg}" @ONLY)
  add_custom_command(OUTPUT "${_xo}" VERBATIM COMMAND_EXPAND_LISTS
    COMMAND ${VITIS_VPP} --compile --mode hls 
                         --work_dir=${_work_hls} 
                         --config=${_hls_cfg}
                         --hls.jobs=${VPP_HLS_JOBS}
    DEPENDS "${ARGS_PL_SOURCE}" ${ARGS_PL_HEADERS} "${_hls_cfg}"
    BYPRODUCTS "${_work_hls}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  )
  add_custom_target(${ARGS_NAME}-xo DEPENDS "${_xo}")

  add_custom_command(OUTPUT "${_xsa}" VERBATIM COMMAND_EXPAND_LISTS
    COMMAND ${VITIS_VPP} --link --target=${XILINX_TARGET} --platform=${XILINX_PLATFORM}
                         --temp_dir=${_work_link}
                         $<IF:$<BOOL:${VPP_REPORT_LEVEL}>,--report_level=${VPP_REPORT_LEVEL},>
                         $<IF:$<BOOL:${VPP_DEBUG}>,--debug,>
                         $<IF:$<BOOL:${VPP_SAVE_TEMPS}>,--save-temps,>
                         --vivado.impl.jobs=${VPP_VIVADO_IMPL_JOBS}
                         --vivado.synth.jobs=${VPP_VIVADO_SYNTH_JOBS}
                         $<IF:$<BOOL:${VPP_OPTIMIZE}>,--optimize=${VPP_OPTIMIZE},>
                         ${VPP_PROFILE_FLAGS}
                         ${_kernel_freq}
                         #${_clock_freq_hz}
                         ${_place_design_pre}
                         $<IF:$<BOOL:${ARGS_XSA_CONFIG}>,--config=${ARGS_XSA_CONFIG},>
                         ${_xo} ${_libadf} --output=${_xsa}
    DEPENDS "${_libadf}" "${_xo}" "${ARGS_XSA_CONFIG}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  )
  add_custom_target(${ARGS_NAME}-xsa DEPENDS "${_xsa}")

  add_custom_command(OUTPUT "${_xclbin}" VERBATIM COMMAND_EXPAND_LISTS
    COMMAND ${VITIS_VPP} --package --target=${XILINX_TARGET} --platform=${XILINX_PLATFORM}
                         --temp_dir=${_work_package}
                         $<IF:$<BOOL:${VPP_REPORT_LEVEL}>,--report_level=${VPP_REPORT_LEVEL},>
                         $<IF:$<BOOL:${VPP_SAVE_TEMPS}>,--save-temps,>
                         ${_xsa} ${_libadf} --output=${_xclbin}
    DEPENDS "${_xsa}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  )

  add_custom_target(${ARGS_NAME}-xclbin DEPENDS "${_xclbin}")

  add_executable(${ARGS_NAME}-host EXCLUDE_FROM_ALL "${ARGS_HOST_SOURCE}")
  set_target_properties(${ARGS_NAME}-host
    PROPERTIES
      OUTPUT_NAME ${ARGS_NAME}
      RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}"
  )
  target_include_directories(${ARGS_NAME}-host PRIVATE ${XRT_INCLUDE_DIRS})
  target_link_libraries(${ARGS_NAME}-host PRIVATE XRT::xrt_coreutil)

  add_custom_target(${ARGS_NAME} DEPENDS ${ARGS_NAME}-xclbin ${ARGS_NAME}-host)

  if(NOT "${XILINX_TARGET}" STREQUAL "hw")
    add_dependencies(${ARGS_NAME} emconfig-json)
  endif()

  if("${XILINX_TARGET}" STREQUAL "hw_emu")
    configure_file("${PROJECT_SOURCE_DIR}/cmake/${_pre_sim_tcl}.in" "${_pre_sim_tcl}" @ONLY)
  endif()

  set_target_properties(${ARGS_NAME}
    PROPERTIES
      XCLBIN "${_xclbin}"
  )

endfunction(vitis_flow)
