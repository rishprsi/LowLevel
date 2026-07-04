#include "structs.h"
#include <stdio.h>
#include <string.h>

bool is_little_endian(void) {
  uint32_t num = 1;
  char *first_byte = (char *)&num;
  return (*first_byte == 1);
}

void u32_to_bytes_le(uint32_t v, uint8_t out[4]) {
  for (size_t i = 0; i < 4; i++) {
    out[i] = (uint8_t)((v >> (8 * i)) & 0xFF);
    /* v >>= 8; */
  }
}

uint32_t u32_from_bytes_le(const uint8_t in[4]) {
  uint32_t new = 0;
  for (size_t i = 0; i < 4; i++) {
    new |= (uint32_t)in[i] << (8 * i);
  }
  return new;
}

void u32_to_bytes_be(uint32_t v, uint8_t out[4]) {
  for (size_t i = 0; i < 4; i++) {
    out[3 - i] = (uint8_t)((v >> (8 * i)) & 0xFF);
  }
}

uint32_t u32_from_bytes_be(const uint8_t in[4]) {
  uint32_t new = in[0];
  for (size_t i = 1; i < 4; i++) {
    new <<= 8;
    new |= in[i];
  }
  return new;
}

uint32_t pack_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  uint32_t rgba = (uint32_t)(r << 24) | (uint32_t)(g << 16) |
                  (uint32_t)(b << 8) | (uint32_t)a;
  return rgba;
}

void unpack_rgba(uint32_t rgba, uint8_t *r, uint8_t *g, uint8_t *b,
                 uint8_t *a) {
  if (r) {
    *r = (uint8_t)(rgba >> 24);
  }
  if (g) {
    *g = (uint8_t)((rgba >> 16) & 0xFF);
  }
  if (b) {
    *b = (uint8_t)((rgba >> 8) & 0xFF);
  }
  if (a) {
    *a = (uint8_t)(rgba & 0xFF);
  }
}
