cmake_minimum_required(VERSION 3.6)
project(avi-maker)
set(CMAKE_CXX_STANDARD 11)
set(SOURCE_FILES
        src/bmp_lib/bmp.h
        src/bmp_lib/bmp.cpp
        src/bmp_lib/bmp_math.cpp
        src/bmp_lib/filter.h
        src/bmp_lib/filter.cpp
        src/main.cpp
        src/avi_lib/avi_rw.cpp
        src/avi_lib/avi_rw.h
        src/avi_lib/avi_io.cpp
        src/AVIMaker/AVIMaker.cpp
        src/AVIMaker/AVIMaker.h
        src/motion_compensation/motion_compensation.cpp
        src/motion_compensation/motion_compensation.h
        src/bmp_lib/bmp_headers.hpp
        src/avi_lib/avi_headers.hpp
        src/bmp_lib/bmp_examples.cpp)

add_executable(avi-maker ${SOURCE_FILES})
add_library(avi-maker-lib STATIC ${SOURCE_FILES})