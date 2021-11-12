#include <iostream>
#include "gtest/gtest.h"

#include "alloc.h"

using namespace std;

const size_t def_block_size = 9;

TEST(Alloc, allocate1)
{
    clear();
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
    clear();
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

TEST(Alloc, try_alloc_over_free)
{
    clear();

    // it should return nullptr because it is
    // also needed memory for block metadata
    void* p = alloc(DEFAULT_SIZE);

    EXPECT_EQ(p, nullptr);
}

TEST(Alloc, current_offset_moves)
{
    clear();
    void* partition1 = alloc((DEFAULT_SIZE / 2) - def_block_size);
    void* some_var = alloc((DEFAULT_SIZE / 4) - def_block_size);

    del(partition1);

    void* partition2 = alloc((DEFAULT_SIZE / 2) - def_block_size);

    mem_info info = get_mem_info();
    EXPECT_EQ(partition2 - def_block_size, (void*)info.mem);
}

TEST(Alloc, empty_size)
{
    clear();
    auto mem_info = get_mem_info();
    EXPECT_EQ(mem_info.free_mem, DEFAULT_SIZE);
}

TEST(Alloc, random_test)
{
    clear();

    srand(time(0));
    typedef struct allocated_info
    {
        void* p;
        size_t size;
    }allocated_info_t;

    vector<allocated_info_t> pointers;

    size_t num_tests = 1000000;
    for(size_t i = 0; i < num_tests; i++) {
        bool to_alloc = rand() % 2;
        cout << to_alloc << endl;
        if(to_alloc) {
            size_t alloc_size = rand() % (DEFAULT_SIZE / 10);
            auto p = alloc(alloc_size);
            if(p != nullptr) {
                pointers.push_back({p, alloc_size});
                cout << "allocated: " << alloc_size << endl;
            }

        } else {
            // do del

            if(pointers.size() > 0) {
                size_t index_to_del = rand() % pointers.size();
                size_t s = pointers[index_to_del].size;

                auto mem_info1 = get_mem_info();

                del(pointers[index_to_del].p);
                pointers.erase(pointers.begin() + index_to_del);

                auto mem_info2 = get_mem_info();

                // here there are problems with allocating because somewhere it is overwrites memory
                ASSERT_EQ(mem_info2.free_mem, mem_info1.free_mem + s + sizeof(struct block));
            }
        }

        auto mem_info = get_mem_info();
        if(mem_info.free_mem > 1024)
            ASSERT_TRUE(false);
    }
}
