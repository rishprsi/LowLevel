#ifndef PROPERDS_SORTING_H
#define PROPERDS_SORTING_H

#include <stddef.h>

/*
 * sorting — five classic sorts, all in-place-API (a[0..n) ascending), all
 * required to handle n == 0 and n == 1, duplicates, already-sorted and
 * reverse-sorted input.
 *
 * quick_sort must choose a decent pivot (median-of-three or random) so
 * that already-sorted and all-equal inputs do not recurse to death.
 *
 * hybrid_sort is quicksort that hands any subrange smaller than 16
 * elements to insertion sort — the standard practical optimization.
 */

/* Stable, O(n^2), great on tiny/nearly-sorted input. */
void insertion_sort(int *a, size_t n);

/* O(n log n), needs a temp buffer, stable. */
void merge_sort(int *a, size_t n);

/* O(n log n) expected; median-of-three (or random) pivot. */
void quick_sort(int *a, size_t n);

/* Heapsort over the array (no separate heap struct needed). */
void heap_sort_arr(int *a, size_t n);

/* Quicksort that switches to insertion sort below 16 elements. */
void hybrid_sort(int *a, size_t n);

#endif /* PROPERDS_SORTING_H */
