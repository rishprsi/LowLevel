#ifndef PROPERC_MEMORY_H
#define PROPERC_MEMORY_H

#include <stdbool.h>
#include <stddef.h>

/*
 * Return a newly malloc'd copy of NUL-terminated string s (caller frees with
 * free()). Returns NULL only if allocation fails.
 */
char *my_strdup(const char *s);

/*
 * A growable array of int.
 * Invariant after vec_init: data may be NULL, len == 0, cap == 0.
 * Always len <= cap.
 */
typedef struct {
    int *data;
    size_t len;
    size_t cap;
} IntVec;

/* Initialize an empty vector (no allocation required yet). */
void vec_init(IntVec *v);

/*
 * Append x, growing capacity as needed (suggested growth: 0 -> 4, then double).
 * Returns true on success, false if (re)allocation failed (vector unchanged).
 */
bool vec_push(IntVec *v, int x);

/*
 * Remove the last element into *out. Returns false (and leaves *out untouched)
 * if the vector is empty.
 */
bool vec_pop(IntVec *v, int *out);

/* Return element i. Precondition: i < v->len. */
int vec_get(const IntVec *v, size_t i);

/* Free the backing storage and reset to the empty state. Safe to call twice. */
void vec_free(IntVec *v);

#endif /* PROPERC_MEMORY_H */
