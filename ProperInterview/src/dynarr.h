#ifndef PROPERINTERVIEW_DYNARR_H
#define PROPERINTERVIEW_DYNARR_H

#include <stdbool.h>
#include <stddef.h>

/*
 * dynarr — a growable array of int (the "implement a vector" interview
 * staple).
 *
 * TARGET DRILL TIME: 30 minutes.
 *
 * Re-drill: `git checkout -- src/dynarr.c`, start the timer, implement
 * every function below, then `make test_dynarr`.
 *
 * Growth policy (part of the contract; the tests check it):
 *   - a fresh array has cap == 0 and data == NULL
 *   - the first allocation is 8 slots
 *   - when full, capacity doubles
 *
 * The classic realloc trap applies: stash realloc's result in a temporary —
 * assigning it straight to da->data leaks the original block if realloc
 * fails, and the contract below requires the array to be UNCHANGED on
 * allocation failure.
 */
typedef struct {
    int *data;
    size_t len;
    size_t cap;
} DynArr;

/* Initialize to the empty state (len = cap = 0, data = NULL). No allocation. */
void da_init(DynArr *da);

/* Free the backing storage and reset to the empty state.
 * Safe to call on an already-empty array (and thus safe to call twice). */
void da_free(DynArr *da);

/* Append x at the end. Returns false only on allocation failure
 * (array unchanged in that case). */
bool da_push(DynArr *da, int x);

/* Remove the last element into *out. Returns false (and leaves *out
 * untouched) if the array is empty. Capacity is not shrunk. */
bool da_pop(DynArr *da, int *out);

/* Return the element at index i. Precondition: i < da->len. */
int da_get(const DynArr *da, size_t i);

/* Overwrite the element at index i. Precondition: i < da->len. */
void da_set(DynArr *da, size_t i, int x);

/* Insert x so it ends up at index i, shifting elements [i, len) one slot
 * right. Precondition: i <= da->len (i == len is an append). Returns false
 * only on allocation failure. Use memmove-style shifting — the regions
 * overlap. */
bool da_insert_at(DynArr *da, size_t i, int x);

/* Remove the element at index i, shifting elements (i, len) one slot left.
 * Precondition: i < da->len. */
void da_remove_at(DynArr *da, size_t i);

/* Number of elements. */
size_t da_len(const DynArr *da);

/* Current capacity. */
size_t da_cap(const DynArr *da);

#endif /* PROPERINTERVIEW_DYNARR_H */
