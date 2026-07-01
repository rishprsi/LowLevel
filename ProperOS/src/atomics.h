#ifndef PROPEROS_ATOMICS_H
#define PROPEROS_ATOMICS_H

#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>

/*
 * Phase 4.2 — C11 atomics: a spinlock and a lock-free SPSC ring buffer.
 *
 * Re-verify this module with `make test_atomics SAN=thread` on a normal
 * machine: ThreadSanitizer is the tool that actually proves the ordering.
 */

/*
 * Spinlock over atomic_flag. spin_lock busy-waits with
 * atomic_flag_test_and_set (acquire semantics) until it wins the flag;
 * spin_unlock clears it (release semantics). No syscalls, no pthreads.
 */
typedef struct {
    atomic_flag flag;
} SpinLock;

#define SPINLOCK_INIT {ATOMIC_FLAG_INIT}

void spin_lock(SpinLock *l);
void spin_unlock(SpinLock *l);

/*
 * Single-producer single-consumer ring buffer of longs, LOCK-FREE:
 * no mutexes, no spinlocks — only C11 atomics on the two indices.
 *
 * Invariants / contract:
 * - cap MUST be a power of two (spsc_init returns -1 otherwise); indices
 *   are masked with (cap - 1). One slot is sacrificed to tell full from
 *   empty, so usable capacity is cap - 1.
 * - EXACTLY ONE thread calls spsc_push and EXACTLY ONE calls spsc_pop.
 * - head is written only by the consumer, tail only by the producer.
 * - The producer publishes a slot with a RELEASE store to tail after
 *   writing the data; the consumer reads tail with ACQUIRE so the data
 *   write is visible before it reads the slot (and symmetrically for head).
 *
 * spsc_push returns false when full; spsc_pop returns false when empty.
 */
typedef struct {
    long *buf;
    size_t cap;              /* power of two */
    _Atomic size_t head;     /* consumer-owned: next slot to pop */
    _Atomic size_t tail;     /* producer-owned: next slot to push */
} SpscRing;

/* Returns 0 on success; -1 if cap is not a power of two (or is 0/1). */
int spsc_init(SpscRing *r, size_t cap);
bool spsc_push(SpscRing *r, long v);
bool spsc_pop(SpscRing *r, long *out);
void spsc_destroy(SpscRing *r);

#endif /* PROPEROS_ATOMICS_H */
