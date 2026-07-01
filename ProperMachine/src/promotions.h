#ifndef PROPERMACHINE_PROMOTIONS_H
#define PROPERMACHINE_PROMOTIONS_H

/*
 * PREDICT-THEN-VERIFY
 * -------------------
 * This module has nothing to implement. Instead, src/promotions.c defines a
 * list of `const int PRED_...` values, each initialized with TODO_PREDICT —
 * an identifier that is deliberately declared NOWHERE. Every prediction you
 * haven't filled in is therefore a hard compile error: you cannot "just run
 * the tests to see the answer". Replace each TODO_PREDICT with the integer
 * you believe the expression evaluates to, THEN build. The tests compute the
 * ground truth at runtime and diff it against your prediction.
 *
 * Every expression below is 100% well-defined C17 on this platform — no
 * undefined behavior, no traps. The point is the *rules*: integer promotion,
 * the usual arithmetic conversions, truncation, and unsigned wraparound.
 *
 * The question for each prediction lives as a comment in src/promotions.c,
 * right where you type the answer.
 */

extern const int PRED_NEG1_LT_1U;             /* is -1 < 1U ? (1/0) */
extern const int PRED_SIZEOF_CHAR_LITERAL;    /* sizeof('a') */
extern const int PRED_UNSIGNED_WRAP_IS_UINTMAX; /* 0u - 1u == UINT_MAX ? (1/0) */
extern const int PRED_NEG7_DIV_2;             /* -7 / 2 */
extern const int PRED_NEG7_MOD_2;             /* -7 % 2 */
extern const int PRED_UCHAR_300;              /* (unsigned char)300 */
extern const int PRED_TRUNC_3_99;             /* (int)3.99 */
extern const int PRED_SIZEOF_SHORT_PLUS_SHORT; /* sizeof(short + short) */
extern const int PRED_A_PLUS_1;               /* 'A' + 1 */
extern const int PRED_SIZEOF_MINUS_5_GT_0;    /* sizeof(int) - 5 > 0 ? (1/0) */
extern const int PRED_SHL31_AS_LL_IS_2P31;    /* (long long)(1u<<31) == 2147483648LL ? */
extern const int PRED_NEG1_PLUS_1U_IS_ZERO;   /* -1 + 1u == 0u ? (1/0) */
extern const int PRED_DIGIT_7_MINUS_0;        /* '7' - '0' */
extern const int PRED_SIZEOF_NO_SIDE_EFFECT;  /* i after sizeof(i++), i started at 5 */

#endif /* PROPERMACHINE_PROMOTIONS_H */
