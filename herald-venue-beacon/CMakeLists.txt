# SPDX-License-Identifier: Apache-2.0

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Zephyr build environment variables set here
set(ENV{ARCH} arm)
set(ENV{BOARD} nrf52840dongle_nrf52840)
#set(ENV{BOARD} nrf52832_mdk)

# Generic Zephyr build command below
cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(herald-venue-beacon)




# Include Herald core library
add_subdirectory(../fmt ../build/fmt)
add_subdirectory(../herald ../build/herald)

# The below is not supported by gcc-arm-eabi
#target_compile_features(app cxx_std_17)

include_directories(${herald_SOURCE_DIR})

target_sources(app PRIVATE
  src/main.cpp
)


target_link_libraries(app PRIVATE herald)