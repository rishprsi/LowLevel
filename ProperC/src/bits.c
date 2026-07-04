#include "bits.h"

int count_set_bits(uint32_t x) {
  int count = 0;
  while (x) {
    x &= x - 1;
    count++;
  }
  return count;
}

bool is_power_of_two(uint32_t x) {
  return x != 0 && ((x & x - 1) == 0);
  /* int count = count_set_bits((uint32_t)x); */
  /* return (count == 1); */
}

uint32_t reverse_bits(uint32_t x) {
  uint32_t new = 0;
  for (int i = 0; i < 31; i++) {
    new |= (x & 1);
    x >>= 1;
    new <<= 1;
  }
  new |= (x & 1);
  return new;
}

uint32_t set_bit(uint32_t x, int n) {
  x |= 1U << n;
  return x;
}

uint32_t clear_bit(uint32_t x, int n) { return x & ~(1U << n); }

uint32_t toggle_bit(uint32_t x, int n) { return x ^ (1U << n); }

int count_trailing_zeros(uint32_t x) {
  if (x == 0) {
    return 32;
  }
  int count = 0;
  while (x % 2 == 0) {
    count++;
    x = x >> 1;
  }
  return count;
}
