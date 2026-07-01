#include "ctest.h"
#include "graph.h"

#include <stdlib.h>

static void check_order(const int *got, const int *want, int n) {
    for (int i = 0; i < n; i++) {
        CHECK_INT_EQ(got[i], want[i]);
    }
}

int main(void) {
    Graph g;
    int order[64], order2[64], topo[64];

    /*
     * Fixed test graph (directed):
     *
     *      0 -> 1 -> 3
     *      0 -> 2      \
     *      1 -> 4       -> 5
     *      2 -> 4      /
     *      4 -> 5
     *      6 (isolated)
     */
    CHECK_TRUE(graph_init(&g, 7));
    CHECK_TRUE(graph_add_edge(&g, 0, 1));
    CHECK_TRUE(graph_add_edge(&g, 0, 2));
    CHECK_TRUE(graph_add_edge(&g, 1, 3));
    CHECK_TRUE(graph_add_edge(&g, 1, 4));
    CHECK_TRUE(graph_add_edge(&g, 2, 4));
    CHECK_TRUE(graph_add_edge(&g, 3, 5));
    CHECK_TRUE(graph_add_edge(&g, 4, 5));

    /* ---- BFS: level order, neighbors in insertion order ---- */
    CHECK_INT_EQ(graph_bfs_order(&g, 0, order), 6); /* 6 unreachable */
    int want_bfs[] = {0, 1, 2, 3, 4, 5};
    check_order(order, want_bfs, 6);

    /* ---- recursive DFS: preorder, first-neighbor-first ---- */
    CHECK_INT_EQ(graph_dfs_order(&g, 0, order), 6);
    int want_dfs[] = {0, 1, 3, 5, 4, 2};
    check_order(order, want_dfs, 6);

    /* ---- iterative DFS must match the recursive order exactly ---- */
    CHECK_INT_EQ(graph_dfs_order_iter(&g, 0, order2), 6);
    check_order(order2, want_dfs, 6);

    /* traversals from a non-root source */
    CHECK_INT_EQ(graph_bfs_order(&g, 2, order), 3); /* 2 4 5 */
    int want_bfs2[] = {2, 4, 5};
    check_order(order, want_bfs2, 3);
    CHECK_INT_EQ(graph_dfs_order(&g, 6, order), 1); /* isolated */
    CHECK_INT_EQ(order[0], 6);
    CHECK_INT_EQ(graph_dfs_order_iter(&g, 6, order), 1);
    CHECK_INT_EQ(order[0], 6);

    /* ---- topo sort on this DAG: smallest-first Kahn is deterministic ---- */
    CHECK_TRUE(graph_topo_sort(&g, topo));
    int want_topo[] = {0, 1, 2, 3, 4, 5, 6};
    check_order(topo, want_topo, 7);
    CHECK_FALSE(graph_has_cycle(&g));

    /* topo order must respect every edge (generic validity check) */
    {
        int pos[7];
        for (int i = 0; i < 7; i++) {
            pos[topo[i]] = i;
        }
        CHECK_TRUE(pos[0] < pos[1] && pos[1] < pos[3] && pos[3] < pos[5]);
        CHECK_TRUE(pos[2] < pos[4] && pos[4] < pos[5]);
    }
    graph_free(&g);
    graph_free(&g); /* double free is safe */

    /* ---- cycle detection ---- */
    CHECK_TRUE(graph_init(&g, 4));
    CHECK_TRUE(graph_add_edge(&g, 0, 1));
    CHECK_TRUE(graph_add_edge(&g, 1, 2));
    CHECK_TRUE(graph_add_edge(&g, 2, 0)); /* 0 -> 1 -> 2 -> 0 */
    CHECK_TRUE(graph_add_edge(&g, 2, 3));
    CHECK_TRUE(graph_has_cycle(&g));
    CHECK_FALSE(graph_topo_sort(&g, topo));
    graph_free(&g);

    /* self-loop is a cycle */
    CHECK_TRUE(graph_init(&g, 2));
    CHECK_TRUE(graph_add_edge(&g, 1, 1));
    CHECK_TRUE(graph_has_cycle(&g));
    graph_free(&g);

    /* edgeless graph: trivially acyclic, topo = identity */
    CHECK_TRUE(graph_init(&g, 3));
    CHECK_FALSE(graph_has_cycle(&g));
    CHECK_TRUE(graph_topo_sort(&g, topo));
    int want_id[] = {0, 1, 2};
    check_order(topo, want_id, 3);
    CHECK_INT_EQ(graph_bfs_order(&g, 1, order), 1);
    CHECK_INT_EQ(order[0], 1);
    graph_free(&g);

    /* ---- randomized: on random DAGs (edges only low->high, so acyclic
     *      by construction) topo must succeed and respect all edges;
     *      dfs_order_iter must always equal dfs_order; adding a back
     *      edge must create a cycle ---- */
    srand(303008);
    for (int trial = 0; trial < 60; trial++) {
        int n = 2 + rand() % 20;
        CHECK_TRUE(graph_init(&g, n));
        int eu[128], ev[128];
        int m = rand() % 60;
        int added = 0;
        for (int e = 0; e < m; e++) {
            int u = rand() % n;
            int v = rand() % n;
            if (u == v) {
                continue;
            }
            if (u > v) { /* force low -> high: guarantees a DAG */
                int t = u;
                u = v;
                v = t;
            }
            CHECK_TRUE(graph_add_edge(&g, u, v));
            eu[added] = u;
            ev[added] = v;
            added++;
        }
        CHECK_FALSE(graph_has_cycle(&g));
        CHECK_TRUE(graph_topo_sort(&g, topo));
        int pos[32];
        for (int i = 0; i < n; i++) {
            pos[topo[i]] = i;
        }
        for (int e = 0; e < added; e++) {
            CHECK_TRUE(pos[eu[e]] < pos[ev[e]]);
        }
        int src = rand() % n;
        int c1 = graph_dfs_order(&g, src, order);
        int c2 = graph_dfs_order_iter(&g, src, order2);
        CHECK_INT_EQ(c1, c2);
        check_order(order, order2, c1);
        /* BFS reaches exactly the same vertex set as DFS */
        int c3 = graph_bfs_order(&g, src, order2);
        CHECK_INT_EQ(c3, c1);
        /* close a cycle: from the last topo vertex with an incoming edge
         * back to the first topo vertex on that path */
        if (added > 0) {
            CHECK_TRUE(graph_add_edge(&g, ev[0], eu[0]));
            CHECK_TRUE(graph_has_cycle(&g));
        }
        graph_free(&g);
    }

    CTEST_END();
}
