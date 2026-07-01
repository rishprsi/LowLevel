#ifndef PROPERMACHINE_LAYOUT_H
#define PROPERMACHINE_LAYOUT_H

/*
 * PREDICT-THEN-VERIFY (see promotions.h for the mechanism): predict sizeof
 * and offsetof for the structs below, on THIS platform (arm64 macOS — the
 * AAPCS64 ABI, which is refreshingly predictable):
 *
 *   char = 1 byte, short = 2, int = 4, long long = 8, double = 8,
 *   and every type's alignment equals its size.
 *
 * The two rules that generate every answer:
 *   1. Each member is placed at the next offset that is a multiple of its
 *      alignment (inserting padding before it if needed).
 *   2. The struct's total size is rounded up to a multiple of the strictest
 *      member alignment (inserting tail padding), so that arrays of the
 *      struct keep every element aligned.
 *
 * Fill in the PRED_* values in src/layout.c (each starts as TODO_PREDICT, a
 * hard compile error). The tests diff them against the real sizeof/offsetof.
 */

/* char at 0, then int needs 4-alignment: where does i land, and how big is
 * the whole thing? */
typedef struct {
    char c;
    int i;
} LCharInt;

/* The classic ordering blunder: char, int, char. Count both padding holes. */
typedef struct {
    char a;
    int b;
    char c;
} LCharIntChar;

/* Same three members, reordered biggest-first. How much does it save? */
typedef struct {
    int b;
    char a;
    char c;
} LIntCharChar;

/* double demands 8-alignment — both inside the struct and for its size. */
typedef struct {
    char c;
    double d;
} LCharDouble;

/* A nested struct's alignment is its strictest MEMBER's alignment (4 here),
 * not its size. Where does inner start? */
typedef struct {
    char c;
    LCharInt inner;
} LNested;

/* No padding needed anywhere. The minimal struct. */
typedef struct {
    char c;
} LJustChar;

/* long long forces 8-alignment; the lone trailing char costs 7 tail bytes. */
typedef struct {
    long long ll;
    char c;
} LLongLongChar;

/* char[3] occupies bytes 0..2; short needs 2-alignment. Where does s go,
 * and what's the total? */
typedef struct {
    char a[3];
    short s;
} LCharsShort;

extern const int PRED_SIZEOF_CHAR_INT;        /* sizeof(LCharInt) */
extern const int PRED_OFFSETOF_CHAR_INT_I;    /* offsetof(LCharInt, i) */
extern const int PRED_SIZEOF_CHAR_INT_CHAR;   /* sizeof(LCharIntChar) */
extern const int PRED_SIZEOF_INT_CHAR_CHAR;   /* sizeof(LIntCharChar) */
extern const int PRED_SIZEOF_CHAR_DOUBLE;     /* sizeof(LCharDouble) */
extern const int PRED_OFFSETOF_CHAR_DOUBLE_D; /* offsetof(LCharDouble, d) */
extern const int PRED_SIZEOF_NESTED;          /* sizeof(LNested) */
extern const int PRED_OFFSETOF_NESTED_INNER;  /* offsetof(LNested, inner) */
extern const int PRED_SIZEOF_JUST_CHAR;       /* sizeof(LJustChar) */
extern const int PRED_SIZEOF_LONGLONG_CHAR;   /* sizeof(LLongLongChar) */
extern const int PRED_SIZEOF_CHARS_SHORT;     /* sizeof(LCharsShort) */
extern const int PRED_OFFSETOF_CHARS_SHORT_S; /* offsetof(LCharsShort, s) */
extern const int PRED_ARRAY2_STRIDE_BYTES;    /* &arr[1] - &arr[0] in bytes,
                                                 for LCharInt arr[2] */

#endif /* PROPERMACHINE_LAYOUT_H */
