#include "alloc.h"
#include "test.h"
#include "single_threaded.h"
#include <alloca.h>

void test_too_large_chunk_alloc() {
    ASSERT_TRUE(alloc(1 << 29) == NULL);
}

void test_simple_chunk_allo() {
    ASSERT_TRUE(alloc(1 << 4) != NULL);

}

void test_single_threaded_batch() {
    ASSERT_TRUE(alloc_init() == 0);
    test_too_large_chunk_alloc();
}