#include "list.h"

#ifndef __BIN_H__
#define __BIN_H__

#define MAX_BIN_SIZE 128  // Maximum number of nodes in the bin
#define POOL_BIN_SIZE 3



typedef struct {
    int stack[MAX_BIN_SIZE];
    int stack_top;
    struct node bin_nodes[MAX_BIN_SIZE];
    struct list free_list;
} bin_t;


#endif