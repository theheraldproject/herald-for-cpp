cmake_minimum_required(VERSION 3.12)

add_executable(herald-mesh-proxy 
  src/main.cpp
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_link_libraries(herald-mesh-proxy PRIVATE herald Threads::Threads)

target_compile_features(herald-mesh-proxy PRIVATE cxx_std_17)

include_directories(
  ${herald_SOURCE_DIR} 
  include
)

install(TARGETS herald-mesh-proxy 
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} 
)
