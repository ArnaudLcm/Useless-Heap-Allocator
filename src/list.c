#include "list.h"

static inline void INIT_LIST_HEAD(struct list_head *list) {
    list->next = list;
    list->prev = list;
}

static inline void _list_add(struct list_head* new_list, struct list_head *prev, struct list_head *next) {
    next->prev = new_list;
    new_list->next = next;
    new_list->prev = prev;
    prev->next = new_list;
}

static inline void list_add(struct list_head* new_list, struct list_head *head) {
    _list_add(new_list, head, head->next);    
}
