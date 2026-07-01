#ifndef PROPERDS_UNIONFIND_H
#define PROPERDS_UNIONFIND_H

#include <stdbool.h>

/*
 * unionfind — disjoint-set union (DSU) over elements 0 .. n-1, with the two
 * classic optimizations:
 *   - union by RANK: attach the shorter tree under the taller one
 *   - path COMPRESSION in find: point every visited node at the root
 *
 * count tracks the current number of disjoint components (starts at n and
 * drops by one on every successful union).
 */
typedef struct {
    int *parent;
    int *rank;
    int n;
    int count; /* number of components */
} UnionFind;

/* Create n singleton sets {0}, {1}, ..., {n-1}. Precondition: n >= 0.
 * Returns false on allocation failure. */
bool uf_init(UnionFind *uf, int n);

/* Free the arrays. Safe to call twice. */
void uf_free(UnionFind *uf);

/* Representative (root) of x's set, compressing the path along the way.
 * Precondition: 0 <= x < n. */
int uf_find(UnionFind *uf, int x);

/* Merge the sets containing a and b (union by rank).
 * Returns true if two different sets were merged, false if a and b were
 * already in the same set. */
bool uf_union(UnionFind *uf, int a, int b);

/* True iff a and b are in the same set. */
bool uf_connected(UnionFind *uf, int a, int b);

/* Current number of disjoint components. */
int uf_count(const UnionFind *uf);

#endif /* PROPERDS_UNIONFIND_H */
