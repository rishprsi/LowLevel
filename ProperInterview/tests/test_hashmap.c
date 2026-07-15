#include "ctest.h"
#include "hashmap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Naive oracle for the differential test: a fixed array of (key, value)
 * pairs with linear-scan lookup. Keys come from a small pool so the random
 * walk produces plenty of hits, upserts, and deletes of existing keys.
 */
#define KEY_POOL 128
static char keypool[KEY_POOL][16];

static int oracle_val[KEY_POOL];
static bool oracle_present[KEY_POOL];

static size_t oracle_len(void) {
    size_t n = 0;
    for (int i = 0; i < KEY_POOL; i++) {
        n += oracle_present[i] ? 1 : 0;
    }
    return n;
}

int main(void) {
    for (int i = 0; i < KEY_POOL; i++) {
        snprintf(keypool[i], sizeof(keypool[i]), "key%d", i);
    }

    HashMap *m;
    int v;

    /* ---- hm_create ---- */
    SECTION("hm_create");
    m = hm_create(4);
    CHECK_PTR_NONNULL(m);
    CHECK_UINT_EQ(hm_len(m), 0);
    v = 31337;
    CHECK_FALSE(hm_get(m, "missing", &v));
    CHECK_INT_EQ(v, 31337); /* *out untouched on miss */

    /* ---- hm_put: insert + upsert + key is copied ---- */
    SECTION("hm_put");
    CHECK_TRUE(hm_put(m, "alpha", 1));
    CHECK_UINT_EQ(hm_len(m), 1);
    CHECK_TRUE(hm_put(m, "beta", 2));
    CHECK_TRUE(hm_put(m, "gamma", 3));
    CHECK_UINT_EQ(hm_len(m), 3);
    CHECK_TRUE(hm_put(m, "beta", 22)); /* upsert: len must not grow */
    CHECK_UINT_EQ(hm_len(m), 3);
    CHECK_TRUE(hm_get(m, "beta", &v));
    CHECK_INT_EQ(v, 22);
    {
        /* the map must have COPIED the key, not stored our pointer */
        char mutating[8];
        strcpy(mutating, "delta");
        CHECK_TRUE(hm_put(m, mutating, 4));
        strcpy(mutating, "XXXXX"); /* clobber the caller's buffer */
        CHECK_TRUE(hm_get(m, "delta", &v));
        CHECK_INT_EQ(v, 4);
        CHECK_FALSE(hm_get(m, "XXXXX", &v));
    }

    /* ---- hm_get ---- */
    SECTION("hm_get");
    CHECK_TRUE(hm_get(m, "alpha", &v));
    CHECK_INT_EQ(v, 1);
    CHECK_TRUE(hm_get(m, "gamma", &v));
    CHECK_INT_EQ(v, 3);
    v = -55;
    CHECK_FALSE(hm_get(m, "alph", &v)); /* prefix is not a match */
    CHECK_FALSE(hm_get(m, "", &v));
    CHECK_INT_EQ(v, -55);

    /* ---- hm_del: hits, misses, delete-then-reinsert ---- */
    SECTION("hm_del");
    CHECK_TRUE(hm_del(m, "beta"));
    CHECK_UINT_EQ(hm_len(m), 3); /* alpha gamma delta */
    CHECK_FALSE(hm_get(m, "beta", &v));
    CHECK_FALSE(hm_del(m, "beta")); /* already gone */
    CHECK_TRUE(hm_put(m, "beta", 200)); /* reinsert works */
    CHECK_TRUE(hm_get(m, "beta", &v));
    CHECK_INT_EQ(v, 200);
    CHECK_UINT_EQ(hm_len(m), 4);

    /* ---- resize: push far past the initial 4 buckets ---- */
    SECTION("resize / rehash");
    for (int i = 0; i < KEY_POOL; i++) {
        CHECK_TRUE_MSG(hm_put(m, keypool[i], i * 7), "i=%d", i);
    }
    CHECK_UINT_EQ(hm_len(m), 4 + KEY_POOL);
    for (int i = 0; i < KEY_POOL; i++) {
        v = -1;
        CHECK_TRUE_MSG(hm_get(m, keypool[i], &v), "key=%s", keypool[i]);
        CHECK_INT_EQ_MSG(v, i * 7, "key=%s", keypool[i]);
    }
    /* the pre-resize entries survived the rehash */
    CHECK_TRUE(hm_get(m, "alpha", &v));
    CHECK_INT_EQ(v, 1);
    CHECK_TRUE(hm_get(m, "delta", &v));
    CHECK_INT_EQ(v, 4);
    hm_destroy(m);

    /* ---- hm_destroy: NULL is a no-op; empty map frees clean ---- */
    SECTION("hm_destroy");
    hm_destroy(NULL);
    m = hm_create(1);
    CHECK_PTR_NONNULL(m);
    hm_destroy(m);
    /* leaks from any of the above would be reported by ASan at exit */
    CHECK_TRUE(1);

    /* ---- randomized differential test vs the naive oracle ---- */
    SECTION("differential vs oracle");
    srand(20260709);
    m = hm_create(2); /* tiny start: forces many resizes */
    CHECK_PTR_NONNULL(m);
    for (int op = 0; op < 2000; op++) {
        int r = rand() % 4;
        int k = rand() % KEY_POOL;
        const char *key = keypool[k];
        if (r == 0 || r == 1) { /* put (upsert) */
            int x = rand() % 100000;
            CHECK_TRUE_MSG(hm_put(m, key, x), "op=%d key=%s", op, key);
            oracle_val[k] = x;
            oracle_present[k] = true;
        } else if (r == 2) { /* get */
            v = -777;
            bool got = hm_get(m, key, &v);
            CHECK_INT_EQ_MSG(got, oracle_present[k], "op=%d key=%s", op, key);
            if (oracle_present[k]) {
                CHECK_INT_EQ_MSG(v, oracle_val[k], "op=%d key=%s", op, key);
            } else {
                CHECK_INT_EQ_MSG(v, -777, "op=%d key=%s", op, key);
            }
        } else { /* del */
            bool did = hm_del(m, key);
            CHECK_INT_EQ_MSG(did, oracle_present[k], "op=%d key=%s", op, key);
            oracle_present[k] = false;
        }
        CHECK_UINT_EQ_MSG(hm_len(m), oracle_len(), "op=%d key=%s", op, key);
    }
    /* final full comparison */
    SECTION("differential final compare");
    for (int k = 0; k < KEY_POOL; k++) {
        v = -777;
        bool got = hm_get(m, keypool[k], &v);
        CHECK_INT_EQ_MSG(got, oracle_present[k], "key=%s", keypool[k]);
        if (oracle_present[k]) {
            CHECK_INT_EQ_MSG(v, oracle_val[k], "key=%s", keypool[k]);
        }
    }
    hm_destroy(m);

    CTEST_END();
}
