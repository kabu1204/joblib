cmake_minimum_required(VERSION 3.20)
project(joblib)

set(CMAKE_CXX_STANDARD 14)

add_library(joblib SHARED library.cpp)

add_subdirectory(test)