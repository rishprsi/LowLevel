#include "ctest.h"
#include "stackqueue.h"

#include <stdlib.h>

int main(void) {
    int out = -777;

    /* ================= Stack ================= */
    Stack s;
    SECTION("stack_init / empty");
    stack_init(&s);
    CHECK_TRUE(stack_is_empty(&s));
    CHECK_FALSE(stack_pop(&s, &out));
    CHECK_FALSE(stack_peek(&s, &out));
    CHECK_INT_EQ(out, -777);

    /* LIFO order across a growth boundary */
    SECTION("stack push/pop/peek LIFO");
    for (int i = 0; i < 10; i++) {
        CHECK_TRUE(stack_push(&s, i * 11));
    }
    CHECK_FALSE(stack_is_empty(&s));
    CHECK_TRUE(stack_peek(&s, &out));
    CHECK_INT_EQ(out, 99);
    CHECK_UINT_EQ(s.len, 10); /* peek does not remove */
    for (int i = 9; i >= 0; i--) {
        CHECK_TRUE(stack_pop(&s, &out));
        CHECK_INT_EQ(out, i * 11);
    }
    CHECK_TRUE(stack_is_empty(&s));
    CHECK_FALSE(stack_pop(&s, &out));
    stack_free(&s);
    stack_free(&s); /* double free of empty state is safe */

    /* randomized differential: stack vs shadow array */
    SECTION("stack differential vs oracle");
    srand(1111);
    stack_init(&s);
    static int sh[2048];
    size_t sn = 0;
    for (int op = 0; op < 3000; op++) {
        if ((rand() % 2 == 0 || sn == 0) && sn < 2048) {
            int x = rand();
            CHECK_TRUE_MSG(stack_push(&s, x), "op=%d", op);
            sh[sn++] = x;
        } else {
            CHECK_TRUE_MSG(stack_peek(&s, &out), "op=%d", op);
            CHECK_INT_EQ_MSG(out, sh[sn - 1], "op=%d", op);
            CHECK_TRUE_MSG(stack_pop(&s, &out), "op=%d", op);
            CHECK_INT_EQ_MSG(out, sh[--sn], "op=%d", op);
        }
        CHECK_INT_EQ_MSG(stack_is_empty(&s), sn == 0, "op=%d", op);
    }
    stack_free(&s);

    /* ================= Queue (ring buffer) ================= */
    Queue q;
    SECTION("queue_init / empty");
    CHECK_TRUE(queue_init(&q, 5));
    CHECK_TRUE(queue_is_empty(&q));
    CHECK_FALSE(queue_is_full(&q));
    out = -777;
    CHECK_FALSE(queue_dequeue(&q, &out));
    CHECK_INT_EQ(out, -777);

    /* fill to capacity, then reject */
    SECTION("queue_enqueue fill / full");
    for (int i = 0; i < 5; i++) {
        CHECK_TRUE(queue_enqueue(&q, i));
    }
    CHECK_TRUE(queue_is_full(&q));
    CHECK_FALSE(queue_enqueue(&q, 99)); /* full: rejected, unchanged */

    /* FIFO order */
    SECTION("queue_dequeue FIFO");
    CHECK_TRUE(queue_dequeue(&q, &out));
    CHECK_INT_EQ(out, 0);
    CHECK_TRUE(queue_dequeue(&q, &out));
    CHECK_INT_EQ(out, 1);

    /* wrap around the ring several times: keep it 3 full for 40 rounds */
    SECTION("queue ring wraparound");
    int next_in = 5, next_out = 2;
    for (int round = 0; round < 40; round++) {
        CHECK_TRUE(queue_enqueue(&q, next_in++));
        CHECK_TRUE(queue_dequeue(&q, &out));
        CHECK_INT_EQ(out, next_out++);
    }
    /* drain the remaining 3 */
    for (int i = 0; i < 3; i++) {
        CHECK_TRUE(queue_dequeue(&q, &out));
        CHECK_INT_EQ(out, next_out++);
    }
    CHECK_TRUE(queue_is_empty(&q));
    CHECK_FALSE(queue_dequeue(&q, &out));
    queue_free(&q);

    /* capacity-1 queue: full and empty at the same boundary */
    SECTION("queue capacity-1 boundary");
    CHECK_TRUE(queue_init(&q, 1));
    CHECK_TRUE(queue_enqueue(&q, 7));
    CHECK_TRUE(queue_is_full(&q));
    CHECK_FALSE(queue_enqueue(&q, 8));
    CHECK_TRUE(queue_dequeue(&q, &out));
    CHECK_INT_EQ(out, 7);
    CHECK_TRUE(queue_is_empty(&q));
    queue_free(&q);

    /* randomized differential: ring queue vs shadow FIFO */
    SECTION("queue differential vs oracle");
    srand(2222);
    CHECK_TRUE(queue_init(&q, 8));
    static int fifo[4000];
    size_t fh = 0, ft = 0; /* fifo[fh..ft) pending */
    for (int op = 0; op < 3000; op++) {
        if (rand() % 2 == 0) {
            int x = rand();
            bool got = queue_enqueue(&q, x);
            CHECK_INT_EQ_MSG(got, ft - fh < 8, "op=%d", op);
            if (got) {
                fifo[ft++ % 4000] = x; /* ft never exceeds 4000 here */
            }
        } else {
            bool got = queue_dequeue(&q, &out);
            CHECK_INT_EQ_MSG(got, ft > fh, "op=%d", op);
            if (got) {
                CHECK_INT_EQ_MSG(out, fifo[fh++ % 4000], "op=%d", op);
            }
        }
        CHECK_INT_EQ_MSG(queue_is_empty(&q), ft == fh, "op=%d", op);
        CHECK_INT_EQ_MSG(queue_is_full(&q), ft - fh == 8, "op=%d", op);
    }
    queue_free(&q);

    CTEST_END();
}
