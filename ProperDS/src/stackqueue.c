#include "stackqueue.h"
#include "todo.h"

void stack_init(Stack *s) {
    (void)s;
    TODO("implement stack_init");
}

void stack_free(Stack *s) {
    (void)s;
    TODO("implement stack_free");
}

bool stack_push(Stack *s, int x) {
    (void)s;
    (void)x;
    TODO("implement stack_push");
}

bool stack_pop(Stack *s, int *out) {
    (void)s;
    (void)out;
    TODO("implement stack_pop");
}

bool stack_peek(const Stack *s, int *out) {
    (void)s;
    (void)out;
    TODO("implement stack_peek");
}

bool stack_is_empty(const Stack *s) {
    (void)s;
    TODO("implement stack_is_empty");
}

bool queue_init(Queue *q, size_t cap) {
    (void)q;
    (void)cap;
    TODO("implement queue_init");
}

void queue_free(Queue *q) {
    (void)q;
    TODO("implement queue_free");
}

bool queue_enqueue(Queue *q, int x) {
    (void)q;
    (void)x;
    TODO("implement queue_enqueue");
}

bool queue_dequeue(Queue *q, int *out) {
    (void)q;
    (void)out;
    TODO("implement queue_dequeue");
}

bool queue_is_full(const Queue *q) {
    (void)q;
    TODO("implement queue_is_full");
}

bool queue_is_empty(const Queue *q) {
    (void)q;
    TODO("implement queue_is_empty");
}
