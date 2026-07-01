#ifndef PROPERC_POINTERS_H
#define PROPERC_POINTERS_H

#include <stddef.h>

/* Swap the two ints pointed to by a and b. */
void swap_int(int *a, int *b);

/* Reverse arr[0..n) in place. n == 0 is a no-op. */
void reverse_array(int *arr, size_t n);

/*
 * Copy n bytes from src to dst and return dst.
 * The regions must NOT overlap (like memcpy).
 */
void *my_memcpy(void *dst, const void *src, size_t n);

/*
 * Copy n bytes from src to dst and return dst.
 * The regions MAY overlap (like memmove) — the result must be as if src was
 * first copied to a temporary buffer.
 */
void *my_memmove(void *dst, const void *src, size_t n);

/*
 * Rotate arr[0..n) left by k positions, in place.
 * k may be >= n (rotate by k % n). n == 0 is a no-op.
 * Example: [1,2,3,4,5] rotate_left k=2 -> [3,4,5,1,2]
 */
void rotate_left(int *arr, size_t n, size_t k);

#endif /* PROPERC_POINTERS_H */
