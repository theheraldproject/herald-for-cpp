cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(GNUInstallDirs
)

set(HEADERS 
	include/heraldns.h
	include/datatypes/grid.h
	include/datatypes/presence.h
	include/intermediate/stdout_intermediate_results.h
	include/mixing/direct_mixing.h
	include/providers/intermediate_results.h
	include/providers/social_mixing.h
	include/providers/transmission.h
	include/simulator/simulator.h
	include/transmission/basic_transmission.h
)

add_library(heraldns 
	${HEADERS}
	src/datatypes/grid.cpp
	src/datatypes/presence.cpp
	src/intermediate/stdout_intermediate_results.cpp
	src/mixing/direct_mixing.cpp
	src/simulator/simulator.cpp
	src/transmission/basic_transmission.cpp
)
set_target_properties(heraldns PROPERTIES PUBLIC_HEADER "${HEADERS}")

# include_directories(${herald_SOURCE_DIR}/fmt/include)

target_include_directories(heraldns 
	PUBLIC 
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>
)

target_compile_features(heraldns PRIVATE cxx_std_17)

# NB: This is here to ensure binaries that link us also link stdc++fs for non-Apple targets
# https://github.com/OpenRCT2/OpenRCT2/pull/10522
if(NOT (APPLE OR MSVC) )
target_link_libraries(heraldns PRIVATE stdc++fs ) # fmt::fmt-header-only
endif(NOT (APPLE OR MSVC) )

install(TARGETS heraldns 
    EXPORT heraldns
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}  
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} 
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/heraldns
)

install(EXPORT heraldns 
	NAMESPACE heraldns::
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/heraldns
)
