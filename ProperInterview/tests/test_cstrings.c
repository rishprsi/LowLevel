#include "cstrings.h"
#include "ctest.h"

#include <limits.h>
#include <stdio.h>

int main(void) {
    /* ---- words_reverse ---- */
    {
        SECTION("words_reverse");
        char a[] = "the quick brown fox";
        CHECK_UINT_EQ(words_reverse(a), 4);
        CHECK_STR_EQ(a, "fox brown quick the");

        char b[] = "hello";
        CHECK_UINT_EQ(words_reverse(b), 1);
        CHECK_STR_EQ(b, "hello");

        char c[] = "a b";
        CHECK_UINT_EQ(words_reverse(c), 2);
        CHECK_STR_EQ(c, "b a");

        char d[] = "";
        CHECK_UINT_EQ(words_reverse(d), 0);
        CHECK_STR_EQ(d, "");

        char e[] = "ab cd ef";
        CHECK_UINT_EQ(words_reverse(e), 3);
        CHECK_STR_EQ(e, "ef cd ab");

        /* words of different lengths — total length is preserved */
        char f[] = "a bb ccc dddd";
        CHECK_UINT_EQ(words_reverse(f), 4);
        CHECK_STR_EQ(f, "dddd ccc bb a");
    }

    /* ---- my_atoi ---- */
    {
        SECTION("my_atoi");
        int v = 12345; /* sentinel: must be untouched on error */

        CHECK_INT_EQ(my_atoi("0", &v), 0);
        CHECK_INT_EQ(v, 0);
        CHECK_INT_EQ(my_atoi("42", &v), 0);
        CHECK_INT_EQ(v, 42);
        CHECK_INT_EQ(my_atoi("-7", &v), 0);
        CHECK_INT_EQ(v, -7);
        CHECK_INT_EQ(my_atoi("+9", &v), 0);
        CHECK_INT_EQ(v, 9);
        CHECK_INT_EQ(my_atoi("007", &v), 0); /* leading zeros are fine */
        CHECK_INT_EQ(v, 7);

        /* exact INT_MAX / INT_MIN must parse */
        char buf[32];
        snprintf(buf, sizeof(buf), "%d", INT_MAX);
        CHECK_INT_EQ(my_atoi(buf, &v), 0);
        CHECK_INT_EQ(v, INT_MAX);
        snprintf(buf, sizeof(buf), "%d", INT_MIN);
        CHECK_INT_EQ(my_atoi(buf, &v), 0);
        CHECK_INT_EQ(v, INT_MIN);

        /* -1: empty / no digits / trailing garbage — v untouched */
        v = 777;
        CHECK_INT_EQ(my_atoi("", &v), -1);
        CHECK_INT_EQ(my_atoi("+", &v), -1);
        CHECK_INT_EQ(my_atoi("-", &v), -1);
        CHECK_INT_EQ(my_atoi("abc", &v), -1);
        CHECK_INT_EQ(my_atoi("12x", &v), -1);
        CHECK_INT_EQ(my_atoi("1 ", &v), -1);
        CHECK_INT_EQ(my_atoi(" 1", &v), -1);
        CHECK_INT_EQ(my_atoi("--5", &v), -1);
        CHECK_INT_EQ(v, 777);

        /* -2: one past INT_MAX / INT_MIN, and something huge — v untouched */
        CHECK_INT_EQ(my_atoi("2147483648", &v), -2);
        CHECK_INT_EQ(my_atoi("-2147483649", &v), -2);
        CHECK_INT_EQ(my_atoi("99999999999999999999", &v), -2);
        CHECK_INT_EQ(my_atoi("-99999999999999999999", &v), -2);
        CHECK_INT_EQ(v, 777);
    }

    /* ---- str_count_words ---- */
    {
        SECTION("str_count_words");
        CHECK_UINT_EQ(str_count_words(""), 0);
        CHECK_UINT_EQ(str_count_words("   "), 0);
        CHECK_UINT_EQ(str_count_words("hello"), 1);
        CHECK_UINT_EQ(str_count_words("hello world"), 2);
        CHECK_UINT_EQ(str_count_words("  a  b "), 2);
        CHECK_UINT_EQ(str_count_words("a\tb\nc"), 3);
        CHECK_UINT_EQ(str_count_words("one   two\t\tthree\n"), 3);
        CHECK_UINT_EQ(str_count_words("\n\t x"), 1);
    }

    CTEST_END();
}
