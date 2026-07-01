#include "ctest.h"
#include "atomics.h"

#include <pthread.h>

/* ---- spinlock: 4 threads x 100k increments must be exact ---- */

enum { SPIN_THREADS = 4, SPIN_INCS = 100000 };

static SpinLock lock = SPINLOCK_INIT;
static long spin_counter = 0;

static void *spin_worker(void *p) {
    (void)p;
    for (int i = 0; i < SPIN_INCS; i++) {
        spin_lock(&lock);
        spin_counter++;
        spin_unlock(&lock);
    }
    return NULL;
}

/* ---- SPSC: producer pushes 0..99999 in order, consumer checks ---- */

enum { SPSC_N = 100000 };

typedef struct {
    SpscRing *r;
    long popped;      /* how many values the consumer got */
    int monotonic;    /* stayed strictly increasing? */
    long full_hits;   /* producer saw "full" at least this many times */
    long empty_hits;  /* consumer saw "empty" at least this many times */
} SpscStats;

static void *spsc_producer(void *p) {
    SpscStats *s = p;
    for (long v = 0; v < SPSC_N; v++) {
        while (!spsc_push(s->r, v)) {
            s->full_hits++;
        }
    }
    return NULL;
}

static void *spsc_consumer(void *p) {
    SpscStats *s = p;
    long expect = 0;
    s->monotonic = 1;
    while (expect < SPSC_N) {
        long v;
        if (!spsc_pop(s->r, &v)) {
            s->empty_hits++;
            continue;
        }
        if (v != expect) {
            s->monotonic = 0;
            return NULL;
        }
        expect++;
        s->popped++;
    }
    return NULL;
}

int main(void) {
    /* spinlock under contention */
    {
        pthread_t t[SPIN_THREADS];
        for (int i = 0; i < SPIN_THREADS; i++) {
            CHECK_INT_EQ(pthread_create(&t[i], NULL, spin_worker, NULL), 0);
        }
        for (int i = 0; i < SPIN_THREADS; i++) {
            pthread_join(t[i], NULL);
        }
        CHECK_INT_EQ(spin_counter, (long)SPIN_THREADS * SPIN_INCS);
    }

    /* spsc_init argument validation */
    {
        SpscRing r;
        CHECK_INT_EQ(spsc_init(&r, 0), -1);
        CHECK_INT_EQ(spsc_init(&r, 1), -1);
        CHECK_INT_EQ(spsc_init(&r, 12), -1); /* not a power of two */
        CHECK_INT_EQ(spsc_init(&r, 8), 0);
        spsc_destroy(&r);
    }

    /* SPSC single-threaded semantics: FIFO, full, empty */
    {
        SpscRing r;
        CHECK_INT_EQ(spsc_init(&r, 4), 0); /* usable capacity 3 */
        long v = -1;
        CHECK_FALSE(spsc_pop(&r, &v)); /* empty */
        CHECK_TRUE(spsc_push(&r, 1));
        CHECK_TRUE(spsc_push(&r, 2));
        CHECK_TRUE(spsc_push(&r, 3));
        CHECK_FALSE(spsc_push(&r, 4)); /* full at cap-1 items */
        CHECK_TRUE(spsc_pop(&r, &v));
        CHECK_INT_EQ(v, 1);
        CHECK_TRUE(spsc_push(&r, 4)); /* slot freed */
        CHECK_TRUE(spsc_pop(&r, &v));
        CHECK_INT_EQ(v, 2);
        CHECK_TRUE(spsc_pop(&r, &v));
        CHECK_INT_EQ(v, 3);
        CHECK_TRUE(spsc_pop(&r, &v));
        CHECK_INT_EQ(v, 4);
        CHECK_FALSE(spsc_pop(&r, &v)); /* empty again */
        spsc_destroy(&r);
    }

    /* SPSC cross-thread: order and count preserved through a tiny ring */
    {
        SpscRing r;
        CHECK_INT_EQ(spsc_init(&r, 64), 0);
        SpscStats stats = {&r, 0, 0, 0, 0};

        pthread_t prod, cons;
        CHECK_INT_EQ(pthread_create(&cons, NULL, spsc_consumer, &stats), 0);
        CHECK_INT_EQ(pthread_create(&prod, NULL, spsc_producer, &stats), 0);
        pthread_join(prod, NULL);
        pthread_join(cons, NULL);

        CHECK_INT_EQ(stats.popped, (long)SPSC_N);
        CHECK_TRUE(stats.monotonic);
        spsc_destroy(&r);
    }

    CTEST_END();
}
