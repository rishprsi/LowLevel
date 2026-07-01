#ifndef PROPERDS_DLL_H
#define PROPERDS_DLL_H

#include <stdbool.h>
#include <stddef.h>

/*
 * dll — a doubly linked list of int with head/tail pointers and a cached
 * length.
 *
 * Invariants (dll_validate checks all of them):
 *   - empty list: head == tail == NULL and len == 0
 *   - head->prev == NULL and tail->next == NULL
 *   - for every node n: n->next->prev == n (when n->next != NULL)
 *   - walking forward from head and backward from tail both visit exactly
 *     len nodes and end at tail/head respectively
 */
typedef struct DllNode {
    int value;
    struct DllNode *prev;
    struct DllNode *next;
} DllNode;

typedef struct {
    DllNode *head;
    DllNode *tail;
    size_t len;
} Dll;

/* Initialize to the empty state. */
void dll_init(Dll *d);

/* Free every node and reset to the empty state. Safe on an empty list. */
void dll_free(Dll *d);

/* Insert value at the front. Returns false only on allocation failure. */
bool dll_push_front(Dll *d, int value);

/* Insert value at the back. Returns false only on allocation failure. */
bool dll_push_back(Dll *d, int value);

/* Remove the front element, storing it in *out.
 * Returns false (and leaves *out untouched) if the list is empty. */
bool dll_pop_front(Dll *d, int *out);

/* Remove the back element, storing it in *out.
 * Returns false (and leaves *out untouched) if the list is empty. */
bool dll_pop_back(Dll *d, int *out);

/* Number of nodes. */
size_t dll_length(const Dll *d);

/* Invariant checker: walks the list in BOTH directions and verifies the
 * next/prev pointers are mutually consistent (see invariants above). */
bool dll_validate(const Dll *d);

#endif /* PROPERDS_DLL_H */
