#ifndef PROPERDS_SLL_H
#define PROPERDS_SLL_H

#include <stdbool.h>
#include <stddef.h>

/*
 * sll — a singly linked list of int.
 *
 * There is no wrapper struct: a list is just an SllNode* head pointer, and
 * the empty list is NULL. Functions that may change which node is first take
 * SllNode **head (pointer-to-pointer), so they work uniformly on empty lists
 * and on removals of the head node — no special-casing.
 */
typedef struct SllNode {
    int value;
    struct SllNode *next;
} SllNode;

/* Allocate a node holding value and link it at the front.
 * Returns false only on allocation failure (list unchanged). */
bool sll_push_front(SllNode **head, int value);

/* Allocate a node holding value and link it at the back.
 * Returns false only on allocation failure (list unchanged). */
bool sll_push_back(SllNode **head, int value);

/* Return the first node whose value == value, or NULL if absent. */
SllNode *sll_find(SllNode *head, int value);

/* Unlink and free the FIRST node whose value == value.
 * Returns true if a node was removed, false if the value was absent. */
bool sll_remove_value(SllNode **head, int value);

/* Reverse the list in place (relink nodes; do not allocate or copy values). */
void sll_reverse(SllNode **head);

/* Number of nodes. */
size_t sll_length(const SllNode *head);

/* Free every node and set *head to NULL. Safe on an empty list. */
void sll_free(SllNode **head);

#endif /* PROPERDS_SLL_H */
