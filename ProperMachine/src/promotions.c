#include "promotions.h"

/*
 * PREDICT-THEN-VERIFY: replace every TODO_PREDICT with the integer you
 * believe the expression evaluates to, BEFORE building. TODO_PREDICT is
 * intentionally never declared anywhere, so each unfilled prediction is a
 * hard compile error. The tests compute the truth at runtime and diff it
 * against you. (All expressions are well-defined C17 — nothing here is UB.)
 */

/* Is `-1 < 1U` true (1) or false (0)?
 * Think: when an int meets an unsigned int in a comparison, which operand
 * gets converted, and what does -1 become? */
const int PRED_NEG1_LT_1U = TODO_PREDICT;

/* What is `sizeof('a')`?
 * Think: in C (unlike C++), what is the TYPE of a character literal? */
const int PRED_SIZEOF_CHAR_LITERAL = TODO_PREDICT;

/* Is `0u - 1u == UINT_MAX` true (1) or false (0)?
 * Think: what does the standard guarantee about unsigned arithmetic that
 * "goes below zero"? */
const int PRED_UNSIGNED_WRAP_IS_UINTMAX = TODO_PREDICT;

/* What is `-7 / 2`?
 * Think: since C99, integer division rounds in which direction? */
const int PRED_NEG7_DIV_2 = TODO_PREDICT;

/* What is `-7 % 2`?
 * Think: the identity (a/b)*b + a%b == a must hold. Given your previous
 * answer, what must the remainder be? */
const int PRED_NEG7_MOD_2 = TODO_PREDICT;

/* What is `(unsigned char)300`?
 * Think: conversion to an unsigned type is defined as reduction modulo
 * (MAX+1). 300 mod 256 = ? */
const int PRED_UCHAR_300 = TODO_PREDICT;

/* What is `(int)3.99`?
 * Think: float -> int conversion doesn't round. What does it do instead? */
const int PRED_TRUNC_3_99 = TODO_PREDICT;

/* Given `short s1, s2;`, what is `sizeof(s1 + s2)`?
 * Think: does the addition happen in short? What does integer promotion do
 * to anything smaller than int before arithmetic? */
const int PRED_SIZEOF_SHORT_PLUS_SHORT = TODO_PREDICT;

/* What is `'A' + 1`?
 * Think: 'A' is 65 in ASCII, and the arithmetic happens as int. */
const int PRED_A_PLUS_1 = TODO_PREDICT;

/* Is `sizeof(int) - 5 > 0` true (1) or false (0)?
 * Think: sizeof yields a size_t (unsigned!). sizeof(int) is 4 here.
 * What is 4 - 5 in unsigned arithmetic? */
const int PRED_SIZEOF_MINUS_5_GT_0 = TODO_PREDICT;

/* Is `(long long)(1u << 31) == 2147483648LL` true (1) or false (0)?
 * Think: 1u << 31 is a perfectly legal UNSIGNED shift (0x80000000u). Does
 * converting that unsigned value to long long keep its value, or does
 * something sign-related happen? */
const int PRED_SHL31_AS_LL_IS_2P31 = TODO_PREDICT;

/* Is `-1 + 1u == 0u` true (1) or false (0)?
 * Think: -1 converts to unsigned first (becoming what?), then the addition
 * wraps modulo 2^32. */
const int PRED_NEG1_PLUS_1U_IS_ZERO = TODO_PREDICT;

/* What is `'7' - '0'`?
 * Think: the standard guarantees the digits '0'..'9' are contiguous. This
 * is THE classic char-digit-to-int idiom. */
const int PRED_DIGIT_7_MINUS_0 = TODO_PREDICT;

/* After `int i = 5; size_t s = sizeof(i++);` what is the value of i?
 * Think: is the operand of sizeof evaluated at all (for a non-VLA type)? */
const int PRED_SIZEOF_NO_SIDE_EFFECT = TODO_PREDICT;
