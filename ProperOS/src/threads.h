#ifndef PROPEROS_THREADS_H
#define PROPEROS_THREADS_H

#include <pthread.h>
#include <stdbool.h>

/*
 * APUE ch.11–12 — pthreads: mutexes, condition variables, a bounded queue,
 * and a minimal thread pool.
 *
 * Re-verify this module with `make test_threads SAN=thread` on a normal
 * machine: ThreadSanitizer is the tool that actually proves the locking.
 */

/*
 * Spawn nthreads threads; each increments a shared long counter
 * increments_each times, protected by a pthread_mutex_t. Join all threads
 * and return the final counter — which must be EXACTLY
 * nthreads * increments_each (that's the whole point of the lock).
 * Returns -1 on any pthread error.
 */
long mutex_counter_run(int nthreads, long increments_each);

/*
 * Bounded blocking queue of longs (classic producer/consumer, APUE §11.6).
 *
 * - bq_push blocks while the queue is FULL; bq_pop blocks while EMPTY.
 * - Use one mutex + TWO condition variables (not_full, not_empty).
 * - Condition waits MUST be in a while loop re-checking the predicate:
 *   spurious wakeups and stolen wakeups are real.
 */
typedef struct {
    long *buf;
    int cap;
    int head; /* next slot to pop  */
    int tail; /* next slot to push */
    int count;
    pthread_mutex_t mu;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;
} BoundedQueue;

/* Initialize with capacity cap (> 0). Returns 0 on success, -1 on error. */
int bq_init(BoundedQueue *q, int cap);

/* Blocking push/pop. Return 0 on success, -1 on error. */
int bq_push(BoundedQueue *q, long v);
int bq_pop(BoundedQueue *q, long *out);

/* Free the buffer and destroy the mutex/condvars. */
void bq_destroy(BoundedQueue *q);

/*
 * Minimal thread pool, built and torn down inside this one call:
 * start nthreads workers pulling task items off an internal queue; each of
 * the ntasks tasks increments a shared atomic counter once. After all tasks
 * are submitted, shut down cleanly (workers must exit their loop — e.g. via
 * sentinel values or a stop flag) and pthread_join every worker.
 * Returns the final counter (== ntasks), or -1 on error.
 */
int tpool_run(int nthreads, int ntasks);

#endif /* PROPEROS_THREADS_H */
