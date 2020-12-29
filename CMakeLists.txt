cmake_minimum_required(VERSION 3.12)

project(herald VERSION 1.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(GNUInstallDirs)

add_subdirectory(fmt)
add_subdirectory(heraldns) 
add_subdirectory(heraldns-tests) 
add_subdirectory(heraldns-cli) 
add_subdirectory(herald)
add_subdirectory(herald-tests)
add_subdirectory(herald-programmer)
add_subdirectory(herald-mesh-proxy)