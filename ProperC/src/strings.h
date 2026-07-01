#ifndef PROPERC_STRINGS_H
#define PROPERC_STRINGS_H

#include <stdbool.h>
#include <stddef.h>

/* Length of NUL-terminated string s, not counting the terminator. */
size_t my_strlen(const char *s);

/* Copy src (including its NUL) into dst. Returns dst. dst must be big enough. */
char *my_strcpy(char *dst, const char *src);

/*
 * strncpy semantics (note the sharp edges):
 *  - copy at most n bytes from src into dst;
 *  - if src is shorter than n, the remainder of dst[0..n) is filled with '\0';
 *  - if src has length >= n, dst is NOT NUL-terminated.
 * Returns dst.
 */
char *my_strncpy(char *dst, const char *src, size_t n);

/*
 * Compare a and b. Return a negative / zero / positive value when a is
 * less than / equal to / greater than b, comparing as unsigned char.
 */
int my_strcmp(const char *a, const char *b);

/* Append src onto the end of dst (in place). Returns dst. */
char *my_strcat(char *dst, const char *src);

/*
 * Return a pointer to the first occurrence of (char)c in s, or NULL.
 * The terminating '\0' is considered part of the string (strchr(s,0) finds it).
 */
char *my_strchr(const char *s, int c);

/*
 * Return a pointer to the first occurrence of substring needle in haystack,
 * or NULL. An empty needle returns haystack.
 */
char *my_strstr(const char *haystack, const char *needle);

/* Reverse s in place. */
void str_reverse(char *s);

/* True if s reads the same forwards and backwards (empty string is true). */
bool is_palindrome(const char *s);

#endif /* PROPERC_STRINGS_H */
