#include "ctest.h"
#include "safety.h"

#include <string.h>

int main(void) {
    /* safe_strcpy: fits */
    {
        char dst[16];
        memset(dst, 'Z', sizeof(dst));
        size_t r = safe_strcpy(dst, sizeof(dst), "hello");
        CHECK_INT_EQ(r, 5);
        CHECK_STR_EQ(dst, "hello");
    }

    /* safe_strcpy: truncation, must NOT overflow and must NUL-terminate */
    {
        char dst[4];
        memset(dst, 'Z', sizeof(dst));
        size_t r = safe_strcpy(dst, sizeof(dst), "hello"); /* only "hel" fits */
        CHECK_INT_EQ(r, 5);                 /* returns full source length */
        CHECK_INT_EQ(strlen(dst), 3);
        CHECK_STR_EQ(dst, "hel");
    }

    /* safe_strcpy: dstsize 0 writes nothing */
    {
        char dst[2] = {'A', 'B'};
        size_t r = safe_strcpy(dst, 0, "hello");
        CHECK_INT_EQ(r, 5);
        CHECK_INT_EQ(dst[0], 'A'); /* untouched */
    }

    /* safe_strcat: fits */
    {
        char dst[16] = "foo";
        size_t r = safe_strcat(dst, sizeof(dst), "bar");
        CHECK_INT_EQ(r, 6);
        CHECK_STR_EQ(dst, "foobar");
    }

    /* safe_strcat: truncation */
    {
        char dst[6] = "foo"; /* room for 2 more chars + NUL */
        size_t r = safe_strcat(dst, sizeof(dst), "bar");
        CHECK_INT_EQ(r, 6);  /* tried to make length 6 */
        CHECK_STR_EQ(dst, "fooba");
        CHECK_INT_EQ(strlen(dst), 5);
    }

    /* copy_n_safe */
    {
        uint8_t dst[4] = {0};
        uint8_t src[5] = {1, 2, 3, 4, 5};
        CHECK_TRUE(copy_n_safe(dst, sizeof(dst), src, 4));
        CHECK_INT_EQ(dst[3], 4);
        CHECK_FALSE(copy_n_safe(dst, sizeof(dst), src, 5)); /* would overflow */
    }

    CTEST_END();
}
