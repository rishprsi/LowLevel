#include "hashmap.h"
#include "todo.h"

/* Define struct HashMap here — its layout is yours to choose. */

HashMap *hm_create(size_t nbuckets) {
    (void)nbuckets;
    TODO("implement hm_create");
}

void hm_destroy(HashMap *m) {
    (void)m;
    TODO("implement hm_destroy");
}

bool hm_put(HashMap *m, const char *key, int val) {
    (void)m;
    (void)key;
    (void)val;
    TODO("implement hm_put");
}

bool hm_get(const HashMap *m, const char *key, int *out) {
    (void)m;
    (void)key;
    (void)out;
    TODO("implement hm_get");
}

bool hm_del(HashMap *m, const char *key) {
    (void)m;
    (void)key;
    TODO("implement hm_del");
}

size_t hm_len(const HashMap *m) {
    (void)m;
    TODO("implement hm_len");
}
