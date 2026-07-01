#include "structs.h"
#include "todo.h"

bool is_little_endian(void) {
    TODO("implement is_little_endian");
}

void u32_to_bytes_le(uint32_t v, uint8_t out[4]) {
    (void)v;
    (void)out;
    TODO("implement u32_to_bytes_le");
}

uint32_t u32_from_bytes_le(const uint8_t in[4]) {
    (void)in;
    TODO("implement u32_from_bytes_le");
}

void u32_to_bytes_be(uint32_t v, uint8_t out[4]) {
    (void)v;
    (void)out;
    TODO("implement u32_to_bytes_be");
}

uint32_t u32_from_bytes_be(const uint8_t in[4]) {
    (void)in;
    TODO("implement u32_from_bytes_be");
}

uint32_t pack_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    (void)r;
    (void)g;
    (void)b;
    (void)a;
    TODO("implement pack_rgba");
}

void unpack_rgba(uint32_t rgba, uint8_t *r, uint8_t *g, uint8_t *b, uint8_t *a) {
    (void)rgba;
    (void)r;
    (void)g;
    (void)b;
    (void)a;
    TODO("implement unpack_rgba");
}
