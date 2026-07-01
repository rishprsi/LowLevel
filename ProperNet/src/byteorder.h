#ifndef PROPERNET_BYTEORDER_H
#define PROPERNET_BYTEORDER_H

#include <stdint.h>

/*
 * Reimplement the classic byte-order conversion quartet WITHOUT any
 * endianness #ifdefs. Your code must be correct on ANY host, big- or
 * little-endian — so byte-swapping unconditionally is wrong, and so is
 * returning x unchanged.
 *
 * Hint: you never need to know the host's endianness. You can always
 * construct the big-endian byte sequence explicitly (shift the value into
 * individual bytes: the most significant byte goes first on the wire), then
 * reinterpret those bytes as an integer with memcpy. The reverse direction
 * reads the bytes back out and reassembles the value with shifts.
 *
 * The tests compare your functions against the real htons/htonl family on
 * many values, and additionally verify the byte layout in memory.
 */

/* Host-order x -> network-order (big-endian) 16-bit value. */
uint16_t my_htons(uint16_t x);

/* Network-order (big-endian) x -> host-order 16-bit value. */
uint16_t my_ntohs(uint16_t x);

/* Host-order x -> network-order (big-endian) 32-bit value. */
uint32_t my_htonl(uint32_t x);

/* Network-order (big-endian) x -> host-order 32-bit value. */
uint32_t my_ntohl(uint32_t x);

#endif /* PROPERNET_BYTEORDER_H */
