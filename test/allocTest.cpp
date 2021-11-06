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
}
