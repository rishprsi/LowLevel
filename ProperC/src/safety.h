#ifndef PROPERC_SAFETY_H
#define PROPERC_SAFETY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * Bounded string operations that must NEVER write past the destination buffer.
 * (Run under AddressSanitizer — any overflow will be caught immediately.)
 */

/*
 * Like OpenBSD strlcpy. Copy src into dst, writing at most dstsize bytes total
 * (including the NUL). dst is always NUL-terminated when dstsize > 0. When
 * dstsize == 0, nothing is written.
 * Returns strlen(src) — the length the function tried to create — so a return
 * value >= dstsize means truncation occurred.
 */
size_t safe_strcpy(char *dst, size_t dstsize, const char *src);

/*
 * Like OpenBSD strlcat. Append src to the NUL-terminated string in dst,
 * writing at most dstsize bytes total. dst stays NUL-terminated.
 * Returns the length it tried to create: min(dstsize, strlen(dst)) + strlen(src).
 */
size_t safe_strcat(char *dst, size_t dstsize, const char *src);

/*
 * Copy n bytes from src to dst only if they fit (n <= dstcap). Returns true if
 * the copy happened, false (and no write) if it would overflow.
 */
bool copy_n_safe(uint8_t *dst, size_t dstcap, const uint8_t *src, size_t n);

#endif /* PROPERC_SAFETY_H */
