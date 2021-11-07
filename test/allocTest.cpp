#include <iostream>
#include "gtest/gtest.h"

#include "alloc.h"

using namespace std;

const size_t def_block_size = 9;

TEST(Alloc, allocate1)
{
    int* var = (int*)alloc(sizeof(int));
    int g = rand() % 100;

    *var = g;
    EXPECT_EQ(*var, g);

    struct mem_info info = get_mem_info();
    EXPECT_EQ(info.free_mem, DEFAULT_SIZE - def_block_size - sizeof(int));
    del(var);
    info = get_mem_info();
    EXPECT_EQ(info.free_mem, DEFAULT_SIZE);
}

TEST(Alloc, allocate_arr)
{
    size_t alloc_size = sizeof(int)*3;
    int* arr = (int*)alloc(alloc_size);
    arr[0] = 123;
    arr[1] = 456;
    arr[2] = 789;

    EXPECT_EQ(arr[0], 123);
    EXPECT_EQ(arr[1], 456);
    EXPECT_EQ(arr[2], 789);

    struct mem_info info = get_mem_info();
    EXPECT_EQ(info.free_mem, DEFAULT_SIZE - alloc_size - def_block_size);
    del(arr);

    info = get_mem_info();
    EXPECT_EQ(info.free_mem, DEFAULT_SIZE);
}
