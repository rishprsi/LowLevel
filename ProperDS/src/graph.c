#include "graph.h"
#include "todo.h"

bool graph_init(Graph *g, int n) {
    (void)g;
    (void)n;
    TODO("implement graph_init");
}

void graph_free(Graph *g) {
    (void)g;
    TODO("implement graph_free");
}

bool graph_add_edge(Graph *g, int u, int v) {
    (void)g;
    (void)u;
    (void)v;
    TODO("implement graph_add_edge");
}

int graph_bfs_order(const Graph *g, int src, int *out) {
    (void)g;
    (void)src;
    (void)out;
    TODO("implement graph_bfs_order");
}

int graph_dfs_order(const Graph *g, int src, int *out) {
    (void)g;
    (void)src;
    (void)out;
    TODO("implement graph_dfs_order");
}

int graph_dfs_order_iter(const Graph *g, int src, int *out) {
    (void)g;
    (void)src;
    (void)out;
    TODO("implement graph_dfs_order_iter");
}

bool graph_topo_sort(const Graph *g, int *out) {
    (void)g;
    (void)out;
    TODO("implement graph_topo_sort");
}

bool graph_has_cycle(const Graph *g) {
    (void)g;
    TODO("implement graph_has_cycle");
}
