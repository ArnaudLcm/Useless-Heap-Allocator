#include <pthread.h>
#include <stdalign.h>
#include <unistd.h>

#include "arena.h"
#include "list.h"
#include "types.h"
#ifndef __ALLOC_H__
#define __ALLOC_H__

#define MASK_CHUNK_SIZE(c) ((c) >> 4)
#define MASK_CHUNK_STATE(c) ((c)&0xF)

#define CHUNK_FREE 0x0
#define CHUNK_USED 0x1

#define MAX_CHUNK_SIZE                                                         \
  ((1 << 28) / 8) // Related to the chunk size definition in chunk_metadata_t

#define ARCH_ALIGNMENT alignof(void *)

typedef struct {
  uint chunk_state : 4;
  uint chunk_size : 28;
  struct node *free_node_ptr;
} __attribute__((packed)) chunk_metadata_t;

enum RoundDirection { ROUND_UP, ROUND_DOWN };


_Static_assert(
    sizeof(chunk_metadata_t) == 12,
    "Chunk metadata size should be 12 byte long."); // Ensure the chunk metadata
                                                    // is 12 byte long

int alloc_init();
void *alloc(ulong size);
void *resize_alloc(void *ptr, ulong new_size);
void *align_address(void *addr, size_t alignment, enum RoundDirection direction);

int dealloc(void *ptr);

#endif