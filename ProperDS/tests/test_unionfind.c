#include "ctest.h"
#include "unionfind.h"

#include <stdlib.h>

/* Differential oracle: brute-force component labels; union relabels. */
#define ON 64
static int label[ON];

static void ora_init(void) {
    for (int i = 0; i < ON; i++) {
        label[i] = i;
    }
}

static bool ora_union(int a, int b) {
    if (label[a] == label[b]) {
        return false;
    }
    int from = label[b], to = label[a];
    for (int i = 0; i < ON; i++) {
        if (label[i] == from) {
            label[i] = to;
        }
    }
    return true;
}

static int ora_count(void) {
    /* count distinct labels the dumb way */
    int c = 0;
    for (int i = 0; i < ON; i++) {
        bool first = true;
        for (int j = 0; j < i; j++) {
            if (label[j] == label[i]) {
                first = false;
                break;
            }
        }
        if (first) {
            c++;
        }
    }
    return c;
}

int main(void) {
    UnionFind uf;

    /* ---- singletons after init ---- */
    CHECK_TRUE(uf_init(&uf, 10));
    CHECK_INT_EQ(uf_count(&uf), 10);
    for (int i = 0; i < 10; i++) {
        CHECK_INT_EQ(uf_find(&uf, i), i);
    }
    CHECK_FALSE(uf_connected(&uf, 0, 1));
    CHECK_TRUE(uf_connected(&uf, 3, 3)); /* reflexive */

    /* ---- union semantics ---- */
    CHECK_TRUE(uf_union(&uf, 0, 1));
    CHECK_INT_EQ(uf_count(&uf), 9);
    CHECK_TRUE(uf_connected(&uf, 0, 1));
    CHECK_FALSE(uf_union(&uf, 0, 1)); /* already merged: count unchanged */
    CHECK_FALSE(uf_union(&uf, 1, 0));
    CHECK_INT_EQ(uf_count(&uf), 9);

    /* transitivity through chained unions */
    CHECK_TRUE(uf_union(&uf, 1, 2));
    CHECK_TRUE(uf_union(&uf, 3, 4));
    CHECK_TRUE(uf_union(&uf, 2, 4)); /* merges {0,1,2} with {3,4} */
    CHECK_INT_EQ(uf_count(&uf), 6);
    CHECK_TRUE(uf_connected(&uf, 0, 3));
    CHECK_TRUE(uf_connected(&uf, 4, 1));
    CHECK_FALSE(uf_connected(&uf, 0, 5));

    /* all find()s within a set agree on one representative */
    int rep = uf_find(&uf, 0);
    CHECK_INT_EQ(uf_find(&uf, 1), rep);
    CHECK_INT_EQ(uf_find(&uf, 2), rep);
    CHECK_INT_EQ(uf_find(&uf, 3), rep);
    CHECK_INT_EQ(uf_find(&uf, 4), rep);

    /* merge everything -> one component */
    for (int i = 0; i < 9; i++) {
        (void)uf_union(&uf, i, i + 1);
    }
    CHECK_INT_EQ(uf_count(&uf), 1);
    CHECK_TRUE(uf_connected(&uf, 0, 9));
    /* path compression observable effect: find(x) twice is stable and
     * parent[x] now points straight at the root */
    int r = uf_find(&uf, 9);
    CHECK_INT_EQ(uf_find(&uf, 9), r);
    CHECK_INT_EQ(uf.parent[9], r);
    uf_free(&uf);
    uf_free(&uf); /* double free is safe */

    /* n == 0 edge case */
    CHECK_TRUE(uf_init(&uf, 0));
    CHECK_INT_EQ(uf_count(&uf), 0);
    uf_free(&uf);

    /* ---- randomized differential test vs the label oracle ---- */
    srand(202007);
    ora_init();
    CHECK_TRUE(uf_init(&uf, ON));
    for (int op = 0; op < 3000; op++) {
        int a = rand() % ON;
        int b = rand() % ON;
        if (rand() % 2 == 0) {
            CHECK_INT_EQ(uf_union(&uf, a, b), ora_union(a, b));
        } else {
            CHECK_INT_EQ(uf_connected(&uf, a, b), label[a] == label[b]);
        }
        CHECK_INT_EQ(uf_count(&uf), ora_count());
    }
    /* final full pairwise agreement */
    for (int a = 0; a < ON; a++) {
        for (int b = a + 1; b < ON; b++) {
            CHECK_INT_EQ(uf_connected(&uf, a, b), label[a] == label[b]);
        }
    }
    uf_free(&uf);

    CTEST_END();
}
