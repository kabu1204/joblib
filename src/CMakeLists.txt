cmake_minimum_required(VERSION 3.20)
project(src)

set(CMAKE_CXX_STANDARD 14)

set(INC_DIR /usr/local/include ../include)
set(LINK_DIR /usr/local/lib/)

include_directories(${INC_DIR})
link_directories(${LINK_DIR})
#link_libraries(tbox)

add_library(joblib SHARED library.cpp parallel/range.cpp parallel/async_stackless.cpp ../include/easylogging/easylogging++.cc)