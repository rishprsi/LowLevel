#include "ctest.h"
#include "floats.h"

#include <math.h>

int main(void) {
    /* ---- (a) predictions vs. runtime ground truth ---- */
    /* volatile keeps every ground truth an honest runtime computation */

    volatile double d01 = 0.1, d02 = 0.2, d03 = 0.3;
    CHECK_INT_EQ(PRED_POINT1_PLUS_POINT2_EQ_POINT3, (d01 + d02 == d03) ? 1 : 0);

    volatile double half = 0.5, quarter = 0.25, threequarters = 0.75;
    CHECK_INT_EQ(PRED_HALF_PLUS_QUARTER_EQ_3QUARTERS,
                 (half + quarter == threequarters) ? 1 : 0);

    volatile int i17 = 16777217; /* 2^24 + 1 */
    volatile float f17 = (float)i17;
    CHECK_INT_EQ(PRED_FLOAT_16777217_EQ_16777216, (f17 == 16777216.0f) ? 1 : 0);

    volatile double neg19 = -1.9;
    CHECK_INT_EQ(PRED_TRUNC_NEG_1_9, (int)neg19);

    volatile float nan1 = NAN, nan2 = NAN;
    CHECK_INT_EQ(PRED_NAN_EQ_NAN, (nan1 == nan2) ? 1 : 0);

    volatile double one = 1.0, zero = 0.0;
    CHECK_INT_EQ(PRED_ONE_OVER_ZERO_IS_INF, (one / zero == INFINITY) ? 1 : 0);

    /* ---- (b) implementations vs. bit-level ground truth ---- */

    /* float_bits: known IEEE-754 patterns */
    CHECK_UINT_EQ(float_bits(1.0f), 0x3F800000u);
    CHECK_UINT_EQ(float_bits(2.0f), 0x40000000u);
    CHECK_UINT_EQ(float_bits(-1.0f), 0xBF800000u);
    CHECK_UINT_EQ(float_bits(0.0f), 0x00000000u);
    CHECK_UINT_EQ(float_bits(-0.0f), 0x80000000u);
    CHECK_UINT_EQ(float_bits(0.5f), 0x3F000000u);
    CHECK_UINT_EQ(float_bits(INFINITY), 0x7F800000u);

    /* bits_float: inverse direction + round trips */
    CHECK_TRUE(bits_float(0x3F800000u) == 1.0f);
    CHECK_TRUE(bits_float(0x40490FDBu) > 3.1415f &&
               bits_float(0x40490FDBu) < 3.1416f); /* closest float to pi */
    {
        static const uint32_t patterns[] = {
            0x00000000u, 0x80000000u, 0x3F800000u, 0x00000001u, /* min subnormal */
            0x7F7FFFFFu,                                        /* FLT_MAX */
        };
        for (size_t i = 0; i < sizeof patterns / sizeof patterns[0]; i++) {
            CHECK_UINT_EQ(float_bits(bits_float(patterns[i])), patterns[i]);
        }
    }

    /* my_isnan: from the bit pattern only */
    CHECK_TRUE(my_isnan(bits_float(0x7FC00000u)));  /* quiet NaN */
    CHECK_TRUE(my_isnan(bits_float(0x7F800001u)));  /* mantissa barely nonzero */
    CHECK_TRUE(my_isnan(bits_float(0xFFC00000u)));  /* negative NaN */
    CHECK_FALSE(my_isnan(bits_float(0x7F800000u))); /* +inf: mantissa == 0 */
    CHECK_FALSE(my_isnan(bits_float(0xFF800000u))); /* -inf */
    CHECK_FALSE(my_isnan(1.0f));
    CHECK_FALSE(my_isnan(0.0f));
    CHECK_TRUE(my_isnan(nan1));

    /* float_sign_bit: the -0.0f case is the whole point */
    CHECK_INT_EQ(float_sign_bit(0.0f), 0);
    CHECK_INT_EQ(float_sign_bit(-0.0f), 1);
    CHECK_INT_EQ(float_sign_bit(1.0f), 0);
    CHECK_INT_EQ(float_sign_bit(-123.5f), 1);
    CHECK_TRUE(-0.0f == 0.0f); /* ...even though they compare equal */

    /* ulp_diff (same-sign finite floats) */
    CHECK_UINT_EQ(ulp_diff(1.0f, 1.0f), 0u);
    CHECK_UINT_EQ(ulp_diff(1.0f, nextafterf(1.0f, 2.0f)), 1u);
    CHECK_UINT_EQ(ulp_diff(nextafterf(1.0f, 2.0f), 1.0f), 1u); /* symmetric */
    CHECK_UINT_EQ(ulp_diff(1.0f, 1.5f), 0x400000u); /* 2^22 floats apart */
    CHECK_UINT_EQ(ulp_diff(-1.0f, nextafterf(-1.0f, -2.0f)), 1u);
    /* 0.1f+0.2f vs 0.3f: unequal, but only barely */
    {
        volatile float a = 0.1f, b = 0.2f, c = 0.3f;
        float sum = a + b;
        CHECK_TRUE(ulp_diff(sum, c) <= 1u);
    }

    CTEST_END();
}
