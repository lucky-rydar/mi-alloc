#pragma once

#pragma pack(push, 1)

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#ifndef DEFAULT_SIZE
#define DEFAULT_SIZE 1024
#endif

// the size of block and 1 byte variable
#define MIN_VAR_SIZE (sizeof(struct block) + 1)

struct block
{
    void* next;
    bool is_used;
};

struct mem_info
{
    size_t free_mem;
    size_t used_mem;
    size_t size;
    size_t* mem;
};

// prints out debug info about the memory
void dump(size_t from, size_t to);
struct mem_info get_mem_info();

void* alloc(size_t s);
void del(void* p);

#ifdef __cplusplus
}
#endif