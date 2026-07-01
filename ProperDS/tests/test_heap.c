#include "ctest.h"
#include "heap.h"

#include <stdlib.h>
#include <string.h>

static int cmp_int(const void *a, const void *b) {
    int x = *(const int *)a;
    int y = *(const int *)b;
    return (x > y) - (x < y);
}

int main(void) {
    Heap h;
    int out = -777;

    /* ---- empty heap ---- */
    heap_init(&h);
    CHECK_UINT_EQ(heap_size(&h), 0);
    CHECK_FALSE(heap_pop_min(&h, &out));
    CHECK_FALSE(heap_peek(&h, &out));
    CHECK_INT_EQ(out, -777);
    CHECK_TRUE(heap_validate(&h));
    heap_free(&h);

    /* ---- push/pop basic ordering incl. duplicates ---- */
    heap_init(&h);
    int in[] = {5, 3, 8, 1, 9, 3, -4, 0};
    for (size_t i = 0; i < 8; i++) {
        CHECK_TRUE(heap_push(&h, in[i]));
        CHECK_TRUE(heap_validate(&h));
    }
    CHECK_UINT_EQ(heap_size(&h), 8);
    CHECK_TRUE(heap_peek(&h, &out));
    CHECK_INT_EQ(out, -4);
    int want[] = {-4, 0, 1, 3, 3, 5, 8, 9};
    for (size_t i = 0; i < 8; i++) {
        CHECK_TRUE(heap_pop_min(&h, &out));
        CHECK_INT_EQ(out, want[i]);
        CHECK_TRUE(heap_validate(&h));
    }
    CHECK_FALSE(heap_pop_min(&h, &out));
    heap_free(&h);

    /* ---- heap_from_array: bottom-up heapify ---- */
    int arr[] = {9, 4, 7, 1, -2, 6, 5};
    CHECK_TRUE(heap_from_array(&h, arr, 7));
    CHECK_UINT_EQ(heap_size(&h), 7);
    CHECK_TRUE(heap_validate(&h)); /* heap property after heapify */
    CHECK_TRUE(heap_peek(&h, &out));
    CHECK_INT_EQ(out, -2);
    /* source array must not be modified */
    CHECK_INT_EQ(arr[0], 9);
    int want2[] = {-2, 1, 4, 5, 6, 7, 9};
    for (size_t i = 0; i < 7; i++) {
        CHECK_TRUE(heap_pop_min(&h, &out));
        CHECK_INT_EQ(out, want2[i]);
    }
    heap_free(&h);

    /* empty heapify */
    CHECK_TRUE(heap_from_array(&h, NULL, 0));
    CHECK_UINT_EQ(heap_size(&h), 0);
    CHECK_TRUE(heap_validate(&h));
    heap_free(&h);

    /* ---- heap_sort ---- */
    int hs[] = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5};
    heap_sort(hs, 11);
    for (size_t i = 0; i + 1 < 11; i++) {
        CHECK_TRUE(hs[i] <= hs[i + 1]);
    }
    CHECK_INT_EQ(hs[0], 1);
    CHECK_INT_EQ(hs[10], 9);
    heap_sort(hs, 0);  /* n == 0 must be a no-op */
    int one[] = {42};
    heap_sort(one, 1);
    CHECK_INT_EQ(one[0], 42);

    /* ---- randomized differential: pop sequence == qsorted copy ---- */
    srand(999005);
    enum { N = 2000 };
    static int vals[N], sorted[N];
    for (int i = 0; i < N; i++) {
        vals[i] = rand() % 500 - 250; /* negatives + duplicates */
    }
    memcpy(sorted, vals, sizeof(vals));
    qsort(sorted, N, sizeof(int), cmp_int);

    /* via n pushes */
    heap_init(&h);
    for (int i = 0; i < N; i++) {
        CHECK_TRUE(heap_push(&h, vals[i]));
    }
    CHECK_TRUE(heap_validate(&h));
    for (int i = 0; i < N; i++) {
        CHECK_TRUE(heap_pop_min(&h, &out));
        CHECK_INT_EQ(out, sorted[i]);
    }
    heap_free(&h);

    /* via bottom-up heapify */
    CHECK_TRUE(heap_from_array(&h, vals, N));
    CHECK_TRUE(heap_validate(&h));
    for (int i = 0; i < N; i++) {
        CHECK_TRUE(heap_pop_min(&h, &out));
        CHECK_INT_EQ(out, sorted[i]);
    }
    heap_free(&h);

    /* via heap_sort */
    static int hsort[N];
    memcpy(hsort, vals, sizeof(vals));
    heap_sort(hsort, N);
    for (int i = 0; i < N; i++) {
        CHECK_INT_EQ(hsort[i], sorted[i]);
    }

    /* interleaved push/pop vs a re-sorted shadow */
    heap_init(&h);
    static int shadow[512];
    size_t slen = 0;
    for (int op = 0; op < 2000; op++) {
        if ((rand() % 2 == 0 || slen == 0) && slen < 512) {
            int x = rand() % 1000;
            CHECK_TRUE(heap_push(&h, x));
            shadow[slen++] = x;
            qsort(shadow, slen, sizeof(int), cmp_int);
        } else {
            CHECK_TRUE(heap_pop_min(&h, &out));
            CHECK_INT_EQ(out, shadow[0]);
            memmove(shadow, shadow + 1, (slen - 1) * sizeof(int));
            slen--;
        }
        CHECK_UINT_EQ(heap_size(&h), slen);
        if (op % 100 == 0) {
            CHECK_TRUE(heap_validate(&h));
        }
    }
    heap_free(&h);

    CTEST_END();
}
