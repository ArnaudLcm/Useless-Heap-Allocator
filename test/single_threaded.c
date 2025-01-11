#include "alloc.h"
#include "single_threaded.h"
#include "test.h"
#include <stdio.h>
#include <stdlib.h>

void test_alloc_full_size() {
    struct pool pool;
    alloc_init(&pool);
    for (int i = 0; i < POOL_SIZE; i++) {
       alloc(&pool);
    }

    ASSERT_TRUE(alloc(&pool) == NULL);

}

void test_alloc_dealloc_mechanism() {
    struct pool pool;
    struct pool_entity* entities[POOL_SIZE];
    alloc_init(&pool);
    for (int i = 0; i < POOL_SIZE; i++) {
       entities[i] = alloc(&pool);
    }

    for (int i = 0; i < POOL_SIZE; i++) {
        dealloc(&pool, entities[i]);
    }

    for (int i = 0; i < POOL_SIZE; i++) {
        ASSERT_TRUE(pool.stack[i] == POOL_SIZE-1-i)
    }
}


void test_single_threaded_batch() {
    test_alloc_full_size();
    test_alloc_dealloc_mechanism();
}