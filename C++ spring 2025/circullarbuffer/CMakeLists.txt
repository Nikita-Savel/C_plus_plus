cmake_minimum_required(VERSION 3.29)
project(C___spring_2025 C)

set(CMAKE_C_STANDARD 20)

include_directories(.)

add_executable(C___spring_2025
    circular_buffer.h
        tests.cpp
        pupu.h)
