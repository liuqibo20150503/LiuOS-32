#include "kernel.h"

#define HEAP_START 0x200000
#define HEAP_SIZE  2 * 1024 * 1024

typedef struct block {
    uint32_t size;
    uint8_t free;
    struct block* next;
} block_t;

static block_t* heap = (block_t*)HEAP_START;

void mem_init(void) {
    heap->size = HEAP_SIZE;
    heap->free = 1;
    heap->next = NULL;
}

void* kmalloc(uint32_t size) {
    if (!size) return NULL;
    size = (size + 7) & ~7;
    block_t* c = heap;
    while (c) {
        if (c->free && c->size >= size + sizeof(block_t)) {
            if (c->size >= size + sizeof(block_t) + 16) {
                block_t* nb = (block_t*)((uint8_t*)c + sizeof(block_t) + size);
                nb->size = c->size - size - sizeof(block_t);
                nb->free = 1;
                nb->next = c->next;
                c->next = nb;
                c->size = size;
            }
            c->free = 0;
            return (void*)((uint8_t*)c + sizeof(block_t));
        }
        c = c->next;
    }
    return NULL;
}

void kfree(void* p) {
    if (!p) return;
    block_t* h = (block_t*)((uint8_t*)p - sizeof(block_t));
    h->free = 1;
    if (h->next && h->next->free) {
        h->size += sizeof(block_t) + h->next->size;
        h->next = h->next->next;
    }
}