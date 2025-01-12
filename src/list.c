#include "list.h"

#include <stdlib.h>

void add_node(struct list* l, struct node* new_node) {
    new_node->next = NULL;
    new_node->prev = NULL;
    if(!l->head) {
        l->head = new_node;
        l->tail = new_node;
        return;
    }

    l->tail->next = new_node;
    new_node->prev = l->tail;
    l->tail = new_node;
}

void remove_node(struct list* l, struct node* node) {
    if (node->prev) {
        node->prev->next = node->next;
    } else {
        l->head = node->next;
    }

    if (node->next) {
        node->next->prev = node->prev;
    } else {
        l->tail = node->prev;
    }

    node->next = NULL;
    node->prev = NULL;
}