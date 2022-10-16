cmake_minimum_required(VERSION 3.14)

project(z3_test CXX)

set(CMAKE_CXX_STANDARD 17)

find_library(z3_lib z3)
message(STATUS "Z3_FOUND: ${z3_lib}")

add_executable(z3_test main.cpp)
target_link_libraries(z3_test ${z3_lib})