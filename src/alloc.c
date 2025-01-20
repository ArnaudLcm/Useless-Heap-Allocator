#include "alloc.h"

#include <pthread.h>
#include <stddef.h>
#include <sys/mman.h>
#include <sys/types.h>

#include "list.h"
#include "log.h"

heap_t heap_global = {.heap_end = NULL, .heap_start = NULL, .heap_size = 0};

enum RoundDirection { ROUND_UP, ROUND_DOWN };

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

static struct node *stack_pop(bin_t *bin) {
    if (bin->stack_top < 0) {
        return NULL;  // No free node available
    }
    int index = bin->stack[bin->stack_top--];
    if (index >= 0 && index < MAX_BIN_SIZE) {
        return &bin->bin_nodes[index];
    }

    return NULL;
}

static void stack_push(struct node *n, bin_t *bin) {
    int index =
        ((ptrdiff_t)n - (ptrdiff_t)bin->bin_nodes) / (sizeof(struct node));  // Calculate index of the node in the array
    if (index >= 0 && index < MAX_BIN_SIZE) {
        bin->stack[++bin->stack_top] = index;
    }
}
/*
 * @brief: This function will ensure every chunks are 4 byte aligned. By default, round it up
 */
static inline void *align_round_chunk(void *addr, enum RoundDirection direction) {
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
    if (heap_global.heap_end != NULL) {
        munmap(heap_global.heap_start, heap_global.heap_size);
        heap_global.heap_start = NULL;
        heap_global.heap_end = NULL;
    }
    pthread_mutex_init(&heap_global.mutex, NULL);

    heap_global.heap_start =
        mmap((void *)sbrk(0), INIT_HEAP_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (heap_global.heap_start == MAP_FAILED) {
        return -1;
    }

    heap_global.heap_end = (void *)heap_global.heap_start + INIT_HEAP_SIZE;

    heap_global.heap_size = INIT_HEAP_SIZE;

    heap_global.heap_start = align_round_chunk(heap_global.heap_start, ROUND_UP);

    // Init free nodes stacks
    RESET_BIN_POOL();

    struct node *first_node = stack_pop(&bin_pool[2]);

    if (!first_node) return -1;  // No available nodes. This should not happen.

    first_node->data = heap_global.heap_start;

    add_node(&bin_pool[2].free_list, first_node);

    chunk_metadata_t *c = heap_global.heap_start;
    c->chunk_state = CHUNK_FREE;
    c->chunk_size = heap_global.heap_end - heap_global.heap_start - sizeof(chunk_metadata_t);

    log_debug("Initialized heap with size %d at start %p", c->chunk_size, heap_global.heap_start);
    return 0;
}

void *alloc(unsigned long size) {
    pthread_mutex_lock(&heap_global.mutex);
    size = (size + ARCH_ALIGNMENT - 1) & ~(ARCH_ALIGNMENT - 1);

    if (size > MAX_CHUNK_SIZE) {
        return NULL;
    }

    // Find the appropriate bin to handle the user request (i.e small, medium or unsorted bins)
    int bin_index = get_bin_index_from_size(size);

    if (bin_pool[bin_index].stack_top < 0) {  // Case where the appropriate bin is empty (i.e no free chunk availble)
        bin_index = 2;
    }

    bin_t *bin = &bin_pool[bin_index];

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
        unsigned long remaining_size = smallest_metadata->chunk_size - size - sizeof(chunk_metadata_t);
        if (remaining_size > sizeof(chunk_metadata_t)) {
            chunk_metadata_t *new_metadata = (void *)smallest_metadata + size + sizeof(chunk_metadata_t);
            new_metadata->chunk_size = remaining_size;
            new_metadata->chunk_state = CHUNK_FREE;

            int receiver_bin_index = get_bin_index_from_size(remaining_size - sizeof(chunk_metadata_t));
            bin_t *receiver_bin = &bin_pool[receiver_bin_index];

            if (receiver_bin->stack_top < 0) {
                receiver_bin = &bin_pool[2];
            }

            struct node *new_node = stack_pop(receiver_bin);
            if (!new_node) {
                return NULL;
            }
            new_node->data = new_metadata;
            add_node(&bin->free_list, new_node);
            new_metadata->free_node_ptr = new_node;
        }
    }

    smallest_metadata->chunk_size = size;
    pthread_mutex_unlock(&heap_global.mutex);

    return (void *)smallest_metadata + sizeof(chunk_metadata_t);
}

/**
 * @brief: Merge free chunks
 */
static int coalesce(struct node *node) {
    chunk_metadata_t *start_metadata = node->data;
    if (!start_metadata) {
        return -1;
    }
    chunk_metadata_t *next_metadata =
        (chunk_metadata_t *)((void *)start_metadata + start_metadata->chunk_size + sizeof(chunk_metadata_t));

    int total_new_chunk_size = start_metadata->chunk_size;
    while ((void *)next_metadata < heap_global.heap_end && next_metadata && next_metadata->chunk_state == CHUNK_FREE) {
        total_new_chunk_size += next_metadata->chunk_size + sizeof(chunk_metadata_t);

        int owner_bin_index = get_bin_index_from_size(next_metadata->chunk_size);

        bin_t *owner_bin = &bin_pool[owner_bin_index];

        remove_node(&owner_bin->free_list, next_metadata->free_node_ptr);
        stack_push(next_metadata->free_node_ptr, owner_bin);
        next_metadata = (void *)next_metadata + next_metadata->chunk_size + sizeof(chunk_metadata_t);
    }

    start_metadata->chunk_size = total_new_chunk_size;
    pthread_mutex_unlock(&heap_global.mutex);
    return 0;
}

int dealloc(void *ptr) {
    pthread_mutex_lock(&heap_global.mutex);
    chunk_metadata_t *chunk_metadata = ptr - sizeof(chunk_metadata_t);

    if (!ptr || (void *)chunk_metadata > heap_global.heap_end || (void *)chunk_metadata < heap_global.heap_start ||
        chunk_metadata->chunk_state != CHUNK_USED) {
        return -1;
    }

    int bin_index = get_bin_index_from_size(chunk_metadata->chunk_size);

    bin_t *bin = &bin_pool[bin_index];

    while (bin->stack_top < 0) {  // if the sorted bin is full, default to the unsorted one
        bin_index = 2;
    }

    bin = &bin_pool[bin_index];

    struct node *new_node = stack_pop(bin);
    if (new_node == NULL) {
        return -1;
    }

    chunk_metadata->chunk_state = CHUNK_FREE;
    chunk_metadata->free_node_ptr = new_node;
    new_node->data = chunk_metadata;

    add_node(&bin->free_list, new_node);

    return coalesce(new_node);
}