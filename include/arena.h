#include "bin.h"

#ifndef __ARENA_H__
#define __ARENA_H__

typedef struct {
    void* arena_start;
    void* arena_end;
    int arena_size;
    bin_t bin_pool[POOL_BIN_SIZE];
} arena_t;

void reset_bin(arena_t* arena);

#endif