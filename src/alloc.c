#include "alloc.h"

size_t free_mem = DEFAULT_SIZE;
size_t used_mem = 0;
size_t mem_size = DEFAULT_SIZE;
uint8_t mem[DEFAULT_SIZE];
void* using = mem;
void* last = mem + DEFAULT_SIZE;

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
    
    struct block* b = using;
    struct block* prev;
    for(size_t i = 0; i < DEFAULT_SIZE / (sizeof(struct block) + 1) + 1; i++) {
        if(b->is_used) {
            prev = b;
            b = b->next;
        } else {
            if(b->next == NULL) {
                b->is_used = true;
                b->next = (void*)b + sizeof(struct block) + s;
                struct block* next = (void*)b->next;
                next->is_used = false;
                next->next = NULL;

                free_mem -= s + sizeof(struct block);
                return (void*)b + sizeof(struct block);
            } else {
                // so there is used memory next
                
                // before seeing how much memory it is
                // connect all the free memory blocks after this
                // in one to allocate memory for this one

                size_t to_next = (void*)b->next - (void*)b;
                if(to_next > sizeof(struct block) * 2 + s + 1) {
                    // there are enought space to allocate
                    // memory for current request and +1 byte
                    
                    // save pointer to the next block
                    void* next_after_free = b->next;

                    // allocate memory for this request
                    b->is_used = true;
                    b->next = (void*)b + sizeof(struct block) + s;

                    // init next block of free memory between current and next
                    struct block* next = (void*)b->next;
                    next->is_used = false;
                    next->next = next_after_free;

                    free_mem -= ((size_t)next - (size_t)b);
                    return (void*)b + sizeof(struct block);
                } else {
                    void* next_after_free = b->next;
                    b->is_used = true;

                    free_mem -= ((size_t)b->next - (size_t)b);
                    return (void*)b + sizeof(struct block) ;
                }
            }
            
        }
    }

    return NULL;
}

void del(void* p) {
    struct block* b = p - sizeof(struct block);
    struct block* next = b->next;
    if(next->is_used) {
        b->is_used = false;
        
        // set 0 to every byte in  this block
        for(uint8_t* data = (void*)b + sizeof(struct block);
            data != b->next;
            data++) {
            *data = 0;
        }
    } else {
        void* after_next = next->next;
        void *midle_next = b->next;
        b->is_used = false;
        b->next = after_next;

        for(uint8_t* data = (void*)b + sizeof(struct block);
            data != midle_next;
            data++) {
            *data = 0;
        }
    }
}