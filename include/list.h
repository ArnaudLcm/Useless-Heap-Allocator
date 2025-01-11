#ifndef __LIST_H__
#define __LIST_H__
// Implementation of intrusive lists
// The implementation's idea comes from the Linux kernel itself:
// https://litux.nl/mirror/kerneldevelopment/0672327201/app01lev1sec2.html

struct list_head {
    struct list_head *next;
    struct list_head *prev;
};
static inline void INIT_LIST_HEAD(struct list_head *list);
static inline void _list_add(struct list_head* new_list, struct list_head *prev, struct list_head *next);
static inline void list_add(struct list_head* new_list, struct list_head *head);

#define LIST_HEAD_INIT(name) \
    { &(name), &(name) }

#define LIST_HEAD(name) struct list_head name = LIST_HEAD_INIT(name)

#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define container_of(ptr, type, member) ({                      \
    const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
    (type *)( (char *)__mptr - offsetof(type,member) );})

/**
* @args: ptr <-> The structure list head ptr.
* @args: type <-> The structure type.
* @args: member <-> The structure member containing the list head.
 */
#define list_entry(ptr, type, member) \
    container_of(ptr, type, member)



#endif