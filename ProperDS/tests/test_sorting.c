#include "ctest.h"
#include "sorting.h"

#include <stdlib.h>
#include <string.h>

static int cmp_int(const void *a, const void *b) {
    int x = *(const int *)a;
    int y = *(const int *)b;
    return (x > y) - (x < y);
}

typedef void (*sort_fn)(int *, size_t);

static sort_fn fns[] = {insertion_sort, merge_sort, quick_sort,
                        heap_sort_arr, hybrid_sort};
static const char *names[] = {"insertion_sort", "merge_sort", "quick_sort",
                              "heap_sort_arr", "hybrid_sort"};
enum { NFNS = 5 };

#define MAXN 3000
static int work[MAXN], want[MAXN];

/* Run one sort on a copy of src and compare against qsort's answer. */
static void check_sort(sort_fn f, const char *name, const int *src, size_t n) {
    memcpy(work, src, n * sizeof(int));
    memcpy(want, src, n * sizeof(int));
    qsort(want, n, sizeof(int), cmp_int);
    f(work, n);
    int bad = 0;
    for (size_t i = 0; i < n; i++) {
        if (work[i] != want[i]) {
            bad++;
        }
    }
    if (bad != 0) {
        fprintf(stderr, "  (%s failed on n=%zu)\n", name, n);
    }
    CHECK_INT_EQ_MSG(bad, 0, "sort=%s n=%zu", name, n);
}

int main(void) {
    static int src[MAXN];

    /* ---- adversarial fixed cases, every sort ---- */
    for (int f = 0; f < NFNS; f++) {
        SECTION(names[f]);
        /* empty */
        check_sort(fns[f], names[f], src, 0);

        /* single element */
        src[0] = 42;
        check_sort(fns[f], names[f], src, 1);

        /* two elements, both orders */
        src[0] = 2;
        src[1] = 1;
        check_sort(fns[f], names[f], src, 2);
        src[0] = 1;
        src[1] = 2;
        check_sort(fns[f], names[f], src, 2);

        /* already sorted (classic quicksort killer) */
        for (int i = 0; i < 2000; i++) {
            src[i] = i;
        }
        check_sort(fns[f], names[f], src, 2000);

        /* reverse sorted */
        for (int i = 0; i < 2000; i++) {
            src[i] = 2000 - i;
        }
        check_sort(fns[f], names[f], src, 2000);

        /* all equal (the other quicksort killer) */
        for (int i = 0; i < 2000; i++) {
            src[i] = 7;
        }
        check_sort(fns[f], names[f], src, 2000);

        /* organ pipe: 0 1 2 ... 2 1 0 */
        for (int i = 0; i < 1000; i++) {
            src[i] = i;
            src[1999 - i] = i;
        }
        check_sort(fns[f], names[f], src, 2000);

        /* few distinct values */
        for (int i = 0; i < 2000; i++) {
            src[i] = i % 3;
        }
        check_sort(fns[f], names[f], src, 2000);

        /* exactly the hybrid cutoff boundary sizes */
        for (int n = 14; n <= 17; n++) {
            for (int i = 0; i < n; i++) {
                src[i] = (i * 37) % 11 - 5;
            }
            check_sort(fns[f], names[f], src, (size_t)n);
        }
    }

    /* ---- randomized differential vs qsort ---- */
    srand(606011);
    for (int trial = 0; trial < 40; trial++) {
        size_t n = (size_t)(rand() % MAXN);
        int range = (trial % 2 == 0) ? 1000000 : 10; /* wide and narrow */
        for (size_t i = 0; i < n; i++) {
            src[i] = rand() % range - range / 2;
        }
        for (int f = 0; f < NFNS; f++) {
            SECTION(names[f]);
            check_sort(fns[f], names[f], src, n);
        }
    }

    CTEST_END();
}
