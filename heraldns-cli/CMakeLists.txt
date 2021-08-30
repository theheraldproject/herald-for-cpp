cmake_minimum_required(VERSION 3.12)

add_executable(heraldns-cli main.cpp)

target_link_libraries(heraldns-cli PRIVATE heraldns )

target_compile_features(heraldns-cli PRIVATE cxx_std_17)

include_directories(${heraldns_SOURCE_DIR})

install(TARGETS heraldns-cli
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} 
)
