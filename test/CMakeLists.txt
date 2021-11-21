cmake_minimum_required(VERSION 3.20)
project(test)

set(CMAKE_CXX_STANDARD 14)

message(STATUS "start building test...")

set(INC_DIR /usr/local/include ../include)
set(LINK_DIR /usr/local/lib/)

include_directories(${INC_DIR})
link_directories(${LINK_DIR} "../cmake-build-debug")
#link_libraries(tbox)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

add_executable(test main.cpp main.h)
target_link_libraries(test joblib)