#include "bin.h"
#include "arena.h"
#include "list.h"
#include <stdlib.h>



void reset_bin(arena_t* arena) {
    for (int i = 0; i < POOL_BIN_SIZE; i++) {                                  \
      arena->bin_pool[i] = (bin_t){.stack = {0},                                      \
                            .bin_nodes = {INIT_NODE(NULL)},                    \
                            .stack_top = MAX_BIN_SIZE - 1,                     \
                            .free_list = INIT_LIST(NULL, NULL)};               \
    }           
}