#include "ctest.h"
#include "avl.h"

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
    Avl t;

    /* ---- empty tree ---- */
    avl_init(&t);
    CHECK_UINT_EQ(avl_size(&t), 0);
    CHECK_INT_EQ(avl_height(&t), 0);
    CHECK_FALSE(avl_contains(&t, 1));
    CHECK_FALSE(avl_remove(&t, 1));
    CHECK_TRUE(avl_validate(&t));
    avl_free(&t);

    /* ---- the four classic rotation cases, checked via validate() ---- */
    /* LL: insert descending */
    avl_init(&t);
    CHECK_TRUE(avl_insert(&t, 30));
    CHECK_TRUE(avl_insert(&t, 20));
    CHECK_TRUE(avl_insert(&t, 10)); /* right rotation */
    CHECK_TRUE(avl_validate(&t));
    CHECK_INT_EQ(avl_height(&t), 2); /* 3 nodes must pack into height 2 */
    CHECK_INT_EQ(t.root->key, 20);
    avl_free(&t);

    /* RR: insert ascending */
    avl_init(&t);
    CHECK_TRUE(avl_insert(&t, 10));
    CHECK_TRUE(avl_insert(&t, 20));
    CHECK_TRUE(avl_insert(&t, 30)); /* left rotation */
    CHECK_TRUE(avl_validate(&t));
    CHECK_INT_EQ(t.root->key, 20);
    avl_free(&t);

    /* LR */
    avl_init(&t);
    CHECK_TRUE(avl_insert(&t, 30));
    CHECK_TRUE(avl_insert(&t, 10));
    CHECK_TRUE(avl_insert(&t, 20)); /* left-right double rotation */
    CHECK_TRUE(avl_validate(&t));
    CHECK_INT_EQ(t.root->key, 20);
    avl_free(&t);

    /* RL */
    avl_init(&t);
    CHECK_TRUE(avl_insert(&t, 10));
    CHECK_TRUE(avl_insert(&t, 30));
    CHECK_TRUE(avl_insert(&t, 20)); /* right-left double rotation */
    CHECK_TRUE(avl_validate(&t));
    CHECK_INT_EQ(t.root->key, 20);
    avl_free(&t);

    /* ---- sorted insertion: height must stay logarithmic ---- */
    avl_init(&t);
    for (int i = 0; i < 1023; i++) {
        CHECK_TRUE(avl_insert(&t, i));
    }
    CHECK_FALSE(avl_insert(&t, 500)); /* duplicate */
    CHECK_UINT_EQ(avl_size(&t), 1023);
    CHECK_TRUE(avl_validate(&t));
    /* 1023 nodes: perfect tree height 10; AVL bound ~1.44*log2(n) = 14 */
    CHECK_TRUE(avl_height(&t) >= 10);
    CHECK_TRUE(avl_height(&t) <= 14);
    /* remove everything, validating along the way */
    for (int i = 0; i < 1023; i++) {
        CHECK_TRUE(avl_remove(&t, i));
        if (i % 64 == 0) {
            CHECK_TRUE(avl_validate(&t));
        }
    }
    CHECK_UINT_EQ(avl_size(&t), 0);
    CHECK_INT_EQ(avl_height(&t), 0);
    CHECK_TRUE(avl_validate(&t));
    avl_free(&t);

    /* ---- randomized differential test: 1000 insert/removes with
     *      validate() after EVERY operation ---- */
    srand(888004);
    avl_init(&t);
    for (int op = 0; op < 1000; op++) {
        int r = rand() % 3;
        int key = rand() % 300;
        if (r == 0) {
            CHECK_INT_EQ(avl_insert(&t, key), ora_insert(key));
        } else if (r == 1) {
            CHECK_INT_EQ(avl_remove(&t, key), ora_remove(key));
        } else {
            CHECK_INT_EQ(avl_contains(&t, key), ora_contains(key));
        }
        CHECK_TRUE(avl_validate(&t)); /* after every single op */
        CHECK_UINT_EQ(avl_size(&t), olen);
    }
    /* final inorder must equal the oracle's sorted contents */
    static int inord[ORA_CAP];
    avl_inorder(&t, inord);
    for (size_t i = 0; i < olen; i++) {
        CHECK_INT_EQ(inord[i], ora[i]);
    }
    avl_free(&t);

    CTEST_END();
}
