#ifndef PROPERDS_AVL_H
#define PROPERDS_AVL_H

#include <stdbool.h>
#include <stddef.h>

/*
 * avl — a self-balancing AVL tree of unique int keys.
 *
 * Height convention (part of the contract):
 *   - height of an empty subtree is 0
 *   - height of a node is 1 + max(height(left), height(right))
 *   - every node CACHES its height in the height field
 *
 * AVL invariant: for every node, balance factor
 *   height(left) - height(right)  is in {-1, 0, +1}.
 * Insert and remove must restore it with single/double rotations, and must
 * keep every cached height correct — avl_validate() recomputes heights
 * from scratch and rejects the tree if any cached value lies.
 */
typedef struct AvlNode {
    int key;
    int height; /* cached: 1 + max(children heights), leaf = 1 */
    struct AvlNode *left;
    struct AvlNode *right;
} AvlNode;

typedef struct {
    AvlNode *root;
    size_t size;
} Avl;

/* Initialize to the empty tree. */
void avl_init(Avl *t);

/* Free every node and reset to empty. Safe on an empty tree. */
void avl_free(Avl *t);

/* Insert key, rebalancing as needed. Returns true if inserted, false if
 * already present (or on allocation failure). */
bool avl_insert(Avl *t, int key);

/* True iff key is in the tree. */
bool avl_contains(const Avl *t, int key);

/* Remove key, rebalancing as needed. Returns true if it was present. */
bool avl_remove(Avl *t, int key);

/* Height of the whole tree (0 if empty). */
int avl_height(const Avl *t);

/* Number of keys. */
size_t avl_size(const Avl *t);

/* Fill out[0 .. size) with the keys in ascending order. */
void avl_inorder(const Avl *t, int *out);

/* Invariant checker — recursively verifies ALL of:
 *   1. BST ordering
 *   2. every cached height equals the recomputed height
 *   3. every balance factor is in {-1, 0, +1}
 *   4. node count equals size
 */
bool avl_validate(const Avl *t);

#endif /* PROPERDS_AVL_H */
