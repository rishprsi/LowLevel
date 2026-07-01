#include "atomics.h"
#include "todo.h"

void spin_lock(SpinLock *l) {
    (void)l;
    TODO("implement spin_lock");
}

void spin_unlock(SpinLock *l) {
    (void)l;
    TODO("implement spin_unlock");
}

int spsc_init(SpscRing *r, size_t cap) {
    (void)r;
    (void)cap;
    TODO("implement spsc_init");
}

bool spsc_push(SpscRing *r, long v) {
    (void)r;
    (void)v;
    TODO("implement spsc_push");
}

bool spsc_pop(SpscRing *r, long *out) {
    (void)r;
    (void)out;
    TODO("implement spsc_pop");
}

void spsc_destroy(SpscRing *r) {
    (void)r;
    TODO("implement spsc_destroy");
}
