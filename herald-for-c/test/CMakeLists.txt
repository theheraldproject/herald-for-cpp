# /*
#  * Copyright 2020-2021 Herald Project Contributors
#  * SPDX-License-Identifier: Apache-2.0
#  * 
#  */
cmake_minimum_required(VERSION 3.12)

set(HERALD_SRC
    "payload/FixedPayloadSupplier.c"

    "ble/BleReceiver.c"
    "ble/BleZephyr.c"

    "database/BleDatabase.c"
    "database/BleLinkedList.c"
)

set(CMAKE_SYSTEM_PROCESSOR ${ARCH})

# Create the herald library
add_library(herald_for_c ${HERALD_SRC})

target_include_directories(herald_for_c PUBLIC include .)

set_target_properties(herald_for_c PROPERTIES LINKER_LANGUAGE C)
