cmake_minimum_required(VERSION 3.20)
project(test)

set(CMAKE_CXX_STANDARD 14)

message(STATUS "start building test...")

include_directories(../include)

link_directories(../cmake-build-relwithdebinfo/)

add_executable(test main.cpp main.h)

target_link_libraries(test joblib)