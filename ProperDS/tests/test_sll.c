#include "ctest.h"
#include "sll.h"

#include <stdlib.h>

/* Collect list values into out[]; returns count (caps at maxn). */
static size_t collect(const SllNode *head, int *out, size_t maxn) {
    size_t n = 0;
    for (; head && n < maxn; head = head->next) {
        out[n++] = head->value;
    }
    return n;
}

int main(void) {
    SllNode *list = NULL;
    int vals[64];

    /* ---- empty-list behavior ---- */
    SECTION("empty list behavior");
    CHECK_UINT_EQ(sll_length(list), 0);
    CHECK_PTR_NULL(sll_find(list, 1));
    CHECK_FALSE(sll_remove_value(&list, 1));
    sll_reverse(&list); /* reversing empty list is a no-op */
    CHECK_PTR_NULL(list);
    sll_free(&list); /* freeing empty list is safe */

    /* ---- push_front / push_back ordering ---- */
    SECTION("sll_push_front / push_back");
    CHECK_TRUE(sll_push_back(&list, 2));  /* [2] */
    CHECK_TRUE(sll_push_front(&list, 1)); /* [1 2] */
    CHECK_TRUE(sll_push_back(&list, 3));  /* [1 2 3] */
    CHECK_TRUE(sll_push_back(&list, 4));  /* [1 2 3 4] */
    CHECK_UINT_EQ(sll_length(list), 4);
    CHECK_UINT_EQ(collect(list, vals, 64), 4);
    CHECK_INT_EQ(vals[0], 1);
    CHECK_INT_EQ(vals[1], 2);
    CHECK_INT_EQ(vals[2], 3);
    CHECK_INT_EQ(vals[3], 4);

    /* ---- find ---- */
    SECTION("sll_find");
    SllNode *n = sll_find(list, 3);
    CHECK_PTR_NONNULL(n);
    CHECK_INT_EQ(n->value, 3);
    CHECK_PTR_NULL(sll_find(list, 42));
    CHECK_TRUE(sll_find(list, 1) == list); /* finds the head node itself */

    /* ---- remove_value: head, middle, tail, absent, first-occurrence ---- */
    SECTION("sll_remove_value");
    CHECK_TRUE(sll_push_back(&list, 2)); /* [1 2 3 4 2] duplicate */
    CHECK_TRUE(sll_remove_value(&list, 2)); /* removes FIRST 2 -> [1 3 4 2] */
    CHECK_UINT_EQ(collect(list, vals, 64), 4);
    CHECK_INT_EQ(vals[0], 1);
    CHECK_INT_EQ(vals[1], 3);
    CHECK_INT_EQ(vals[2], 4);
    CHECK_INT_EQ(vals[3], 2);
    CHECK_TRUE(sll_remove_value(&list, 1)); /* head -> [3 4 2] */
    CHECK_TRUE(sll_remove_value(&list, 2)); /* tail -> [3 4] */
    CHECK_FALSE(sll_remove_value(&list, 99));
    CHECK_UINT_EQ(sll_length(list), 2);

    /* ---- reverse ---- */
    SECTION("sll_reverse");
    sll_reverse(&list); /* [4 3] */
    CHECK_UINT_EQ(collect(list, vals, 64), 2);
    CHECK_INT_EQ(vals[0], 4);
    CHECK_INT_EQ(vals[1], 3);
    CHECK_TRUE(sll_push_front(&list, 5)); /* [5 4 3] */
    sll_reverse(&list);                   /* [3 4 5] */
    CHECK_UINT_EQ(collect(list, vals, 64), 3);
    CHECK_INT_EQ(vals[0], 3);
    CHECK_INT_EQ(vals[1], 4);
    CHECK_INT_EQ(vals[2], 5);
    sll_free(&list);
    CHECK_PTR_NULL(list);

    /* single-element reverse */
    SECTION("sll_reverse single element");
    CHECK_TRUE(sll_push_front(&list, 7));
    sll_reverse(&list);
    CHECK_UINT_EQ(sll_length(list), 1);
    CHECK_INT_EQ(list->value, 7);
    sll_free(&list);

    /* ---- randomized differential test vs a shadow array ---- */
    SECTION("differential vs oracle");
    srand(424242);
    static int shadow[512];
    size_t slen = 0;
    for (int op = 0; op < 2000; op++) {
        int r = rand() % 5;
        int x = rand() % 50; /* small range so duplicates + hits happen */
        if (r == 0 && slen < 512) { /* push_front */
            CHECK_TRUE_MSG(sll_push_front(&list, x), "op=%d r=%d x=%d", op, r, x);
            for (size_t k = slen; k > 0; k--) {
                shadow[k] = shadow[k - 1];
            }
            shadow[0] = x;
            slen++;
        } else if (r == 1 && slen < 512) { /* push_back */
            CHECK_TRUE_MSG(sll_push_back(&list, x), "op=%d r=%d x=%d", op, r, x);
            shadow[slen++] = x;
        } else if (r == 2) { /* remove first occurrence */
            bool got = sll_remove_value(&list, x);
            bool want = false;
            for (size_t k = 0; k < slen; k++) {
                if (shadow[k] == x) {
                    for (size_t j = k; j + 1 < slen; j++) {
                        shadow[j] = shadow[j + 1];
                    }
                    slen--;
                    want = true;
                    break;
                }
            }
            CHECK_INT_EQ_MSG(got, want, "op=%d r=%d x=%d", op, r, x);
        } else if (r == 3) { /* find */
            SllNode *f = sll_find(list, x);
            bool want = false;
            for (size_t k = 0; k < slen; k++) {
                if (shadow[k] == x) {
                    want = true;
                    break;
                }
            }
            CHECK_INT_EQ_MSG(f != NULL, want, "op=%d r=%d x=%d", op, r, x);
        } else { /* reverse */
            sll_reverse(&list);
            for (size_t k = 0; k < slen / 2; k++) {
                int t = shadow[k];
                shadow[k] = shadow[slen - 1 - k];
                shadow[slen - 1 - k] = t;
            }
        }
        CHECK_UINT_EQ_MSG(sll_length(list), slen, "op=%d r=%d x=%d", op, r, x);
    }
    SECTION("differential final compare");
    static int got[512];
    size_t gn = collect(list, got, 512);
    CHECK_UINT_EQ(gn, slen);
    for (size_t k = 0; k < slen && k < gn; k++) {
        CHECK_INT_EQ_MSG(got[k], shadow[k], "k=%zu", k);
    }
    sll_free(&list);
    CHECK_PTR_NULL(list);

    CTEST_END();
}
