/*
 * bench — the memory-hierarchy measurement drill (fully provided, no stubs).
 *
 * Build & run:   make bench      (compiled with -O2, NO sanitizers)
 *
 * Two experiments, both doing the same trivial work per element (read an
 * integer, add it to a sum) — only the ACCESS PATTERN differs:
 *
 *   1. summing a contiguous array (sequential)  vs.  walking a linked list
 *      whose heap-allocated nodes are visited in shuffled order
 *      (pointer-chasing that defeats the hardware prefetcher)
 *
 *   2. summing a 4096x4096 int matrix row-major (stride 4 bytes)  vs.
 *      column-major (stride 16 KiB — a fresh cache line, and often a fresh
 *      TLB page, on nearly every access)
 *
 * There is no pass/fail. The numbers are the lesson.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

enum {
    LIST_N = 1000000, /* elements in the array and in the list */
    MAT_N  = 4096,    /* matrix is MAT_N x MAT_N ints (64 MiB)  */
};

typedef struct Node {
    long value;
    struct Node *next;
} Node;

static double now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1e9 + (double)ts.tv_nsec;
}

/* xorshift32 — deterministic shuffle, same list layout on every run */
static uint32_t rng_state = 0x12345678u;
static uint32_t rng_next(void) {
    uint32_t x = rng_state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return rng_state = x;
}

static void *xmalloc(size_t n) {
    void *p = malloc(n);
    if (p == NULL) {
        fprintf(stderr, "bench: out of memory\n");
        exit(1);
    }
    return p;
}

static void bench_array_vs_list(void) {
    long *arr = xmalloc(LIST_N * sizeof *arr);
    Node **nodes = xmalloc(LIST_N * sizeof *nodes);

    for (long i = 0; i < LIST_N; i++) {
        arr[i] = i & 0xFF;
    }

    /* heap-allocate every node separately, then shuffle the VISIT order so
     * each ->next hops to an unrelated heap address: the prefetcher cannot
     * guess where the walk goes next */
    for (long i = 0; i < LIST_N; i++) {
        nodes[i] = xmalloc(sizeof(Node));
        nodes[i]->value = i & 0xFF;
    }
    for (long i = LIST_N - 1; i > 0; i--) {
        long j = (long)(rng_next() % (uint32_t)(i + 1));
        Node *tmp = nodes[i];
        nodes[i] = nodes[j];
        nodes[j] = tmp;
    }
    for (long i = 0; i < LIST_N - 1; i++) {
        nodes[i]->next = nodes[i + 1];
    }
    nodes[LIST_N - 1]->next = NULL;
    Node *head = nodes[0];

    /* one untimed warmup pass over each, so page faults don't pollute t */
    long warm = 0;
    for (long i = 0; i < LIST_N; i++) {
        warm += arr[i];
    }
    for (Node *p = head; p != NULL; p = p->next) {
        warm += p->value;
    }

    long sum_arr = 0, sum_list = 0;
    double t0 = now_ns();
    for (long i = 0; i < LIST_N; i++) {
        sum_arr += arr[i];
    }
    double t1 = now_ns();
    for (Node *p = head; p != NULL; p = p->next) {
        sum_list += p->value;
    }
    double t2 = now_ns();

    double ns_arr = (t1 - t0) / LIST_N;
    double ns_list = (t2 - t1) / LIST_N;
    printf("1) sequential array vs pointer-chasing linked list  (n = %d)\n",
           (int)LIST_N);
    printf("   array traversal:  %8.3f ns/element   (sum %ld)\n", ns_arr,
           sum_arr);
    printf("   list  traversal:  %8.3f ns/element   (sum %ld, warm %ld)\n",
           ns_list, sum_list, warm);
    printf("   ratio (list/array): %.1fx\n\n", ns_list / ns_arr);

    for (long i = 0; i < LIST_N; i++) {
        free(nodes[i]);
    }
    free(nodes);
    free(arr);
}

static void bench_matrix(void) {
    const size_t n = MAT_N;
    int *m = xmalloc(n * n * sizeof *m);
    for (size_t i = 0; i < n * n; i++) {
        m[i] = 1; /* also faults every page in before we start timing */
    }

    long sum_row = 0, sum_col = 0;
    double t0 = now_ns();
    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            sum_row += m[i * n + j]; /* row-major: adjacent bytes */
        }
    }
    double t1 = now_ns();
    for (size_t j = 0; j < n; j++) {
        for (size_t i = 0; i < n; i++) {
            sum_col += m[i * n + j]; /* column-major: 16 KiB stride */
        }
    }
    double t2 = now_ns();

    double total = (double)n * (double)n;
    printf("2) row-major vs column-major sum of a %dx%d int matrix (64 MiB)\n",
           (int)MAT_N, (int)MAT_N);
    printf("   row-major:  %8.3f ns/element   (sum %ld)\n", (t1 - t0) / total,
           sum_row);
    printf("   col-major:  %8.3f ns/element   (sum %ld)\n", (t2 - t1) / total,
           sum_col);
    printf("   ratio (col/row): %.1fx\n\n", (t2 - t1) / (t1 - t0));

    free(m);
}

int main(void) {
    printf("ProperMachine bench — the memory hierarchy, made visible\n\n");
    bench_array_vs_list();
    bench_matrix();
    printf("Interpretation: every loop above does identical work per element\n"
           "(load an integer, add it). The ratios are the cache hierarchy\n"
           "becoming visible: sequential access streams whole cache lines and\n"
           "lets the prefetcher run ahead, while pointer-chasing and large\n"
           "strides turn nearly every access into a cache (and TLB) miss that\n"
           "stalls the core for the full memory latency.\n");
    return 0;
}
