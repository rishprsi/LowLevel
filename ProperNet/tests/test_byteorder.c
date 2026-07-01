#include "ctest.h"
#include "byteorder.h"

#include <arpa/inet.h>
#include <string.h>

int main(void) {
    static const uint16_t vals16[] = {
        0x0000u, 0x0001u, 0x00FFu, 0xFF00u, 0x1234u, 0x8000u, 0xABCDu, 0xFFFFu,
    };
    static const uint32_t vals32[] = {
        0x00000000u, 0x00000001u, 0x000000FFu, 0xFF000000u, 0x12345678u,
        0x80000000u, 0xDEADBEEFu, 0xFFFFFFFFu, 0x0000FFFFu, 0xFFFF0000u,
    };

    /* agree with the real thing on many values, and round-trip */
    for (size_t i = 0; i < sizeof vals16 / sizeof vals16[0]; i++) {
        uint16_t v = vals16[i];
        CHECK_UINT_EQ(my_htons(v), htons(v));
        CHECK_UINT_EQ(my_ntohs(v), ntohs(v));
        CHECK_UINT_EQ(my_ntohs(my_htons(v)), v);
    }
    for (size_t i = 0; i < sizeof vals32 / sizeof vals32[0]; i++) {
        uint32_t v = vals32[i];
        CHECK_UINT_EQ(my_htonl(v), htonl(v));
        CHECK_UINT_EQ(my_ntohl(v), ntohl(v));
        CHECK_UINT_EQ(my_ntohl(my_htonl(v)), v);
    }

    /* byte-level ground truth: network order is big-endian in memory */
    uint16_t n16 = my_htons(0x1234u);
    uint8_t b2[2];
    memcpy(b2, &n16, sizeof b2);
    CHECK_UINT_EQ(b2[0], 0x12u);
    CHECK_UINT_EQ(b2[1], 0x34u);

    uint32_t n32 = my_htonl(0x12345678u);
    uint8_t b4[4];
    memcpy(b4, &n32, sizeof b4);
    CHECK_UINT_EQ(b4[0], 0x12u);
    CHECK_UINT_EQ(b4[1], 0x34u);
    CHECK_UINT_EQ(b4[2], 0x56u);
    CHECK_UINT_EQ(b4[3], 0x78u);

    CTEST_END();
}
