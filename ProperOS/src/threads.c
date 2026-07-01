#include "threads.h"
#include "todo.h"

long mutex_counter_run(int nthreads, long increments_each) {
    (void)nthreads;
    (void)increments_each;
    TODO("implement mutex_counter_run");
}

int bq_init(BoundedQueue *q, int cap) {
    (void)q;
    (void)cap;
    TODO("implement bq_init");
}

int bq_push(BoundedQueue *q, long v) {
    (void)q;
    (void)v;
    TODO("implement bq_push");
}

int bq_pop(BoundedQueue *q, long *out) {
    (void)q;
    (void)out;
    TODO("implement bq_pop");
}

void bq_destroy(BoundedQueue *q) {
    (void)q;
    TODO("implement bq_destroy");
}

int tpool_run(int nthreads, int ntasks) {
    (void)nthreads;
    (void)ntasks;
    TODO("implement tpool_run");
}
