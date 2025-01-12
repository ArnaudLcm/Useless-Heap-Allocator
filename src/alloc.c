#include "alloc.h"

#include <sys/mman.h>

#include "list.h"
#include "log.h"
#include "types.h"

heap_t heap_global = {.heap_end = NULL, .heap_start = NULL};

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

/*
 * @brief: This function will ensure every chunks are 4 byte aligned. By default, round it up
 */
static inline void *align_round_chunk(void *addr, enum RoundDirection direction) {
    uintptr_t address = (uintptr_t)addr;

    // If the address is already aligned, return it as is
    if ((address & 3) == 0) {
        return addr;
    }

    switch (direction) {
        case ROUND_DOWN:
            return (void *)(address & ~3);
        case ROUND_UP:
        default:
            return (void *)((address + 3) & ~3);
    }
}

int alloc_init() {
    heap_global.heap_start =
        mmap((void *)sbrk(0), INIT_HEAP_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    heap_global.heap_end = (void *)heap_global.heap_start + INIT_HEAP_SIZE;

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
    size = (size + 3) & ~3;

    if (size > MAX_CHUNK_SIZE) {
        return NULL;
    }

    struct node *free_node = bin.head;

    while (free_node && free_node->data) {
        chunk_metadata_t *metadata = free_node->data;
        if (metadata->chunk_size >= size) {  // Check if the chunk is large enough to handle the user request.
            struct node *new_node = allocate_node();
            if (new_node == NULL) {
                return NULL;
            }

            remove_node(&bin, free_node);
            metadata->chunk_state = CHUNK_USED;

            if (metadata->chunk_size > size) {
                chunk_metadata_t *new_chunk_metadata = (void *)metadata + size + sizeof(chunk_metadata_t);
                new_chunk_metadata->chunk_size = metadata->chunk_size - size - sizeof(chunk_metadata_t);

                new_node->data = new_chunk_metadata;

                add_node(&bin, new_node);
            }

            return (void *)metadata + sizeof(chunk_metadata_t);
        }

        free_node = free_node->next;
    }

    return NULL;
}

int dealloc(void *ptr) {
    chunk_metadata_t *chunk_metadata = ptr - sizeof(chunk_metadata_t);

    if (!ptr || (void *)chunk_metadata > heap_global.heap_end || (void *)chunk_metadata < heap_global.heap_start ||
        MASK_CHUNK_STATE(chunk_metadata->chunk_state) != CHUNK_USED) {
        return -1;
    }
    struct node *new_node = allocate_node();
    if (new_node == NULL) {
        return -1;
    }

    chunk_metadata->chunk_state = CHUNK_FREE;
    new_node->data = chunk_metadata;

    add_node(&bin, new_node);
    return 0;
}