#include <assert.h>
#include "types.h"
#ifndef __ALLOC_H__
#define __ALLOC_H__

#define MASK_CHUNK_SIZE(c) ((c) >> 4)
#define MASK_CHUNK_STATE(c) ((c) & 0xF)

// Alignment required for all chunks
#define ALIGNMENT_CHUNK 8


typedef struct {
    uint chunk_state : 4;
    uint chunk_size : 28;
} __attribute__((packed)) chunk_metadata_t;


static_assert(sizeof(chunk_metadata_t) == 4, "Chunk metadata size should be 4 byte long."); // Ensure the chunk metadata is 4 byte long

int alloc_init();
void* alloc();

int dealloc(void* ptr);

#endif