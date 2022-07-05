#include <iostream>
#include <cstdlib>
#include <functional>
#include <vector>
#include <list>
#include <chrono>

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

    mem_info_t info = get_mem_info();
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

    mem_info_t info = get_mem_info();
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

    mem_info_t info = get_mem_info();
    EXPECT_EQ((void*)((size_t)partition2 - (size_t)def_block_size), (void*)info.mem);
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

    size_t num_tests = 10000;
    for(size_t i = 0; i < num_tests; i++) {
        bool to_alloc = rand() % 2;
        if(to_alloc) {
            size_t alloc_size = rand() % (DEFAULT_SIZE / 10);

            auto p = alloc(alloc_size);
            if(p != nullptr) {
                pointers.push_back({p, alloc_size});
            }

        } else {
            // do delete

            if(pointers.size() > 0) {
                size_t index_to_del = rand() % pointers.size();
                size_t s = pointers[index_to_del].size;

                auto mem_info1 = get_mem_info();

                del(pointers[index_to_del].p);
                pointers.erase(pointers.begin() + index_to_del);

                auto mem_info2 = get_mem_info();

                ASSERT_EQ(mem_info2.free_mem, mem_info1.free_mem + s + SIZEOF_STRUCT_BLOCK);
            }
        }

        auto mem_info = get_mem_info();
        if(mem_info.free_mem > DEFAULT_SIZE)
            ASSERT_TRUE(false);
    }
}

struct performance_data
{
    size_t num_tests;
    size_t num_allocs;
    size_t num_deallocs;
    float avg_time_alloc;
    float avg_time_del;
};

std::vector<std::pair<size_t, size_t>> sizes = {
    {1, 8},
    {32768, 65536},
    {1048576, 2097152}
};

performance_data allocation_test(size_t test_amount, std::function<void*(size_t)> alloc_func, std::function<void(void*)> del_func, size_t size_i) {
    clear();

    std::vector<int> dealloc_measures;
    std::vector<int> alloc_measures;
    std::list<void*> pointers;
    for(size_t i = 0; i < test_amount; i++) {
        bool do_alloc = (rand() % 2) == 0;
        if(!do_alloc && pointers.size() > 0) {
            // get iterator to random element
            auto it = pointers.begin();
            std::advance(it, rand() % pointers.size());

            // delete it
            auto begin = std::chrono::steady_clock::now();
            del_func(*it);
            auto end = std::chrono::steady_clock::now();
            pointers.erase(it);
            auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
            dealloc_measures.push_back(duration.count());
        } else if (do_alloc) { // do alloc
            auto begin = std::chrono::steady_clock::now();
            auto min_size = sizes[size_i].first;
            auto max_size = sizes[size_i].second;
            int var_size = rand() % (max_size - min_size) + min_size;
            auto p = alloc_func(var_size);
            auto end = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
            alloc_measures.push_back(duration.count());
            pointers.push_back(p);
        }
    }

    // print average results
    int sum_alloc = 0;
    int sum_dealloc = 0;
    for(auto& measure : alloc_measures) {
        sum_alloc += measure;
    }
    for(auto& measure : dealloc_measures) {
        sum_dealloc += measure;
    }

    performance_data data;
    data.num_tests = test_amount;
    data.num_allocs = alloc_measures.size();
    data.num_deallocs = dealloc_measures.size();
    data.avg_time_alloc = (float)sum_alloc / alloc_measures.size();
    data.avg_time_del = (float)sum_dealloc / dealloc_measures.size();

    return data;
}

TEST(Alloc, performance_test)
{
    size_t subtests_amount = 1000;
    size_t tests_amount = 10;
    size_t big_variables = 1;

    float avg_custom_alloc = 0;
    float avg_custom_del = 0;
    float avg_std_alloc = 0;
    float avg_std_del = 0;

    printf("Alloc performance test\n");
    {
        printf("\nCustom allocator:\n");
        std::vector<float> allocation_measures;
        std::vector<float> deallocation_measures;
        for(size_t i = 0; i < tests_amount; i++) {
            auto data = allocation_test(subtests_amount, alloc, del, big_variables);
            allocation_measures.push_back(data.avg_time_alloc);
            deallocation_measures.push_back(data.avg_time_del);
        }

        float sum_alloc = 0;
        float sum_dealloc = 0;
        for(auto& measure : allocation_measures) {
            sum_alloc += measure;
        }
        for(auto& measure : deallocation_measures) {
            sum_dealloc += measure;
        }

        avg_custom_alloc = (float)sum_alloc / allocation_measures.size();
        avg_custom_del = (float)sum_dealloc / deallocation_measures.size();

        // print average results
        printf("allocation time: %f\n", avg_custom_alloc);
        printf("deallocation time: %f\n", avg_custom_del);
    }

    {
        printf("\nC native allocator\n");
        std::vector<float> allocation_measures;
        std::vector<float> deallocation_measures;
        for(size_t i = 0; i < tests_amount; i++) {
            auto data = allocation_test(subtests_amount, malloc, free, big_variables);
            allocation_measures.push_back(data.avg_time_alloc);
            deallocation_measures.push_back(data.avg_time_del);
        }

        float sum_alloc = 0;
        float sum_dealloc = 0;
        for(auto& measure : allocation_measures) {
            sum_alloc += measure;
        }
        for(auto& measure : deallocation_measures) {
            sum_dealloc += measure;
        }

        avg_std_alloc = (float)sum_alloc / allocation_measures.size();
        avg_std_del = (float)sum_dealloc / deallocation_measures.size();

        // print average results
        printf("allocation time: %f\n", avg_std_alloc);
        printf("deallocation time: %f\n", avg_std_del);
    }

    printf("\n");

    // print in % how much the custom allocator is faster
    printf("Custom allocator is %f%% faster\n", (avg_std_alloc - avg_custom_alloc) / avg_std_alloc * 100);

    // print in % how much the custom deallocator is faster
    printf("Custom deallocator is %f%% faster\n", (avg_std_del - avg_custom_del) / avg_std_del * 100);

    // print in % how the sum of allocation and deallocation is faster
    printf("Sum of allocation and deallocation is %f%% faster\n",
        (avg_std_alloc + avg_std_del - avg_custom_alloc - avg_custom_del) / (avg_std_alloc + avg_std_del) * 100);
}
