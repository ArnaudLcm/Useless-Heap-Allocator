#include <stdatomic.h>


#ifndef __ALLOC_H__
#define __ALLOC_H__

#define POOL_SIZE 20
struct pool_entity {
    int field;
};


struct pool {
    struct pool_entity entities[POOL_SIZE];
    atomic_int stack_top;
    int stack[POOL_SIZE];
};

void alloc_init(struct pool* pool);
struct pool_entity* alloc(struct pool* pool);

void dealloc(struct pool* pool, struct pool_entity* entity);

#endif