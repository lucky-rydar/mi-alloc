#include "alloc.h"

size_t free_mem = DEFAULT_SIZE;
size_t used_mem = 0;
size_t mem_size = DEFAULT_SIZE;
uint8_t mem[DEFAULT_SIZE];

// pointer to current offset to free memory
void* using = mem;

// addres of the last element in mem
void* last = mem + DEFAULT_SIZE - 1;

void block_set_zero(struct block* b) {
    void* local_last;
    
    if(b->next == NULL) {
        local_last = last; 
    } else {
        local_last = b->next;
    }

    for(uint8_t* data = (void*)b + sizeof(struct block);
        data != local_last;
        data++) {
        *data = 0;
    }
}

void block_set_next(struct block* b, void* next) {
    if(next == NULL || next > last) {
        b->next = NULL;
    } else {
        b->next = next;
    }
}

// connects all the next block to this if they are free
void select_fist_used_for(struct block* b) {
    struct block* first_unused = b->next;
    for(;;) {
        if(first_unused->next != NULL) {
            if(first_unused->is_used == false) {
                first_unused = first_unused->next;
            } else {
                b->next = first_unused;
                b->is_used = false;
                return;
            }
        } else {
            b->next = NULL;
            b->is_used = false;
            return;
        }
    }
    b->next = first_unused;
}

void* data_by_block(struct block* b) {
    return (void*)b + sizeof(struct block);
}

// size of non block space
size_t block_data_size(struct block* b) {
    return (size_t)b->next - (size_t)data_by_block(b);
}

// size of a full block
size_t block_size(struct block* b) {
    return (size_t)b->next - (size_t)b;
}

void dump(size_t from, size_t to) {
    printf("free: %d\n", free_mem);

    uint8_t* m = mem;

    for(size_t i = from; i < to; i++) {
        printf("%x ",m[i]);
    }
    printf("\n");

    struct block* b = (struct block*)mem;
    for( ; b->next != NULL; ) {
        size_t distance = (size_t)b->next - (size_t)b;
        size_t block_size = sizeof(struct block);
        printf("%p?%d %d:%d\n", b, b->is_used, distance, distance - block_size);
        b = b->next;   
    }
}

void* alloc(size_t s) {
    if(free_mem < s + sizeof(struct block))
        return NULL;

    if(using + sizeof(struct block) + s > last) {
        using = mem;
    }
    
    struct block* b = using;
    for(size_t i = 0; i < DEFAULT_SIZE / (sizeof(struct block) + 1) + 1; i++) {
        if(b->is_used) {
            b = b->next;
            continue;
        } else if(!b->is_used) {
            if(b->next == NULL && block_data_size(b) >= s) {
                b->is_used = true;
                
                //block_set_next(b, (void*)b + sizeof(struct block) + s);
                b->next = (void*)b + sizeof(struct block) + s;
                
                struct block* next = (void*)b->next;
                next->is_used = false;

                //block_set_next(next, NULL);
                next->next = NULL;
                
                free_mem -= s + sizeof(struct block);
                using = (void*)b + sizeof(struct block) + s;
                return data_by_block(b);
            } else if(b->next != NULL) {
                // there are block after this
                select_fist_used_for(b);
                block_set_zero(b);

                size_t b_size = block_size(b);
                size_t d_size = block_data_size(b);
                if(d_size < s) {
                    b = b->next;
                    continue;
                }

                if(b_size > sizeof(struct block) * 2 + s + 1) {
                    // there are enought space to allocate
                    // memory for current request and +1 byte
                    
                    // save pointer to the next block
                    void* next_after_free = b->next;

                    // allocate memory for this request
                    b->is_used = true;
                    //block_set_next(b, (void*)b + sizeof(struct block) + s);
                    b->next = (void*)b + sizeof(struct block) + s;

                    // init next block of free memory between current and next
                    struct block* next = (void*)b->next;
                    next->is_used = false;
                    //block_set_next(next, next_after_free);
                    next->next = next_after_free;
                    
                    free_mem -= ((size_t)next - (size_t)b);
                    return data_by_block(b);
                } else if(b_size < sizeof(struct block) * 2 + s + 1 && b_size >= sizeof(struct block) + s){
                    void* next_after_free = b->next;
                    b->is_used = true;

                    free_mem -= ((size_t)b->next - (size_t)b);
                    return data_by_block(b);
                }

            }
        }
    }
    return NULL;
}

void del(void* p) {
    if(p == NULL || p < (void*)mem || p > last) {
        return;
    }

    struct block* b = p - sizeof(struct block);
    size_t b_size = block_size(b);

    struct block* next = b->next;
    if(next->is_used) {
        b->is_used = false;
        block_set_zero(b);
    } else {
        select_fist_used_for(b);
        block_set_zero(b);
    }

    free_mem += b_size;
}

struct mem_info get_mem_info() {
    struct mem_info ret = {
        .free_mem = free_mem,
        .used_mem = used_mem,
        .size = mem_size,
        .mem = mem
    };
    return ret;
}