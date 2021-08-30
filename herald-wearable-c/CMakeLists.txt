# /*
#  * Copyright 2020-2021 Herald Project Contributors
#  * SPDX-License-Identifier: Apache-2.0
#  * 
#  */

# Set minimum version
cmake_minimum_required(VERSION 3.13.1)

set(HERALD_BASE "../herald-for-c")

# Include Herald CMAKE
include(${HERALD_BASE}/herald.cmake)

# Set the project
project(herald-wearable VERSION 1.0.0 LANGUAGES C)

# add the sources
target_sources(app PRIVATE
    src/main.c
    src/abtt_payload.c
)
