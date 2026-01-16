cmake_minimum_required(VERSION 3.23)
# 3.12 -> <PackageName>_ROOT
# 3.13 -> relative target_sources (CMP0076)
# 3.23 -> target_sources(FILE_SET)

################################
# Project
################################

project(acap-gemm
        VERSION 0.1.0
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(GNUInstallDirs)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

################################
# Build configuration
################################

if(NOT DEFINED ACAP_GEMM_MAIN_PROJECT)
  if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(ACAP_GEMM_MAIN_PROJECT ON)
  else()
    set(ACAP_GEMM_MAIN_PROJECT OFF)
  endif()
endif()

option(ENABLE_TESTS "Enable tests" ${ACAP_GEMM_MAIN_PROJECT})
option(ENABLE_WERROR "Set -Werror" OFF)

################################
# Project configuration
################################

include(cmake/xilinx-setup.cmake)

################################
# Subdirectories
################################

add_subdirectory(src) # src-mcast
add_subdirectory(src-trad)
add_subdirectory(src-ideal)

