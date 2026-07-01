#include "ctest.h"
#include "search.h"

#include <stdlib.h>

static int cmp_int(const void *a, const void *b) {
    int x = *(const int *)a;
    int y = *(const int *)b;
    return (x > y) - (x < y);
}

/* Linear-scan oracles. */
static size_t ora_lower(const int *a, size_t n, int key) {
    for (size_t i = 0; i < n; i++) {
        if (a[i] >= key) {
            return i;
        }
    }
    return n;
}

static size_t ora_upper(const int *a, size_t n, int key) {
    for (size_t i = 0; i < n; i++) {
        if (a[i] > key) {
            return i;
        }
    }
    return n;
}

int main(void) {
    /* ---- fixed vectors with duplicates ---- */
    int a[] = {1, 3, 3, 3, 5, 8, 8, 10};
    size_t n = 8;

    /* binary_search_idx: hit must land on a matching element */
    int idx = binary_search_idx(a, n, 3);
    CHECK_TRUE(idx >= 0 && idx < (int)n);
    CHECK_INT_EQ(a[idx], 3);
    idx = binary_search_idx(a, n, 1); /* first element */
    CHECK_INT_EQ(idx, 0);
    idx = binary_search_idx(a, n, 10); /* last element */
    CHECK_INT_EQ(idx, 7);
    CHECK_INT_EQ(binary_search_idx(a, n, 2), -1);  /* gap */
    CHECK_INT_EQ(binary_search_idx(a, n, 0), -1);  /* below range */
    CHECK_INT_EQ(binary_search_idx(a, n, 99), -1); /* above range */

    /* lower/upper: duplicates run [1, 4) for key 3 */
    CHECK_UINT_EQ(lower_bound(a, n, 3), 1);
    CHECK_UINT_EQ(upper_bound(a, n, 3), 4);
    /* absent key: both point at the insertion spot */
    CHECK_UINT_EQ(lower_bound(a, n, 4), 4);
    CHECK_UINT_EQ(upper_bound(a, n, 4), 4);
    /* below / above everything */
    CHECK_UINT_EQ(lower_bound(a, n, -5), 0);
    CHECK_UINT_EQ(upper_bound(a, n, -5), 0);
    CHECK_UINT_EQ(lower_bound(a, n, 11), n);
    CHECK_UINT_EQ(upper_bound(a, n, 11), n);
    /* boundary keys */
    CHECK_UINT_EQ(lower_bound(a, n, 1), 0);
    CHECK_UINT_EQ(upper_bound(a, n, 1), 1);
    CHECK_UINT_EQ(lower_bound(a, n, 10), 7);
    CHECK_UINT_EQ(upper_bound(a, n, 10), n);

    /* ---- empty array ---- */
    CHECK_INT_EQ(binary_search_idx(NULL, 0, 5), -1);
    CHECK_UINT_EQ(lower_bound(NULL, 0, 5), 0);
    CHECK_UINT_EQ(upper_bound(NULL, 0, 5), 0);

    /* ---- all-equal array ---- */
    int eq[] = {4, 4, 4, 4, 4};
    CHECK_UINT_EQ(lower_bound(eq, 5, 4), 0);
    CHECK_UINT_EQ(upper_bound(eq, 5, 4), 5);
    CHECK_UINT_EQ(lower_bound(eq, 5, 3), 0);
    CHECK_UINT_EQ(upper_bound(eq, 5, 5), 5);
    idx = binary_search_idx(eq, 5, 4);
    CHECK_TRUE(idx >= 0 && idx < 5);

    /* single element */
    int one[] = {7};
    CHECK_INT_EQ(binary_search_idx(one, 1, 7), 0);
    CHECK_INT_EQ(binary_search_idx(one, 1, 6), -1);
    CHECK_UINT_EQ(lower_bound(one, 1, 7), 0);
    CHECK_UINT_EQ(upper_bound(one, 1, 7), 1);

    /* ---- fuzz: hundreds of random sorted arrays vs linear oracles ---- */
    srand(707012);
    static int arr[512];
    for (int trial = 0; trial < 400; trial++) {
        size_t len = (size_t)(rand() % 65); /* 0..64, includes empty */
        int range = 1 + rand() % 20;        /* narrow: many duplicates */
        for (size_t i = 0; i < len; i++) {
            arr[i] = rand() % range;
        }
        qsort(arr, len, sizeof(int), cmp_int);
        for (int probe = 0; probe < 8; probe++) {
            int key = rand() % (range + 4) - 2; /* in and out of range */
            size_t lb = lower_bound(arr, len, key);
            size_t ub = upper_bound(arr, len, key);
            CHECK_UINT_EQ(lb, ora_lower(arr, len, key));
            CHECK_UINT_EQ(ub, ora_upper(arr, len, key));
            CHECK_TRUE(lb <= ub);
            int bs = binary_search_idx(arr, len, key);
            if (lb < ub) { /* key present */
                CHECK_TRUE(bs >= (int)lb && bs < (int)ub);
                CHECK_INT_EQ(arr[bs], key);
            } else {
                CHECK_INT_EQ(bs, -1);
            }
        }
    }

    CTEST_END();
}
