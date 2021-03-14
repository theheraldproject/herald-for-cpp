cmake_minimum_required(VERSION 3.12)

find_package(Doxygen
             OPTIONAL_COMPONENTS dot mscgen dia)

doxygen_add_docs(herald-docs 
  ../herald/include
  ALL
)

# Output placed in herald-for-cpp/build/doxygen/html
