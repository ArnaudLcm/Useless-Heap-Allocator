#include "stack.h"
#include <stdlib.h>
#include <stddef.h>

void init_stack(int* stack) {
    for (int i = 0; i < MAX_BIN_SIZE; i++) {
        stack[i] = i;
    }
}


struct node *stack_pop(bin_t *bin) {
    if (bin->stack_top < 0) {
        return NULL;  // No free node available
    }
    int index = bin->stack[bin->stack_top--];
    if (index >= 0 && index < MAX_BIN_SIZE) {
        return &bin->bin_nodes[index];
    }

    return NULL;
}

void stack_push(struct node *n, bin_t *bin) {
    int index =
        ((ptrdiff_t)n - (ptrdiff_t)bin->bin_nodes) / (sizeof(struct node));  // Calculate index of the node in the array
    if (index >= 0 && index < MAX_BIN_SIZE) {
        bin->stack[++bin->stack_top] = index;
    }
}