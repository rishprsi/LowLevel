#include "ctest.h"
#include "strfns.h"

#include <string.h>

int main(void) {
    SECTION("my_strlen");
    CHECK_UINT_EQ(my_strlen(""), 0);
    CHECK_UINT_EQ(my_strlen("a"), 1);
    CHECK_UINT_EQ(my_strlen("hello"), 5);
    CHECK_UINT_EQ(my_strlen("with space"), 10);

    SECTION("my_strcpy");
    {
        char buf[32];
        memset(buf, 'X', sizeof(buf));
        CHECK(my_strcpy(buf, "hello") == buf); /* returns dst */
        CHECK_STR_EQ(buf, "hello");
        CHECK_INT_EQ(buf[5], '\0'); /* terminator actually copied */

        char empty[4];
        memset(empty, 'X', sizeof(empty));
        my_strcpy(empty, "");
        CHECK_STR_EQ(empty, "");
        CHECK_INT_EQ(empty[0], '\0');
    }

    SECTION("my_strcmp");
    CHECK_INT_EQ(my_strcmp("", ""), 0);
    CHECK_INT_EQ(my_strcmp("abc", "abc"), 0);
    CHECK_TRUE(my_strcmp("abc", "abd") < 0);   /* differing byte */
    CHECK_TRUE(my_strcmp("abd", "abc") > 0);
    CHECK_TRUE(my_strcmp("ab", "abc") < 0);    /* prefix sorts first */
    CHECK_TRUE(my_strcmp("abc", "ab") > 0);
    CHECK_TRUE(my_strcmp("", "a") < 0);        /* empty vs non-empty */
    /* unsigned-char comparison: 0xFF must sort AFTER ASCII 'a' */
    CHECK_TRUE(my_strcmp("\xff", "a") > 0);
    CHECK_TRUE(my_strcmp("a", "\xff") < 0);

    SECTION("my_memcpy");
    {
        unsigned char src[8] = {1, 2, 3, 4, 5, 6, 7, 8};
        unsigned char dst[8] = {0};
        CHECK(my_memcpy(dst, src, sizeof(src)) == dst); /* returns dst */
        for (size_t i = 0; i < sizeof(src); i++) {
            CHECK_INT_EQ_MSG(dst[i], src[i], "i=%zu", i);
        }

        /* n == 0 copies nothing */
        unsigned char sentinel[2] = {0xAA, 0xBB};
        my_memcpy(sentinel, src, 0);
        CHECK_INT_EQ(sentinel[0], 0xAA);
        CHECK_INT_EQ(sentinel[1], 0xBB);

        /* copying a whole struct (embedded NUL bytes must survive) */
        struct point {
            int x, y;
            char tag[4];
        } a = {-7, 42, "pt"}, b = {0, 0, ""};
        my_memcpy(&b, &a, sizeof(a));
        CHECK_INT_EQ(b.x, -7);
        CHECK_INT_EQ(b.y, 42);
        CHECK_STR_EQ(b.tag, "pt");
    }
    /* NOTE: overlapping regions are FORBIDDEN by the contract (that is
     * memmove's job), so there is deliberately no overlap test here. */

    SECTION("my_strcat");
    {
        char buf[32] = "foo";
        CHECK(my_strcat(buf, "bar") == buf); /* returns dst */
        CHECK_STR_EQ(buf, "foobar");

        char onto_empty[16] = "";
        my_strcat(onto_empty, "abc"); /* strcat onto empty dst */
        CHECK_STR_EQ(onto_empty, "abc");

        my_strcat(buf, ""); /* appending empty changes nothing */
        CHECK_STR_EQ(buf, "foobar");
    }

    CTEST_END();
}
