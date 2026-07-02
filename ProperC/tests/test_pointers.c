#include "ctest.h"
#include "pointers.h"

int main(void) {
    /* swap_int */
    {
        SECTION("swap_int");
        int a = 3, b = 7;
        swap_int(&a, &b);
        CHECK_INT_EQ(a, 7);
        CHECK_INT_EQ(b, 3);
    }

    /* reverse_array */
    {
        SECTION("reverse_array");
        int v[] = {1, 2, 3, 4, 5};
        reverse_array(v, 5);
        int want[] = {5, 4, 3, 2, 1};
        for (int i = 0; i < 5; i++) {
            CHECK_INT_EQ(v[i], want[i]);
        }
        int two[] = {9, 8};
        reverse_array(two, 2);
        CHECK_INT_EQ(two[0], 8);
        CHECK_INT_EQ(two[1], 9);
        int one[] = {42};
        reverse_array(one, 1);
        CHECK_INT_EQ(one[0], 42);
        reverse_array(one, 0); /* no-op, must not touch memory */
        CHECK_INT_EQ(one[0], 42);
    }

    /* my_memcpy (non-overlapping) */
    {
        SECTION("my_memcpy");
        char src[] = "hello world";
        char dst[16] = {0};
        void *r = my_memcpy(dst, src, sizeof("hello world"));
        CHECK(r == dst);
        CHECK_STR_EQ(dst, "hello world");
    }

    /* my_memmove (overlapping, forward and backward) */
    {
        SECTION("my_memmove");
        char buf[16] = "abcdefgh";
        /* shift "abcdef" right by 2 -> dst overlaps src */
        my_memmove(buf + 2, buf, 6);
        buf[8] = '\0';
        CHECK_STR_EQ(buf, "ababcdef");

        char buf2[16] = "abcdefgh";
        /* shift left by 2 */
        my_memmove(buf2, buf2 + 2, 6);
        buf2[6] = '\0';
        CHECK_STR_EQ(buf2, "cdefgh");
    }

    /* rotate_left */
    {
        SECTION("rotate_left");
        int v[] = {1, 2, 3, 4, 5};
        rotate_left(v, 5, 2);
        int want[] = {3, 4, 5, 1, 2};
        for (int i = 0; i < 5; i++) {
            CHECK_INT_EQ(v[i], want[i]);
        }
        int w[] = {1, 2, 3, 4, 5};
        rotate_left(w, 5, 7); /* 7 % 5 == 2 */
        for (int i = 0; i < 5; i++) {
            CHECK_INT_EQ(w[i], want[i]);
        }
        int z[] = {1, 2, 3};
        rotate_left(z, 3, 3); /* full rotation -> unchanged */
        CHECK_INT_EQ(z[0], 1);
        CHECK_INT_EQ(z[1], 2);
        CHECK_INT_EQ(z[2], 3);
    }

    CTEST_END();
}
