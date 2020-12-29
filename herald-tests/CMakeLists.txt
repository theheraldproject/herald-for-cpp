cmake_minimum_required(VERSION 3.12)

add_executable(herald-tests
	datatypes-tests.cpp
	beaconpayload-tests.cpp
	extendeddata-tests.cpp
)

include_directories(${herald_SOURCE_DIR} ../fmt/include)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_link_libraries(herald-tests PRIVATE herald Threads::Threads)

target_compile_features(herald-tests PRIVATE cxx_std_17)
