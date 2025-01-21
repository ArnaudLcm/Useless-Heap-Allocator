#include "arena.h"
#include "alloc.h"
#include "stack.h"
#include <pthread.h>
#include <stdlib.h>

#include "bin.h"
#include "list.h"

arena_t global_arena = {
    .bin_pool = {{.stack = {0},
                  .bin_nodes = {INIT_NODE(NULL)},
                  .stack_top = MAX_BIN_SIZE - 1,
                  .free_list = INIT_LIST(NULL, NULL)},
                 {.stack = {0},
                  .bin_nodes = {INIT_NODE(NULL)},
                  .stack_top = MAX_BIN_SIZE - 1,
                  .free_list = INIT_LIST(NULL, NULL)},
                 {.stack = {0},
                  .bin_nodes = {INIT_NODE(NULL)},
                  .stack_top = MAX_BIN_SIZE - 1,
                  .free_list = INIT_LIST(NULL, NULL)}},
    .arena_size = 0,
    .arena_end = NULL,
    .arena_start = NULL,
};


__thread arena_t* local_arena = NULL;


int init_arena(arena_t* arena, ulong arena_size, void* start_addr) {
    arena->arena_start = align_round_chunk(start_addr, ROUND_UP);
    arena->arena_end = start_addr + arena_size;

    arena->arena_size = arena_size;

    arena->arena_start = align_round_chunk(arena->arena_start, ROUND_UP);

    // Init free nodes stacks
    reset_bin(arena);

    struct node *first_node = stack_pop(&arena->bin_pool[2]);

    if (!first_node) return -1;  // No available nodes. This should not happen.

    first_node->data = arena->arena_start;

    add_node(&arena->bin_pool[2].free_list, first_node);

    chunk_metadata_t *c = arena->arena_start;
    c->chunk_state = CHUNK_FREE;
    c->chunk_size = arena->arena_end - arena->arena_start - sizeof(chunk_metadata_t);


    if(arena == &global_arena) {
        pthread_mutex_init(&global_arena.mutex, NULL);
    }

    return 0;
}


void reset_bin(arena_t* arena) {
    for (int i = 0; i < POOL_BIN_SIZE; i++) {
        arena->bin_pool[i] = (bin_t){.stack = {0},
                                     .bin_nodes = {INIT_NODE(NULL)},
                                     .stack_top = MAX_BIN_SIZE - 1,
                                     .free_list = INIT_LIST(NULL, NULL)};
    }
}