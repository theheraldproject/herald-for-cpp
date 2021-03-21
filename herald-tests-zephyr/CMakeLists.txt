# SPDX-License-Identifier: Apache-2.0

set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

set(NO_BUILD_TYPE_WARNING ON)

# Zephyr build environment variables set here
#set(ENV{BOARD} nrf52840dongle_nrf52840) # nRF52840 USB dongle
#set(ENV{BOARD} nrf52dk_nrf52832) # nRF52832 USB dongle
#set(ENV{BOARD} nrf5340dk_nrf5340_cpuapp) # nRF5340 DK using secure app core
# Detect if already set. If not, default to latest nRF DK board (5340)
if(DEFINED ENV{BOARD})
  message("Using board specified in BOARD variable: $ENV{BOARD}")
else()
  message("No board specified in BOARD variable. Defaulting to nrf5340dk app core")
  set(ENV{BOARD} nrf5340dk_nrf5340_cpuapp) # nRF5340 DK using secure app core
  set(ENV{ARCH} arm)
  message("BOARD variable now set to: $ENV{BOARD}")
endif()

# Specify additional PROJ settings based on build settings
set(BASE_CONFIG ../config/zephyr)
set(OVERLAY_CONFIG
  ${BASE_CONFIG}/base.conf
  ${BASE_CONFIG}/receiver.conf
  ${BASE_CONFIG}/transmitter.conf
)

if($ENV{BOARD} MATCHES .*nrf5340.*)
  set(OVERLAY_CONFIG 
    ${OVERLAY_CONFIG} 
    ${BASE_CONFIG}/nrf5340.conf
  )
elseif($ENV{BOARD} MATCHES .*nrf52840.*)
  set(OVERLAY_CONFIG 
    ${OVERLAY_CONFIG} 
    ${BASE_CONFIG}/nrf52840.conf
  )
elseif($ENV{BOARD} MATCHES .*nrf52832.*)
  set(OVERLAY_CONFIG 
    ${OVERLAY_CONFIG} 
    ${BASE_CONFIG}/nrf52832.conf
  )
endif()
if(CMAKE_BUILD_TYPE MATCHES Debug)
  set(OVERLAY_CONFIG 
    ${OVERLAY_CONFIG} 
    ${BASE_CONFIG}/debug.conf
  )
else()
  set(OVERLAY_CONFIG 
    ${OVERLAY_CONFIG} 
    ${BASE_CONFIG}/release.conf
  )
endif()


# Generic Zephyr build command below
cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(herald-wearable VERSION 1.2.0 LANGUAGES CXX)

# NOTE: Don't include C library as it bloats the binary beyond DFU max load size (i.e. beyond half of the available flash)

set(HERALD_TARGET zephyr)


# WARNING DO NOT USE THE ZEPHYR DEFAULT EXTERNAL PROJECT INCLUDE METHOD
# IT DOES NOT PROVIDE ZEPHYR BUILD SETTINGS CORRECTLY WHEN CROSS COMPILING ON WINDOWS


# Include Herald core library
set(HERALD_BASE ${CMAKE_CURRENT_SOURCE_DIR}/../herald)
include(../herald/herald.cmake) # include sources and headers

message("HERALD HEADERS: ${HERALD_HEADERS}")

include_directories(
  ..
  ../herald/include
)

if(CMAKE_BUILD_TYPE MATCHES Debug)
  add_definitions(-DHERALD_LOG_LEVEL=4 -DCONFIG_APP_LOG_LEVEL=4)
else()
  add_definitions(-DHERALD_LOG_LEVEL=1 -DCONFIG_APP_LOG_LEVEL=1)
endif()
#HERALD_LOG_LEVEL 4 = debug, 3 = info, 2 = warn / errors (Zephyr LOG_WARN), 1 = info contacts log only (Zephyr LOG_ERR)

# gcc arm does not warn about missing defs until runtime
# NOTE: Catch2 uses exceptions, so we enable -fexceptions
add_compile_options(--no-undefined -fexceptions -ffunction-sections -fdata-sections -s -Wl,-z,defs,norelro,--strict,--strict_symbols,--gc-sections,--hash-style=gnu)

target_sources(app PRIVATE
  ${HERALD_SOURCES}
  ${HERALD_SOURCES_ZEPHYR}
  # Simple Payload - Choose ONE
  # Option 1: MBED TLS
  #${HERALD_SOURCES_MBEDTLS}
  # Option 2: TinyCrypt
  ${HERALD_SOURCES_TINYCRYPT}

  # Now tests source
  # root with main
	#../herald-tests/datatypes-tests.cpp
  # ^^^ FIX THIS FROM DYING ALL THE TIME -> MAY NEED TO REWRITE DATA CLASS - SEE STL STRING TRICK
	
	# Low level
	# ../herald-tests/sensorlogger-tests.cpp # Don't run this on Zephyr
	#../herald-tests/errorcontactlog-tests.cpp
	#../herald-tests/data-tests.cpp
	#../herald-tests/blemacaddress-tests.cpp
	#../herald-tests/targetidentifier-tests.cpp

  # mid level
	../herald-tests/analysisrunner-tests.cpp
	#../herald-tests/analysissensor-tests.cpp
	#../herald-tests/beaconpayload-tests.cpp
	#../herald-tests/extendeddata-tests.cpp
	#../herald-tests/fixedpayload-tests.cpp
	#../herald-tests/simplepayload-tests.cpp
	#../herald-tests/bledevice-tests.cpp
	#../herald-tests/sample-tests.cpp
	#../herald-tests/ranges-tests.cpp

  # high level
	#../herald-tests/advertparser-tests.cpp
	#../herald-tests/bledatabase-tests.cpp
	#../herald-tests/blecoordinator-tests.cpp
	#../herald-tests/coordinator-tests.cpp

  # now main func
  src/main.cpp
)
