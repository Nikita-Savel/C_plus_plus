cmake_minimum_required(VERSION 3.29)
project(list_stackallocator)

set(CMAKE_CXX_STANDARD 20)

include_directories(.)

add_executable(list_stackallocator
    stackallocator.cpp
    stackallocator.h)
