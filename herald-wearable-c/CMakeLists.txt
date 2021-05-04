# /*
#  * Copyright 2020-2021 Herald Project Contributors
#  * SPDX-License-Identifier: Apache-2.0
#  * 
#  */

# Set minimum version
cmake_minimum_required(VERSION 3.13.1)



set(HERALD_BASE "../herald-for-c")

set(BASE_CONFIG ${HERALD_BASE}/config)

set(OVERLAY_CONFIG 
    ${BASE_CONFIG}/base.conf
  )

# include the zephyr package, this creates the `app` target
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

# Set the project
project(herald-wearable VERSION 1.0.0 LANGUAGES C)



# include herald
#add_subdirectory(../herald herald_build)
include(${HERALD_BASE}/herald.cmake)

target_include_directories(app PUBLIC include ${HERALD_BASE} ${HERALD_BASE}/include)

# add the sources
target_sources(app PRIVATE
    ${HERALD_SOURCES}
    src/main.c
    src/abtt_payload.c
)

#target_link_libraries(app PRIVATE herald_for_c)
