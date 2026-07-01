#include "ctest.h"
#include "bst.h"

#include <stdlib.h>

/* Differential oracle: a sorted array of unique ints. */
#define ORA_CAP 1024
static int ora[ORA_CAP];
static size_t olen = 0;

static size_t ora_lower(int key) {
    size_t i = 0;
    while (i < olen && ora[i] < key) {
        i++;
    }
    return i;
}

static bool ora_contains(int key) {
    size_t i = ora_lower(key);
    return i < olen && ora[i] == key;
}

static bool ora_insert(int key) {
    size_t i = ora_lower(key);
    if (i < olen && ora[i] == key) {
        return false;
    }
    for (size_t k = olen; k > i; k--) {
        ora[k] = ora[k - 1];
    }
    ora[i] = key;
    olen++;
    return true;
}

static bool ora_remove(int key) {
    size_t i = ora_lower(key);
    if (i == olen || ora[i] != key) {
        return false;
    }
    for (size_t k = i; k + 1 < olen; k++) {
        ora[k] = ora[k + 1];
    }
    olen--;
    return true;
}

int main(void) {
    Bst t;
    int out = -777;

    /* ---- empty tree ---- */
    bst_init(&t);
    CHECK_UINT_EQ(bst_size(&t), 0);
    CHECK_FALSE(bst_contains(&t, 5));
    CHECK_FALSE(bst_remove(&t, 5));
    CHECK_FALSE(bst_min(&t, &out));
    CHECK_FALSE(bst_max(&t, &out));
    CHECK_INT_EQ(out, -777);
    CHECK_TRUE(bst_validate(&t));
    bst_free(&t); /* safe on empty */

    /* ---- insert, duplicates, min/max, inorder ---- */
    bst_init(&t);
    int keys[] = {50, 30, 70, 20, 40, 60, 80};
    for (size_t i = 0; i < 7; i++) {
        CHECK_TRUE(bst_insert(&t, keys[i]));
    }
    CHECK_FALSE(bst_insert(&t, 40)); /* duplicate rejected */
    CHECK_UINT_EQ(bst_size(&t), 7);
    CHECK_TRUE(bst_validate(&t));
    CHECK_TRUE(bst_min(&t, &out));
    CHECK_INT_EQ(out, 20);
    CHECK_TRUE(bst_max(&t, &out));
    CHECK_INT_EQ(out, 80);
    int sorted[16];
    bst_inorder(&t, sorted);
    int want_sorted[] = {20, 30, 40, 50, 60, 70, 80};
    for (size_t i = 0; i < 7; i++) {
        CHECK_INT_EQ(sorted[i], want_sorted[i]);
    }

    /* ---- remove: all three cases ---- */
    CHECK_TRUE(bst_remove(&t, 20)); /* case 1: leaf */
    CHECK_FALSE(bst_contains(&t, 20));
    CHECK_TRUE(bst_validate(&t));

    CHECK_TRUE(bst_insert(&t, 45)); /* give 40 one child */
    CHECK_TRUE(bst_remove(&t, 40)); /* case 2: one child */
    CHECK_TRUE(bst_contains(&t, 45));
    CHECK_TRUE(bst_validate(&t));

    CHECK_TRUE(bst_remove(&t, 50)); /* case 3: two children (the root) */
    CHECK_FALSE(bst_contains(&t, 50));
    CHECK_TRUE(bst_contains(&t, 45));
    CHECK_TRUE(bst_contains(&t, 70));
    CHECK_TRUE(bst_validate(&t));
    CHECK_UINT_EQ(bst_size(&t), 5);
    CHECK_FALSE(bst_remove(&t, 999)); /* absent */
    bst_free(&t);
    CHECK_UINT_EQ(bst_size(&t), 0);

    /* ---- degenerate shape: sorted insertion still works ---- */
    bst_init(&t);
    for (int i = 0; i < 64; i++) {
        CHECK_TRUE(bst_insert(&t, i));
    }
    CHECK_TRUE(bst_validate(&t));
    CHECK_TRUE(bst_min(&t, &out));
    CHECK_INT_EQ(out, 0);
    CHECK_TRUE(bst_max(&t, &out));
    CHECK_INT_EQ(out, 63);
    bst_free(&t);

    /* ---- randomized differential test vs the sorted-array oracle ---- */
    srand(777003);
    bst_init(&t);
    for (int op = 0; op < 3000; op++) {
        int r = rand() % 3;
        int key = rand() % 400; /* small range: plenty of dup/remove hits */
        if (r == 0) {
            CHECK_INT_EQ(bst_insert(&t, key), ora_insert(key));
        } else if (r == 1) {
            CHECK_INT_EQ(bst_remove(&t, key), ora_remove(key));
        } else {
            CHECK_INT_EQ(bst_contains(&t, key), ora_contains(key));
        }
        CHECK_UINT_EQ(bst_size(&t), olen);
        if (op % 50 == 0) {
            CHECK_TRUE(bst_validate(&t));
        }
    }
    CHECK_TRUE(bst_validate(&t));
    /* min/max and full inorder against the oracle */
    if (olen > 0) {
        CHECK_TRUE(bst_min(&t, &out));
        CHECK_INT_EQ(out, ora[0]);
        CHECK_TRUE(bst_max(&t, &out));
        CHECK_INT_EQ(out, ora[olen - 1]);
    }
    static int inord[ORA_CAP];
    bst_inorder(&t, inord);
    for (size_t i = 0; i < olen; i++) {
        CHECK_INT_EQ(inord[i], ora[i]);
    }
    bst_free(&t);

    CTEST_END();
}
