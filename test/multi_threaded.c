#include "multi_threaded.h"

#include "alloc.h"
#include "test.h"

static void* thread_pool_alloc(void* args) {
    void* new_alloc = alloc((1 << 4));

    ASSERT_TRUE(new_alloc != NULL);

    int* allocated_int = (int*)new_alloc;
    *allocated_int = 1234;

    ASSERT_TRUE(*allocated_int == 1234);

}
static void test_concurrent_allocs() {
    pthread_t threads[5];
    for (size_t i = 0; i < 5; i++) {
        pthread_create(&threads[i], NULL, thread_pool_alloc, NULL);
    }

    for (size_t i = 0; i < 5; i++) {
        pthread_join(threads[i], NULL);
    }
}

void test_multi_threaded_batch() { test_concurrent_allocs(); }