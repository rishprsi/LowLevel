#ifndef PROPERDS_INTRUSIVE_H
#define PROPERDS_INTRUSIVE_H

#include <stdbool.h>
#include <stddef.h>

/*
 * intrusive — a Linux-kernel-style intrusive circular doubly linked list.
 *
 * Instead of the list allocating nodes that point at your data, YOU embed a
 * `struct list_head` inside your own struct, and recover the containing
 * struct with container_of(). The list itself never allocates.
 *
 * The list is circular with a sentinel: an empty list is a head whose next
 * and prev both point at itself. Iteration runs from head->next until you
 * get back to the head sentinel:
 *
 *     struct task { int id; struct list_head link; };
 *     struct list_head todo = LIST_INIT(todo);
 *     ...
 *     for (struct list_head *p = todo.next; p != &todo; p = p->next) {
 *         struct task *t = container_of(p, struct task, link);
 *         ...
 *     }
 */
struct list_head {
    struct list_head *next;
    struct list_head *prev;
};

/* Static initializer: struct list_head h = LIST_INIT(h); makes h empty. */
#define LIST_INIT(name) { &(name), &(name) }

/* Recover a pointer to the struct that embeds `member`, given a pointer to
 * that member. Example:
 *     struct task *t = container_of(p, struct task, link);
 */
#define container_of(ptr, type, member)                                        \
    ((type *)((char *)(ptr) - offsetof(type, member)))

/* Runtime initializer: make head an empty list (next = prev = head). */
void list_head_init(struct list_head *head);

/* Insert node immediately AFTER head (i.e. at the front of the list). */
void list_add(struct list_head *node, struct list_head *head);

/* Insert node immediately BEFORE head (i.e. at the tail of the list). */
void list_add_tail(struct list_head *node, struct list_head *head);

/* Unlink entry from whatever list it is on. The entry's own next/prev are
 * left dangling — it must be re-initialized before reuse. */
void list_del(struct list_head *entry);

/* True iff the list contains no entries besides the sentinel. */
bool list_empty(const struct list_head *head);

#endif /* PROPERDS_INTRUSIVE_H */
