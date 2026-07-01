#include "hashchain.h"
#include "todo.h"

uint64_t hashchain_fnv1a(const char *s) {
    (void)s;
    TODO("implement hashchain_fnv1a");
}

bool hashchain_init(HashChain *h) {
    (void)h;
    TODO("implement hashchain_init");
}

void hashchain_free(HashChain *h) {
    (void)h;
    TODO("implement hashchain_free");
}

bool hashchain_put(HashChain *h, const char *key, int value) {
    (void)h;
    (void)key;
    (void)value;
    TODO("implement hashchain_put");
}

bool hashchain_get(const HashChain *h, const char *key, int *out) {
    (void)h;
    (void)key;
    (void)out;
    TODO("implement hashchain_get");
}

bool hashchain_delete(HashChain *h, const char *key) {
    (void)h;
    (void)key;
    TODO("implement hashchain_delete");
}

size_t hashchain_len(const HashChain *h) {
    (void)h;
    TODO("implement hashchain_len");
}

bool hashchain_validate(const HashChain *h) {
    (void)h;
    TODO("implement hashchain_validate");
}
