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

#define SIZEOF_STRUCT_BLOCK sizeof(block_t)

// the size of block and 1 byte variable
#define MIN_VAR_SIZE (SIZEOF_STRUCT_BLOCK + 1)

typedef struct
{
    void* next;
    bool is_used;
} block_t;

typedef struct
{
    size_t free_mem;
    size_t used_mem;
    size_t size;
    uint8_t* mem;
} mem_info_t;

// prints out debug info about the memory (for debug mainly)
void dump(size_t from, size_t to);
mem_info_t get_mem_info();

void* alloc(size_t s);
void del(void* p);

// it clears the hole heap memory (not recommended to use)
void clear();

#ifdef __cplusplus
}
#endif