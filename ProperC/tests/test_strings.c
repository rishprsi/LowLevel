#include "ctest.h"
#include "strings.h"

int main(void) {
    SECTION("my_strlen");
    CHECK_INT_EQ(my_strlen(""), 0);
    CHECK_INT_EQ(my_strlen("hello"), 5);
    CHECK_INT_EQ(my_strlen("a\0b"), 1);

    /* my_strcpy */
    {
        SECTION("my_strcpy");
        char dst[16];
        char *r = my_strcpy(dst, "hello");
        CHECK(r == dst);
        CHECK_STR_EQ(dst, "hello");
    }

    /* my_strncpy semantics */
    {
        SECTION("my_strncpy");
        /* src shorter than n: padded with '\0' */
        char dst[8];
        for (int i = 0; i < 8; i++) {
            dst[i] = 'x';
        }
        my_strncpy(dst, "ab", 5);
        CHECK_INT_EQ(dst[0], 'a');
        CHECK_INT_EQ(dst[1], 'b');
        CHECK_INT_EQ(dst[2], '\0');
        CHECK_INT_EQ(dst[3], '\0');
        CHECK_INT_EQ(dst[4], '\0');
        CHECK_INT_EQ(dst[5], 'x'); /* untouched beyond n */

        /* src length >= n: no NUL terminator written */
        char d2[8];
        for (int i = 0; i < 8; i++) {
            d2[i] = 'x';
        }
        my_strncpy(d2, "abcdef", 3);
        CHECK_INT_EQ(d2[0], 'a');
        CHECK_INT_EQ(d2[1], 'b');
        CHECK_INT_EQ(d2[2], 'c');
        CHECK_INT_EQ(d2[3], 'x'); /* not terminated */
    }

    SECTION("my_strcmp");
    CHECK_INT_EQ(my_strcmp("abc", "abc"), 0);
    CHECK(my_strcmp("abc", "abd") < 0);
    CHECK(my_strcmp("abd", "abc") > 0);
    CHECK(my_strcmp("ab", "abc") < 0);  /* prefix is smaller */
    CHECK(my_strcmp("abc", "ab") > 0);

    /* my_strcat */
    {
        SECTION("my_strcat");
        char dst[16] = "foo";
        char *r = my_strcat(dst, "bar");
        CHECK(r == dst);
        CHECK_STR_EQ(dst, "foobar");
    }

    /* my_strchr */
    {
        SECTION("my_strchr");
        const char *s = "hello";
        CHECK(my_strchr(s, 'l') == s + 2);
        CHECK(my_strchr(s, 'h') == s);
        CHECK_PTR_NULL(my_strchr(s, 'z'));
        CHECK(my_strchr(s, '\0') == s + 5); /* finds the terminator */
    }

    /* my_strstr */
    {
        SECTION("my_strstr");
        const char *h = "hello world";
        CHECK(my_strstr(h, "wor") == h + 6);
        CHECK(my_strstr(h, "hello") == h);
        CHECK_PTR_NULL(my_strstr(h, "xyz"));
        CHECK(my_strstr(h, "") == h); /* empty needle */
        CHECK_PTR_NULL(my_strstr("ab", "abc")); /* needle longer */
    }

    /* str_reverse */
    {
        SECTION("str_reverse");
        char a[] = "abc";
        str_reverse(a);
        CHECK_STR_EQ(a, "cba");
        char b[] = "ab";
        str_reverse(b);
        CHECK_STR_EQ(b, "ba");
        char c[] = "x";
        str_reverse(c);
        CHECK_STR_EQ(c, "x");
        char e[] = "";
        str_reverse(e);
        CHECK_STR_EQ(e, "");
    }

    SECTION("is_palindrome");
    CHECK_TRUE(is_palindrome("racecar"));
    CHECK_TRUE(is_palindrome("abba"));
    CHECK_TRUE(is_palindrome(""));
    CHECK_TRUE(is_palindrome("a"));
    CHECK_FALSE(is_palindrome("abca"));
    CHECK_FALSE(is_palindrome("ab"));

    CTEST_END();
}
