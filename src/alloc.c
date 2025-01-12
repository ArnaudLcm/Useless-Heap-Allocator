#include "alloc.h"

#include <stdint.h>
#include <unistd.h>

#include "list.h"
#include "log.h"
#include "types.h"

enum RoundDirection { ROUND_UP, ROUND_DOWN };

static struct node bin_nodes[MAX_BIN_SIZE] = {INIT_NODE(NULL)};

static uchar node_bitmap[BITMAP_SIZE] = {0};

static struct list bin = INIT_LIST(NULL, NULL);

void set_node_bit(uchar *bitmap, int index) { bitmap[index / 8] |= (1 << (index % 8)); }

void clear_node_bit(uchar *bitmap, int index) { bitmap[index / 8] &= ~(1 << (index % 8)); }

int is_node_bit_set(uchar *bitmap, int index) { return bitmap[index / 8] & (1 << (index % 8)); }

struct node *allocate_node() {
    for (int i = 0; i < MAX_BIN_SIZE; i++) {
        if (!is_node_bit_set(node_bitmap, i)) {
            set_node_bit(node_bitmap, i);  // Mark as used
            return &bin_nodes[i];
        }
    }
    return NULL;  // No free node available
}

void free_node(struct node *n) {
    int index = n - bin_nodes;  // Calculate index of the node in the array
    if (index >= 0 && index < MAX_BIN_SIZE) {
        clear_node_bit(node_bitmap, index);  // Mark as free
    }
}

/*
 * @brief: This function will ensure every chunks are 1 byte aligned. By default, round it up
 */
static void *align_round_chunk(void *addr, enum RoundDirection direction) {
    uintptr_t address = (uintptr_t)addr;

    // If the address is already aligned, return it as is
    if ((address & 0x7) == 0) {
        return addr;
    }

    // Round up to the next multiple of 8
    switch (direction) {
        case ROUND_DOWN:
            return (void *)(address & ~0x7);
        case ROUND_UP:
        default:
            return (void *)((address + 7) & ~0x7);
    }
}

int alloc_init(heap_t *heap) {
    void *bottom_heap = (void *)sbrk(INIT_HEAP_SIZE);
    if (bottom_heap == (void *)-1) {
        return -1;
    }

    bottom_heap = align_round_chunk(bottom_heap, ROUND_UP);
    heap->heap_start = (uintptr_t)bottom_heap;

    void *top_heap = (void *)sbrk(0);
    if (top_heap == (void *)-1) {
        return -1;
    }
    top_heap = align_round_chunk(top_heap, ROUND_DOWN);
    heap->heap_end = (uintptr_t)top_heap;

    // Initialize the bin with static storage
    struct node *first_node = allocate_node();
    if (!first_node) {
        return -1;  // No available nodes
    }
    *first_node = INIT_NODE((void *)heap->heap_start);

    struct list bin = INIT_LIST(first_node, first_node);

    chunk_metadata_t *c = (chunk_metadata_t *)heap->heap_start;
    c->chunk_state = 0;
    c->chunk_size = heap->heap_end - heap->heap_start - sizeof(chunk_metadata_t);

    log_debug("Initialized heap with size %d at start %p", c->chunk_size, (void *)heap->heap_start);
    return 0;
}