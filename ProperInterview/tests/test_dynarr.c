#include "ctest.h"
#include "dynarr.h"

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
    DynArr da;

    /* ---- da_init / empty state ---- */
    SECTION("da_init");
    da_init(&da);
    CHECK_UINT_EQ(da_len(&da), 0);
    CHECK_UINT_EQ(da_cap(&da), 0);
    CHECK_PTR_NULL(da.data);

    /* ---- da_free: safe on empty, resets state ---- */
    SECTION("da_free");
    da_free(&da); /* free of a never-allocated array must be safe */
    CHECK_UINT_EQ(da_len(&da), 0);
    CHECK_PTR_NULL(da.data);
    da_free(&da); /* and safe to call twice */

    /* ---- da_push growth policy: 0 -> 8 -> 16 ---- */
    SECTION("da_push");
    da_init(&da);
    CHECK_TRUE(da_push(&da, 100));
    CHECK_UINT_EQ(da_cap(&da), 8);
    for (int x = 101; x <= 107; x++) {
        CHECK_TRUE(da_push(&da, x));
    }
    CHECK_UINT_EQ(da_len(&da), 8);
    CHECK_UINT_EQ(da_cap(&da), 8);
    CHECK_TRUE(da_push(&da, 108)); /* ninth push doubles */
    CHECK_UINT_EQ(da_cap(&da), 16);
    CHECK_UINT_EQ(da_len(&da), 9);
    CHECK_INT_EQ(da_get(&da, 0), 100);
    CHECK_INT_EQ(da_get(&da, 8), 108);

    /* ---- da_pop ---- */
    SECTION("da_pop");
    int out = -999;
    CHECK_TRUE(da_pop(&da, &out));
    CHECK_INT_EQ(out, 108);
    CHECK_UINT_EQ(da_len(&da), 8);
    while (da_len(&da) > 0) {
        CHECK_TRUE(da_pop(&da, &out));
    }
    CHECK_INT_EQ(out, 100); /* last pop returned the first element */
    out = 424242;
    CHECK_FALSE(da_pop(&da, &out)); /* empty: false, *out untouched */
    CHECK_INT_EQ(out, 424242);
    da_free(&da);

    /* ---- da_get / da_set ---- */
    SECTION("da_get / da_set");
    da_init(&da);
    for (int x = 0; x < 5; x++) {
        CHECK_TRUE(da_push(&da, x * 10));
    }
    CHECK_INT_EQ(da_get(&da, 0), 0);
    CHECK_INT_EQ(da_get(&da, 4), 40);
    da_set(&da, 2, -5);
    CHECK_INT_EQ(da_get(&da, 2), -5);
    CHECK_INT_EQ(da_get(&da, 1), 10); /* neighbors untouched */
    CHECK_INT_EQ(da_get(&da, 3), 30);
    CHECK_UINT_EQ(da_len(&da), 5);

    /* ---- da_insert_at ---- */
    SECTION("da_insert_at");
    CHECK_TRUE(da_insert_at(&da, 0, -1)); /* front */
    CHECK_INT_EQ(da_get(&da, 0), -1);
    CHECK_INT_EQ(da_get(&da, 1), 0);
    CHECK_TRUE(da_insert_at(&da, da_len(&da), -2)); /* end == append */
    CHECK_INT_EQ(da_get(&da, da_len(&da) - 1), -2);
    CHECK_TRUE(da_insert_at(&da, 3, 555)); /* middle */
    CHECK_INT_EQ(da_get(&da, 3), 555);
    CHECK_INT_EQ(da_get(&da, 4), -5); /* shifted right */
    CHECK_UINT_EQ(da_len(&da), 8);

    /* ---- da_remove_at ---- */
    SECTION("da_remove_at");
    da_remove_at(&da, 3); /* remove the 555 */
    CHECK_INT_EQ(da_get(&da, 3), -5);
    da_remove_at(&da, 0); /* remove the front -1 */
    CHECK_INT_EQ(da_get(&da, 0), 0);
    da_remove_at(&da, da_len(&da) - 1); /* remove the tail -2 */
    CHECK_UINT_EQ(da_len(&da), 5);
    CHECK_INT_EQ(da_get(&da, da_len(&da) - 1), 40);
    da_free(&da);
    CHECK_UINT_EQ(da_len(&da), 0);
    CHECK_PTR_NULL(da.data);

    /* ---- randomized differential test vs the shadow array ---- */
    SECTION("differential vs oracle");
    srand(20260708);
    da_init(&da);
    for (int op = 0; op < 2000; op++) {
        int r = rand() % 6;
        if (r == 0 || (shadow_len == 0 && r != 5)) { /* push */
            if (shadow_len < SHADOW_CAP) {
                int x = rand();
                CHECK_TRUE_MSG(da_push(&da, x), "op=%d r=%d", op, r);
                shadow[shadow_len++] = x;
            }
        } else if (r == 1) { /* pop */
            int got = -1;
            CHECK_TRUE_MSG(da_pop(&da, &got), "op=%d r=%d", op, r);
            CHECK_INT_EQ_MSG(got, shadow[shadow_len - 1], "op=%d r=%d", op, r);
            shadow_len--;
        } else if (r == 2) { /* set */
            size_t i = (size_t)rand() % shadow_len;
            int x = rand();
            da_set(&da, i, x);
            shadow[i] = x;
        } else if (r == 3) { /* get */
            size_t i = (size_t)rand() % shadow_len;
            CHECK_INT_EQ_MSG(da_get(&da, i), shadow[i], "op=%d r=%d i=%zu", op,
                             r, i);
        } else if (r == 4) { /* remove_at */
            size_t i = (size_t)rand() % shadow_len;
            da_remove_at(&da, i);
            shadow_remove_at(i);
        } else { /* insert_at (i == len allowed) */
            if (shadow_len < SHADOW_CAP) {
                size_t i = (size_t)rand() % (shadow_len + 1);
                int x = rand();
                CHECK_TRUE_MSG(da_insert_at(&da, i, x), "op=%d r=%d i=%zu", op,
                               r, i);
                shadow_insert_at(i, x);
            }
        }
        CHECK_UINT_EQ_MSG(da_len(&da), shadow_len, "op=%d r=%d", op, r);
    }
    /* final full comparison */
    SECTION("differential final compare");
    for (size_t i = 0; i < shadow_len; i++) {
        CHECK_INT_EQ_MSG(da_get(&da, i), shadow[i], "i=%zu", i);
    }
    da_free(&da);

    CTEST_END();
}
