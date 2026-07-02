#include "ctest.h"
#include "ub.h"

int main(void) {
    /* checked_add_i32 */
    {
        SECTION("checked_add_i32");
        int32_t out = 999;
        CHECK_TRUE(checked_add_i32(1, 2, &out));
        CHECK_INT_EQ(out, 3);

        CHECK_TRUE(checked_add_i32(INT32_MAX, INT32_MIN, &out));
        CHECK_INT_EQ(out, -1);

        out = 777;
        CHECK_FALSE(checked_add_i32(INT32_MAX, 1, &out));
        CHECK_INT_EQ(out, 777); /* unchanged on overflow */

        CHECK_FALSE(checked_add_i32(INT32_MIN, -1, &out));
        CHECK_INT_EQ(out, 777);
    }

    /* checked_mul_i32 */
    {
        SECTION("checked_mul_i32");
        int32_t out = 0;
        CHECK_TRUE(checked_mul_i32(3, 4, &out));
        CHECK_INT_EQ(out, 12);

        CHECK_TRUE(checked_mul_i32(-2, 3, &out));
        CHECK_INT_EQ(out, -6);

        out = 555;
        CHECK_FALSE(checked_mul_i32(INT32_MAX, 2, &out));
        CHECK_INT_EQ(out, 555);
        CHECK_FALSE(checked_mul_i32(INT32_MIN, -1, &out)); /* +2^31 overflows */
        CHECK_FALSE(checked_mul_i32(100000, 100000, &out));
    }

    SECTION("avg_no_overflow");
    CHECK_INT_EQ(avg_no_overflow(2, 4), 3);
    CHECK_INT_EQ(avg_no_overflow(-4, -2), -3);
    CHECK_INT_EQ(avg_no_overflow(INT32_MAX, INT32_MAX), INT32_MAX);
    CHECK_INT_EQ(avg_no_overflow(INT32_MIN, INT32_MIN), INT32_MIN);
    CHECK_INT_EQ(avg_no_overflow(INT32_MAX, INT32_MIN), 0);

    SECTION("safe_lshift");
    CHECK_UINT_EQ(safe_lshift(1u, 0), 1u);
    CHECK_UINT_EQ(safe_lshift(1u, 31), 0x80000000u);
    CHECK_UINT_EQ(safe_lshift(1u, 32), 0u);   /* would be UB if done directly */
    CHECK_UINT_EQ(safe_lshift(5u, 40), 0u);
    CHECK_UINT_EQ(safe_lshift(0xFFu, 4), 0xFF0u);

    /* load_u32_le */
    {
        SECTION("load_u32_le");
        uint8_t b1[4] = {0x04, 0x03, 0x02, 0x01};
        CHECK_UINT_EQ(load_u32_le(b1), 0x01020304u);
        uint8_t b2[4] = {0xFF, 0xFF, 0xFF, 0xFF};
        CHECK_UINT_EQ(load_u32_le(b2), 0xFFFFFFFFu);
        /* unaligned: load from offset 1 of a 5-byte buffer */
        uint8_t b3[5] = {0x00, 0x04, 0x03, 0x02, 0x01};
        CHECK_UINT_EQ(load_u32_le(b3 + 1), 0x01020304u);
    }

    CTEST_END();
}
