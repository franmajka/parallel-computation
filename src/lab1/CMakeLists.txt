cmake_minimum_required(VERSION 3.0.0)
project(lab1 VERSION 0.1.0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

add_executable(lab1 main.cpp)
