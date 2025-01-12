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

#define MAX_CHUNK_SIZE ((1 << 28) / 8)

#define MAX_BIN_SIZE 128  // Maximum number of nodes in the bin

#define BITMAP_SIZE ((MAX_BIN_SIZE + 7) / 8)

typedef struct {
    void* heap_start;
    void* heap_end;
} heap_t;

typedef struct {
    uint chunk_state : 4;
    uint chunk_size : 28;
} __attribute__((packed)) chunk_metadata_t;


static struct node bin_nodes[MAX_BIN_SIZE] = {INIT_NODE(NULL)};

static uchar node_bitmap[BITMAP_SIZE] = {0};

static struct list bin = INIT_LIST(NULL, NULL);

extern heap_t heap_global;


_Static_assert(sizeof(chunk_metadata_t) == 4,
              "Chunk metadata size should be 4 byte long.");  // Ensure the chunk metadata is 4 byte long

int alloc_init();
void* alloc(ulong size);
int resize_alloc(void* ptr, ulong new_size);


int dealloc(void* ptr);

#endif