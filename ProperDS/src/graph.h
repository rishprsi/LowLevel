#ifndef PROPERDS_GRAPH_H
#define PROPERDS_GRAPH_H

#include <stdbool.h>
#include <stddef.h>

/*
 * graph — a directed graph over vertices 0 .. n-1, stored as adjacency
 * lists (a growable int array per vertex).
 *
 * ORDER CONTRACT (the tests depend on it): each vertex's neighbors are
 * visited in the order the edges were added (graph_add_edge appends).
 *
 * Traversal contracts:
 *   - bfs_order: classic queue BFS from src; enqueue unvisited neighbors
 *     in adjacency order. Fills out[] with vertices in dequeue order and
 *     returns how many were reached.
 *   - dfs_order: RECURSIVE preorder DFS from src; visit a vertex, then
 *     recurse into its unvisited neighbors in adjacency order.
 *   - dfs_order_iter: same visit order as dfs_order, but implemented with
 *     an EXPLICIT stack and no recursion. (Hint: pop a vertex, skip if
 *     visited, mark + output it, then push its neighbors in REVERSE
 *     adjacency order so the first neighbor is processed first.)
 *   - topo_sort: Kahn's algorithm (repeatedly remove indegree-0 vertices;
 *     process ties in ascending vertex number). Fills out[0..n) and
 *     returns true, or returns false if the graph has a cycle.
 *   - has_cycle: true iff the graph contains a directed cycle.
 */
typedef struct {
    int *to;    /* neighbor array */
    size_t len;
    size_t cap;
} GraphAdj;

typedef struct {
    int n;
    GraphAdj *adj; /* adj[v] = out-neighbors of v */
} Graph;

/* Create a graph with n vertices and no edges. Precondition: n >= 0.
 * Returns false on allocation failure. */
bool graph_init(Graph *g, int n);

/* Free all adjacency storage. Safe to call twice. */
void graph_free(Graph *g);

/* Add the directed edge u -> v (appended to u's list; parallel edges and
 * self-loops are allowed). Preconditions: 0 <= u, v < n.
 * Returns false only on allocation failure. */
bool graph_add_edge(Graph *g, int u, int v);

/* BFS from src; fills out[] in visit order, returns number reached. */
int graph_bfs_order(const Graph *g, int src, int *out);

/* Recursive preorder DFS from src; fills out[], returns number reached. */
int graph_dfs_order(const Graph *g, int src, int *out);

/* Iterative DFS with an explicit stack. MUST produce exactly the same
 * order as graph_dfs_order. */
int graph_dfs_order_iter(const Graph *g, int src, int *out);

/* Kahn's topological sort (smallest ready vertex first). Fills out[0..n)
 * and returns true; returns false if the graph has a cycle. */
bool graph_topo_sort(const Graph *g, int *out);

/* True iff the graph contains a directed cycle. */
bool graph_has_cycle(const Graph *g);

#endif /* PROPERDS_GRAPH_H */
