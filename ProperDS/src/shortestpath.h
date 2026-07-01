#ifndef PROPERDS_SHORTESTPATH_H
#define PROPERDS_SHORTESTPATH_H

#include <stdbool.h>
#include <stddef.h>

/*
 * shortestpath — single-source shortest paths on a weighted digraph.
 *
 * WGraph is a small adjacency-list weighted digraph over vertices
 * 0 .. n-1. Edge weights are ints; Bellman-Ford accepts negative weights,
 * Dijkstra REQUIRES all weights >= 0 (precondition, not checked).
 *
 * Distance output convention for both algorithms:
 *   dist[v] = length of the shortest path src -> v (as long long, so sums
 *             of int weights cannot overflow), or
 *   dist[v] = -1 if v is unreachable from src.
 *   dist[src] = 0.
 */
typedef struct {
    int to;
    int w;
} WEdge;

typedef struct {
    WEdge *edges;
    size_t len;
    size_t cap;
} WAdj;

typedef struct {
    int n;
    WAdj *adj;
} WGraph;

/* Create a weighted digraph with n vertices, no edges. Precondition:
 * n >= 0. Returns false on allocation failure. */
bool wgraph_init(WGraph *g, int n);

/* Free all adjacency storage. Safe to call twice. */
void wgraph_free(WGraph *g);

/* Add directed edge u -> v with weight w. Preconditions: 0 <= u, v < n.
 * Returns false only on allocation failure. */
bool wgraph_add_edge(WGraph *g, int u, int v, int w);

/* Dijkstra from src. Precondition: every edge weight >= 0.
 * Fills dist[0..n) using the convention above. (An O(n^2) scan for the
 * closest unvisited vertex is perfectly acceptable here.) */
void dijkstra(const WGraph *g, int src, long long *dist);

/* Bellman-Ford from src. Handles negative weights. Fills dist[0..n).
 * Returns false if a NEGATIVE CYCLE is reachable from src (dist contents
 * are then unspecified), true otherwise. */
bool bellman_ford(const WGraph *g, int src, long long *dist);

#endif /* PROPERDS_SHORTESTPATH_H */
