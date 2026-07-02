#include "ctest.h"
#include "funcptr.h"

static int dbl(int x) { return x * 2; }
static bool is_even(int x) { return x % 2 == 0; }
static int add(int a, int x) { return a + x; }
static int mul(int a, int x) { return a * x; }
static int asc(int a, int b) { return (a > b) - (a < b); }
static int desc(int a, int b) { return (b > a) - (b < a); }

int main(void) {
    /* int_map */
    {
        SECTION("int_map");
        int v[] = {1, 2, 3};
        int_map(v, 3, dbl);
        CHECK_INT_EQ(v[0], 2);
        CHECK_INT_EQ(v[1], 4);
        CHECK_INT_EQ(v[2], 6);
    }

    /* int_filter */
    {
        SECTION("int_filter");
        int src[] = {1, 2, 3, 4, 5, 6};
        int dst[6];
        size_t k = int_filter(src, 6, dst, is_even);
        CHECK_INT_EQ(k, 3);
        CHECK_INT_EQ(dst[0], 2);
        CHECK_INT_EQ(dst[1], 4);
        CHECK_INT_EQ(dst[2], 6);
    }

    /* int_reduce */
    {
        SECTION("int_reduce");
        int v[] = {1, 2, 3, 4};
        CHECK_INT_EQ(int_reduce(v, 4, 0, add), 10);
        CHECK_INT_EQ(int_reduce(v, 4, 1, mul), 24);
        CHECK_INT_EQ(int_reduce(v, 0, 42, add), 42); /* empty -> init */
    }

    /* sort_ints */
    {
        SECTION("sort_ints");
        int v[] = {3, 1, 2, 5, 4};
        sort_ints(v, 5, asc);
        for (int i = 0; i < 5; i++) {
            CHECK_INT_EQ(v[i], i + 1);
        }
        sort_ints(v, 5, desc);
        for (int i = 0; i < 5; i++) {
            CHECK_INT_EQ(v[i], 5 - i);
        }
    }

    CTEST_END();
}
