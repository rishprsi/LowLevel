#include "ctest.h"
#include "shortestpath.h"

#include <stdlib.h>

int main(void) {
    WGraph g;
    long long dist[32], dist2[32];

    /*
     * Fixed graph with known answers:
     *
     *   0 --1--> 1 --2--> 2
     *   0 --------10-----> 2     (longer direct route)
     *   2 --3--> 3
     *   4 is unreachable
     */
    SECTION("wgraph_init / add_edge");
    CHECK_TRUE(wgraph_init(&g, 5));
    CHECK_TRUE(wgraph_add_edge(&g, 0, 1, 1));
    CHECK_TRUE(wgraph_add_edge(&g, 1, 2, 2));
    CHECK_TRUE(wgraph_add_edge(&g, 0, 2, 10));
    CHECK_TRUE(wgraph_add_edge(&g, 2, 3, 3));

    SECTION("dijkstra");
    dijkstra(&g, 0, dist);
    CHECK_INT_EQ(dist[0], 0);
    CHECK_INT_EQ(dist[1], 1);
    CHECK_INT_EQ(dist[2], 3); /* via 1, not the direct 10 */
    CHECK_INT_EQ(dist[3], 6);
    CHECK_INT_EQ(dist[4], -1); /* unreachable */

    SECTION("bellman_ford matches dijkstra");
    CHECK_TRUE(bellman_ford(&g, 0, dist2));
    for (int v = 0; v < 5; v++) {
        CHECK_INT_EQ(dist2[v], dist[v]);
    }

    /* from a different source: 2 only reaches itself and 3 */
    SECTION("dijkstra from other source");
    dijkstra(&g, 2, dist);
    CHECK_INT_EQ(dist[2], 0);
    CHECK_INT_EQ(dist[3], 3);
    CHECK_INT_EQ(dist[0], -1);
    CHECK_INT_EQ(dist[1], -1);
    wgraph_free(&g);
    wgraph_free(&g); /* double free is safe */

    /* ---- negative edges (no negative cycle): Bellman-Ford only ---- */
    SECTION("bellman_ford negative edges");
    CHECK_TRUE(wgraph_init(&g, 4));
    CHECK_TRUE(wgraph_add_edge(&g, 0, 1, 4));
    CHECK_TRUE(wgraph_add_edge(&g, 0, 2, 5));
    CHECK_TRUE(wgraph_add_edge(&g, 2, 1, -3)); /* 0->2->1 = 2, beats 4 */
    CHECK_TRUE(wgraph_add_edge(&g, 1, 3, 1));
    CHECK_TRUE(bellman_ford(&g, 0, dist));
    CHECK_INT_EQ(dist[0], 0);
    CHECK_INT_EQ(dist[1], 2);
    CHECK_INT_EQ(dist[2], 5);
    CHECK_INT_EQ(dist[3], 3);
    wgraph_free(&g);

    /* ---- reachable negative cycle -> false ---- */
    SECTION("bellman_ford reachable neg cycle");
    CHECK_TRUE(wgraph_init(&g, 4));
    CHECK_TRUE(wgraph_add_edge(&g, 0, 1, 1));
    CHECK_TRUE(wgraph_add_edge(&g, 1, 2, -2));
    CHECK_TRUE(wgraph_add_edge(&g, 2, 1, 1)); /* 1->2->1 loses 1 each lap */
    CHECK_TRUE(wgraph_add_edge(&g, 2, 3, 1));
    CHECK_FALSE(bellman_ford(&g, 0, dist));
    wgraph_free(&g);

    /* ---- UNREACHABLE negative cycle -> still true ---- */
    SECTION("bellman_ford unreachable neg cycle");
    CHECK_TRUE(wgraph_init(&g, 4));
    CHECK_TRUE(wgraph_add_edge(&g, 0, 1, 7));
    CHECK_TRUE(wgraph_add_edge(&g, 2, 3, -5)); /* cycle 2<->3 ... */
    CHECK_TRUE(wgraph_add_edge(&g, 3, 2, -5)); /* ...not reachable from 0 */
    CHECK_TRUE(bellman_ford(&g, 0, dist));
    CHECK_INT_EQ(dist[0], 0);
    CHECK_INT_EQ(dist[1], 7);
    CHECK_INT_EQ(dist[2], -1);
    CHECK_INT_EQ(dist[3], -1);
    wgraph_free(&g);

    /* single vertex, no edges */
    SECTION("single vertex no edges");
    CHECK_TRUE(wgraph_init(&g, 1));
    dijkstra(&g, 0, dist);
    CHECK_INT_EQ(dist[0], 0);
    CHECK_TRUE(bellman_ford(&g, 0, dist));
    CHECK_INT_EQ(dist[0], 0);
    wgraph_free(&g);

    /* ---- randomized cross-check: with non-negative weights,
     *      dijkstra and bellman_ford must agree exactly ---- */
    SECTION("differential dijkstra vs bellman_ford");
    srand(404009);
    for (int trial = 0; trial < 50; trial++) {
        int n = 2 + rand() % 24;
        CHECK_TRUE_MSG(wgraph_init(&g, n), "trial=%d n=%d", trial, n);
        int m = rand() % 100;
        for (int e = 0; e < m; e++) {
            int u = rand() % n;
            int v = rand() % n;
            int w = rand() % 50; /* non-negative */
            CHECK_TRUE_MSG(wgraph_add_edge(&g, u, v, w),
                           "trial=%d e=%d u=%d v=%d w=%d", trial, e, u, v, w);
        }
        int src = rand() % n;
        dijkstra(&g, src, dist);
        CHECK_TRUE_MSG(bellman_ford(&g, src, dist2), "trial=%d n=%d src=%d",
                       trial, n, src);
        CHECK_INT_EQ_MSG(dist[src], 0, "trial=%d n=%d src=%d", trial, n, src);
        for (int v = 0; v < n; v++) {
            CHECK_INT_EQ_MSG(dist[v], dist2[v], "trial=%d n=%d src=%d v=%d",
                             trial, n, src, v);
        }
        wgraph_free(&g);
    }

    CTEST_END();
}
