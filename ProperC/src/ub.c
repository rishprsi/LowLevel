#include "ub.h"
#include <stdint.h>

bool checked_add_i32(int32_t a, int32_t b, int32_t *out) {
  if (b > 0 && a > INT32_MAX - b) {
    return false;
  }
  if (b < 0 && a < INT32_MIN - b) {
    return false;
  }
  *out = a + b;
  return true;
}

bool checked_mul_i32(int32_t a, int32_t b, int32_t *out) {
  int64_t target = (int64_t)a * (int64_t)b;
  if (target < INT32_MIN || target > INT32_MAX) {
    return false;
  }
  *out = (int32_t)target;
  return true;
}

int32_t avg_no_overflow(int32_t a, int32_t b) {
  return (int32_t)(((int64_t)a + (int64_t)b) / 2);
}

uint32_t safe_lshift(uint32_t x, unsigned k) {
  if (k >= 32) {
    return 0;
  } else {
    return x << k;
  }
}

uint32_t load_u32_le(const uint8_t *p) {
  uint32_t x = (uint32_t)p[0] << 0 | (uint32_t)p[1] << 8 |
               (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
  return x;
}
