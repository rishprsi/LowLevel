#ifndef PROPERMACHINE_FLOATS_H
#define PROPERMACHINE_FLOATS_H

#include <stdbool.h>
#include <stdint.h>

/*
 * This module mixes BOTH of this repo's mechanisms:
 *
 *  (a) PREDICT-THEN-VERIFY: the PRED_* constants below are defined in
 *      src/floats.c, each initialized with TODO_PREDICT — an identifier
 *      that is deliberately declared nowhere, so every unfilled prediction
 *      is a hard compile error. Fill in your predicted value FIRST; the
 *      tests compute the truth at runtime and diff it against you.
 *
 *  (b) implement-stubs: the functions below are TODO stubs in src/floats.c;
 *      implement them and the tests verify bit-level ground truths.
 *
 * Everything assumes IEEE-754 binary32 for float (true on this platform,
 * and on anything you're likely to touch).
 */

/* --- (a) predictions (1 = true, 0 = false, unless noted) --- */

extern const int PRED_POINT1_PLUS_POINT2_EQ_POINT3;  /* 0.1 + 0.2 == 0.3 ? */
extern const int PRED_HALF_PLUS_QUARTER_EQ_3QUARTERS; /* 0.5 + 0.25 == 0.75 ? */
extern const int PRED_FLOAT_16777217_EQ_16777216;    /* (float)16777217 == 16777216.0f ? */
extern const int PRED_TRUNC_NEG_1_9;                 /* (int)-1.9 (a value, not 1/0) */
extern const int PRED_NAN_EQ_NAN;                    /* NAN == NAN ? */
extern const int PRED_ONE_OVER_ZERO_IS_INF;          /* 1.0/0.0 == INFINITY ? (doubles) */

/* --- (b) implement --- */

/*
 * Return the raw IEEE-754 bit pattern of f as a uint32_t.
 * MUST be done with memcpy — casting &f to uint32_t* violates strict
 * aliasing (undefined behavior), and the compiler is allowed to punish you.
 */
uint32_t float_bits(float f);

/* Inverse of float_bits: reinterpret a bit pattern as a float (memcpy again). */
float bits_float(uint32_t bits);

/*
 * True iff f is a NaN — decided ONLY from the bit pattern (do NOT call
 * isnan()). Layout reminder: [1 sign][8 exponent][23 mantissa].
 * NaN = exponent bits all 1 AND mantissa nonzero (all-1 exponent with a
 * ZERO mantissa is infinity, not NaN).
 */
bool my_isnan(float f);

/*
 * The sign bit of f (0 or 1), read from the bits.
 * Note: must return 1 for -0.0f, even though -0.0f == 0.0f compares equal —
 * that's exactly why you can't implement this with `f < 0`.
 */
int float_sign_bit(float f);

/*
 * Distance between a and b in "units in the last place": the absolute
 * difference of their bit patterns.
 *
 * SIMPLIFIED CONTRACT: a and b are finite floats of the SAME sign. (For
 * same-sign finite floats, the bit patterns are ordered exactly like the
 * floats themselves, so bit-pattern distance == how many representable
 * floats apart they are.) Behavior for mixed signs / NaN / inf is
 * unspecified here.
 */
uint32_t ulp_diff(float a, float b);

#endif /* PROPERMACHINE_FLOATS_H */
