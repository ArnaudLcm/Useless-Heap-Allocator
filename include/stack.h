#include "bin.h"
#ifndef __STACK_H__
#define __STACK_H__

struct node *stack_pop(bin_t *bin);
void stack_push(struct node *n, bin_t *bin);
void init_stack(int* stack);
#endif