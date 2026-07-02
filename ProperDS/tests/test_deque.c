#include "ctest.h"
#include "deque.h"

#include <stdlib.h>
#include <string.h>

/*
 * Differential oracle: a dumb contiguous array where push_front/pop_front
 * memmove the whole thing. Obviously correct, obviously slow — perfect.
 */
#define ORA_CAP 4096
static int ora[ORA_CAP];
static size_t ora_len = 0;

static void ora_push_front(int x) {
    memmove(ora + 1, ora, ora_len * sizeof(int));
    ora[0] = x;
    ora_len++;
}

static void ora_push_back(int x) {
    ora[ora_len++] = x;
}

static int ora_pop_front(void) {
    int x = ora[0];
    memmove(ora, ora + 1, (ora_len - 1) * sizeof(int));
    ora_len--;
    return x;
}

static int ora_pop_back(void) {
    return ora[--ora_len];
}

int main(void) {
    Deque d;
    int out = -777;

    /* ---- empty-state behavior ---- */
    SECTION("deque_init / empty");
    deque_init(&d);
    CHECK_UINT_EQ(deque_len(&d), 0);
    CHECK_TRUE(deque_validate(&d));
    CHECK_FALSE(deque_pop_front(&d, &out));
    CHECK_FALSE(deque_pop_back(&d, &out));
    CHECK_INT_EQ(out, -777);
    deque_free(&d); /* free of empty deque is safe */

    /* ---- basic ordering: it's a deque, not two stacks ---- */
    SECTION("deque push/pop both ends");
    deque_init(&d);
    CHECK_TRUE(deque_push_back(&d, 2));  /* [2] */
    CHECK_TRUE(deque_push_front(&d, 1)); /* [1 2] */
    CHECK_TRUE(deque_push_back(&d, 3));  /* [1 2 3] */
    CHECK_TRUE(deque_push_front(&d, 0)); /* [0 1 2 3] */
    CHECK_UINT_EQ(deque_len(&d), 4);
    CHECK_TRUE(deque_validate(&d));
    CHECK_TRUE(deque_pop_front(&d, &out));
    CHECK_INT_EQ(out, 0);
    CHECK_TRUE(deque_pop_back(&d, &out));
    CHECK_INT_EQ(out, 3);
    CHECK_TRUE(deque_pop_front(&d, &out));
    CHECK_INT_EQ(out, 1);
    CHECK_TRUE(deque_pop_back(&d, &out));
    CHECK_INT_EQ(out, 2);
    CHECK_UINT_EQ(deque_len(&d), 0);
    CHECK_TRUE(deque_validate(&d));

    /* ---- force wraparound + growth: fill via push_front only ---- */
    SECTION("deque wraparound + growth");
    for (int i = 0; i < 100; i++) {
        CHECK_TRUE(deque_push_front(&d, i));
        CHECK_TRUE(deque_validate(&d));
    }
    CHECK_UINT_EQ(deque_len(&d), 100);
    for (int i = 99; i >= 0; i--) {
        CHECK_TRUE(deque_pop_front(&d, &out));
        CHECK_INT_EQ(out, i);
    }
    deque_free(&d);

    /* ---- randomized differential test vs the memmove oracle ---- */
    SECTION("differential vs oracle");
    srand(31337);
    deque_init(&d);
    for (int op = 0; op < 4000; op++) {
        int r = rand() % 4;
        int x = rand();
        if (r == 0 && ora_len < ORA_CAP) {
            CHECK_TRUE_MSG(deque_push_front(&d, x), "op=%d r=%d x=%d", op, r, x);
            ora_push_front(x);
        } else if (r == 1 && ora_len < ORA_CAP) {
            CHECK_TRUE_MSG(deque_push_back(&d, x), "op=%d r=%d x=%d", op, r, x);
            ora_push_back(x);
        } else if (r == 2) {
            bool got = deque_pop_front(&d, &out);
            CHECK_INT_EQ_MSG(got, ora_len > 0, "op=%d r=%d", op, r);
            if (got) {
                CHECK_INT_EQ_MSG(out, ora_pop_front(), "op=%d r=%d", op, r);
            }
        } else if (r == 3) {
            bool got = deque_pop_back(&d, &out);
            CHECK_INT_EQ_MSG(got, ora_len > 0, "op=%d r=%d", op, r);
            if (got) {
                CHECK_INT_EQ_MSG(out, ora_pop_back(), "op=%d r=%d", op, r);
            }
        }
        CHECK_TRUE_MSG(deque_validate(&d), "op=%d r=%d", op, r);
        CHECK_UINT_EQ_MSG(deque_len(&d), ora_len, "op=%d r=%d", op, r);
    }
    /* drain the remainder front-first and compare */
    SECTION("differential drain compare");
    while (ora_len > 0) {
        CHECK_TRUE_MSG(deque_pop_front(&d, &out), "ora_len=%zu", ora_len);
        CHECK_INT_EQ_MSG(out, ora_pop_front(), "ora_len=%zu", ora_len);
    }
    CHECK_UINT_EQ(deque_len(&d), 0);
    deque_free(&d);

    CTEST_END();
}
