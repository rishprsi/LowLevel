#include "ctest.h"
#include "hashopen.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Local copy of the contract hash, so the test can PLAN collisions without
 * trusting the implementation under test. */
static uint64_t local_fnv1a(const char *s) {
    uint64_t h = 14695981039346656037ULL;
    for (const unsigned char *p = (const unsigned char *)s; *p; p++) {
        h ^= *p;
        h *= 1099511628211ULL;
    }
    return h;
}

/* Differential oracle: flat linear-scan key -> value array. */
#define KEYSPACE 300
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
    HashOpen h;
    int out = -777;
    char key[32];

    /* ---- contract hash ---- */
    SECTION("hashopen_fnv1a");
    CHECK_UINT_EQ(hashopen_fnv1a(""), 0xcbf29ce484222325ULL);
    CHECK_UINT_EQ(hashopen_fnv1a("foobar"), 0x85944171f73967e8ULL);

    /* ---- basic put/get/delete/upsert ---- */
    SECTION("hashopen put/get/delete/upsert");
    CHECK_TRUE(hashopen_init(&h));
    CHECK_UINT_EQ(h.nslots, 16);
    CHECK_FALSE(hashopen_get(&h, "missing", &out));
    CHECK_INT_EQ(out, -777);
    CHECK_FALSE(hashopen_delete(&h, "missing"));
    CHECK_TRUE(hashopen_put(&h, "one", 1));
    CHECK_TRUE(hashopen_put(&h, "two", 2));
    CHECK_TRUE(hashopen_put(&h, "one", 111)); /* upsert */
    CHECK_UINT_EQ(hashopen_len(&h), 2);
    CHECK_TRUE(hashopen_get(&h, "one", &out));
    CHECK_INT_EQ(out, 111);
    CHECK_TRUE(hashopen_validate(&h));

    /* table owns its keys */
    SECTION("hashopen owns key copies");
    strcpy(key, "caller");
    CHECK_TRUE(hashopen_put(&h, key, 42));
    strcpy(key, "XXXXXX");
    CHECK_TRUE(hashopen_get(&h, "caller", &out));
    CHECK_INT_EQ(out, 42);
    hashopen_free(&h);

    /*
     * ---- TARGETED TOMBSTONE TEST ----
     * Find three distinct keys whose home slot in a 16-slot table is the
     * SAME (using the documented hash). Insert k1 then k2 then k3: k2 and
     * k3 are pushed past k1 by linear probing. Delete k1 -> tombstone at
     * the shared home slot. get(k2)/get(k3) MUST probe past the tombstone.
     * A naive "stop at first non-occupied slot" lookup fails here.
     */
    SECTION("hashopen tombstone probing");
    char colls[3][16];
    {
        int found = 0;
        for (int i = 0; found < 3 && i < 100000; i++) {
            char cand[16];
            snprintf(cand, sizeof(cand), "t%d", i);
            if (local_fnv1a(cand) % 16 == 7) { /* arbitrary shared bucket */
                snprintf(colls[found], sizeof(colls[found]), "%s", cand);
                found++;
            }
        }
        CHECK_INT_EQ(found, 3); /* the keyspace surely has 3 colliders */
    }
    CHECK_TRUE(hashopen_init(&h));
    CHECK_TRUE(hashopen_put(&h, colls[0], 100));
    CHECK_TRUE(hashopen_put(&h, colls[1], 200));
    CHECK_TRUE(hashopen_put(&h, colls[2], 300));
    CHECK_TRUE(hashopen_delete(&h, colls[0])); /* tombstone at home slot */
    CHECK_UINT_EQ(hashopen_len(&h), 2);
    CHECK_TRUE(hashopen_get(&h, colls[1], &out)); /* must pass tombstone */
    CHECK_INT_EQ(out, 200);
    CHECK_TRUE(hashopen_get(&h, colls[2], &out));
    CHECK_INT_EQ(out, 300);
    CHECK_FALSE(hashopen_get(&h, colls[0], &out)); /* really gone */
    CHECK_TRUE(hashopen_validate(&h));
    /* re-insert: may recycle the tombstone; must not duplicate */
    SECTION("hashopen recycle tombstone");
    CHECK_TRUE(hashopen_put(&h, colls[0], 101));
    CHECK_UINT_EQ(hashopen_len(&h), 3);
    CHECK_TRUE(hashopen_get(&h, colls[0], &out));
    CHECK_INT_EQ(out, 101);
    CHECK_TRUE(hashopen_validate(&h));
    hashopen_free(&h);

    /* ---- resize: 30 keys > 0.7 * 16, so the table must grow;
     *      rehash must also clean tombstones ---- */
    SECTION("hashopen resize / rehash");
    CHECK_TRUE(hashopen_init(&h));
    for (int i = 0; i < 30; i++) {
        snprintf(key, sizeof(key), "grow%d", i);
        CHECK_TRUE(hashopen_put(&h, key, i));
    }
    CHECK_UINT_EQ(hashopen_len(&h), 30);
    CHECK_TRUE(h.nslots > 16);
    CHECK_TRUE(h.used * 10 <= h.nslots * 7); /* back under the load cap */
    CHECK_TRUE(hashopen_validate(&h));
    for (int i = 0; i < 30; i++) {
        snprintf(key, sizeof(key), "grow%d", i);
        CHECK_TRUE(hashopen_get(&h, key, &out));
        CHECK_INT_EQ(out, i);
    }
    /* delete half -> tombstones; then force churn and confirm no key lost */
    SECTION("hashopen delete half + churn");
    for (int i = 0; i < 30; i += 2) {
        snprintf(key, sizeof(key), "grow%d", i);
        CHECK_TRUE(hashopen_delete(&h, key));
    }
    CHECK_UINT_EQ(hashopen_len(&h), 15);
    CHECK_TRUE(hashopen_validate(&h));
    for (int i = 1; i < 30; i += 2) {
        snprintf(key, sizeof(key), "grow%d", i);
        CHECK_TRUE(hashopen_get(&h, key, &out));
        CHECK_INT_EQ(out, i);
    }
    hashopen_free(&h);

    /* ---- randomized differential test: ~2000 ops vs the flat oracle ---- */
    SECTION("differential vs oracle");
    srand(555002);
    CHECK_TRUE(hashopen_init(&h));
    for (int op = 0; op < 2000; op++) {
        int r = rand() % 3;
        snprintf(key, sizeof(key), "k%d", rand() % KEYSPACE);
        if (r == 0) { /* put */
            int v = rand();
            CHECK_TRUE_MSG(hashopen_put(&h, key, v), "op=%d key=%s v=%d", op,
                           key, v);
            ora_put(key, v);
        } else if (r == 1) { /* get */
            OraPair *p = ora_find(key);
            bool got = hashopen_get(&h, key, &out);
            CHECK_INT_EQ_MSG(got, p != NULL, "op=%d key=%s", op, key);
            if (p && got) {
                CHECK_INT_EQ_MSG(out, p->value, "op=%d key=%s", op, key);
            }
        } else { /* delete (this churns tombstones constantly) */
            bool want = ora_delete(key);
            CHECK_INT_EQ_MSG(hashopen_delete(&h, key), want, "op=%d key=%s", op,
                             key);
        }
        CHECK_UINT_EQ_MSG(hashopen_len(&h), olen, "op=%d r=%d key=%s", op, r,
                          key);
        if (op % 100 == 0) {
            CHECK_TRUE_MSG(hashopen_validate(&h), "op=%d", op);
        }
    }
    SECTION("differential final sweep");
    CHECK_TRUE(hashopen_validate(&h));
    for (size_t i = 0; i < olen; i++) {
        CHECK_TRUE_MSG(hashopen_get(&h, opairs[i].key, &out), "i=%zu key=%s", i,
                       opairs[i].key);
        CHECK_INT_EQ_MSG(out, opairs[i].value, "i=%zu key=%s", i,
                         opairs[i].key);
    }
    hashopen_free(&h);
    hashopen_free(&h); /* double free is safe per contract */

    CTEST_END();
}
