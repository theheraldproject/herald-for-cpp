# SPDX-License-Identifier: Apache-2.0

set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Zephyr build environment variables set here
# TODO detect if these have already been set externally as overrides
set(ENV{ARCH} arm)
set(ENV{BOARD} nrf52840dongle_nrf52840) # nRF52840 USB dongle
#set(ENV{BOARD} nrf52dk_nrf52832) # nRF52832 USB dongle

# Generic Zephyr build command below
cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(herald-venue-beacon VERSION 1.2.0 LANGUAGES CXX)
# NOTE: Don't include C library as it bloats the binary beyond DFU max load size (i.e. beyond half of the available flash)

set(HERALD_TARGET zephyr)


# WARNING DO NOT USE THE ZEPHYR DEFAULT EXTERNAL PROJECT INCLUDE METHOD
# IT DOES NOT PROVIDE ZEPHYR BUILD SETTINGS CORRECTLY WHEN CROSS COMPILING ON WINDOWS


# The external static library that we are linking with does not know
# how to build for this platform so we export all the flags used in
# this zephyr build to the external build system.
#
# Other external build systems may be self-contained enough that they
# do not need any build information from zephyr. Or they may be
# incompatible with certain zephyr options and need them to be
# filtered out.
zephyr_get_include_directories_for_lang_as_string(       C includes)
zephyr_get_system_include_directories_for_lang_as_string(C system_includes)
zephyr_get_compile_definitions_for_lang_as_string(       C definitions)
zephyr_get_compile_options_for_lang_as_string(           C options)

zephyr_get_include_directories_for_lang_as_string(       CXX includes_cxx)
zephyr_get_system_include_directories_for_lang_as_string(CXX system_includes_cxx)
zephyr_get_compile_definitions_for_lang_as_string(       CXX definitions_cxx)
zephyr_get_compile_options_for_lang_as_string(           CXX options_cxx)

#set(external_project_cflags
#  ${includes}  "-I${CMAKE_CURRENT_SOURCE_DIR}/../fmt/include" ${definitions} ${options} ${system_includes}
#)

#set(external_project_cxxflags
#  ${includes_cxx} ${definitions_cxx} ${options_cxx} ${system_includes_cxx} 
#)

#include(ExternalProject)

# Add an external project to be able download and build the third
# party library. In this case downloading is not necessary as it has
# been committed to the repository.
#set(mylib_src_dir   ${CMAKE_CURRENT_SOURCE_DIR}/../herald )
#set(mylib_build_dir ${CMAKE_CURRENT_BINARY_DIR}/herald )

#set(MYLIB_LIB_DIR     ${mylib_build_dir})
#set(MYLIB_INCLUDE_DIR ${mylib_src_dir}/include)

#if(CMAKE_GENERATOR STREQUAL "Unix Makefiles")
# https://www.gnu.org/software/make/manual/html_node/MAKE-Variable.html
#set(submake "$(MAKE)")
#else() # Obviously no MAKEFLAGS. Let's hope a "make" can be found somewhere.
#set(submake "C:\\Program Files\\CMake\\bin\\cmake.EXE")
#endif()

#ExternalProject_Add(
#  mylib_project                 # Name for custom target
#  PREFIX     ${CMAKE_CURRENT_SOURCE_DIR}/build/herald # Root dir for entire project
#  SOURCE_DIR ${mylib_src_dir}
#  BINARY_DIR ${mylib_src_dir} # This particular build system is invoked from the root
#  CONFIGURE_COMMAND ""    # Skip configuring the project, e.g. with autoconf
#  BUILD_COMMAND
#    ${submake}
#      PREFIX=${mylib_build_dir}
#      CC=${CMAKE_C_COMPILER}
#      AR=${CMAKE_AR}
#      CFLAGS=${external_project_cflags}
#      CXX=${CMAKE_CXX_COMPILER}
#      CXXFLAGS=${external_project_cxxflags}
#  INSTALL_COMMAND ""      # This particular build system has no install command
#  BUILD_BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/herald/libherald.a
#  )

# Create a wrapper CMake library that our app can link with
#add_library(mylib_lib STATIC IMPORTED GLOBAL)
#add_dependencies(
#  mylib_lib
#  mylib_project
#)
#set_target_properties(mylib_lib PROPERTIES IMPORTED_LOCATION             ${MYLIB_LIB_DIR}/libherald.a)
#set_target_properties(mylib_lib PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${MYLIB_INCLUDE_DIR})











#message("CXX INCLUDES BEFORE: ${includes_cxx}")


# Include Herald core library
#add_subdirectory(../fmt ./build/fmt)
#add_subdirectory(../herald ./build/herald)

set(HERALD_BASE ${CMAKE_CURRENT_SOURCE_DIR}/../herald)
include(../herald/herald.cmake) # include sources and headers

message("HERALD HEADERS: ${HERALD_HEADERS}")

# The below is not supported by gcc-arm-eabi
#target_compile_features(app cxx_std_17)

include_directories(../herald/include)
#include_directories(../fmt/include)

target_sources(app PRIVATE
  ${HERALD_HEADERS}
  ${HERALD_HEADERS_ZEPHYR}
  ${HERALD_SOURCES}
  ${HERALD_SOURCES_ZEPHYR}
  src/main.cpp
)

#target_link_libraries(app PUBLIC fmt)

#target_link_libraries(app PUBLIC mylib_lib fmt)