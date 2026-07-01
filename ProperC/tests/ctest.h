#ifndef PROPERC_CTEST_H
#define PROPERC_CTEST_H

/*
 * ctest.h — a tiny zero-dependency test framework.
 *
 * Each test file is its own program with a main() that calls some CHECK_*
 * macros and ends with CTEST_END(). A failing check prints
 *   FAIL file:line: <what> (got X, want Y)
 * and the program exits nonzero, so `make test` halts on the first bad module.
 *
 * Because every test is a separate translation unit / binary, it's fine for
 * these counters to live in the header as statics.
 */

#include <stdio.h>
#include <string.h>

static int ctest_passed = 0;
static int ctest_failed = 0;

#define CHECK(cond)                                                            \
    do {                                                                       \
        if (cond) {                                                            \
            ctest_passed++;                                                    \
        } else {                                                               \
            ctest_failed++;                                                    \
            fprintf(stderr, "FAIL %s:%d: CHECK(%s)\n", __FILE__, __LINE__,     \
                    #cond);                                                    \
        }                                                                      \
    } while (0)

#define CHECK_TRUE(cond)  CHECK(cond)
#define CHECK_FALSE(cond) CHECK(!(cond))

#define CHECK_INT_EQ(got, want)                                                \
    do {                                                                       \
        long long _g = (long long)(got);                                       \
        long long _w = (long long)(want);                                      \
        if (_g == _w) {                                                        \
            ctest_passed++;                                                    \
        } else {                                                               \
            ctest_failed++;                                                    \
            fprintf(stderr,                                                    \
                    "FAIL %s:%d: %s == %s (got %lld, want %lld)\n", __FILE__,   \
                    __LINE__, #got, #want, _g, _w);                            \
        }                                                                      \
    } while (0)

#define CHECK_UINT_EQ(got, want)                                               \
    do {                                                                       \
        unsigned long long _g = (unsigned long long)(got);                     \
        unsigned long long _w = (unsigned long long)(want);                    \
        if (_g == _w) {                                                        \
            ctest_passed++;                                                    \
        } else {                                                               \
            ctest_failed++;                                                    \
            fprintf(stderr,                                                    \
                    "FAIL %s:%d: %s == %s (got %llu, want %llu)\n", __FILE__,   \
                    __LINE__, #got, #want, _g, _w);                            \
        }                                                                      \
    } while (0)

#define CHECK_STR_EQ(got, want)                                                \
    do {                                                                       \
        const char *_g = (got);                                                \
        const char *_w = (want);                                               \
        if (_g && _w && strcmp(_g, _w) == 0) {                                 \
            ctest_passed++;                                                    \
        } else {                                                               \
            ctest_failed++;                                                    \
            fprintf(stderr,                                                    \
                    "FAIL %s:%d: %s == %s (got \"%s\", want \"%s\")\n",         \
                    __FILE__, __LINE__, #got, #want, _g ? _g : "(null)",       \
                    _w ? _w : "(null)");                                       \
        }                                                                      \
    } while (0)

#define CHECK_PTR_NULL(p)    CHECK((p) == NULL)
#define CHECK_PTR_NONNULL(p) CHECK((p) != NULL)

#define CTEST_END()                                                            \
    do {                                                                       \
        fprintf(stderr, "%s: %d passed, %d failed\n", __FILE__, ctest_passed,  \
                ctest_failed);                                                 \
        return ctest_failed ? 1 : 0;                                           \
    } while (0)

#endif /* PROPERC_CTEST_H */
