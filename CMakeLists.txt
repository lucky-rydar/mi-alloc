project(mi-alloc)

cmake_minimum_required(VERSION 3.8)

set(DIRS 
    "src/"
    "test/"
    "lib/googletest/googletest/include"
)

include_directories(${DIRS})

# build lib
set(LIB_MIALLOC_SRCS
    "src/alloc.c"
)

set(DEFAULT_SIZE 1024)

add_library(libmialloc ${LIB_MIALLOC_SRCS})
target_compile_options(libmialloc PRIVATE "-DDEFAULT_SIZE=${DEFAULT_SIZE}")

# tests
add_subdirectory(lib/googletest)
include_directories(lib/googletest/googletest/include)

file(GLOB TEST_SRCS "test/*.cpp")

add_executable(gtest_run ${TEST_SRCS})

target_link_libraries(gtest_run gtest_main)
target_link_libraries(gtest_run libmialloc)

target_compile_options(gtest_run PRIVATE "-DDEFAULT_SIZE=${DEFAULT_SIZE}")
