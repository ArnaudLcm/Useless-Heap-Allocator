#include <stdlib.h>

#include "list.h"
#include "test.h"
void init_list() {
    struct list l = INIT_LIST(NULL, NULL);
    struct node n = INIT_NODE(NULL);

    add_node(&l, &n);
    ASSERT_TRUE(l.head == &n);
    ASSERT_TRUE(l.tail == &n);
}

void add_nodes() {
    struct list l = INIT_LIST(NULL, NULL);
    struct node n = INIT_NODE(NULL);
    struct node n2 = INIT_NODE(NULL);

    add_node(&l, &n);
    add_node(&l, &n2);

    ASSERT_TRUE(l.head == &n);
    ASSERT_TRUE(l.tail == &n2);
}

void add_nodes_then_remove() {
    struct list l = INIT_LIST(NULL, NULL);
    struct node n = INIT_NODE(NULL);
    struct node n2 = INIT_NODE(NULL);
    struct node n3 = INIT_NODE(NULL);

    add_node(&l, &n);
    add_node(&l, &n2);
    add_node(&l, &n3);

    ASSERT_TRUE(l.head == &n);
    ASSERT_TRUE(l.tail == &n3);

    remove_node(&l, &n2);

    ASSERT_TRUE(l.head->next == &n3);
    ASSERT_TRUE(l.head == n3.prev);

    remove_node(&l, &n);

    ASSERT_TRUE(l.head == &n3);
}

void test_operations_on_list_batch() {
    init_list();
    add_nodes();
    add_nodes_then_remove();
}