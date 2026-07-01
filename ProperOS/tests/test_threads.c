#include "ctest.h"
#include "threads.h"

#include <stdlib.h>

/* ---- producer/consumer fixture for the bounded queue ---- */

enum { NPROD = 4, NCONS = 4, ITEMS_PER_PROD = 10000 };
enum { TOTAL = NPROD * ITEMS_PER_PROD }; /* 40k items */

typedef struct {
    BoundedQueue *q;
    int id;
} ProdArg;

typedef struct {
    BoundedQueue *q;
    int npop;
    long sum;
    long count;
} ConsArg;

static void *producer(void *p) {
    ProdArg *a = p;
    long base = (long)a->id * ITEMS_PER_PROD;
    for (long i = 0; i < ITEMS_PER_PROD; i++) {
        if (bq_push(a->q, base + i) != 0) {
            return (void *)1;
        }
    }
    return NULL;
}

static void *consumer(void *p) {
    ConsArg *a = p;
    for (int i = 0; i < a->npop; i++) {
        long v;
        if (bq_pop(a->q, &v) != 0) {
            return (void *)1;
        }
        a->sum += v;
        a->count++;
    }
    return NULL;
}

int main(void) {
    /* mutex counter: result must be exact */
    CHECK_INT_EQ(mutex_counter_run(4, 50000), 200000L);
    CHECK_INT_EQ(mutex_counter_run(1, 1), 1L);
    CHECK_INT_EQ(mutex_counter_run(8, 10000), 80000L);

    /* bounded queue, single-threaded FIFO semantics first */
    {
        BoundedQueue q;
        CHECK_INT_EQ(bq_init(&q, 4), 0);
        CHECK_INT_EQ(bq_push(&q, 10), 0);
        CHECK_INT_EQ(bq_push(&q, 20), 0);
        CHECK_INT_EQ(bq_push(&q, 30), 0);
        long v = 0;
        CHECK_INT_EQ(bq_pop(&q, &v), 0);
        CHECK_INT_EQ(v, 10);
        CHECK_INT_EQ(bq_pop(&q, &v), 0);
        CHECK_INT_EQ(v, 20);
        CHECK_INT_EQ(bq_push(&q, 40), 0);
        CHECK_INT_EQ(bq_pop(&q, &v), 0);
        CHECK_INT_EQ(v, 30);
        CHECK_INT_EQ(bq_pop(&q, &v), 0);
        CHECK_INT_EQ(v, 40);
        bq_destroy(&q);
        CHECK_INT_EQ(bq_init(&q, 0), -1); /* invalid capacity */
    }

    /* bounded queue: 4 producers / 4 consumers moving exactly 40k items.
     * Capacity 8 << 40k forces real blocking on both conditions. */
    {
        BoundedQueue q;
        CHECK_INT_EQ(bq_init(&q, 8), 0);

        pthread_t prod_t[NPROD], cons_t[NCONS];
        ProdArg pa[NPROD];
        ConsArg ca[NCONS];

        for (int i = 0; i < NCONS; i++) {
            ca[i] = (ConsArg){&q, TOTAL / NCONS, 0, 0};
            CHECK_INT_EQ(pthread_create(&cons_t[i], NULL, consumer, &ca[i]), 0);
        }
        for (int i = 0; i < NPROD; i++) {
            pa[i] = (ProdArg){&q, i};
            CHECK_INT_EQ(pthread_create(&prod_t[i], NULL, producer, &pa[i]), 0);
        }

        for (int i = 0; i < NPROD; i++) {
            void *ret;
            pthread_join(prod_t[i], &ret);
            CHECK_PTR_NULL(ret);
        }
        long got_count = 0, got_sum = 0;
        for (int i = 0; i < NCONS; i++) {
            void *ret;
            pthread_join(cons_t[i], &ret);
            CHECK_PTR_NULL(ret);
            got_count += ca[i].count;
            got_sum += ca[i].sum;
        }

        CHECK_INT_EQ(got_count, (long)TOTAL);
        /* values were 0..TOTAL-1 exactly once: sum = TOTAL*(TOTAL-1)/2 */
        CHECK_INT_EQ(got_sum, (long)TOTAL * (TOTAL - 1) / 2);
        bq_destroy(&q);
    }

    /* thread pool */
    CHECK_INT_EQ(tpool_run(4, 1000), 1000);
    CHECK_INT_EQ(tpool_run(1, 100), 100);
    CHECK_INT_EQ(tpool_run(8, 0), 0); /* no tasks: clean startup+shutdown */

    CTEST_END();
}
