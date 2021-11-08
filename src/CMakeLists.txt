cmake_minimum_required(VERSION 3.20)
project(src)

set(CMAKE_CXX_STANDARD 14)

include_directories(../include)

add_library(joblib SHARED library.cpp range.cpp generator.cpp ../include/easylogging/easylogging++.cc)