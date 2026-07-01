#include "ctest.h"
#include "structs.h"

#include <string.h>

int main(void) {
    /* is_little_endian must agree with ground truth on this machine */
    {
        uint32_t v = 0x01020304u;
        uint8_t bytes[4];
        memcpy(bytes, &v, 4);
        bool truth = (bytes[0] == 0x04);
        CHECK(is_little_endian() == truth);
    }

    /* little-endian (de)serialization */
    {
        uint8_t out[4];
        u32_to_bytes_le(0x01020304u, out);
        CHECK_UINT_EQ(out[0], 0x04);
        CHECK_UINT_EQ(out[1], 0x03);
        CHECK_UINT_EQ(out[2], 0x02);
        CHECK_UINT_EQ(out[3], 0x01);
        CHECK_UINT_EQ(u32_from_bytes_le(out), 0x01020304u);
    }

    /* big-endian (de)serialization */
    {
        uint8_t out[4];
        u32_to_bytes_be(0x01020304u, out);
        CHECK_UINT_EQ(out[0], 0x01);
        CHECK_UINT_EQ(out[1], 0x02);
        CHECK_UINT_EQ(out[2], 0x03);
        CHECK_UINT_EQ(out[3], 0x04);
        CHECK_UINT_EQ(u32_from_bytes_be(out), 0x01020304u);
    }

    /* round trips for several values */
    {
        uint32_t vals[] = {0u, 1u, 0xFFFFFFFFu, 0xDEADBEEFu, 0x12345678u};
        for (size_t i = 0; i < sizeof(vals) / sizeof(vals[0]); i++) {
            uint8_t le[4], be[4];
            u32_to_bytes_le(vals[i], le);
            u32_to_bytes_be(vals[i], be);
            CHECK_UINT_EQ(u32_from_bytes_le(le), vals[i]);
            CHECK_UINT_EQ(u32_from_bytes_be(be), vals[i]);
        }
    }

    /* rgba pack/unpack */
    {
        CHECK_UINT_EQ(pack_rgba(0x11, 0x22, 0x33, 0x44), 0x11223344u);
        uint8_t r, g, b, a;
        unpack_rgba(0xAABBCCDDu, &r, &g, &b, &a);
        CHECK_UINT_EQ(r, 0xAA);
        CHECK_UINT_EQ(g, 0xBB);
        CHECK_UINT_EQ(b, 0xCC);
        CHECK_UINT_EQ(a, 0xDD);
        /* NULL outputs must be tolerated */
        unpack_rgba(0xAABBCCDDu, NULL, &g, NULL, &a);
        CHECK_UINT_EQ(g, 0xBB);
        CHECK_UINT_EQ(a, 0xDD);
    }

    CTEST_END();
}
