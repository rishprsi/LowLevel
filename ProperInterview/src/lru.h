#ifndef PROPERINTERVIEW_LRU_H
#define PROPERINTERVIEW_LRU_H

#include <stddef.h>

/*
 * lru — an LRU (least-recently-used) cache: THE classic interview
 * data-structure question (LeetCode 146).
 *
 * TARGET DRILL TIME: 45 minutes.
 *
 * Re-drill: `git checkout -- src/lru.c`, start the timer, implement every
 * function below, then `make test_lru`.
 *
 * Keys and values are ints; keys are non-negative and values are
 * non-negative (so lru_get can use -1 as its "absent" sentinel).
 *
 * REQUIRED SHAPE — both operations must be O(1):
 *   - a hash table from key -> node, for O(1) lookup, and
 *   - a doubly-linked list of nodes in recency order (most-recent at one
 *     end, least-recent at the other), for O(1) promote and O(1) evict.
 * A node must be unlinkable from the middle of the list in O(1) — that is
 * exactly why the list must be DOUBLY linked. Sentinel head/tail nodes
 * remove all the head/tail special cases and are strongly recommended.
 *
 * "Used" means touched by lru_get (hit) or lru_put (insert or update).
 */
typedef struct LRU LRU;

/*
 * Allocate a cache that holds at most capacity entries (capacity >= 1).
 * Returns NULL if allocation fails.
 */
LRU *lru_create(size_t capacity);

/* Free the cache and everything it owns. NULL is a no-op. */
void lru_destroy(LRU *c);

/*
 * If key is present: promote it to most-recently-used and return its
 * value. Otherwise return -1 (a miss does NOT change recency order).
 */
int lru_get(LRU *c, int key);

/*
 * Insert or update (upsert): set key's value and promote it to
 * most-recently-used. If key was absent and the cache is full, evict the
 * least-recently-used entry first.
 */
void lru_put(LRU *c, int key, int value);

/* Number of entries currently cached (<= capacity). */
size_t lru_len(const LRU *c);

#endif /* PROPERINTERVIEW_LRU_H */
