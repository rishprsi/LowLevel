#ifndef PROPERC_BITS_H
#define PROPERC_BITS_H

#include <stdbool.h>
#include <stdint.h>

/* Number of bits set to 1 in x. */
int count_set_bits(uint32_t x);

/* True iff x is a power of two (x > 0 with exactly one bit set; 0 is false). */
bool is_power_of_two(uint32_t x);

/* Return x with its 32 bits reversed (bit 0 <-> bit 31, etc.). */
uint32_t reverse_bits(uint32_t x);

/* Return x with bit n set.    Precondition: 0 <= n <= 31. */
uint32_t set_bit(uint32_t x, int n);
/* Return x with bit n cleared. Precondition: 0 <= n <= 31. */
uint32_t clear_bit(uint32_t x, int n);
/* Return x with bit n toggled. Precondition: 0 <= n <= 31. */
uint32_t toggle_bit(uint32_t x, int n);

/* Number of trailing zero bits (from the LSB). Defined as 32 when x == 0. */
int count_trailing_zeros(uint32_t x);

#endif /* PROPERC_BITS_H */
