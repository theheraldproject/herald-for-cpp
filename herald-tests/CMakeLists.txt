cmake_minimum_required(VERSION 3.12)

add_executable(herald-tests
  # Test templates
	test-templates.h
	test-templates.cpp
	test-util.h 

	# Catch all to ensure all files in Herald are included in code coverage reports
	codecoverage-tests.cpp

  # base data types
	allocatablearray-tests.cpp
	bytearrayprinter-tests.cpp
	memoryarena-tests.cpp
	datatypes-tests.cpp
	base64string-tests.cpp
	datetime-tests.cpp
	randomuuid-tests.cpp
	uint8-tests.cpp 
	uint16-tests.cpp 
	uint32-tests.cpp 
	uint64-tests.cpp 
	sha256-tests.cpp
	distribution-tests.cpp
	
	# Low level
	sensorlogger-tests.cpp
	errorcontactlog-tests.cpp
	data-tests.cpp
	datatypesdataderived-tests.cpp
	blemacaddress-tests.cpp
	targetidentifier-tests.cpp
 
	# Low level cross platform 
	test-util.cpp 
	crossplatform-tests.cpp 

  # mid level
	beaconpayload-tests.cpp
	extendeddata-tests.cpp
	fixedpayload-tests.cpp
	# simplepayload-tests.cpp
	bledevice-tests.cpp
	sample-tests.cpp
	ranges-tests.cpp
	analysisrunner-tests.cpp
	analysissensor-tests.cpp
	gaussian-tests.cpp

  # high level
	advertparser-tests.cpp
	bledatabase-tests.cpp
	blecoordinator-tests.cpp
	coordinator-tests.cpp

	# App level
	nordicuart-tests.cpp

	# Usage level
	exposure-risk-tests.cpp
	exposure-manager-tests-new.cpp

	# main test file
	main.cpp
)

include_directories(${herald_SOURCE_DIR})

# set(THREADS_PREFER_PTHREAD_FLAG ON)
# find_package(Threads REQUIRED)

target_link_libraries(herald-tests PRIVATE herald) # Threads::Threads
target_code_coverage(herald-tests AUTO EXTERNAL EXCLUDE *-tests.cpp test-*.h *ain.cpp *est-util.cpp *est-templates.h *atch.hpp /usr/include/**/*) # EXCLUDE ONLY WORKS WITH PATTERNS

if (WIN32)
  #add_compile_options(-Wl,--stack,100000000)
  #set_target_properties(herald-tests PROPERTIES LINK_FLAGS -Wl,--stack,10000000)
  #set_target_properties(herald-tests PROPERTIES LINK_FLAGS /STACK:10000000)
  add_compile_options(/STACK:1000000000000)
  set_target_properties(herald-tests PROPERTIES LINK_FLAGS /STACK:1000000000000)
else()
	# GCov flags for template classes
	add_compile_options(--coverage -fno-inline -fno-inline-small-functions -fno-default-inline)
endif ()
target_compile_features(herald-tests PRIVATE cxx_std_17)
