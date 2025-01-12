#include <assert.h>
#include <stdint.h>
#include <unistd.h>

#include "types.h"
#ifndef __ALLOC_H__
#define __ALLOC_H__

#define MASK_CHUNK_SIZE(c) ((c) >> 4)
#define MASK_CHUNK_STATE(c) ((c) & 0xF)

#define INIT_HEAP_SIZE 102

#define MAX_CHUNK_SIZE 33554432  // 2**28 / 8

#define MAX_BIN_SIZE 128  // Maximum number of nodes in the bin

#define BITMAP_SIZE ((MAX_BIN_SIZE + 7) / 8)

typedef struct {
    uintptr_t heap_start;
    uintptr_t heap_end;
} heap_t;

typedef struct {
    uint chunk_state : 4;
    uint chunk_size : 28;
} __attribute__((packed)) chunk_metadata_t;

static_assert(sizeof(chunk_metadata_t) == 4,
              "Chunk metadata size should be 4 byte long.");  // Ensure the chunk metadata is 4 byte long

int alloc_init(heap_t* heap);
void* alloc(heap_t* heap);

int dealloc(heap_t* heap, void* ptr);

#endif