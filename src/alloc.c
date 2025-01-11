#include "alloc.h"
#include <stddef.h>


void alloc_init(struct pool* pool) {
    pool->stack_top = ATOMIC_VAR_INIT(POOL_SIZE - 1);
    for (size_t i = 0; i < POOL_SIZE; i++) {
        pool->stack[i] = i;
    }
    return;
}

struct pool_entity* alloc(struct pool *pool) {
    if(pool->stack_top < 0) return NULL;

    return &pool->entities[atomic_fetch_sub(&pool->stack_top, 1)];
}

void dealloc(struct pool *pool, struct pool_entity *entity) {
    int index = ((ptrdiff_t) entity - (ptrdiff_t) pool)/sizeof(struct pool_entity);
    pool->stack[atomic_fetch_add(&pool->stack_top, 1) + 1] = index;
    return;
}