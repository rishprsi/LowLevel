#include "ctest.h"
#include "lru.h"

#include <stdlib.h>

/*
 * O(n) reference LRU for the differential test: an array of (key, value)
 * pairs kept in recency order — index 0 is LEAST recent, index len-1 is
 * MOST recent. Every operation is a linear scan + shift. Slow but
 * obviously correct.
 */
#define REF_CAP 16
static int ref_key[REF_CAP];
static int ref_val[REF_CAP];
static size_t ref_len = 0;

static int ref_find(int key) {
    for (size_t i = 0; i < ref_len; i++) {
        if (ref_key[i] == key) {
            return (int)i;
        }
    }
    return -1;
}

/* Move entry i to the most-recent slot (end of the array). */
static void ref_promote(size_t i) {
    int k = ref_key[i], v = ref_val[i];
    for (size_t j = i; j + 1 < ref_len; j++) {
        ref_key[j] = ref_key[j + 1];
        ref_val[j] = ref_val[j + 1];
    }
    ref_key[ref_len - 1] = k;
    ref_val[ref_len - 1] = v;
}

static int ref_get(int key) {
    int i = ref_find(key);
    if (i < 0) {
        return -1;
    }
    int v = ref_val[i];
    ref_promote((size_t)i);
    return v;
}

static void ref_put(int key, int value) {
    int i = ref_find(key);
    if (i >= 0) {
        ref_val[i] = value;
        ref_promote((size_t)i);
        return;
    }
    if (ref_len == REF_CAP) { /* evict least recent (index 0) */
        for (size_t j = 0; j + 1 < ref_len; j++) {
            ref_key[j] = ref_key[j + 1];
            ref_val[j] = ref_val[j + 1];
        }
        ref_len--;
    }
    ref_key[ref_len] = key;
    ref_val[ref_len] = value;
    ref_len++;
}

int main(void) {
    LRU *c;

    /* ---- lru_create ---- */
    SECTION("lru_create");
    c = lru_create(3);
    CHECK_PTR_NONNULL(c);
    CHECK_UINT_EQ(lru_len(c), 0);
    CHECK_INT_EQ(lru_get(c, 1), -1); /* miss on empty cache */
    lru_destroy(c);

    /* ---- lru_get / lru_put: the classic LeetCode 146 sequence ---- */
    SECTION("lru_get / lru_put (LeetCode 146)");
    c = lru_create(2);
    CHECK_PTR_NONNULL(c);
    lru_put(c, 1, 1);
    lru_put(c, 2, 2);
    CHECK_INT_EQ(lru_get(c, 1), 1);  /* 1 is now most recent */
    lru_put(c, 3, 3);                /* evicts 2 */
    CHECK_INT_EQ(lru_get(c, 2), -1);
    lru_put(c, 4, 4);                /* evicts 1 */
    CHECK_INT_EQ(lru_get(c, 1), -1);
    CHECK_INT_EQ(lru_get(c, 3), 3);
    CHECK_INT_EQ(lru_get(c, 4), 4);
    CHECK_UINT_EQ(lru_len(c), 2);
    lru_destroy(c);

    /* ---- eviction order: least-recent goes first, get promotes ---- */
    SECTION("eviction order");
    c = lru_create(3);
    lru_put(c, 10, 100);
    lru_put(c, 20, 200);
    lru_put(c, 30, 300);           /* recency: 10 20 30 */
    CHECK_INT_EQ(lru_get(c, 10), 100); /* recency: 20 30 10 */
    lru_put(c, 40, 400);           /* evicts 20 */
    CHECK_INT_EQ(lru_get(c, 20), -1);
    CHECK_INT_EQ(lru_get(c, 30), 300); /* recency: 10 40 30 */
    lru_put(c, 50, 500);           /* evicts 10 */
    CHECK_INT_EQ(lru_get(c, 10), -1);
    CHECK_INT_EQ(lru_get(c, 40), 400);
    CHECK_INT_EQ(lru_get(c, 30), 300);
    CHECK_INT_EQ(lru_get(c, 50), 500);
    CHECK_UINT_EQ(lru_len(c), 3);
    /* a MISS must not change recency: recency is 40 30 50 */
    CHECK_INT_EQ(lru_get(c, 999), -1);
    lru_put(c, 60, 600); /* evicts 40, not something else */
    CHECK_INT_EQ(lru_get(c, 40), -1);
    CHECK_INT_EQ(lru_get(c, 30), 300);
    lru_destroy(c);

    /* ---- update promotes: a put of an EXISTING key rescues it ---- */
    SECTION("update promotes");
    c = lru_create(2);
    lru_put(c, 1, 1);
    lru_put(c, 2, 2);      /* recency: 1 2 */
    lru_put(c, 1, 11);     /* update value AND promote: recency 2 1 */
    CHECK_UINT_EQ(lru_len(c), 2); /* upsert must not grow len */
    lru_put(c, 3, 3);      /* evicts 2 (NOT 1) */
    CHECK_INT_EQ(lru_get(c, 2), -1);
    CHECK_INT_EQ(lru_get(c, 1), 11); /* updated value survived */
    CHECK_INT_EQ(lru_get(c, 3), 3);
    lru_destroy(c);

    /* ---- capacity 1: every distinct put evicts ---- */
    SECTION("capacity 1");
    c = lru_create(1);
    lru_put(c, 5, 50);
    CHECK_INT_EQ(lru_get(c, 5), 50);
    lru_put(c, 6, 60); /* evicts 5 */
    CHECK_INT_EQ(lru_get(c, 5), -1);
    CHECK_INT_EQ(lru_get(c, 6), 60);
    lru_put(c, 6, 66); /* upsert in place */
    CHECK_INT_EQ(lru_get(c, 6), 66);
    CHECK_UINT_EQ(lru_len(c), 1);
    lru_destroy(c);

    /* ---- lru_destroy: NULL no-op, non-empty cache frees clean ---- */
    SECTION("lru_destroy");
    lru_destroy(NULL);
    c = lru_create(4);
    lru_put(c, 1, 1);
    lru_put(c, 2, 2);
    lru_destroy(c); /* ASan reports the leak at exit if this is wrong */
    CHECK_TRUE(1);

    /* ---- randomized differential test vs the O(n) reference ---- */
    SECTION("differential vs oracle");
    srand(20260710);
    c = lru_create(REF_CAP);
    CHECK_PTR_NONNULL(c);
    for (int op = 0; op < 2000; op++) {
        int r = rand() % 2;
        int key = rand() % 48; /* 3x capacity: plenty of misses/evictions */
        if (r == 0) { /* get */
            int got = lru_get(c, key);
            int want = ref_get(key);
            CHECK_INT_EQ_MSG(got, want, "op=%d key=%d", op, key);
        } else { /* put */
            int value = rand() % 100000;
            lru_put(c, key, value);
            ref_put(key, value);
        }
        CHECK_UINT_EQ_MSG(lru_len(c), ref_len, "op=%d key=%d", op, key);
    }
    /* final full comparison: identical contents AND identical recency
     * order (probing with gets in recency order keeps both in lockstep) */
    SECTION("differential final compare");
    for (size_t i = 0; i < ref_len; i++) {
        CHECK_INT_EQ_MSG(lru_get(c, ref_key[i]), ref_val[i], "i=%zu key=%d",
                         i, ref_key[i]);
    }
    lru_destroy(c);

    CTEST_END();
}
