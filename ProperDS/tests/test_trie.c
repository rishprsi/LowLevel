#include "ctest.h"
#include "trie.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Differential oracle: flat array of stored words, linear scan. */
#define MAXW 200
static char words[MAXW][8];
static size_t nwords = 0;

static bool ora_contains(const char *w) {
    for (size_t i = 0; i < nwords; i++) {
        if (strcmp(words[i], w) == 0) {
            return true;
        }
    }
    return false;
}

static bool ora_starts_with(const char *p) {
    size_t n = strlen(p);
    for (size_t i = 0; i < nwords; i++) {
        if (strncmp(words[i], p, n) == 0) {
            return true;
        }
    }
    return false;
}

static bool ora_insert(const char *w) {
    if (ora_contains(w)) {
        return false;
    }
    snprintf(words[nwords], sizeof(words[nwords]), "%s", w);
    nwords++;
    return true;
}

static bool ora_remove(const char *w) {
    for (size_t i = 0; i < nwords; i++) {
        if (strcmp(words[i], w) == 0) {
            nwords--;
            if (i != nwords) { /* strcpy forbids src == dst */
                strcpy(words[i], words[nwords]);
            }
            return true;
        }
    }
    return false;
}

int main(void) {
    Trie t;

    /* ---- empty trie ----
     * starts_with(p) is true iff SOME INSERTED WORD starts with p, so on
     * an empty trie even starts_with("") is false. */
    CHECK_TRUE(trie_init(&t));
    CHECK_FALSE(trie_contains(&t, "a"));
    CHECK_FALSE(trie_contains(&t, ""));
    CHECK_FALSE(trie_starts_with(&t, ""));
    CHECK_FALSE(trie_starts_with(&t, "a"));
    trie_free(&t);
    trie_free(&t); /* double free is safe */

    CHECK_TRUE(trie_init(&t));

    /* ---- prefix vs whole-word distinction (the classic trap) ---- */
    CHECK_TRUE(trie_insert(&t, "apple"));
    CHECK_TRUE(trie_contains(&t, "apple"));
    CHECK_FALSE(trie_contains(&t, "app"));     /* prefix, not a word */
    CHECK_FALSE(trie_contains(&t, "apples"));  /* extension, not a word */
    CHECK_TRUE(trie_starts_with(&t, "app"));
    CHECK_TRUE(trie_starts_with(&t, "apple"));
    CHECK_FALSE(trie_starts_with(&t, "apples"));
    CHECK_FALSE(trie_starts_with(&t, "b"));
    CHECK_TRUE(trie_starts_with(&t, "")); /* every word starts with "" */

    /* now insert the prefix as its own word */
    CHECK_TRUE(trie_insert(&t, "app"));
    CHECK_TRUE(trie_contains(&t, "app"));
    CHECK_FALSE(trie_insert(&t, "app")); /* duplicate */

    /* ---- empty string is a valid word ---- */
    CHECK_FALSE(trie_contains(&t, ""));
    CHECK_TRUE(trie_insert(&t, ""));
    CHECK_TRUE(trie_contains(&t, ""));
    CHECK_TRUE(trie_remove(&t, ""));
    CHECK_FALSE(trie_contains(&t, ""));

    /* ---- removal: word vs prefix-sharing words ---- */
    CHECK_TRUE(trie_insert(&t, "application"));
    CHECK_TRUE(trie_remove(&t, "apple"));
    CHECK_FALSE(trie_contains(&t, "apple"));
    CHECK_TRUE(trie_contains(&t, "app"));         /* shared prefix intact */
    CHECK_TRUE(trie_contains(&t, "application")); /* longer word intact */
    CHECK_TRUE(trie_starts_with(&t, "appl"));     /* via "application" */
    CHECK_FALSE(trie_remove(&t, "apple"));        /* already gone */
    CHECK_FALSE(trie_remove(&t, "banana"));       /* never there */

    /* removing the long word must prune, but keep "app" */
    CHECK_TRUE(trie_remove(&t, "application"));
    CHECK_FALSE(trie_starts_with(&t, "appl")); /* pruned branch is gone */
    CHECK_TRUE(trie_contains(&t, "app"));
    CHECK_TRUE(trie_remove(&t, "app"));
    CHECK_FALSE(trie_starts_with(&t, "a")); /* trie is empty again */
    trie_free(&t);

    /* ---- randomized differential test vs the word-list oracle ---- */
    srand(101006);
    CHECK_TRUE(trie_init(&t));
    static const char *alphabet = "abc"; /* tiny alphabet: dense overlaps */
    for (int op = 0; op < 2500; op++) {
        char w[8];
        int len = rand() % 6; /* 0..5, includes "" */
        for (int i = 0; i < len; i++) {
            w[i] = alphabet[rand() % 3];
        }
        w[len] = '\0';
        int r = rand() % 4;
        if (r == 0 && nwords < MAXW) {
            CHECK_INT_EQ(trie_insert(&t, w), ora_insert(w));
        } else if (r == 1) {
            CHECK_INT_EQ(trie_remove(&t, w), ora_remove(w));
        } else if (r == 2) {
            CHECK_INT_EQ(trie_contains(&t, w), ora_contains(w));
        } else {
            CHECK_INT_EQ(trie_starts_with(&t, w), ora_starts_with(w));
        }
    }
    /* final sweep */
    for (size_t i = 0; i < nwords; i++) {
        CHECK_TRUE(trie_contains(&t, words[i]));
    }
    trie_free(&t);

    CTEST_END();
}
