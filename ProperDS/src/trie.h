#ifndef PROPERDS_TRIE_H
#define PROPERDS_TRIE_H

#include <stdbool.h>
#include <stddef.h>

/*
 * trie — a prefix tree over the lowercase alphabet 'a'..'z'.
 *
 * Every node has 26 child slots (child[c - 'a']) and an is_word flag that
 * marks the END of an inserted word. This is what distinguishes a stored
 * WORD from a mere PREFIX: after inserting "apple", contains("app") is
 * false but starts_with("app") is true.
 *
 * The empty string "" is a valid word (it terminates at the root).
 * All inputs are guaranteed to contain only 'a'..'z' characters.
 */
typedef struct TrieNode {
    struct TrieNode *child[26];
    bool is_word;
} TrieNode;

typedef struct {
    TrieNode *root; /* always non-NULL after init */
} Trie;

/* Allocate the root node. Returns false on allocation failure. */
bool trie_init(Trie *t);

/* Free every node. Safe to call twice. */
void trie_free(Trie *t);

/* Insert word. Returns true if newly inserted, false if it was already
 * present (or on allocation failure). */
bool trie_insert(Trie *t, const char *word);

/* True iff word was inserted as a WHOLE word (not just as a prefix of
 * some other word). */
bool trie_contains(const Trie *t, const char *word);

/* True iff at least one inserted word starts with prefix. Every word
 * starts with "". */
bool trie_starts_with(const Trie *t, const char *prefix);

/* Remove word. Returns true if it was present. Nodes that no longer lead
 * to any word should be freed (pruned), but other words sharing a prefix
 * must survive. */
bool trie_remove(Trie *t, const char *word);

#endif /* PROPERDS_TRIE_H */
