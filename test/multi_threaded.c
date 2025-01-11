#include "alloc.h"
#include "test.h"
#include "multi_threaded.h"
#include <bits/pthreadtypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

static void* thread_pool_alloc(void* args) {
    alloc((struct pool*) args);
}


void test_multi_threaded_batch() {
    pthread_t threads[POOL_SIZE];
    struct pool pool;
    alloc_init(&pool);
    for (size_t i = 0; i < POOL_SIZE; i++) {
        pthread_create(&threads[i], NULL, thread_pool_alloc, &pool);
    }

    for (size_t i = 0; i < POOL_SIZE; i++) {
        pthread_join(threads[i], NULL);
    }

    ASSERT_TRUE(pool.stack_top == -1);
}