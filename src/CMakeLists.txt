cmake_minimum_required(VERSION 2.8)
project(tracer_lib)
set(SOURCE_LIB safeout.cpp)
add_library(tracer STATIC ${SOURCE_LIB})
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/safeout.h ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
