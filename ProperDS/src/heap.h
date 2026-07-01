#ifndef PROPERDS_HEAP_H
#define PROPERDS_HEAP_H

#include <stdbool.h>
#include <stddef.h>

/*
 * heap — a binary MIN-heap of int in a growable array.
 *
 * Layout: element i's children are 2i+1 and 2i+2; its parent is (i-1)/2.
 * Heap property: every element is <= both of its children.
 *
 * heap_from_array must use BOTTOM-UP heapify (sift-down from the last
 * internal node to the root) — O(n), not n pushes.
 *
 * heap_sort sorts ascending and is built ON the heap (e.g. heapify then
 * repeatedly pop the min). Named heap_sort, not heapsort, because macOS
 * <stdlib.h> already declares a BSD function called heapsort.
 */
typedef struct {
    int *data;
    size_t len;
    size_t cap;
} Heap;

/* Initialize to empty (len = cap = 0, data = NULL). */
void heap_init(Heap *h);

/* Free the backing storage and reset to empty. Safe on an empty heap. */
void heap_free(Heap *h);

/* Insert x. Returns false only on allocation failure. */
bool heap_push(Heap *h, int x);

/* Remove the minimum into *out. Returns false (out untouched) if empty. */
bool heap_pop_min(Heap *h, int *out);

/* Read the minimum into *out without removing. Returns false if empty. */
bool heap_peek(const Heap *h, int *out);

/* Number of elements. */
size_t heap_size(const Heap *h);

/* Initialize h to contain a[0..n) using bottom-up heapify.
 * h must be uninitialized or freed. Returns false on allocation failure. */
bool heap_from_array(Heap *h, const int *a, size_t n);

/* Sort a[0..n) ascending, using the heap. */
void heap_sort(int *a, size_t n);

/* Invariant checker: every element <= its children, and len <= cap. */
bool heap_validate(const Heap *h);

#endif /* PROPERDS_HEAP_H */
