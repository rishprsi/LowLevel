#include "ctest.h"
#include "promotions.h"

#include <limits.h>
#include <stddef.h>

/*
 * These tests DELIBERATELY perform the "suspicious" mixed-sign comparisons
 * and unevaluated side effects that the exercise is about, so we silence the
 * (otherwise very sensible) warnings for this file only. All of it is
 * well-defined C17.
 */
#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wsign-compare"
#endif
#if defined(__clang__)
#pragma clang diagnostic ignored "-Wunevaluated-expression"
#endif

int main(void) {
    /* volatile keeps every ground truth an honest runtime computation */

    SECTION("signed/unsigned comparison");
    volatile int neg1 = -1;
    volatile unsigned one_u = 1u;
    CHECK_INT_EQ(PRED_NEG1_LT_1U, (neg1 < one_u) ? 1 : 0);

    SECTION("sizeof char literal");
    CHECK_INT_EQ(PRED_SIZEOF_CHAR_LITERAL, (int)sizeof('a'));

    SECTION("unsigned wraparound");
    volatile unsigned zero_u = 0u;
    CHECK_INT_EQ(PRED_UNSIGNED_WRAP_IS_UINTMAX,
                 (zero_u - 1u == UINT_MAX) ? 1 : 0);

    SECTION("negative division/modulo");
    volatile int a = -7, b = 2;
    CHECK_INT_EQ(PRED_NEG7_DIV_2, a / b);
    CHECK_INT_EQ(PRED_NEG7_MOD_2, a % b);

    SECTION("narrowing conversion");
    volatile int big = 300;
    CHECK_INT_EQ(PRED_UCHAR_300, (unsigned char)big);

    SECTION("double-to-int truncation");
    volatile double d399 = 3.99;
    CHECK_INT_EQ(PRED_TRUNC_3_99, (int)d399);

    SECTION("integer promotion (short+short)");
    volatile short s1 = 1, s2 = 2;
    CHECK_INT_EQ(PRED_SIZEOF_SHORT_PLUS_SHORT, (int)sizeof(s1 + s2));

    SECTION("char arithmetic promotion");
    volatile char ch_a = 'A';
    CHECK_INT_EQ(PRED_A_PLUS_1, ch_a + 1);

    SECTION("unsigned size_t comparison");
    CHECK_INT_EQ(PRED_SIZEOF_MINUS_5_GT_0, (sizeof(int) - 5 > 0) ? 1 : 0);

    SECTION("shift and promotion");
    volatile unsigned uone = 1u;
    CHECK_INT_EQ(PRED_SHL31_AS_LL_IS_2P31,
                 ((long long)(uone << 31) == 2147483648LL) ? 1 : 0);

    SECTION("signed/unsigned arithmetic");
    CHECK_INT_EQ(PRED_NEG1_PLUS_1U_IS_ZERO, (neg1 + one_u == 0u) ? 1 : 0);

    SECTION("digit char subtraction");
    volatile char c7 = '7', c0 = '0';
    CHECK_INT_EQ(PRED_DIGIT_7_MINUS_0, c7 - c0);

    SECTION("sizeof has no side effect");
    volatile int i = 5;
    size_t s = sizeof(i++); /* the whole point: i++ is NOT evaluated */
    (void)s;
    CHECK_INT_EQ(PRED_SIZEOF_NO_SIDE_EFFECT, i);

    CTEST_END();
}
