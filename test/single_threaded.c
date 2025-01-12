#include "single_threaded.h"

#include "alloc.h"
#include "test.h"

void test_too_large_chunk_alloc() { ASSERT_TRUE(alloc(1 << 29) == NULL); }

void test_chunk_allocs_and_dealloc() {
    ASSERT_TRUE(alloc(1 << 4) == heap_global.heap_start + sizeof(chunk_metadata_t));

    void* new_alloc = alloc(1 << 4);
    ASSERT_TRUE(new_alloc == heap_global.heap_start + sizeof(chunk_metadata_t) + (1 << 4) + sizeof(chunk_metadata_t));

    ASSERT_TRUE(dealloc(new_alloc) == 0);
}

void test_single_threaded_batch() {
    ASSERT_TRUE(alloc_init() == 0);
    test_too_large_chunk_alloc();
    test_chunk_allocs_and_dealloc();
}