cmake_minimum_required(VERSION 3.12)

add_executable(herald-tests
  # root with main
	datatypes-tests.cpp
	
	# Low level
	sensorlogger-tests.cpp
	errorcontactlog-tests.cpp
	data-tests.cpp
	blemacaddress-tests.cpp
	targetidentifier-tests.cpp

  # mid level
	beaconpayload-tests.cpp
	extendeddata-tests.cpp
	fixedpayload-tests.cpp
	simplepayload-tests.cpp
	bledevice-tests.cpp
	sample-tests.cpp
	ranges-tests.cpp
	analysisrunner-tests.cpp
	analysissensor-tests.cpp

  # high level
	advertparser-tests.cpp
	bledatabase-tests.cpp
	blecoordinator-tests.cpp
	coordinator-tests.cpp

	# main test file
	main.cpp
)

include_directories(${herald_SOURCE_DIR})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_link_libraries(herald-tests PRIVATE herald Threads::Threads)

target_compile_features(herald-tests PRIVATE cxx_std_17)
