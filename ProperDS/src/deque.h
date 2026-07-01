#ifndef PROPERDS_DEQUE_H
#define PROPERDS_DEQUE_H

#include <stdbool.h>
#include <stddef.h>

/*
 * deque — a GROWABLE circular deque of int.
 *
 * Elements live at physical slots data[(head + i) % cap] for logical index
 * i in [0, len). push/pop at either end are O(1); when full, the buffer
 * grows (0 -> 4, then doubles) and the elements are re-linearized so that
 * head becomes 0 again.
 */
typedef struct {
    int *data;
    size_t cap;  /* physical slots */
    size_t head; /* physical index of logical element 0 */
    size_t len;  /* number of elements */
} Deque;

/* Initialize to empty (data = NULL, cap = head = len = 0). */
void deque_init(Deque *d);

/* Free the backing storage and reset to empty. Safe on an empty deque. */
void deque_free(Deque *d);

/* Insert x as the new FIRST element. Returns false only on allocation
 * failure. */
bool deque_push_front(Deque *d, int x);

/* Insert x as the new LAST element. Returns false only on allocation
 * failure. */
bool deque_push_back(Deque *d, int x);

/* Remove the first element into *out. Returns false (out untouched) if
 * empty. */
bool deque_pop_front(Deque *d, int *out);

/* Remove the last element into *out. Returns false (out untouched) if
 * empty. */
bool deque_pop_back(Deque *d, int *out);

/* Number of elements. */
size_t deque_len(const Deque *d);

/* Invariant checker: len <= cap, head < cap whenever cap > 0, and
 * data == NULL exactly when cap == 0. */
bool deque_validate(const Deque *d);

#endif /* PROPERDS_DEQUE_H */
