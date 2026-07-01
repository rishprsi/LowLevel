#ifndef PROPERDS_STACKQUEUE_H
#define PROPERDS_STACKQUEUE_H

#include <stdbool.h>
#include <stddef.h>

/*
 * stackqueue — two adapters over plain arrays:
 *
 *   Stack — growable array-backed LIFO (grows 0 -> 4, then doubles).
 *   Queue — FIXED-capacity ring-buffer FIFO. Elements live at logical
 *           positions data[(head + i) % cap]; enqueue at the tail,
 *           dequeue at the head, indices wrap around.
 */
typedef struct {
    int *data;
    size_t len;
    size_t cap;
} Stack;

/* Initialize to empty (len = cap = 0, data = NULL). */
void stack_init(Stack *s);

/* Free the backing storage and reset to empty. Safe on an empty stack. */
void stack_free(Stack *s);

/* Push x on top. Returns false only on allocation failure. */
bool stack_push(Stack *s, int x);

/* Pop the top into *out. Returns false (out untouched) if empty. */
bool stack_pop(Stack *s, int *out);

/* Read the top into *out without removing. Returns false if empty. */
bool stack_peek(const Stack *s, int *out);

bool stack_is_empty(const Stack *s);

typedef struct {
    int *data;
    size_t cap;  /* fixed at init */
    size_t head; /* index of the oldest element */
    size_t len;  /* number of elements currently stored */
} Queue;

/* Allocate a ring buffer of exactly cap slots. Precondition: cap > 0.
 * Returns false only on allocation failure. */
bool queue_init(Queue *q, size_t cap);

/* Free the backing storage. */
void queue_free(Queue *q);

/* Append x at the tail. Returns false (queue unchanged) if full. */
bool queue_enqueue(Queue *q, int x);

/* Remove the oldest element into *out. Returns false (out untouched) if
 * empty. */
bool queue_dequeue(Queue *q, int *out);

bool queue_is_full(const Queue *q);
bool queue_is_empty(const Queue *q);

#endif /* PROPERDS_STACKQUEUE_H */
