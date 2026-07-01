#include "ctest.h"
#include "mst.h"

#include <stdlib.h>

int main(void) {
    /*
     * Classic small graph (CLRS-style), MST weight known by hand:
     *
     *     0 --4-- 1
     *     0 --8-- 2
     *     1 --8-- 2
     *     1 --11- 3
     *     2 --7-- 3
     *     2 --1-- 4
     *     3 --2-- 4
     *     3 --6-- 5
     *     4 --9-- 5
     *
     * MST: 0-1(4), 2-4(1), 3-4(2), 2-3? no — take 0-1=4, 2-4=1, 3-4=2,
     * 3-5=6, and connect {0,1} to the rest via 2-0=8 or 1-2=8 -> total
     * 4+1+2+6+8 = 21.
     */
    MstEdge g1[] = {
        {0, 1, 4}, {0, 2, 8}, {1, 2, 8}, {1, 3, 11}, {2, 3, 7},
        {2, 4, 1}, {3, 4, 2}, {3, 5, 6}, {4, 5, 9},
    };
    CHECK_INT_EQ(kruskal(6, g1, 9), 21);
    CHECK_INT_EQ(prim(6, g1, 9), 21);

    /* triangle: MST takes the two cheap sides */
    MstEdge tri[] = {{0, 1, 1}, {1, 2, 2}, {0, 2, 10}};
    CHECK_INT_EQ(kruskal(3, tri, 3), 3);
    CHECK_INT_EQ(prim(3, tri, 3), 3);

    /* parallel edges: the cheaper duplicate must win */
    MstEdge par[] = {{0, 1, 9}, {0, 1, 2}};
    CHECK_INT_EQ(kruskal(2, par, 2), 2);
    CHECK_INT_EQ(prim(2, par, 2), 2);

    /* self-loops must never be picked */
    MstEdge loops[] = {{0, 0, -100}, {0, 1, 5}, {1, 1, -100}};
    CHECK_INT_EQ(kruskal(2, loops, 3), 5);
    CHECK_INT_EQ(prim(2, loops, 3), 5);

    /* single vertex: empty MST */
    CHECK_INT_EQ(kruskal(1, NULL, 0), 0);
    CHECK_INT_EQ(prim(1, NULL, 0), 0);

    /* disconnected graph: no spanning tree */
    MstEdge disc[] = {{0, 1, 1}, {2, 3, 1}};
    CHECK_INT_EQ(kruskal(4, disc, 2), -1);
    CHECK_INT_EQ(prim(4, disc, 2), -1);

    /* two vertices, no edges: also disconnected */
    CHECK_INT_EQ(kruskal(2, NULL, 0), -1);
    CHECK_INT_EQ(prim(2, NULL, 0), -1);

    /* kruskal must not clobber the caller's edge array */
    MstEdge keep[] = {{0, 1, 3}, {1, 2, 1}, {0, 2, 2}};
    CHECK_INT_EQ(kruskal(3, keep, 3), 3);
    CHECK_INT_EQ(keep[0].w, 3);
    CHECK_INT_EQ(keep[1].w, 1);
    CHECK_INT_EQ(keep[2].w, 2);

    /* ---- randomized cross-check on guaranteed-connected graphs ---- */
    srand(505010);
    static MstEdge edges[600];
    for (int trial = 0; trial < 60; trial++) {
        int n = 2 + rand() % 30;
        size_t m = 0;
        /* random spanning tree first: connect i to a random j < i */
        for (int i = 1; i < n; i++) {
            edges[m].u = rand() % i;
            edges[m].v = i;
            edges[m].w = rand() % 100;
            m++;
        }
        /* then extra random edges (may duplicate, may self-loop) */
        int extra = rand() % 60;
        for (int e = 0; e < extra; e++) {
            edges[m].u = rand() % n;
            edges[m].v = rand() % n;
            edges[m].w = rand() % 100;
            m++;
        }
        long long k = kruskal(n, edges, m);
        long long p = prim(n, edges, m);
        CHECK_TRUE(k >= 0); /* connected by construction */
        CHECK_INT_EQ(k, p); /* the two algorithms must agree */
        /* MST weight can never exceed the spanning tree we built */
        long long st = 0;
        for (int i = 0; i < n - 1; i++) {
            st += edges[i].w;
        }
        CHECK_TRUE(k <= st);
    }

    CTEST_END();
}
