cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

include(GNUInstallDirs
)

set(HERALD_BASE .)
include(herald.cmake)

if (WIN32)
  set(PLATFORM_SOURCES
    ${HERALD_SOURCES_WINDOWS}
  )
else()
  if(HERALD_TARGET STREQUAL zephyr)
    set(PLATFORM_SOURCES
      ${HERALD_SOURCES_ZEPHYR}
    )
  else()
    if(NOT (MSVC) )
      set(PLATFORM_SOURCES
        ${HERALD_SOURCES_OPENSSL}
      )
    endif()
  endif()
endif()

add_library(herald 
	${HERALD_HEADERS}
  ${HERALD_SOURCES}
  ${PLATFORM_SOURCES}
)

if (WIN32)
target_code_coverage(herald AUTO)
endif()

set_target_properties(herald PROPERTIES PUBLIC_HEADER "${HERALD_HEADERS}")

target_include_directories(herald 
	PUBLIC 
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>
)

if (MSVC)
  target_link_libraries(herald Crypt32.lib Bcrypt.lib)
endif()

if(HERALD_TARGET STREQUAL zephyr)  
  message("HERALD TARGET is ZEPHYR RTOS ${HERALD_TARGET}")
  message("CXX INCLUDES WITHIN: ${includes_cxx}")
  message("Is CXX compiler stuff defined?" ${includex_cxx})

  #set(CXXFLAGS
  #  ${includes_cxx} ${system_includes_cxx}
  #)

  #target_include_directories(herald SYSTEM PRIVATE
  #  $ENV{ZEPHYR_BASE}/include
  #)
#  target_include_directories(herald PRIVATE 
#    ${cxxincludes}
#    ${cxxsystem_includes}
#  )

  # Zephyr CMakeLists.txt should define the include folders and CXX settings we need by default 
  #set(CMAKE_CXX_FLAGS ${Z_CFLAGS} ${CXXFLAGS})
endif()


#target_compile_features(herald PRIVATE cxx_std_17)

add_compile_options(-Wl,-Wno-unused-function)

if(NOT (MSVC OR (HERALD_TARGET STREQUAL zephyr)) )
  # Use the DEV location (not system location) to pick up OpenSSL v3
  # see .vscode/settings.json for CMake path overrides
  find_package(OpenSSL 3.1.0 REQUIRED)
  target_link_libraries(herald OpenSSL::Crypto)
endif()

install(TARGETS herald 
    EXPORT herald
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}  
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} 
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/herald
)

install(EXPORT herald 
	NAMESPACE herald::
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/herald
)
