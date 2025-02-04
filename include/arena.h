#include <pthread.h>
#include <sys/types.h>

#include "bin.h"
#include "types.h"

#ifndef __ARENA_H__
#define __ARENA_H__

#define ARENA_SIZE 1024 * 1024


typedef struct {
    void* arena_start;
    void* arena_end;
    int arena_size;
    bin_t arena_bin;
} global_arena_t;

typedef struct {
    void* arena_start;
    void* arena_end;
    int arena_size;
    bin_t bin_pool[POOL_BIN_SIZE];
    pthread_mutex_t mutex; // This mutex is used by the global arena to ensure safe arena allocation 
} arena_t;

_Static_assert(ARENA_SIZE > sizeof(arena_t), "Default arena size can't be smaller than the arena struct.");

extern arena_t global_arena;

extern __thread arena_t* local_arena;

int init_arena(arena_t* arena, ulong arena_size, void* start_addr);

void reset_bin(arena_t* arena);

#endif