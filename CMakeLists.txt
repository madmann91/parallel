cmake_minimum_required(VERSION 2.8.1 FATAL_ERROR)

project(PARALLEL)

find_package(OpenMP QUIET)
if (OPENMP_FOUND)
    set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS})
endif ()

include_directories(include)

set(CMAKE_CXX_STANDARD 11)
add_subdirectory(test)
