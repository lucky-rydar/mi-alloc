project(mi-alloc)

cmake_minimum_required(VERSION 3.8)

include_directories(src/)

add_executable(${CMAKE_PROJECT_NAME} src/main.cpp src/alloc.c)
