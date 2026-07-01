#ifndef PROPERDS_VECTOR_H
#define PROPERDS_VECTOR_H

#include <stdbool.h>
#include <stddef.h>

/*
 * vector — a growable array of int.
 *
 * Growth policy (part of the contract; the tests check it):
 *   - a fresh vector has cap == 0 and data == NULL
 *   - the first allocation is 4 slots
 *   - when full, capacity doubles
 */
typedef struct {
    int *data;
    size_t len;
    size_t cap;
} Vector;

/* Initialize to the empty state (len = cap = 0, data = NULL). */
void vector_init(Vector *v);

/* Free the backing storage and reset to the empty state.
 * Safe to call on an already-empty vector. */
void vector_free(Vector *v);

/* Append x at the end. Returns false only on allocation failure. */
bool vector_push(Vector *v, int x);

/* Remove and return the last element. Precondition: v->len > 0. */
int vector_pop(Vector *v);

/* Return the element at index i. Precondition: i < v->len. */
int vector_get(const Vector *v, size_t i);

/* Overwrite the element at index i. Precondition: i < v->len. */
void vector_set(Vector *v, size_t i, int x);

/* Insert x so that it ends up at index i, shifting elements [i, len) one
 * slot to the right. Precondition: i <= v->len (i == len is an append).
 * Returns false only on allocation failure. */
bool vector_insert_at(Vector *v, size_t i, int x);

/* Remove the element at index i, shifting elements (i, len) one slot to
 * the left. Precondition: i < v->len. */
void vector_remove_at(Vector *v, size_t i);

/* Number of elements. */
size_t vector_len(const Vector *v);

/* Current capacity. */
size_t vector_cap(const Vector *v);

/* Invariant checker: len <= cap, and data == NULL exactly when cap == 0.
 * The tests call this after mutations. */
bool vector_validate(const Vector *v);

#endif /* PROPERDS_VECTOR_H */
