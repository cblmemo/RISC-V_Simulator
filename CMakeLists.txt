cmake_minimum_required(VERSION 3.10.2)
project(RISC-V_Simulator)

set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")

add_executable(code src/main.cpp)