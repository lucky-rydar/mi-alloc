#include "alloc.h"

size_t free_mem = DEFAULT_SIZE;
size_t used_mem = 0;
size_t mem_size = DEFAULT_SIZE;
uint8_t mem[DEFAULT_SIZE];

// pointer to current offset to free memory
void* using = mem;

// addres of the last element in mem
void* last = mem + DEFAULT_SIZE - 1;

void block_set_zero(block_t* b) {
    void* local_last;
    
    if(b->next == NULL) {
        local_last = mem + DEFAULT_SIZE;
    } else {
        local_last = b->next;
    }

    for(uint8_t* data = (void*)b + SIZEOF_STRUCT_BLOCK;
        data < (uint8_t*)local_last;
        data++) {
        *data = 0;
    }
}

void block_set_next(block_t* b, void* next) {
    if(next == NULL || next > last) {
        b->next = NULL;
    } else {
        b->next = next;
    }
}

// connects all the next block to this if they are free
void select_fist_used_for(block_t* b) {
    if(b->next == NULL)
        return;

    block_t* first_unused = b->next;
    for(;;) {
        if(first_unused == NULL) {
            b->next = NULL;
            return;
        }
        if(!first_unused->is_used)
            first_unused = first_unused->next;
        else
            break;
    }
    b->next = first_unused;
}

#define DATA_BY_BLOCK(b) ((void*)b + SIZEOF_STRUCT_BLOCK)
void* data_by_block(block_t* b) {
    return (void*)b + SIZEOF_STRUCT_BLOCK;
}

/**
 * this function is created to return real next address
 * for example if next is null it must be first address
 * after memory ends
 */
void* get_real_next(block_t* b) {
    if(b->next == NULL) {
        return mem + DEFAULT_SIZE;
    } else {
        return b->next;
    }
}

// size of non block space
#define BLOCK_DATA_SIZE(b) ((size_t)get_real_next(b) - (size_t)data_by_block(b))
size_t block_data_size(block_t* b) {
    return (size_t)get_real_next(b) - (size_t)data_by_block(b);
}

// size of a full block
#define BLOCK_SIZE(b) ((size_t)get_real_next(b) - (size_t)b)
size_t block_size(block_t* b) {
    return (size_t)get_real_next(b) - (size_t)b;
}

void dump(size_t from, size_t to) {
    printf("free: %d\n", free_mem);

    uint8_t* m = mem;

    for(size_t i = from; i < to; i++) {
        printf("%x ",m[i]);
    }
    printf("\n");

    block_t* b = (block_t*)mem;
    for( ; b->next != NULL; ) {
        size_t distance = (size_t)b->next - (size_t)b;
        size_t block_size = SIZEOF_STRUCT_BLOCK;
        printf("%p?%d %d:%d\n", b, b->is_used, distance, distance - block_size);
        b = b->next;   
    }
}

/**
 * So, it is needed to check some situations 
 * 1) [free enough][used]
 *    ^
 * 
 * 2) [free and points to null]
 *    ^
 *
 * get address of next block starts
 *     if it is defined so it is the same
 *         like this:
 *         [block][next block]
 *                ^
 *                just the address of the next block
 * 
 *     else if null it is first not available address
 *         like this:
 *         [allocator memory][other memory]
 *                           ^
 *                           it starts here
 * 
 *
 * After getting next address it is needed to check
 * if it is available to allocate requested memory and
 * block_size + 1 byte.
 * 
 * If it is possible, so create new block and set next
 * to the new free block allocated like this:
 *     before:
 *         [free + block_size * 2 + 1(min)][used]
 *     after:
 *         [requested + block][block_size + 1(min)][used]
 *         ^
 *         return this pointer
 */
void* alloc(size_t s) {
    if(free_mem < s + SIZEOF_STRUCT_BLOCK)
        return NULL;

    if(using + SIZEOF_STRUCT_BLOCK + s >= last) {
        using = mem;
    }
    
    block_t* b = using;
    for(size_t i = 0; i < (DEFAULT_SIZE / MIN_VAR_SIZE) + 1; i++) {
        if(b->is_used) {
            if(b->next != NULL)
                b = b->next;
            else if(b-> next == NULL || (size_t)b->next > (size_t)mem + DEFAULT_SIZE)
                b = (block_t*)mem;
        } else {
            // connect with forward free blocks
            select_fist_used_for(b);

            size_t d_size = BLOCK_DATA_SIZE(b);
            if(d_size == s) {
                // just mark used and return
                b->is_used = true;

                free_mem -= BLOCK_SIZE(b);
                return DATA_BY_BLOCK(b);
            } else if(d_size >= s + MIN_VAR_SIZE) {
                // it is needed to divide the block
                
                // init divided free block
                void* after_next = b->next;
                block_t* divided_free_block = (void*)b + SIZEOF_STRUCT_BLOCK + s;
                divided_free_block->next = after_next;
                divided_free_block->is_used = false;

                // init needed block
                b->is_used = true;
                b->next = divided_free_block;

                free_mem -= BLOCK_SIZE(b);
                return DATA_BY_BLOCK(b);
            } else {
                // size is nought enough
                if(b->next != NULL)
                    b = b->next;
                else if(b-> next == NULL || (size_t)b->next > (size_t)mem + DEFAULT_SIZE)
                    b = (block_t*)mem;
                continue;
            }
        }
    }
    return NULL;
}

void del(void* p) {
    if(p == NULL || p < (void*)mem || p >= (void*)mem + DEFAULT_SIZE) {
        return;
    }

    block_t* b = p - SIZEOF_STRUCT_BLOCK;
    b->is_used = false;
    size_t b_size = BLOCK_SIZE(b);

    select_fist_used_for(b);

    free_mem += b_size;
}

mem_info_t get_mem_info() {
    mem_info_t ret = {
        .free_mem = free_mem,
        .used_mem = used_mem,
        .size = mem_size,
        .mem = mem
    };
    return ret;
}

void clear() {
    for(size_t i = 0; i < DEFAULT_SIZE; i++) {
        mem[i] = 0;
    }
    using = mem;
    free_mem = DEFAULT_SIZE;
}
