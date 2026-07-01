#ifndef PROPERC_STRUCTS_H
#define PROPERC_STRUCTS_H

#include <stdbool.h>
#include <stdint.h>

/*
 * Data representation: how multi-byte values map to bytes, and byte order.
 * (Implement these WITHOUT type-punning through incompatible pointers — use
 * shifts/masks or memcpy, never `*(uint32_t*)bytes`, to stay clear of
 * alignment and strict-aliasing UB.)
 */

/* True iff this machine stores the least-significant byte of a uint32 first. */
bool is_little_endian(void);

/* Serialize v into out[0..3] in little-endian order (out[0] = least sig). */
void u32_to_bytes_le(uint32_t v, uint8_t out[4]);
/* Deserialize a little-endian uint32 from in[0..3]. */
uint32_t u32_from_bytes_le(const uint8_t in[4]);

/* Serialize v into out[0..3] in big-endian order (out[0] = most sig). */
void u32_to_bytes_be(uint32_t v, uint8_t out[4]);
/* Deserialize a big-endian uint32 from in[0..3]. */
uint32_t u32_from_bytes_be(const uint8_t in[4]);

/* Pack four channels into 0xRRGGBBAA (r is the most-significant byte). */
uint32_t pack_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
/* Inverse of pack_rgba. Any of the out pointers may be non-NULL. */
void unpack_rgba(uint32_t rgba, uint8_t *r, uint8_t *g, uint8_t *b, uint8_t *a);

#endif /* PROPERC_STRUCTS_H */
