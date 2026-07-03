#include "ctest.h"
#include "memory.h"

#include <stdlib.h>
#include <string.h>

int main(void) {
    /* my_strdup */
    {
        SECTION("my_strdup");
        const char *orig = "hello";
        char *copy = my_strdup(orig);
        CHECK_PTR_NONNULL(copy);
        CHECK(copy != orig);          /* must be a distinct allocation */
        CHECK_STR_EQ(copy, "hello");
        copy[0] = 'H';                /* writable, independent of orig */
        CHECK_STR_EQ(orig, "hello");
        free(copy);

        char *empty = my_strdup("");
        CHECK_PTR_NONNULL(empty);
        CHECK_STR_EQ(empty, "");
        free(empty);
    }

    /* IntVec — one SECTION per function so a failure in one still lets the
     * others run and report (they do chain: get/pop build on push's data). */
    {
        IntVec v;

        SECTION("vec_init");
        vec_init(&v);
        CHECK_INT_EQ(v.len, 0);
        CHECK_INT_EQ(v.cap, 0); /* data may be NULL; len and cap must be 0 */

        SECTION("vec_push");
        for (int i = 0; i < 10; i++) {
            CHECK_TRUE(vec_push(&v, i * i));
        }
        CHECK_INT_EQ(v.len, 10);
        CHECK_TRUE(v.cap >= v.len);

        SECTION("vec_get");
        for (int i = 0; i < 10; i++) {
            CHECK_INT_EQ(vec_get(&v, (size_t)i), i * i);
        }

        SECTION("vec_pop");
        int out = -1;
        CHECK_TRUE(vec_pop(&v, &out));
        CHECK_INT_EQ(out, 81); /* 9*9 */
        CHECK_INT_EQ(v.len, 9);
        /* drain the rest — bounded so a vec_pop that never shrinks len (or a
         * vec_push that set len wrong) fails fast instead of hanging forever */
        for (int guard = 0; v.len > 0 && guard < 100000; guard++) {
            CHECK_TRUE(vec_pop(&v, &out));
        }
        CHECK_INT_EQ(v.len, 0);
        /* pop on empty */
        out = 12345;
        CHECK_FALSE(vec_pop(&v, &out));
        CHECK_INT_EQ(out, 12345); /* left untouched */

        SECTION("vec_free");
        vec_free(&v);
        CHECK_INT_EQ(v.len, 0);
        CHECK_INT_EQ(v.cap, 0);
        vec_free(&v); /* double free of the vector wrapper must be safe */
    }

    CTEST_END();
}
