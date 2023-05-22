cmake_minimum_required(VERSION 3.0.0)
project(lab4 VERSION 0.1.0)

add_executable(lab4-server server/main.cpp server/TcpConnection.cpp server/helpers.cpp)
