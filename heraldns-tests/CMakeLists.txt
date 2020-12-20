cmake_minimum_required(VERSION 3.12)

add_executable(heraldns-tests
	basictrans-tests.cpp
	datatypes-tests.cpp
	presence-tests.cpp
)

include_directories(${heraldns_SOURCE_DIR} ..)

target_link_libraries(heraldns-tests PRIVATE heraldns)

target_compile_features(heraldns-tests PRIVATE cxx_std_17)
