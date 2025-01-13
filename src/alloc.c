#include "alloc.h"

#include <sys/mman.h>

#include "list.h"
#include "log.h"
#include "types.h"

heap_t heap_global = {.heap_end = NULL, .heap_start = NULL, .heap_size = 0};

enum RoundDirection { ROUND_UP, ROUND_DOWN };

void set_node_bit(uchar *bitmap, int index) { bitmap[index / 8] |= (1 << (index % 8)); }

void clear_node_bit(uchar *bitmap, int index) { bitmap[index / 8] &= ~(1 << (index % 8)); }

int is_node_bit_set(uchar *bitmap, int index) { return bitmap[index / 8] & (1 << (index % 8)); }

static struct node *allocate_node() {
    for (int i = 0; i < MAX_BIN_SIZE; i++) {
        if (!is_node_bit_set(node_bitmap, i)) {
            set_node_bit(node_bitmap, i);  // Mark as used
            return &bin_nodes[i];
        }
    }
    return NULL;  // No free node available
}

static void free_node(struct node *n) {
    int index = n - bin_nodes;  // Calculate index of the node in the array
    if (index >= 0 && index < MAX_BIN_SIZE) {
        clear_node_bit(node_bitmap, index);  // Mark as free
    }
}

static void reset_nodes() {
    for (size_t i = 0; i < MAX_BIN_SIZE; i++) {
        clear_node_bit(node_bitmap, i);
        remove_node(&bin, &bin_nodes[i]);
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
        reset_nodes();
    }

    heap_global.heap_start =
        mmap((void *)sbrk(0), INIT_HEAP_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    heap_global.heap_end = (void *)heap_global.heap_start + INIT_HEAP_SIZE;

    heap_global.heap_size = INIT_HEAP_SIZE;

    if (heap_global.heap_start == MAP_FAILED) {
        return -1;
    }

    heap_global.heap_start = align_round_chunk(heap_global.heap_start, ROUND_UP);

    struct node *first_node = allocate_node();

    if (!first_node) {
        return -1;  // No available nodes
    }

    first_node->data = heap_global.heap_start;

    add_node(&bin, first_node);

    chunk_metadata_t *c = heap_global.heap_start;
    c->chunk_state = 0;
    c->chunk_size = heap_global.heap_end - heap_global.heap_start - sizeof(chunk_metadata_t);

    log_debug("Initialized heap with size %d at start %p", c->chunk_size, heap_global.heap_start);
    return 0;
}

void *alloc(unsigned long size) {
    size = (size + ARCH_ALIGNMENT - 1) & ~(ARCH_ALIGNMENT - 1);

    if (size > MAX_CHUNK_SIZE) {
        return NULL;
    }

    // Find the smallest suitable free node
    struct node *smallest_free_node = NULL;
    chunk_metadata_t *smallest_metadata = NULL;

    for (struct node *current = bin.head; current; current = current->next) {
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

    remove_node(&bin, smallest_free_node);
    free_node(smallest_free_node);
    smallest_metadata->free_node_ptr = NULL;

    smallest_metadata->chunk_state = CHUNK_USED;

    // Check if splitting the chunk is necessary
    unsigned long remaining_size = smallest_metadata->chunk_size - size - sizeof(chunk_metadata_t);
    if (remaining_size > sizeof(chunk_metadata_t)) {
        chunk_metadata_t *new_metadata = (void *)smallest_metadata + size + sizeof(chunk_metadata_t);
        new_metadata->chunk_size = remaining_size;
        new_metadata->chunk_state = CHUNK_FREE;

        struct node *new_node = allocate_node();
        if (!new_node) {
            return NULL;
        }
        new_node->data = new_metadata;
        add_node(&bin, new_node);
        new_metadata->free_node_ptr = new_node;
    }

    smallest_metadata->chunk_size = size;

    return (void *)smallest_metadata + sizeof(chunk_metadata_t);
}

static int coalesce(struct node *node) {
    chunk_metadata_t *metadata = node->data;
    chunk_metadata_t *start_metadata = metadata;
    if (!metadata) {
        return -1;
    }
    int total_new_chunk_size = metadata->chunk_size;
    while (metadata && metadata->chunk_state == CHUNK_FREE &&
           ((void *)metadata + metadata->chunk_size + sizeof(chunk_metadata_t) < heap_global.heap_end)) {
        remove_node(&bin, metadata->free_node_ptr);
        free_node(metadata->free_node_ptr);
        metadata = (void *)metadata + metadata->chunk_size + sizeof(chunk_metadata_t);
        total_new_chunk_size += metadata->chunk_size + sizeof(chunk_metadata_t);
    }

    start_metadata->chunk_size = total_new_chunk_size;

    return 0;
}

int dealloc(void *ptr) {
    chunk_metadata_t *chunk_metadata = ptr - sizeof(chunk_metadata_t);

    if (!ptr || (void *)chunk_metadata > heap_global.heap_end || (void *)chunk_metadata < heap_global.heap_start ||
        chunk_metadata->chunk_state != CHUNK_USED) {
        return -1;
    }

    struct node *new_node = allocate_node();
    if (new_node == NULL) {
        return -1;
    }

    chunk_metadata->chunk_state = CHUNK_FREE;
    chunk_metadata->free_node_ptr = new_node;
    new_node->data = chunk_metadata;

    add_node(&bin, new_node);



    return coalesce(new_node);
}