#ifndef PROPERINTERVIEW_CSTRINGS_H
#define PROPERINTERVIEW_CSTRINGS_H

#include <stddef.h>

/*
 * cstrings — classic interview string work, in place and allocation-conscious.
 *
 * TARGET DRILL TIME: 20 minutes.
 *
 * Re-drill: `git checkout -- src/cstrings.c`, start the timer, implement
 * every function below, then `make test_cstrings`.
 */

/*
 * Reverse the ORDER OF WORDS in s, in place: "the quick fox" becomes
 * "fox quick the". Words are separated by exactly one space; there is no
 * leading or trailing space. The empty string has zero words and is left
 * unchanged; a string with no spaces is one word and is left unchanged.
 *
 * Returns the number of words.
 *
 * The classic in-place technique: reverse the whole string, then reverse
 * each word individually (or vice versa). No allocation allowed by spirit —
 * everything fits in O(1) extra space.
 */
size_t words_reverse(char *s);

/*
 * Parse s as a decimal integer: an optional single '+' or '-' followed by
 * one or more digits, and NOTHING else (no leading/trailing whitespace, no
 * trailing garbage).
 *
 * On success: write the value to *out and return 0.
 * Return -1 if s is empty, has no digits (e.g. "", "+", "abc"), or has
 * trailing garbage (e.g. "12x", "1 ").
 * Return -2 if the value does not fit in an int (overflow/underflow).
 *
 * *out is left UNTOUCHED on any error.
 *
 * The overflow check must happen BEFORE the multiply/add that would
 * overflow — signed overflow is undefined behavior and UBSan will flag it.
 * Remember the asymmetry: INT_MIN's magnitude is one larger than INT_MAX's,
 * so "-2147483648" is valid while "2147483648" is not (on 32-bit int).
 */
int my_atoi(const char *s, int *out);

/*
 * Count the words in s, where words are maximal runs of non-whitespace and
 * separators are ARBITRARY runs of whitespace (' ', '\t', '\n', ...).
 * Leading and trailing whitespace is allowed: "  a  b " has 2 words; ""
 * and "   " have 0.
 *
 * Use isspace() from <ctype.h> — and remember it must be fed an
 * unsigned char (passing a negative plain char is UB).
 */
size_t str_count_words(const char *s);

#endif /* PROPERINTERVIEW_CSTRINGS_H */
