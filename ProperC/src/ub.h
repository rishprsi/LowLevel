#ifndef PROPERC_UB_H
#define PROPERC_UB_H

#include <stdbool.h>
#include <stdint.h>

/*
 * Avoiding undefined behavior. Each of these must compute its answer WITHOUT
 * ever performing the UB it guards against (signed overflow, out-of-range
 * shifts, misaligned/aliased loads). The tests run under UBSan, which will
 * abort if you trigger UB on the way to the "right" answer.
 */

/*
 * If a + b is representable in int32_t, store it in *out and return true.
 * Otherwise return false and leave *out unchanged. Must NOT compute a + b when
 * it would overflow.
 */
bool checked_add_i32(int32_t a, int32_t b, int32_t *out);

/* Same contract as checked_add_i32, for multiplication. */
bool checked_mul_i32(int32_t a, int32_t b, int32_t *out);

/*
 * Midpoint of a and b, truncated toward zero, computed without overflow
 * (so avg(INT32_MAX, INT32_MAX) == INT32_MAX, not UB).
 */
int32_t avg_no_overflow(int32_t a, int32_t b);

/*
 * x << k, but defined for all k: shifting by >= 32 yields 0 rather than UB.
 */
uint32_t safe_lshift(uint32_t x, unsigned k);

/*
 * Load a little-endian uint32 from the 4 bytes at p. p need not be aligned,
 * so do this with byte reads + shifts (not a uint32_t* cast).
 */
uint32_t load_u32_le(const uint8_t *p);

#endif /* PROPERC_UB_H */
