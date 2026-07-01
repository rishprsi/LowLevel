#ifndef PROPERDS_SEARCH_H
#define PROPERDS_SEARCH_H

#include <stddef.h>

/*
 * search — binary search and its two civilized cousins, over a SORTED
 * (ascending, duplicates allowed) int array a[0..n).
 *
 * Semantics match C++'s std::lower_bound / std::upper_bound:
 *
 *   lower_bound(a, n, key) = the FIRST index i with a[i] >= key,
 *                            or n if every element is < key.
 *   upper_bound(a, n, key) = the FIRST index i with a[i] >  key,
 *                            or n if every element is <= key.
 *
 * So [lower_bound, upper_bound) is exactly the run of elements == key,
 * and upper_bound - lower_bound is the number of occurrences.
 *
 * binary_search_idx returns SOME index holding key (any one, if there are
 * duplicates), or -1 if key is absent. All three must be O(log n) — the
 * tests run them over arrays big enough that a linear implementation
 * would still pass timing, but you'd only be cheating yourself.
 */

/* Index of some occurrence of key, or -1 if absent. */
int binary_search_idx(const int *a, size_t n, int key);

/* First index i with a[i] >= key, else n. */
size_t lower_bound(const int *a, size_t n, int key);

/* First index i with a[i] > key, else n. */
size_t upper_bound(const int *a, size_t n, int key);

#endif /* PROPERDS_SEARCH_H */
