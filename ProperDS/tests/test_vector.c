#include "ctest.h"
#include "vector.h"

#include <stdlib.h>

/* Differential oracle: a fixed-size shadow array with the same contents. */
#define SHADOW_CAP 4096
static int shadow[SHADOW_CAP];
static size_t shadow_len = 0;

static void shadow_insert_at(size_t i, int x) {
    for (size_t k = shadow_len; k > i; k--) {
        shadow[k] = shadow[k - 1];
    }
    shadow[i] = x;
    shadow_len++;
}

static void shadow_remove_at(size_t i) {
    for (size_t k = i; k + 1 < shadow_len; k++) {
        shadow[k] = shadow[k + 1];
    }
    shadow_len--;
}

int main(void) {
    Vector v;

    /* ---- unit tests: init / empty state ---- */
    vector_init(&v);
    CHECK_UINT_EQ(vector_len(&v), 0);
    CHECK_UINT_EQ(vector_cap(&v), 0);
    CHECK_PTR_NULL(v.data);
    CHECK_TRUE(vector_validate(&v));
    vector_free(&v); /* free of empty vector must be safe */

    /* ---- unit tests: growth policy 0 -> 4 -> 8 ---- */
    vector_init(&v);
    CHECK_TRUE(vector_push(&v, 10));
    CHECK_UINT_EQ(vector_cap(&v), 4);
    CHECK_TRUE(vector_push(&v, 11));
    CHECK_TRUE(vector_push(&v, 12));
    CHECK_TRUE(vector_push(&v, 13));
    CHECK_UINT_EQ(vector_cap(&v), 4);
    CHECK_TRUE(vector_push(&v, 14));
    CHECK_UINT_EQ(vector_cap(&v), 8);
    CHECK_UINT_EQ(vector_len(&v), 5);
    CHECK_TRUE(vector_validate(&v));

    /* ---- unit tests: get / set / pop ---- */
    CHECK_INT_EQ(vector_get(&v, 0), 10);
    CHECK_INT_EQ(vector_get(&v, 4), 14);
    vector_set(&v, 2, 99);
    CHECK_INT_EQ(vector_get(&v, 2), 99);
    CHECK_INT_EQ(vector_pop(&v), 14);
    CHECK_UINT_EQ(vector_len(&v), 4);

    /* ---- unit tests: insert_at / remove_at ---- */
    CHECK_TRUE(vector_insert_at(&v, 0, -1)); /* front */
    CHECK_INT_EQ(vector_get(&v, 0), -1);
    CHECK_INT_EQ(vector_get(&v, 1), 10);
    CHECK_TRUE(vector_insert_at(&v, vector_len(&v), -2)); /* == append */
    CHECK_INT_EQ(vector_get(&v, vector_len(&v) - 1), -2);
    CHECK_TRUE(vector_insert_at(&v, 3, 555)); /* middle */
    CHECK_INT_EQ(vector_get(&v, 3), 555);
    CHECK_UINT_EQ(vector_len(&v), 7);
    vector_remove_at(&v, 3);
    CHECK_INT_EQ(vector_get(&v, 3), 99);
    vector_remove_at(&v, 0);
    CHECK_INT_EQ(vector_get(&v, 0), 10);
    vector_remove_at(&v, vector_len(&v) - 1); /* last */
    CHECK_UINT_EQ(vector_len(&v), 4);
    CHECK_TRUE(vector_validate(&v));
    vector_free(&v);
    CHECK_UINT_EQ(vector_len(&v), 0);
    CHECK_PTR_NULL(v.data);

    /* ---- randomized differential test vs the shadow array ---- */
    srand(20260701);
    vector_init(&v);
    for (int op = 0; op < 3000; op++) {
        int r = rand() % 6;
        if (r == 0 || (shadow_len == 0 && r != 5)) { /* push */
            if (shadow_len < SHADOW_CAP) {
                int x = rand();
                CHECK_TRUE(vector_push(&v, x));
                shadow[shadow_len++] = x;
            }
        } else if (r == 1) { /* pop */
            int got = vector_pop(&v);
            CHECK_INT_EQ(got, shadow[shadow_len - 1]);
            shadow_len--;
        } else if (r == 2) { /* set */
            size_t i = (size_t)rand() % shadow_len;
            int x = rand();
            vector_set(&v, i, x);
            shadow[i] = x;
        } else if (r == 3) { /* get */
            size_t i = (size_t)rand() % shadow_len;
            CHECK_INT_EQ(vector_get(&v, i), shadow[i]);
        } else if (r == 4) { /* remove_at */
            size_t i = (size_t)rand() % shadow_len;
            vector_remove_at(&v, i);
            shadow_remove_at(i);
        } else { /* insert_at (i == len allowed) */
            if (shadow_len < SHADOW_CAP) {
                size_t i = (size_t)rand() % (shadow_len + 1);
                int x = rand();
                CHECK_TRUE(vector_insert_at(&v, i, x));
                shadow_insert_at(i, x);
            }
        }
        CHECK_TRUE(vector_validate(&v));
        CHECK_UINT_EQ(vector_len(&v), shadow_len);
    }
    /* final full comparison */
    for (size_t i = 0; i < shadow_len; i++) {
        CHECK_INT_EQ(vector_get(&v, i), shadow[i]);
    }
    vector_free(&v);

    CTEST_END();
}
