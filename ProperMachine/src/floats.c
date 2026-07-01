#include "floats.h"
#include "todo.h"

/* ---- (a) PREDICT-THEN-VERIFY: replace each TODO_PREDICT with your
 *      predicted value BEFORE building (see the note in floats.h). ---- */

/* Is `0.1 + 0.2 == 0.3` true (1) or false (0)?
 * Think: can 0.1, 0.2 or 0.3 be represented exactly in binary at all? */
const int PRED_POINT1_PLUS_POINT2_EQ_POINT3 = TODO_PREDICT;

/* Is `0.5 + 0.25 == 0.75` true (1) or false (0)?
 * Think: which decimal fractions ARE exact in binary? */
const int PRED_HALF_PLUS_QUARTER_EQ_3QUARTERS = TODO_PREDICT;

/* Is `(float)16777217 == 16777216.0f` true (1) or false (0)?
 * Think: a float has a 24-bit significand. 16777216 = 2^24. What happens to
 * the very next integer? */
const int PRED_FLOAT_16777217_EQ_16777216 = TODO_PREDICT;

/* What is `(int)-1.9`?
 * Think: float -> int truncates toward zero, also for negatives. */
const int PRED_TRUNC_NEG_1_9 = TODO_PREDICT;

/* Is `NAN == NAN` true (1) or false (0)?
 * Think: NaN is defined to compare unequal to EVERYTHING. Everything. */
const int PRED_NAN_EQ_NAN = TODO_PREDICT;

/* With doubles, is `1.0 / 0.0 == INFINITY` true (1) or false (0)?
 * Think: IEC 60559 defines x/0 for x > 0 (this is not the integer case!). */
const int PRED_ONE_OVER_ZERO_IS_INF = TODO_PREDICT;

/* ---- (b) implement-stubs ---- */

uint32_t float_bits(float f) {
    (void)f;
    TODO("implement float_bits");
}

float bits_float(uint32_t bits) {
    (void)bits;
    TODO("implement bits_float");
}

bool my_isnan(float f) {
    (void)f;
    TODO("implement my_isnan");
}

int float_sign_bit(float f) {
    (void)f;
    TODO("implement float_sign_bit");
}

uint32_t ulp_diff(float a, float b) {
    (void)a;
    (void)b;
    TODO("implement ulp_diff");
}
