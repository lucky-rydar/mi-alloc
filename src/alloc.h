#pragma once

#pragma pack(push, 1)

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#define DEFAULT_SIZE 1024

struct block
{
    void* next;
    bool is_used;
};

void dump(size_t from, size_t to);
void* alloc(size_t s);
void del(void* p);

#ifdef __cplusplus
}
#endif