cmake_minimum_required(VERSION 3.12)

project(herald VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}")
# Include the sanitizer module
include(code-coverage)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

#IF(CMAKE_BUILD_TYPE MATCHES DEBUG)
add_definitions(-DHERALD_LOG_LEVEL=4)
#ENDIF(CMAKE_BUILD_TYPE MATCHES DEBUG)

include(GNUInstallDirs)

add_subdirectory(heraldns) 
add_subdirectory(heraldns-tests) 
add_subdirectory(heraldns-cli) 
add_subdirectory(herald)
add_subdirectory(herald-tests)
add_subdirectory(herald-programmer)
add_subdirectory(herald-mesh-proxy)
add_subdirectory(doxygen)