project(mi-alloc)

cmake_minimum_required(VERSION 3.8)

set(DIRS 
    "src/"
    "test/"
    "lib/googletest/googletest/include"
)

include_directories(${DIRS})

add_library(liballoc src/alloc.c)
add_executable(${CMAKE_PROJECT_NAME} src/main.cpp)
target_link_libraries(${CMAKE_PROJECT_NAME} liballoc)


# tests
add_subdirectory(lib/googletest)
include_directories(lib/googletest/googletest/include)

file(GLOB TEST_SRCS "test/*.cpp")

add_library(liballoc_test src/alloc.c)
add_executable(gtest_run ${TEST_SRCS})

target_link_libraries(gtest_run liballoc_test)
target_link_libraries(gtest_run gtest_main)

target_compile_options(liballoc_test PRIVATE "-DDEFAULT_SIZE=64")
target_compile_options(gtest_run PRIVATE "-DDEFAULT_SIZE=64")
