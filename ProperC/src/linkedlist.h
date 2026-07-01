#ifndef PROPERC_LINKEDLIST_H
#define PROPERC_LINKEDLIST_H

#include <stdbool.h>
#include <stddef.h>

typedef struct Node {
    int val;
    struct Node *next;
} Node;

/*
 * Allocate a node holding val and prepend it to the list. Returns the new head.
 * If allocation fails, returns the original head unchanged.
 */
Node *list_push_front(Node *head, int val);

/* Number of nodes in the (acyclic) list. */
size_t list_length(const Node *head);

/* Reverse the list in place and return the new head. */
Node *list_reverse(Node *head);

/*
 * Value of the middle node via the slow/fast technique. For even length,
 * returns the second of the two middle nodes. Precondition: head != NULL.
 */
int list_middle(const Node *head);

/* True iff the list contains a cycle (Floyd's tortoise & hare). */
bool list_has_cycle(const Node *head);

/* Free every node of an acyclic list. */
void list_free(Node *head);

#endif /* PROPERC_LINKEDLIST_H */
