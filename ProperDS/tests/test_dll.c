#include "ctest.h"
#include "dll.h"

#include <stdlib.h>

int main(void) {
    Dll d;
    int out = -777;

    /* ---- empty-list behavior ---- */
    dll_init(&d);
    CHECK_UINT_EQ(dll_length(&d), 0);
    CHECK_TRUE(dll_validate(&d));
    CHECK_FALSE(dll_pop_front(&d, &out));
    CHECK_FALSE(dll_pop_back(&d, &out));
    CHECK_INT_EQ(out, -777); /* out untouched on failure */
    dll_free(&d);            /* freeing empty list is safe */
    CHECK_TRUE(dll_validate(&d));

    /* ---- push/pop at both ends ---- */
    CHECK_TRUE(dll_push_back(&d, 2));  /* [2] */
    CHECK_TRUE(dll_validate(&d));
    CHECK_TRUE(dll_push_front(&d, 1)); /* [1 2] */
    CHECK_TRUE(dll_push_back(&d, 3));  /* [1 2 3] */
    CHECK_TRUE(dll_push_back(&d, 4));  /* [1 2 3 4] */
    CHECK_TRUE(dll_validate(&d));
    CHECK_UINT_EQ(dll_length(&d), 4);
    CHECK_INT_EQ(d.head->value, 1);
    CHECK_INT_EQ(d.tail->value, 4);

    CHECK_TRUE(dll_pop_front(&d, &out));
    CHECK_INT_EQ(out, 1);
    CHECK_TRUE(dll_pop_back(&d, &out));
    CHECK_INT_EQ(out, 4);
    CHECK_TRUE(dll_validate(&d));
    CHECK_UINT_EQ(dll_length(&d), 2);

    /* drain down to empty via alternating ends */
    CHECK_TRUE(dll_pop_back(&d, &out));
    CHECK_INT_EQ(out, 3);
    CHECK_TRUE(dll_validate(&d)); /* single node: head == tail */
    CHECK_TRUE(d.head == d.tail);
    CHECK_TRUE(dll_pop_front(&d, &out));
    CHECK_INT_EQ(out, 2);
    CHECK_TRUE(dll_validate(&d));
    CHECK_UINT_EQ(dll_length(&d), 0);
    CHECK_FALSE(dll_pop_front(&d, &out));
    dll_free(&d);

    /* ---- randomized differential test vs a shadow deque array ---- */
    srand(987654);
    static int shadow[1024];
    size_t shead = 512, slen = 0; /* shadow occupies [shead, shead+slen) */
    dll_init(&d);
    for (int op = 0; op < 3000; op++) {
        int r = rand() % 4;
        int x = rand();
        if (r == 0 && shead > 0 && slen < 500) { /* push_front */
            CHECK_TRUE(dll_push_front(&d, x));
            shadow[--shead] = x;
            slen++;
        } else if (r == 1 && shead + slen < 1024 && slen < 500) { /* push_back */
            CHECK_TRUE(dll_push_back(&d, x));
            shadow[shead + slen] = x;
            slen++;
        } else if (r == 2) { /* pop_front */
            bool got = dll_pop_front(&d, &out);
            CHECK_INT_EQ(got, slen > 0);
            if (slen > 0) {
                CHECK_INT_EQ(out, shadow[shead]);
                shead++;
                slen--;
                if (slen == 0) {
                    shead = 512; /* recenter when empty */
                }
            }
        } else { /* pop_back */
            bool got = dll_pop_back(&d, &out);
            CHECK_INT_EQ(got, slen > 0);
            if (slen > 0) {
                CHECK_INT_EQ(out, shadow[shead + slen - 1]);
                slen--;
                if (slen == 0) {
                    shead = 512;
                }
            }
        }
        CHECK_TRUE(dll_validate(&d));
        CHECK_UINT_EQ(dll_length(&d), slen);
    }
    /* drain and compare the remainder */
    while (slen > 0) {
        CHECK_TRUE(dll_pop_front(&d, &out));
        CHECK_INT_EQ(out, shadow[shead]);
        shead++;
        slen--;
    }
    CHECK_TRUE(dll_validate(&d));
    dll_free(&d);

    CTEST_END();
}
