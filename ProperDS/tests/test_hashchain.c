#include "ctest.h"
#include "hashchain.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Differential oracle: a flat, linear-scan key -> value array. Dumb but
 * obviously correct.
 */
#define KEYSPACE 300 /* keys "k0".."k299": small enough to force reuse */
typedef struct {
    char key[16];
    int value;
} OraPair;
static OraPair opairs[KEYSPACE];
static size_t olen = 0;

static OraPair *ora_find(const char *key) {
    for (size_t i = 0; i < olen; i++) {
        if (strcmp(opairs[i].key, key) == 0) {
            return &opairs[i];
        }
    }
    return NULL;
}

static void ora_put(const char *key, int value) {
    OraPair *p = ora_find(key);
    if (p) {
        p->value = value;
        return;
    }
    snprintf(opairs[olen].key, sizeof(opairs[olen].key), "%s", key);
    opairs[olen].value = value;
    olen++;
}

static bool ora_delete(const char *key) {
    OraPair *p = ora_find(key);
    if (!p) {
        return false;
    }
    *p = opairs[--olen];
    return true;
}

int main(void) {
    HashChain h;
    int out = -777;
    char key[32];

    /* ---- known FNV-1a vectors (the documented contract hash) ---- */
    SECTION("hashchain_fnv1a");
    CHECK_UINT_EQ(hashchain_fnv1a(""), 0xcbf29ce484222325ULL);
    CHECK_UINT_EQ(hashchain_fnv1a("a"), 0xaf63dc4c8601ec8cULL);
    CHECK_UINT_EQ(hashchain_fnv1a("foobar"), 0x85944171f73967e8ULL);

    /* ---- basic put/get/delete ---- */
    SECTION("hashchain put/get/delete");
    CHECK_TRUE(hashchain_init(&h));
    CHECK_UINT_EQ(h.nbuckets, 8);
    CHECK_UINT_EQ(hashchain_len(&h), 0);
    CHECK_FALSE(hashchain_get(&h, "missing", &out));
    CHECK_INT_EQ(out, -777);
    CHECK_FALSE(hashchain_delete(&h, "missing"));

    CHECK_TRUE(hashchain_put(&h, "one", 1));
    CHECK_TRUE(hashchain_put(&h, "two", 2));
    CHECK_UINT_EQ(hashchain_len(&h), 2);
    CHECK_TRUE(hashchain_get(&h, "one", &out));
    CHECK_INT_EQ(out, 1);
    CHECK_TRUE(hashchain_get(&h, "two", &out));
    CHECK_INT_EQ(out, 2);
    CHECK_TRUE(hashchain_validate(&h));

    /* upsert overwrites, does not duplicate */
    SECTION("hashchain_put upsert");
    CHECK_TRUE(hashchain_put(&h, "one", 111));
    CHECK_UINT_EQ(hashchain_len(&h), 2);
    CHECK_TRUE(hashchain_get(&h, "one", &out));
    CHECK_INT_EQ(out, 111);

    /* the table owns its key copies: mutate the caller's buffer */
    SECTION("hashchain owns key copies");
    strcpy(key, "caller");
    CHECK_TRUE(hashchain_put(&h, key, 42));
    strcpy(key, "XXXXXX");
    CHECK_TRUE(hashchain_get(&h, "caller", &out));
    CHECK_INT_EQ(out, 42);

    SECTION("hashchain_delete");
    CHECK_TRUE(hashchain_delete(&h, "one"));
    CHECK_FALSE(hashchain_get(&h, "one", &out));
    CHECK_FALSE(hashchain_delete(&h, "one")); /* second delete: absent */
    CHECK_UINT_EQ(hashchain_len(&h), 2);
    CHECK_TRUE(hashchain_validate(&h));
    hashchain_free(&h);

    /* ---- resize: 20 keys must push 8 buckets past load factor 1.0 ---- */
    SECTION("hashchain resize / rehash");
    CHECK_TRUE(hashchain_init(&h));
    for (int i = 0; i < 20; i++) {
        snprintf(key, sizeof(key), "grow%d", i);
        CHECK_TRUE(hashchain_put(&h, key, i));
    }
    CHECK_UINT_EQ(hashchain_len(&h), 20);
    CHECK_TRUE(h.nbuckets > 8);           /* it grew */
    CHECK_TRUE(h.nbuckets >= h.len);      /* load factor <= 1.0 again */
    CHECK_TRUE(hashchain_validate(&h));   /* entries re-bucketed correctly */
    for (int i = 0; i < 20; i++) {
        snprintf(key, sizeof(key), "grow%d", i);
        CHECK_TRUE(hashchain_get(&h, key, &out));
        CHECK_INT_EQ(out, i);
    }
    hashchain_free(&h);

    /* ---- randomized differential test: ~2000 ops vs the flat oracle ---- */
    SECTION("differential vs oracle");
    srand(555001);
    CHECK_TRUE(hashchain_init(&h));
    for (int op = 0; op < 2000; op++) {
        int r = rand() % 3;
        snprintf(key, sizeof(key), "k%d", rand() % KEYSPACE);
        if (r == 0) { /* put */
            int v = rand();
            CHECK_TRUE_MSG(hashchain_put(&h, key, v), "op=%d key=%s", op, key);
            ora_put(key, v);
        } else if (r == 1) { /* get */
            OraPair *p = ora_find(key);
            bool got = hashchain_get(&h, key, &out);
            CHECK_INT_EQ_MSG(got, p != NULL, "op=%d key=%s", op, key);
            if (p && got) {
                CHECK_INT_EQ_MSG(out, p->value, "op=%d key=%s", op, key);
            }
        } else { /* delete */
            bool want = ora_delete(key);
            CHECK_INT_EQ_MSG(hashchain_delete(&h, key), want, "op=%d key=%s", op,
                             key);
        }
        CHECK_UINT_EQ_MSG(hashchain_len(&h), olen, "op=%d key=%s", op, key);
        if (op % 100 == 0) {
            CHECK_TRUE_MSG(hashchain_validate(&h), "op=%d key=%s", op, key);
        }
    }
    /* final sweep: every oracle pair present with the right value */
    SECTION("differential final sweep");
    CHECK_TRUE(hashchain_validate(&h));
    for (size_t i = 0; i < olen; i++) {
        CHECK_TRUE_MSG(hashchain_get(&h, opairs[i].key, &out), "i=%zu key=%s", i,
                       opairs[i].key);
        CHECK_INT_EQ_MSG(out, opairs[i].value, "i=%zu key=%s", i,
                         opairs[i].key);
    }
    hashchain_free(&h);
    hashchain_free(&h); /* double free is safe per contract */

    CTEST_END();
}
