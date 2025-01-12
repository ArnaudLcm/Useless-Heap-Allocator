#include "single_threaded.h"

#include "alloc.h"
#include "test.h"

void test_too_large_chunk_alloc() { ASSERT_TRUE(alloc(1 << 29) == NULL); }

void test_valid_chunk_alloc() {
    ASSERT_TRUE(alloc(1 << 4) == heap_global.heap_start + sizeof(chunk_metadata_t));

    void* new_alloc = alloc(1 << 4);
    ASSERT_TRUE(new_alloc == heap_global.heap_start + sizeof(chunk_metadata_t) + (1 << 4) + sizeof(chunk_metadata_t));
}

void test_valid_chunk_alloc_and_dealloc() {
    void* new_alloc = alloc(1 << 4);

    ASSERT_TRUE(dealloc(new_alloc) == 0);

    ASSERT_TRUE(dealloc(new_alloc) == -1);  // Check that we can't double free
}

void test_alloc_chunk_with_padding() {
    void* new_alloc = alloc(1 << 2);
    void* second_alloc = alloc(1 << 4);
    ASSERT_TRUE(((uintptr_t)second_alloc & (ARCH_ALIGNMENT - 1)) == 0);
}

void test_single_threaded_batch() {
    ASSERT_TRUE(alloc_init() == 0);
    test_too_large_chunk_alloc();
    test_valid_chunk_alloc();
    test_valid_chunk_alloc_and_dealloc();
    test_alloc_chunk_with_padding();
}