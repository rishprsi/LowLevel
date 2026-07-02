#include "ctest.h"
#include "strdp.h"

#include <stdlib.h>
#include <string.h>

/* ---- brute-force oracles for tiny inputs ---- */

/* Recursive Levenshtein, exponential but fine for len <= 7. */
static int brute_edit(const char *a, const char *b) {
    if (*a == '\0') {
        return (int)strlen(b);
    }
    if (*b == '\0') {
        return (int)strlen(a);
    }
    if (*a == *b) {
        return brute_edit(a + 1, b + 1);
    }
    int del = brute_edit(a + 1, b);     /* delete a[0] */
    int ins = brute_edit(a, b + 1);     /* insert b[0] */
    int sub = brute_edit(a + 1, b + 1); /* substitute */
    int m = del < ins ? del : ins;
    m = m < sub ? m : sub;
    return 1 + m;
}

/* LIS by trying all 2^n subsequences (n <= 12). */
static int brute_lis(const int *a, size_t n) {
    int best = 0;
    for (unsigned mask = 0; mask < (1u << n); mask++) {
        int count = 0;
        int last = 0;
        int ok = 1;
        for (size_t i = 0; i < n && ok; i++) {
            if (mask & (1u << i)) {
                if (count > 0 && a[i] <= last) {
                    ok = 0;
                } else {
                    last = a[i];
                    count++;
                }
            }
        }
        if (ok && count > best) {
            best = count;
        }
    }
    return best;
}

/* Naive substring scan. */
static int brute_find(const char *hay, const char *nee) {
    size_t nh = strlen(hay), nn = strlen(nee);
    if (nn > nh) {
        return -1;
    }
    for (size_t i = 0; i + nn <= nh; i++) {
        if (strncmp(hay + i, nee, nn) == 0) {
            return (int)i;
        }
    }
    return -1;
}

/* Knapsack by trying all 2^n item subsets (n <= 12). */
static int brute_knapsack(const int *w, const int *v, size_t n, int cap) {
    int best = 0;
    for (unsigned mask = 0; mask < (1u << n); mask++) {
        int tw = 0, tv = 0;
        for (size_t i = 0; i < n; i++) {
            if (mask & (1u << i)) {
                tw += w[i];
                tv += v[i];
            }
        }
        if (tw <= cap && tv > best) {
            best = tv;
        }
    }
    return best;
}

int main(void) {
    /* ================= prefix_function ================= */
    SECTION("prefix_function");
    int pi[64];
    prefix_function("abcabcd", pi);
    int want1[] = {0, 0, 0, 1, 2, 3, 0};
    for (int i = 0; i < 7; i++) {
        CHECK_INT_EQ(pi[i], want1[i]);
    }
    prefix_function("aabaaab", pi);
    int want2[] = {0, 1, 0, 1, 2, 2, 3};
    for (int i = 0; i < 7; i++) {
        CHECK_INT_EQ(pi[i], want2[i]);
    }
    prefix_function("aaaa", pi);
    int want3[] = {0, 1, 2, 3};
    for (int i = 0; i < 4; i++) {
        CHECK_INT_EQ(pi[i], want3[i]);
    }
    prefix_function("a", pi);
    CHECK_INT_EQ(pi[0], 0);
    prefix_function("", pi); /* must not touch pi at all — just not crash */

    /* ================= kmp_search ================= */
    SECTION("kmp_search");
    CHECK_INT_EQ(kmp_search("hello world", "world"), 6);
    CHECK_INT_EQ(kmp_search("hello world", "hello"), 0);
    CHECK_INT_EQ(kmp_search("hello world", "xyz"), -1);
    CHECK_INT_EQ(kmp_search("aaaab", "aab"), 2);     /* overlapping run */
    CHECK_INT_EQ(kmp_search("ababab", "abab"), 0);   /* first, not last */
    CHECK_INT_EQ(kmp_search("mississippi", "issip"), 4);
    /* the mandated edge cases */
    CHECK_INT_EQ(kmp_search("abc", ""), 0);   /* empty needle */
    CHECK_INT_EQ(kmp_search("", ""), 0);      /* both empty */
    CHECK_INT_EQ(kmp_search("", "a"), -1);    /* empty haystack */
    CHECK_INT_EQ(kmp_search("ab", "abc"), -1); /* needle longer */
    CHECK_INT_EQ(kmp_search("same", "same"), 0); /* needle == haystack */

    /* fuzz vs naive scan, tiny alphabet to force real matches */
    SECTION("kmp_search differential vs naive");
    srand(808013);
    for (int trial = 0; trial < 300; trial++) {
        char hay[33], nee[6];
        int nh = rand() % 32;
        int nn = rand() % 5;
        for (int i = 0; i < nh; i++) {
            hay[i] = (char)('a' + rand() % 2);
        }
        hay[nh] = '\0';
        for (int i = 0; i < nn; i++) {
            nee[i] = (char)('a' + rand() % 2);
        }
        nee[nn] = '\0';
        CHECK_INT_EQ_MSG(kmp_search(hay, nee), brute_find(hay, nee),
                         "trial=%d hay=\"%s\" nee=\"%s\"", trial, hay, nee);
    }

    /* ================= edit_distance ================= */
    SECTION("edit_distance");
    CHECK_INT_EQ(edit_distance("kitten", "sitting"), 3);
    CHECK_INT_EQ(edit_distance("flaw", "lawn"), 2);
    CHECK_INT_EQ(edit_distance("", ""), 0);
    CHECK_INT_EQ(edit_distance("", "abc"), 3);
    CHECK_INT_EQ(edit_distance("abc", ""), 3);
    CHECK_INT_EQ(edit_distance("same", "same"), 0);
    CHECK_INT_EQ(edit_distance("a", "b"), 1);
    CHECK_INT_EQ(edit_distance("intention", "execution"), 5);

    /* brute-force cross-check for all short strings over {a,b,c} */
    SECTION("edit_distance differential vs brute");
    for (int trial = 0; trial < 200; trial++) {
        char sa[8], sb[8];
        int la = rand() % 7, lb = rand() % 7;
        for (int i = 0; i < la; i++) {
            sa[i] = (char)('a' + rand() % 3);
        }
        sa[la] = '\0';
        for (int i = 0; i < lb; i++) {
            sb[i] = (char)('a' + rand() % 3);
        }
        sb[lb] = '\0';
        CHECK_INT_EQ_MSG(edit_distance(sa, sb), brute_edit(sa, sb),
                         "trial=%d sa=\"%s\" sb=\"%s\"", trial, sa, sb);
    }

    /* ================= lis_length ================= */
    SECTION("lis_length");
    int lis1[] = {10, 9, 2, 5, 3, 7, 101, 18};
    CHECK_INT_EQ(lis_length(lis1, 8), 4); /* 2 3 7 18 (or 101) */
    int lis2[] = {5, 4, 3, 2, 1};
    CHECK_INT_EQ(lis_length(lis2, 5), 1); /* strictly decreasing */
    int lis3[] = {1, 2, 3, 4};
    CHECK_INT_EQ(lis_length(lis3, 4), 4);
    int lis4[] = {7, 7, 7, 7};
    CHECK_INT_EQ(lis_length(lis4, 4), 1); /* STRICTLY increasing */
    CHECK_INT_EQ(lis_length(NULL, 0), 0);
    int lis5[] = {42};
    CHECK_INT_EQ(lis_length(lis5, 1), 1);

    /* brute-force cross-check at n <= 12 */
    SECTION("lis_length differential vs brute");
    for (int trial = 0; trial < 150; trial++) {
        int arr[12];
        size_t n = (size_t)(rand() % 13);
        for (size_t i = 0; i < n; i++) {
            arr[i] = rand() % 10; /* duplicates likely */
        }
        CHECK_INT_EQ_MSG(lis_length(arr, n), brute_lis(arr, n), "trial=%d n=%zu",
                         trial, n);
    }

    /* ================= coin_change_min ================= */
    SECTION("coin_change_min");
    int us[] = {1, 5, 10, 25};
    CHECK_INT_EQ(coin_change_min(us, 4, 0), 0);
    CHECK_INT_EQ(coin_change_min(us, 4, 6), 2);   /* 5+1 */
    CHECK_INT_EQ(coin_change_min(us, 4, 30), 2);  /* 25+5 */
    CHECK_INT_EQ(coin_change_min(us, 4, 63), 6);  /* 25+25+10+1+1+1 */
    /* greedy trap: 6 = 3+3 beats 4+1+1 */
    int trap[] = {1, 3, 4};
    CHECK_INT_EQ(coin_change_min(trap, 3, 6), 2);
    /* impossible amounts */
    int evens[] = {2, 4};
    CHECK_INT_EQ(coin_change_min(evens, 2, 7), -1);
    CHECK_INT_EQ(coin_change_min(evens, 2, 8), 2);
    int three[] = {3};
    CHECK_INT_EQ(coin_change_min(three, 1, 1), -1);
    CHECK_INT_EQ(coin_change_min(three, 1, 9), 3);

    /* ================= knapsack_01 ================= */
    SECTION("knapsack_01");
    {
        int w[] = {1, 3, 4, 5};
        int v[] = {1, 4, 5, 7};
        CHECK_INT_EQ(knapsack_01(w, v, 4, 7), 9); /* items 3+4 */
        CHECK_INT_EQ(knapsack_01(w, v, 4, 0), 0);
        CHECK_INT_EQ(knapsack_01(w, v, 4, 100), 17); /* take all */
        CHECK_INT_EQ(knapsack_01(NULL, NULL, 0, 10), 0); /* no items */
    }
    /* each item at most ONCE: one heavy-value item can't be doubled */
    {
        int w[] = {5};
        int v[] = {10};
        CHECK_INT_EQ(knapsack_01(w, v, 1, 10), 10); /* not 20 */
    }
    /* brute-force cross-check at n <= 12 */
    SECTION("knapsack_01 differential vs brute");
    for (int trial = 0; trial < 150; trial++) {
        int w[12], v[12];
        size_t n = (size_t)(rand() % 13);
        int cap = rand() % 30;
        for (size_t i = 0; i < n; i++) {
            w[i] = rand() % 10;
            v[i] = rand() % 20;
        }
        CHECK_INT_EQ_MSG(knapsack_01(w, v, n, cap), brute_knapsack(w, v, n, cap),
                         "trial=%d n=%zu cap=%d", trial, n, cap);
    }

    CTEST_END();
}
