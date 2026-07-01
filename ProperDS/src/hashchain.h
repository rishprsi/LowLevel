#ifndef PROPERDS_HASHCHAIN_H
#define PROPERDS_HASHCHAIN_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * hashchain — a separate-chaining hash table mapping string keys -> int.
 *
 * Contract details the tests rely on:
 *   - hash function: FNV-1a, 64-bit
 *       h = 14695981039346656037  (offset basis)
 *       for each byte b: h ^= b; h *= 1099511628211  (FNV prime)
 *     bucket index = h % nbuckets
 *   - initial table: 8 buckets
 *   - resize: after an insert makes len > nbuckets (load factor > 1.0),
 *     double nbuckets and rehash every entry
 *   - the table stores ITS OWN COPY of each key (caller's string may die)
 */
typedef struct HcEntry {
    char *key; /* owned copy */
    int value;
    struct HcEntry *next;
} HcEntry;

typedef struct {
    HcEntry **buckets;
    size_t nbuckets;
    size_t len;
} HashChain;

/* The contract hash, exposed so you (and the tests) can reason about
 * bucket placement. */
uint64_t hashchain_fnv1a(const char *s);

/* Allocate the initial 8-bucket table. Returns false on allocation
 * failure. */
bool hashchain_init(HashChain *h);

/* Free all entries, keys, and the bucket array. Safe to call twice. */
void hashchain_free(HashChain *h);

/* Upsert: insert key -> value, or overwrite the value if key is already
 * present. Returns false only on allocation failure. */
bool hashchain_put(HashChain *h, const char *key, int value);

/* If key is present, store its value in *out and return true.
 * Otherwise return false and leave *out untouched. */
bool hashchain_get(const HashChain *h, const char *key, int *out);

/* Remove key. Returns true if it was present, false otherwise. */
bool hashchain_delete(HashChain *h, const char *key);

/* Number of distinct keys stored. */
size_t hashchain_len(const HashChain *h);

/* Invariant checker: every entry sits in the bucket its hash demands, and
 * the entry count across all chains equals len. */
bool hashchain_validate(const HashChain *h);

#endif /* PROPERDS_HASHCHAIN_H */
