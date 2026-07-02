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
    SECTION("avl_init / empty");
    avl_init(&t);
    CHECK_UINT_EQ(avl_size(&t), 0);
    CHECK_INT_EQ(avl_height(&t), 0);
    CHECK_FALSE(avl_contains(&t, 1));
    CHECK_FALSE(avl_remove(&t, 1));
    CHECK_TRUE(avl_validate(&t));
    avl_free(&t);

    /* ---- the four classic rotation cases, checked via validate() ---- */
    /* LL: insert descending */
    SECTION("avl_insert LL rotation");
    avl_init(&t);
    CHECK_TRUE(avl_insert(&t, 30));
    CHECK_TRUE(avl_insert(&t, 20));
    CHECK_TRUE(avl_insert(&t, 10)); /* right rotation */
    CHECK_TRUE(avl_validate(&t));
    CHECK_INT_EQ(avl_height(&t), 2); /* 3 nodes must pack into height 2 */
    CHECK_INT_EQ(t.root->key, 20);
    avl_free(&t);

    /* RR: insert ascending */
    SECTION("avl_insert RR rotation");
    avl_init(&t);
    CHECK_TRUE(avl_insert(&t, 10));
    CHECK_TRUE(avl_insert(&t, 20));
    CHECK_TRUE(avl_insert(&t, 30)); /* left rotation */
    CHECK_TRUE(avl_validate(&t));
    CHECK_INT_EQ(t.root->key, 20);
    avl_free(&t);

    /* LR */
    SECTION("avl_insert LR rotation");
    avl_init(&t);
    CHECK_TRUE(avl_insert(&t, 30));
    CHECK_TRUE(avl_insert(&t, 10));
    CHECK_TRUE(avl_insert(&t, 20)); /* left-right double rotation */
    CHECK_TRUE(avl_validate(&t));
    CHECK_INT_EQ(t.root->key, 20);
    avl_free(&t);

    /* RL */
    SECTION("avl_insert RL rotation");
    avl_init(&t);
    CHECK_TRUE(avl_insert(&t, 10));
    CHECK_TRUE(avl_insert(&t, 30));
    CHECK_TRUE(avl_insert(&t, 20)); /* right-left double rotation */
    CHECK_TRUE(avl_validate(&t));
    CHECK_INT_EQ(t.root->key, 20);
    avl_free(&t);

    /* ---- sorted insertion: height must stay logarithmic ---- */
    SECTION("avl_insert sorted / logarithmic height");
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
    SECTION("avl_remove all + validate");
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
    SECTION("differential vs oracle + validate");
    srand(888004);
    avl_init(&t);
    for (int op = 0; op < 1000; op++) {
        int r = rand() % 3;
        int key = rand() % 300;
        if (r == 0) {
            CHECK_INT_EQ_MSG(avl_insert(&t, key), ora_insert(key),
                             "op=%d key=%d", op, key);
        } else if (r == 1) {
            CHECK_INT_EQ_MSG(avl_remove(&t, key), ora_remove(key),
                             "op=%d key=%d", op, key);
        } else {
            CHECK_INT_EQ_MSG(avl_contains(&t, key), ora_contains(key),
                             "op=%d key=%d", op, key);
        }
        CHECK_TRUE_MSG(avl_validate(&t), "op=%d r=%d key=%d", op, r,
                       key); /* after every single op */
        CHECK_UINT_EQ_MSG(avl_size(&t), olen, "op=%d r=%d key=%d", op, r, key);
    }
    /* final inorder must equal the oracle's sorted contents */
    SECTION("differential final inorder");
    static int inord[ORA_CAP];
    avl_inorder(&t, inord);
    for (size_t i = 0; i < olen; i++) {
        CHECK_INT_EQ_MSG(inord[i], ora[i], "i=%zu", i);
    }
    avl_free(&t);

    CTEST_END();
}
