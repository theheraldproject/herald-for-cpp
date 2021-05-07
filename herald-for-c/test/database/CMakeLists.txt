# /*
#  * Copyright 2020-2021 Herald Project Contributors
#  * SPDX-License-Identifier: Apache-2.0
#  * 
#  */

cmake_minimum_required(VERSION 3.7)
project(database_tests)

set(HERALD_BASE "../../")

# HERALD LIB

include(${HERALD_BASE}/herald.cmake)

add_library(herald_ble_linked_lists
    "${HERALD_BASE}/database/BleLinkedList.c"
    )

target_include_directories(herald_ble_linked_lists PUBLIC 
    "${HERALD_BASE}"
    )
#target_sources(herald PUBLIC ${HERALD_SOURCES})

# TEST RUNNER
set(TESTS "BleLinkedList_tests.cpp")
add_executable(db_test_runner ${TESTS} ../test_all.cpp)

# Link test runner with herald
target_link_libraries(db_test_runner herald_ble_linked_lists libCppUTest.a)

add_custom_command(TARGET db_test_runner POST_BUILD
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/db_test_runner
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running tests"
    )
