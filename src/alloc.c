#include "alloc.h"

#include <sys/mman.h>

#include "list.h"
#include "log.h"
#include "types.h"

enum RoundDirection { ROUND_UP, ROUND_DOWN };

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
    heap->heap_start =
        mmap((void *)sbrk(0), INIT_HEAP_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    heap->heap_end = (void *)heap->heap_start + INIT_HEAP_SIZE;

    if (heap->heap_start == MAP_FAILED) {
        return -1;
    }

    heap->heap_start = align_round_chunk(heap->heap_start, ROUND_UP);

    struct node *first_node = allocate_node();

    if (!first_node) {
        return -1;  // No available nodes
    }

    first_node->data = heap->heap_start;

    add_node(&bin, first_node);

    chunk_metadata_t *c = heap->heap_start;
    c->chunk_state = 0;
    c->chunk_size = heap->heap_end - heap->heap_start - sizeof(chunk_metadata_t);

    log_debug("Initialized heap with size %d at start %p", c->chunk_size, heap->heap_start);
    return 0;
}