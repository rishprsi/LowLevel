#include "structs.h"
#include <string.h>

bool is_little_endian(void) {
  uint32_t num = 1;
  char *first_byte = (char *)&num;
  return (*first_byte == 1);
}

void u32_to_bytes_le(uint32_t v, uint8_t out[4]) {
  memcpy(out, &v, sizeof(uint8_t) * 4);
}

uint32_t u32_from_bytes_le(const uint8_t in[4]) {
  (void)in;
  return (uint32_t)in[0];
}

void u32_to_bytes_be(uint32_t v, uint8_t out[4]) {
  (void)v;
  (void)out;
  return;
}

uint32_t u32_from_bytes_be(const uint8_t in[4]) {
  (void)in;
  return (uint32_t)in[0];
}

uint32_t pack_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  (void)r;
  (void)g;
  (void)b;
  (void)a;
  return (uint32_t)(r);
}

void unpack_rgba(uint32_t rgba, uint8_t *r, uint8_t *g, uint8_t *b,
                 uint8_t *a) {
  (void)rgba;
  (void)r;
  (void)g;
  (void)b;
  (void)a;
  return;
}
