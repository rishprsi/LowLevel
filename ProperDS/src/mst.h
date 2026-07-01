#ifndef PROPERDS_MST_H
#define PROPERDS_MST_H

#include <stddef.h>

/*
 * mst — minimum spanning tree of a CONNECTED, UNDIRECTED, weighted graph
 * given as an edge list over vertices 0 .. n-1.
 *
 * Both functions return the total weight of a minimum spanning tree, or
 * -1 if the graph is not connected (no spanning tree exists). n >= 1;
 * a single vertex with no edges has MST weight 0. Parallel edges and
 * self-loops may appear in the input (self-loops can never be in an MST).
 *
 * kruskal must use a UNION-FIND private to your implementation (do not
 * include unionfind.h — each module is compiled standalone; write the
 * little DSU you need inside mst.c).
 *
 * prim can use the simple O(n^2) form: grow the tree from vertex 0,
 * repeatedly adding the cheapest edge that connects a new vertex.
 */
typedef struct {
    int u;
    int v;
    int w;
} MstEdge;

/* Kruskal's algorithm: sort edges by weight, add each edge that joins two
 * different components (union-find), until n-1 edges are in the tree.
 * The input array must not be modified (sort a copy). */
long long kruskal(int n, const MstEdge *edges, size_t m);

/* Prim's algorithm: same result, different route. */
long long prim(int n, const MstEdge *edges, size_t m);

#endif /* PROPERDS_MST_H */
