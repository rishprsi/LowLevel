#include "shortestpath.h"
#include "todo.h"

bool wgraph_init(WGraph *g, int n) {
    (void)g;
    (void)n;
    TODO("implement wgraph_init");
}

void wgraph_free(WGraph *g) {
    (void)g;
    TODO("implement wgraph_free");
}

bool wgraph_add_edge(WGraph *g, int u, int v, int w) {
    (void)g;
    (void)u;
    (void)v;
    (void)w;
    TODO("implement wgraph_add_edge");
}

void dijkstra(const WGraph *g, int src, long long *dist) {
    (void)g;
    (void)src;
    (void)dist;
    TODO("implement dijkstra");
}

bool bellman_ford(const WGraph *g, int src, long long *dist) {
    (void)g;
    (void)src;
    (void)dist;
    TODO("implement bellman_ford");
}
