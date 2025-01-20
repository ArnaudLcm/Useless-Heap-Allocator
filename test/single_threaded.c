#include "single_threaded.h"

#include "alloc.h"
#include "test.h"

void test_too_large_chunk_alloc() { ASSERT_TRUE(alloc(1 << 29) == NULL); }

void test_valid_chunk_alloc() {
    ASSERT_TRUE(alloc_init() == 0);
    void* first_alloc = alloc(1 << 4);
    ASSERT_TRUE(first_alloc == global_arena.arena_start + sizeof(chunk_metadata_t));

    void* new_alloc = alloc(1 << 4);

    ASSERT_TRUE(new_alloc == global_arena.arena_start + sizeof(chunk_metadata_t) + (1 << 4) + sizeof(chunk_metadata_t));
}

void test_valid_chunk_alloc_and_dealloc() {
    ASSERT_TRUE(alloc_init() == 0);

    void* new_alloc = alloc(1 << 4);

    ASSERT_TRUE(dealloc(new_alloc) == 0);
    ASSERT_TRUE(dealloc(new_alloc) == -1);  // Check that we can't double free
}

void test_valid_chunk_alloc_and_resize_in_place() {
    ASSERT_TRUE(alloc_init() == 0);

    int* new_alloc = alloc(1 << 4);

    *new_alloc = 1234;

    ASSERT_TRUE(new_alloc != NULL);
    int* realloc = (int*)resize_alloc(new_alloc, 1 << 8);
    ASSERT_TRUE(new_alloc == realloc); // Check we did a resize in place
    ASSERT_TRUE(*new_alloc == *realloc);
}


void test_valid_chunk_alloc_and_resize_with_copy() {
    ASSERT_TRUE(alloc_init() == 0);

    int* new_alloc = (int*)alloc(1 << 4);

    *new_alloc = 1234;

    int* second_alloc = (int*)alloc(1 << 4);

    *second_alloc = 1234; 

    int* third_alloc = alloc(1 << 4);


    ASSERT_TRUE(new_alloc != NULL);
    ASSERT_TRUE(second_alloc != NULL);
    ASSERT_TRUE(third_alloc != NULL);
    int* realloc = (int*)resize_alloc(second_alloc, 1 << 8);
    ASSERT_TRUE(new_alloc != realloc); // Check we have not done a resize in place
    ASSERT_TRUE(*new_alloc == *realloc);
}


void test_alloc_chunk_with_padding() {
    ASSERT_TRUE(alloc_init() == 0);
    void* new_alloc = alloc(1 << 2);
    void* second_alloc = alloc(1 << 4);
    ASSERT_TRUE(((uintptr_t)second_alloc & (ARCH_ALIGNMENT - 1)) == 0);
}

void test_alloc_chunk_with_coalesce() {
    ASSERT_TRUE(alloc_init() == 0);
    void* new_alloc = alloc(1 << 4);
    void* second_alloc = alloc(1 << 4);
    void* third_alloc = alloc(1 << 4);
    void* fourth_alloc = alloc(1 << 4);
    ASSERT_TRUE(dealloc(third_alloc) == 0);
    ASSERT_TRUE(dealloc(second_alloc) == 0);
    chunk_metadata_t* second_metadata = (chunk_metadata_t*)(second_alloc - sizeof(chunk_metadata_t));
    ASSERT_TRUE(second_metadata->chunk_size == ((1 << 4) + (1 << 4) + sizeof(chunk_metadata_t)))
}

void test_single_threaded_batch() {
    test_too_large_chunk_alloc();
    test_valid_chunk_alloc();
    test_valid_chunk_alloc_and_dealloc();
    test_alloc_chunk_with_padding();
    test_alloc_chunk_with_coalesce();
    test_valid_chunk_alloc_and_resize_in_place();
    test_valid_chunk_alloc_and_resize_with_copy();
}