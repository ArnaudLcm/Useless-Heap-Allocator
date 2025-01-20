#include <pthread.h>
#include <stdalign.h>
#include <stdlib.h>
#include <unistd.h>

#include "list.h"
#include "types.h"
#ifndef __ALLOC_H__
#define __ALLOC_H__

#define MASK_CHUNK_SIZE(c) ((c) >> 4)
#define MASK_CHUNK_STATE(c) ((c) & 0xF)

#define CHUNK_FREE 0x0
#define CHUNK_USED 0x1

#define INIT_HEAP_SIZE 0x1000

#define MAX_CHUNK_SIZE ((1 << 28) / 8) // Related to the chunk size definition in chunk_metadata_t

#define MAX_BIN_SIZE 128  // Maximum number of nodes in the bin

#define ARCH_ALIGNMENT alignof(void*)

typedef struct {
    void* heap_start;
    void* heap_end;
    int heap_size;
    pthread_mutex_t mutex;
} heap_t;

typedef struct {
    uint chunk_state : 4;
    uint chunk_size : 28;
    struct node* free_node_ptr;
} __attribute__((packed)) chunk_metadata_t;

typedef struct {
    int stack[MAX_BIN_SIZE];
    int stack_top;
    struct node bin_nodes[MAX_BIN_SIZE];
    struct list free_list;
} bin_t;

#define POOL_BIN_SIZE 3

#define RESET_BIN_POOL() \
    do { \
        for (int i = 0; i < POOL_BIN_SIZE; i++) { \
            bin_pool[i] = (bin_t){ .stack = {0}, .bin_nodes = {INIT_NODE(NULL)}, .stack_top = MAX_BIN_SIZE - 1, .free_list = INIT_LIST(NULL, NULL) }; \
        } \
    } while (0)

static bin_t bin_pool[POOL_BIN_SIZE] = {
    { .stack = {0}, .bin_nodes = {INIT_NODE(NULL)}, .stack_top = MAX_BIN_SIZE - 1, .free_list = INIT_LIST(NULL, NULL) },
    { .stack = {0}, .bin_nodes = {INIT_NODE(NULL)}, .stack_top = MAX_BIN_SIZE - 1, .free_list = INIT_LIST(NULL, NULL) },
    { .stack = {0}, .bin_nodes = {INIT_NODE(NULL)}, .stack_top = MAX_BIN_SIZE - 1, .free_list = INIT_LIST(NULL, NULL) }
};

extern heap_t heap_global;

_Static_assert(sizeof(chunk_metadata_t) == 12,
               "Chunk metadata size should be 12 byte long.");  // Ensure the chunk metadata is 12 byte long

int alloc_init();
void* alloc(ulong size);
int resize_alloc(void* ptr, ulong new_size);

int dealloc(void* ptr);

#endif