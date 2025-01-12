#ifndef __LIST_H__
#define __LIST_H__
// Implementation of non-intrusive lists

struct node {
    struct node* next;
    struct node* prev;
    void* data;
};

struct list {
    struct node* head;
    struct node* tail;
};

void add_node(struct list* l, struct node* new_node);

void remove_node(struct list* l, struct node* node);


#define INIT_LIST(head_ptr, tail_ptr) (struct list){ .head = (head_ptr), .tail = (tail_ptr) }
#define INIT_NODE(data_ptr) (struct node){ .next = (NULL), .prev = (NULL), .data = (data_ptr) }
#endif