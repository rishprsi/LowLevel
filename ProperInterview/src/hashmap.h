#ifndef PROPERINTERVIEW_HASHMAP_H
#define PROPERINTERVIEW_HASHMAP_H

#include <stdbool.h>
#include <stddef.h>

/*
 * hashmap — a string-key -> int hash table with separate chaining and
 * automatic resizing.
 *
 * TARGET DRILL TIME: 60 minutes.
 *
 * Re-drill: `git checkout -- src/hashmap.c`, start the timer, implement
 * every function below, then `make test_hashmap`.
 *
 * The struct is defined in the .c file, so its layout is yours to choose;
 * the canonical shape is an array of nbuckets singly-linked entry lists.
 * Collisions are resolved by CHAINING (prepending to the bucket's list is
 * simplest).
 *
 * HASH (part of the contract in spirit — any decent hash passes the tests,
 * but implement FNV-1a for the practice):
 *
 *     uint64_t h = 14695981039346656037u;          // FNV offset basis
 *     for each byte b of the key:
 *         h ^= b;                                   // xor FIRST,
 *         h *= 1099511628211u;                      // then multiply
 *     bucket = h % nbuckets;
 *
 * RESIZE (part of the contract; the tests do not observe bucket counts
 * directly but push enough keys through that a broken resize shows up as
 * lost/duplicated entries): when, after an insert, len > nbuckets (load
 * factor exceeds 1.0), double the bucket count and rehash every entry into
 * the new table. Entries move — any pointers into the old bucket array are
 * invalid after a resize. If the resize allocation fails, keep the old
 * table (the map still works, just with longer chains).
 *
 * OWNERSHIP: hm_put COPIES the key into memory the map owns. The caller's
 * buffer may be freed or overwritten immediately after hm_put returns. The
 * map frees its copies in hm_del / hm_destroy.
 */
typedef struct HashMap HashMap;

/*
 * Allocate a map with nbuckets initial buckets (nbuckets >= 1). Returns
 * NULL if allocation fails.
 */
HashMap *hm_create(size_t nbuckets);

/* Free all entries, all key copies, and the map itself. NULL is a no-op. */
void hm_destroy(HashMap *m);

/*
 * Insert or update (upsert): if key is present, overwrite its value;
 * otherwise add a new entry holding a COPY of key. Returns false only if
 * an allocation fails (map unchanged in that case). May trigger a resize.
 */
bool hm_put(HashMap *m, const char *key, int val);

/*
 * Look up key. If present, write its value to *out and return true.
 * Otherwise return false and leave *out untouched.
 */
bool hm_get(const HashMap *m, const char *key, int *out);

/*
 * Remove key, freeing the entry and the map's copy of the key. Returns
 * true if it was present, false if not. The unlink is the classic
 * pointer-to-pointer (or prev-pointer) exercise: removing the head of a
 * chain must work.
 */
bool hm_del(HashMap *m, const char *key);

/* Number of entries currently stored. */
size_t hm_len(const HashMap *m);

#endif /* PROPERINTERVIEW_HASHMAP_H */
