#include "lru.h"
#include "todo.h"

/* Define struct LRU (and its node type) here — the layout is yours to
 * choose, but get/put must be O(1): hash table + doubly-linked list. */

LRU *lru_create(size_t capacity) {
    (void)capacity;
    TODO("implement lru_create");
}

void lru_destroy(LRU *c) {
    (void)c;
    TODO("implement lru_destroy");
}

int lru_get(LRU *c, int key) {
    (void)c;
    (void)key;
    TODO("implement lru_get");
}

void lru_put(LRU *c, int key, int value) {
    (void)c;
    (void)key;
    (void)value;
    TODO("implement lru_put");
}

size_t lru_len(const LRU *c) {
    (void)c;
    TODO("implement lru_len");
}
