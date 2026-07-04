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
  int count = count_set_bits((uint32_t)x);
  return (count == 1);
}

uint32_t reverse_bits(uint32_t x) {
  (void)x;
  return (uint32_t)5;
}

uint32_t set_bit(uint32_t x, int n) {
  (void)x;
  (void)n;
  return (uint32_t)5;
}

uint32_t clear_bit(uint32_t x, int n) {
  (void)x;
  (void)n;
  return (uint32_t)5;
}

uint32_t toggle_bit(uint32_t x, int n) {
  (void)x;
  (void)n;
  return (uint32_t)5;
}

int count_trailing_zeros(uint32_t x) {
  (void)x;
  return (uint32_t)5;
}
