#include "alloc.h"

#include <pthread.h>
#include <stddef.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <threads.h>
#include <stdint.h>

#include "list.h"
#include "log.h"
#include "stack.h"


static int get_bin_index_from_size(ulong size) {
    switch (size) {
        case 512:
            return 0;
        case 1024:
            return 1;
        default:
            return 2;
    }
}
/*
 * @brief: This function will ensure every chunks are 4 byte aligned. By default, round it up
 */
void *align_round_chunk(void *addr, enum RoundDirection direction) {
    uintptr_t address = (uintptr_t)addr;

    // If the address is already aligned, return it as is
    if ((address & (ARCH_ALIGNMENT - 1)) == 0) {
        return addr;
    }

    switch (direction) {
        case ROUND_DOWN:
            return (void *)(address & ~(ARCH_ALIGNMENT - 1));
        case ROUND_UP:
        default:
            return (void *)((address + ARCH_ALIGNMENT - 1) & ~(ARCH_ALIGNMENT - 1));
    }
}

int alloc_init() {
    if (global_arena.arena_end != NULL) {
        munmap(global_arena.arena_start, global_arena.arena_size);
        global_arena.arena_start = NULL;
        global_arena.arena_end = NULL;
        local_arena = NULL;
    }

    void *global_arena_start =
        mmap((void *)sbrk(0), INIT_HEAP_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (global_arena_start == MAP_FAILED) {
        return -1;
    }

    return init_arena(&global_arena, INIT_HEAP_SIZE, global_arena_start);
}

static int shrink_chunk(chunk_metadata_t *metadata, ulong total_size, bin_t *bin, arena_t *arena) {
    unsigned long remaining_size = metadata->chunk_size - total_size - sizeof(chunk_metadata_t);
    if (remaining_size > sizeof(chunk_metadata_t)) {
        chunk_metadata_t *new_metadata = (void *)metadata + total_size + sizeof(chunk_metadata_t);
        new_metadata->chunk_size = remaining_size;
        new_metadata->chunk_state = CHUNK_FREE;

        int receiver_bin_index = get_bin_index_from_size(remaining_size - sizeof(chunk_metadata_t));
        bin_t *receiver_bin = &arena->bin_pool[receiver_bin_index];

        if (receiver_bin->stack_top < 0) {
            receiver_bin = &arena->bin_pool[2];
        }

        struct node *new_node = stack_pop(receiver_bin);
        if (!new_node) {
            return -1;
        }
        new_node->data = new_metadata;
        add_node(&bin->free_list, new_node);
        new_metadata->free_node_ptr = new_node;
    }

    return 0;
}

static void *alloc_with_arena(unsigned long size, arena_t *arena) {
    size = (size + ARCH_ALIGNMENT - 1) & ~(ARCH_ALIGNMENT - 1);

    if (size > MAX_CHUNK_SIZE) {
        return NULL;
    }

    // Find the appropriate bin to handle the user request (i.e small, medium or unsorted bins)
    int bin_index = get_bin_index_from_size(size);

    if (arena->bin_pool[bin_index].stack_top <
        0) {  // Case where the appropriate bin is empty (i.e no free chunk availble)
        bin_index = 2;
    }

    bin_t *bin = &arena->bin_pool[bin_index];

    // Find the smallest suitable free node in case of unsorted bin, otherwise take the first free node
    struct node *smallest_free_node = NULL;
    chunk_metadata_t *smallest_metadata = NULL;

    for (struct node *current = bin->free_list.head; current; current = current->next) {
        chunk_metadata_t *metadata = current->data;
        if (metadata && metadata->chunk_size >= size) {
            if (!smallest_free_node || metadata->chunk_size < smallest_metadata->chunk_size) {
                smallest_free_node = current;
                smallest_metadata = metadata;
            }
        }
    }

    if (!smallest_free_node || !smallest_metadata) {
        return NULL;  // No suitable chunk found
    }

    remove_node(&bin->free_list, smallest_free_node);
    stack_push(smallest_free_node, bin);
    smallest_metadata->free_node_ptr = NULL;

    smallest_metadata->chunk_state = CHUNK_USED;

    if (bin_index == 2) {  // Case: Unsorted bin
        // Check if splitting the chunk is necessary
        if (shrink_chunk(smallest_metadata, size, bin, arena) < 0) {
            return NULL;
        }
    }

    smallest_metadata->chunk_size = size;

    void *new_alloc = (void *)smallest_metadata + sizeof(chunk_metadata_t);
    return new_alloc;
}

static void *alloc_local_arena() {

    pthread_mutex_lock(&global_arena.mutex);
    void* local_arena_addr = alloc_with_arena(ARENA_SIZE, &global_arena);
    pthread_mutex_unlock(&global_arena.mutex);
    local_arena = (arena_t*) local_arena_addr;
    if (local_arena_addr != NULL && init_arena(local_arena, ARENA_SIZE, (void*)local_arena + sizeof(arena_t)) == 0) {
        return local_arena;
    }

    return NULL;
}

void *alloc(unsigned long size) {
    if (global_arena.arena_start == NULL) {  // Heap was not initialized
        return NULL;
    }  

    if (local_arena == NULL && alloc_local_arena() == NULL) {
        return NULL;
    }
    return alloc_with_arena(size, local_arena);
}

/**
 * @brief: Merge free chunks
 */
static int coalesce(struct node *node, arena_t *arena) {
    chunk_metadata_t *start_metadata = node->data;
    if (!start_metadata) {
        return -1;
    }
    chunk_metadata_t *next_metadata =
        (chunk_metadata_t *)((void *)start_metadata + start_metadata->chunk_size + sizeof(chunk_metadata_t));

    int total_new_chunk_size = start_metadata->chunk_size;
    while ((void *)next_metadata < arena->arena_end && next_metadata && next_metadata->chunk_state == CHUNK_FREE) {
        total_new_chunk_size += next_metadata->chunk_size + sizeof(chunk_metadata_t);

        int owner_bin_index = get_bin_index_from_size(next_metadata->chunk_size);

        bin_t *owner_bin = &arena->bin_pool[owner_bin_index];

        remove_node(&owner_bin->free_list, next_metadata->free_node_ptr);
        stack_push(next_metadata->free_node_ptr, owner_bin);
        next_metadata = (void *)next_metadata + next_metadata->chunk_size + sizeof(chunk_metadata_t);
    }

    start_metadata->chunk_size = total_new_chunk_size;
    return 0;
}

int dealloc(void *ptr) {
    chunk_metadata_t *chunk_metadata = ptr - sizeof(chunk_metadata_t);

    if (!ptr || (void *)chunk_metadata > global_arena.arena_end || (void *)chunk_metadata < global_arena.arena_start ||
        chunk_metadata->chunk_state != CHUNK_USED) {
        return -1;
    }

    int bin_index = get_bin_index_from_size(chunk_metadata->chunk_size);

    bin_t *bin = &global_arena.bin_pool[bin_index];

    while (bin->stack_top < 0) {  // if the sorted bin is full, default to the unsorted one
        bin_index = 2;
    }

    bin = &global_arena.bin_pool[bin_index];

    struct node *new_node = stack_pop(bin);
    if (new_node == NULL) {
        return -1;
    }

    chunk_metadata->chunk_state = CHUNK_FREE;
    chunk_metadata->free_node_ptr = new_node;
    new_node->data = chunk_metadata;

    add_node(&bin->free_list, new_node);

    return coalesce(new_node, local_arena);
}

void *resize_alloc(void *ptr, ulong new_size) {
    if (ptr == NULL) {
        return NULL;
    }

    if (new_size == 0) {
        dealloc(ptr);
        return NULL;
    }

    chunk_metadata_t *chunk_metadata = ptr - sizeof(chunk_metadata_t);
    chunk_metadata_t *next_chunk_metadata = ptr + chunk_metadata->chunk_size;
    int curr_size = chunk_metadata->chunk_size;

    // First case: Try to resize in place
    while (curr_size < new_size && next_chunk_metadata->chunk_state == CHUNK_FREE) {
        curr_size += next_chunk_metadata->chunk_size;
        next_chunk_metadata = next_chunk_metadata + next_chunk_metadata->chunk_size;
    }

    if (curr_size >= new_size) {  // We can resize in place
        dealloc(ptr);             // We set the node to free and coalesce
        // Find the appropriate bin to handle the user request (i.e small, medium or unsorted bins)
        int bin_index = get_bin_index_from_size(new_size);

        if (global_arena.bin_pool[bin_index].stack_top <
            0) {  // Case where the appropriate bin is empty (i.e no free chunk availble)
            bin_index = 2;
        }

        bin_t *bin = &global_arena.bin_pool[bin_index];

        remove_node(&bin->free_list, chunk_metadata->free_node_ptr);
        stack_push(chunk_metadata->free_node_ptr, bin);
        chunk_metadata->free_node_ptr = NULL;

        chunk_metadata->chunk_state = CHUNK_USED;

        // Check if splitting the chunk is necessary
        if (shrink_chunk(chunk_metadata, new_size, bin, local_arena) < 0) {
            return NULL;
        }

        chunk_metadata->chunk_size = new_size;

        return (void *)chunk_metadata + sizeof(chunk_metadata_t);
    }

    // We need to copy the content of the node

    void *new_alloc = alloc(new_size);

    if (new_alloc == NULL) {  // Allocation failed
        return NULL;
    }

    memcpy(new_alloc, ptr, chunk_metadata->chunk_size);

    dealloc(ptr);

    return new_alloc;
}