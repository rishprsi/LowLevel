#ifndef PROPERINTERVIEW_STRFNS_H
#define PROPERINTERVIEW_STRFNS_H

#include <stddef.h>

/*
 * strfns — libc string basics from scratch.
 *
 * TARGET DRILL TIME: 15 minutes.
 *
 * Re-drill: `git checkout -- src/strfns.c`, start the timer, implement
 * every function below, then `make test_strfns`.
 */

/* Length of NUL-terminated string s, excluding the terminator. */
size_t my_strlen(const char *s);

/*
 * Copy src into dst including the NUL terminator. The caller guarantees dst
 * has room. Regions must not overlap. Returns dst.
 */
char *my_strcpy(char *dst, const char *src);

/*
 * Lexicographic comparison: negative if a < b, zero if equal, positive if
 * a > b. Bytes must be compared as unsigned char (bytes >= 0x80 sort AFTER
 * plain ASCII, even on platforms where char is signed).
 */
int my_strcmp(const char *a, const char *b);

/*
 * Copy exactly n bytes from src to dst. The regions must NOT overlap — that
 * is the caller's promise, not something to detect (this is memcpy, not
 * memmove). Copy through unsigned char *. Returns dst.
 */
void *my_memcpy(void *dst, const void *src, size_t n);

/*
 * Append src (including its NUL) at the end of NUL-terminated dst. The
 * caller guarantees dst has room for the combined string. Returns dst.
 */
char *my_strcat(char *dst, const char *src);

#endif /* PROPERINTERVIEW_STRFNS_H */
