#ifndef PROPERDS_HASHOPEN_H
#define PROPERDS_HASHOPEN_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * hashopen — an open-addressing, linear-probing hash table with TOMBSTONES,
 * mapping string keys -> int.
 *
 * Every slot is in one of three states:
 *   HO_EMPTY     — never used; a probe sequence STOPS here
 *   HO_OCCUPIED  — holds a live key/value
 *   HO_TOMBSTONE — a key was deleted here; a probe sequence must CONTINUE
 *                  past it (this is the whole point: keys inserted after a
 *                  collision may live beyond this slot)
 *
 * Contract details the tests rely on:
 *   - hash function: FNV-1a 64-bit (same as hashchain); home slot is
 *     hash % nslots; probing is linear with step 1, wrapping around
 *   - initial table: exactly 16 slots
 *   - delete marks the slot HO_TOMBSTONE (frees the key, len--)
 *   - put may recycle the first tombstone seen on its probe path
 *   - resize: when a put would make (occupied + tombstones) exceed
 *     load factor 0.7, rehash into a fresh table (smallest power of two
 *     >= 16 that keeps live entries under 0.7) — rehashing drops all
 *     tombstones
 *   - the table stores ITS OWN COPY of each key
 */
typedef enum {
    HO_EMPTY = 0,
    HO_OCCUPIED = 1,
    HO_TOMBSTONE = 2,
} HoState;

typedef struct {
    char *key; /* owned copy; NULL unless HO_OCCUPIED */
    int value;
    unsigned char state; /* one of HoState */
} HoSlot;

typedef struct {
    HoSlot *slots;
    size_t nslots;
    size_t len;  /* live (occupied) entries */
    size_t used; /* occupied + tombstones */
} HashOpen;

/* Same contract hash as hashchain (FNV-1a 64). */
uint64_t hashopen_fnv1a(const char *s);

/* Allocate the initial 16-slot table, all HO_EMPTY.
 * Returns false on allocation failure. */
bool hashopen_init(HashOpen *h);

/* Free all keys and the slot array. Safe to call twice. */
void hashopen_free(HashOpen *h);

/* Upsert: insert key -> value, or overwrite if present.
 * Returns false only on allocation failure. */
bool hashopen_put(HashOpen *h, const char *key, int value);

/* If key is present, store its value in *out and return true.
 * Otherwise return false and leave *out untouched. */
bool hashopen_get(const HashOpen *h, const char *key, int *out);

/* Remove key (leaving a tombstone). Returns true if it was present. */
bool hashopen_delete(HashOpen *h, const char *key);

/* Number of live keys. */
size_t hashopen_len(const HashOpen *h);

/* Invariant checker: occupied-slot count == len, occupied + tombstone
 * count == used, used stays within the 0.7 load factor, and every live
 * key is reachable by probing from its home slot. */
bool hashopen_validate(const HashOpen *h);

#endif /* PROPERDS_HASHOPEN_H */
