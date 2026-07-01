#include "layout.h"

/*
 * PREDICT-THEN-VERIFY: replace every TODO_PREDICT with the byte count you
 * predict, BEFORE building (see layout.h for the two layout rules and the
 * platform's sizes/alignments). Each struct's definition in layout.h carries
 * a comment pointing at what to think through.
 */

/* { char c; int i; } */
const int PRED_SIZEOF_CHAR_INT = TODO_PREDICT;
const int PRED_OFFSETOF_CHAR_INT_I = TODO_PREDICT;

/* { char a; int b; char c; } — the ordering blunder */
const int PRED_SIZEOF_CHAR_INT_CHAR = TODO_PREDICT;

/* { int b; char a; char c; } — same members, biggest first */
const int PRED_SIZEOF_INT_CHAR_CHAR = TODO_PREDICT;

/* { char c; double d; } */
const int PRED_SIZEOF_CHAR_DOUBLE = TODO_PREDICT;
const int PRED_OFFSETOF_CHAR_DOUBLE_D = TODO_PREDICT;

/* { char c; LCharInt inner; } — a nested struct aligns like its strictest
 * member, not like its size */
const int PRED_SIZEOF_NESTED = TODO_PREDICT;
const int PRED_OFFSETOF_NESTED_INNER = TODO_PREDICT;

/* { char c; } */
const int PRED_SIZEOF_JUST_CHAR = TODO_PREDICT;

/* { long long ll; char c; } — count the tail padding */
const int PRED_SIZEOF_LONGLONG_CHAR = TODO_PREDICT;

/* { char a[3]; short s; } */
const int PRED_SIZEOF_CHARS_SHORT = TODO_PREDICT;
const int PRED_OFFSETOF_CHARS_SHORT_S = TODO_PREDICT;

/* LCharInt arr[2]: the distance from &arr[0] to &arr[1] in bytes.
 * (This is exactly why structs have tail padding.) */
const int PRED_ARRAY2_STRIDE_BYTES = TODO_PREDICT;
