#include "hashopen.h"
#include "todo.h"

uint64_t hashopen_fnv1a(const char *s) {
    (void)s;
    TODO("implement hashopen_fnv1a");
}

bool hashopen_init(HashOpen *h) {
    (void)h;
    TODO("implement hashopen_init");
}

void hashopen_free(HashOpen *h) {
    (void)h;
    TODO("implement hashopen_free");
}

bool hashopen_put(HashOpen *h, const char *key, int value) {
    (void)h;
    (void)key;
    (void)value;
    TODO("implement hashopen_put");
}

bool hashopen_get(const HashOpen *h, const char *key, int *out) {
    (void)h;
    (void)key;
    (void)out;
    TODO("implement hashopen_get");
}

bool hashopen_delete(HashOpen *h, const char *key) {
    (void)h;
    (void)key;
    TODO("implement hashopen_delete");
}

size_t hashopen_len(const HashOpen *h) {
    (void)h;
    TODO("implement hashopen_len");
}

bool hashopen_validate(const HashOpen *h) {
    (void)h;
    TODO("implement hashopen_validate");
}
