cmake_minimum_required(VERSION 3.19)
project(roofline-demo)

set(CMAKE_CXX_STANDARD 14)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_compile_options(-march=native)

# Pull in embedded third-party dependencies
add_subdirectory(third_party)

add_executable(roofline-demo main.cpp third_party/hopscotch/src/rw_roofline.cpp third_party/hopscotch/include/rw_roofline.h third_party/hopscotch/src/common.cpp third_party/hopscotch/include/common.h)

# Add embedded third-party dependencies to `CSC2222_Roofline` target
add_third_party_dependency_hopscotch(roofline-demo)
add_third_party_dependency_tables_and_graphs(roofline-demo)

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(roofline-demo PUBLIC OpenMP::OpenMP_CXX)
endif()