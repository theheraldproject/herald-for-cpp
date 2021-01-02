# SPDX-License-Identifier: Apache-2.0

set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Zephyr build environment variables set here
# TODO detect if these have already been set externally as overrides
set(ENV{ARCH} arm)
set(ENV{BOARD} nrf52840dongle_nrf52840) # nRF52840 USB dongle
#set(ENV{BOARD} nrf52dk_nrf52832) # nRF52832 USB dongle
#set(ENV{BOARD} qemu_cortex_m3 ) # emulator for testing app load issues

# Generic Zephyr build command below
cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(herald-wearable CXX)
# NOTE: Don't include C library as it bloats the binary beyond DFU max load size (i.e. beyond half of the available flash)

set(HERALD_TARGET zephyr)


# WARNING DO NOT USE THE ZEPHYR DEFAULT EXTERNAL PROJECT INCLUDE METHOD
# IT DOES NOT PROVIDE ZEPHYR BUILD SETTINGS CORRECTLY WHEN CROSS COMPILING ON WINDOWS



# Include Herald core library
#add_subdirectory(../fmt ./build/fmt)
#add_subdirectory(../herald ./build/herald)

set(HERALD_BASE ${CMAKE_CURRENT_SOURCE_DIR}/../herald)
include(../herald/herald.cmake) # include sources and headers

message("HERALD HEADERS: ${HERALD_HEADERS}")

include_directories(../herald/include) # including this causes uart.h to fail complaining about incomplete type const struct device in zephyr
include_directories(../fmt/include)

add_definitions(-DCONFIG_HERALD_LOG_LEVEL=4 -DCONFIG_APP_LOG_LEVEL=4)

# gcc arm does not warn about missing defs until runtime
add_compile_options(-Wl,-z,defs,--strict,--strict_symbols,--no-undefined)

target_sources(app PRIVATE
#  ${HERALD_HEADERS}
#  ${HERALD_HEADERS_ZEPHYR}
  ../fmt/src/format.cc
  ${HERALD_SOURCES}
  ${HERALD_SOURCES_ZEPHYR}
  src/main.cpp
)

#target_link_libraries(app PUBLIC fmt)