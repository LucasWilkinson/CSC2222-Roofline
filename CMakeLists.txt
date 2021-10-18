cmake_minimum_required(VERSION 3.16)
project(roofline-demo)

set(CMAKE_CXX_STANDARD 14)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_compile_options(-march=native)

add_compile_options(-Wno-format -Wno-deprecated-declarations) # Warnings from third party code

# Pull in embedded third-party dependencies
add_subdirectory(third_party)

add_executable(roofline-demo main.cpp
    micro-benchmarks/rw_roofline.cpp micro-benchmarks/rw_roofline.h
    micro-benchmarks/common.cpp micro-benchmarks/common.h
)

# Add embedded third-party dependencies to `CSC2222_Roofline` target
add_third_party_dependency_tables_and_graphs(roofline-demo)

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(roofline-demo PUBLIC OpenMP::OpenMP_CXX)
endif()