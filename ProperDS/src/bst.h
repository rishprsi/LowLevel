#ifndef PROPERDS_BST_H
#define PROPERDS_BST_H

#include <stdbool.h>
#include <stddef.h>

/*
 * bst — an UNBALANCED binary search tree of unique int keys.
 *
 * Ordering invariant: for every node, all keys in its left subtree are
 * strictly smaller and all keys in its right subtree strictly larger.
 * Duplicates are rejected (insert returns false and changes nothing).
 *
 * Removal must handle all three classic cases:
 *   1. leaf
 *   2. one child (splice the child up)
 *   3. two children (replace the key with the inorder successor — the
 *      minimum of the right subtree — then remove that successor node)
 */
typedef struct BstNode {
    int key;
    struct BstNode *left;
    struct BstNode *right;
} BstNode;

typedef struct {
    BstNode *root;
    size_t size;
} Bst;

/* Initialize to the empty tree. */
void bst_init(Bst *t);

/* Free every node and reset to empty. Safe on an empty tree. */
void bst_free(Bst *t);

/* Insert key. Returns true if inserted, false if already present
 * (or on allocation failure). */
bool bst_insert(Bst *t, int key);

/* True iff key is in the tree. */
bool bst_contains(const Bst *t, int key);

/* Remove key. Returns true if it was present, false otherwise. */
bool bst_remove(Bst *t, int key);

/* Smallest key -> *out. Returns false (out untouched) if the tree is
 * empty. */
bool bst_min(const Bst *t, int *out);

/* Largest key -> *out. Returns false (out untouched) if empty. */
bool bst_max(const Bst *t, int *out);

/* Number of keys. */
size_t bst_size(const Bst *t);

/* Fill out[0 .. size) with the keys in ascending (inorder) order.
 * The caller guarantees out has room for bst_size(t) ints. */
void bst_inorder(const Bst *t, int *out);

/* Invariant checker: the ordering invariant holds everywhere and the node
 * count equals size. */
bool bst_validate(const Bst *t);

#endif /* PROPERDS_BST_H */
