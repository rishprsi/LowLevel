#ifndef PROPERC_CTEST_H
#define PROPERC_CTEST_H

/*
 * ctest.h — a tiny zero-dependency test framework.
 *
 * Each test file is its own program with a main() that calls some CHECK_*
 * macros and ends with CTEST_END().
 *
 * Use SECTION("name") to label the function/topic the following checks
 * exercise. A failing check prints a labeled block:
 *
 *   FAIL file:line [section #N]
 *     input:    <the expression under test>
 *     expected: <what it should have been>
 *     actual:   <what it actually was>
 *     context:  <optional; only from the *_MSG variants>
 *
 * where `section` is the current SECTION label and `#N` is the index of the
 * check within that section — so a failure tells you exactly which function,
 * which of its checks broke, the input, and the expected vs actual value.
 * CTEST_END() also lists every section that had a failure. The program exits
 * nonzero, so `make test` halts on the first bad module.
 *
 * The *_MSG variants take a trailing printf-style format + args and add a
 * `context:` line. Use them in randomized / differential loops where the
 * failing input (seed, index, op sequence) is not visible in the static
 * expression, e.g.
 *
 *   CHECK_INT_EQ_MSG(vec_get(&v, i), shadow[i], "op=%d i=%zu seed=%u",
 *                    op, i, seed);
 *
 * Because every test is a separate translation unit / binary, it's fine for
 * these counters to live in the header as statics.
 */

#include <stdio.h>
#include <string.h>

static int ctest_passed = 0;
static int ctest_failed = 0;

/* Function/topic currently under test, and the check index within it. */
static const char *ctest_section = "(top level)";
static int ctest_section_idx = 0;

/* Distinct sections that had at least one failure (for the summary line). */
static const char *ctest_failed_sections[128];
static int ctest_failed_section_count = 0;

/*
 * SECTION("fn") — label the function/topic the following checks exercise.
 * The label plus a per-section check number appears on every failure line,
 * and every failing label is listed in the end-of-run summary.
 */
#define SECTION(name)                                                          \
    do {                                                                       \
        ctest_section = (name);                                                \
        ctest_section_idx = 0;                                                 \
    } while (0)

/* Record a failure and remember its section (deduplicated) for the summary.
 * Marked unused so a test that skips (calls CTEST_END with no CHECK, e.g. a
 * Linux-only module on macOS) still compiles clean under -Werror. */
__attribute__((unused)) static void ctest_note_fail(void) {
    ctest_failed++;
    for (int _i = 0; _i < ctest_failed_section_count; _i++) {
        if (ctest_failed_sections[_i] == ctest_section) {
            return;
        }
    }
    if (ctest_failed_section_count <
        (int)(sizeof(ctest_failed_sections) / sizeof(ctest_failed_sections[0]))) {
        ctest_failed_sections[ctest_failed_section_count++] = ctest_section;
    }
}

/* Print the "FAIL file:line [section #N]" header for the current check. */
#define CTEST__HDR()                                                           \
    fprintf(stderr, "FAIL %s:%d [%s #%d]\n", __FILE__, __LINE__,               \
            ctest_section, ctest_section_idx)

/* Optional trailing "context:" line for the *_MSG variants. */
#define CTEST__CTX(...)                                                        \
    do {                                                                       \
        fprintf(stderr, "  context:  ");                                       \
        fprintf(stderr, __VA_ARGS__);                                          \
        fprintf(stderr, "\n");                                                 \
    } while (0)

/* ---- boolean checks ---------------------------------------------------- */

#define CTEST__BOOL(cond, instr, exp, act, CTX)                                \
    do {                                                                       \
        ctest_section_idx++;                                                   \
        if (cond) {                                                            \
            ctest_passed++;                                                    \
        } else {                                                               \
            ctest_note_fail();                                                 \
            CTEST__HDR();                                                      \
            fprintf(stderr,                                                    \
                    "  input:    %s\n  expected: %s\n  actual:   %s\n",         \
                    instr, exp, act);                                          \
            CTX;                                                               \
        }                                                                      \
    } while (0)

#define CHECK(cond)        CTEST__BOOL((cond), #cond, "true", "false", (void)0)
#define CHECK_TRUE(cond)   CTEST__BOOL((cond), #cond, "true", "false", (void)0)
#define CHECK_FALSE(cond)  CTEST__BOOL(!(cond), #cond, "false", "true", (void)0)

#define CHECK_MSG(cond, ...)                                                   \
    CTEST__BOOL((cond), #cond, "true", "false", CTEST__CTX(__VA_ARGS__))
#define CHECK_TRUE_MSG(cond, ...)                                              \
    CTEST__BOOL((cond), #cond, "true", "false", CTEST__CTX(__VA_ARGS__))
#define CHECK_FALSE_MSG(cond, ...)                                             \
    CTEST__BOOL(!(cond), #cond, "false", "true", CTEST__CTX(__VA_ARGS__))

/* ---- integer equality -------------------------------------------------- */

#define CTEST__INT_EQ(got, want, CTX)                                          \
    do {                                                                       \
        ctest_section_idx++;                                                   \
        long long _g = (long long)(got);                                       \
        long long _w = (long long)(want);                                      \
        if (_g == _w) {                                                        \
            ctest_passed++;                                                    \
        } else {                                                               \
            ctest_note_fail();                                                 \
            CTEST__HDR();                                                      \
            fprintf(stderr,                                                    \
                    "  input:    %s\n  expected: %lld\n  actual:   %lld\n",     \
                    #got, _w, _g);                                             \
            CTX;                                                               \
        }                                                                      \
    } while (0)

#define CHECK_INT_EQ(got, want)          CTEST__INT_EQ(got, want, (void)0)
#define CHECK_INT_EQ_MSG(got, want, ...)                                       \
    CTEST__INT_EQ(got, want, CTEST__CTX(__VA_ARGS__))

/* ---- unsigned integer equality ----------------------------------------- */

#define CTEST__UINT_EQ(got, want, CTX)                                         \
    do {                                                                       \
        ctest_section_idx++;                                                   \
        unsigned long long _g = (unsigned long long)(got);                     \
        unsigned long long _w = (unsigned long long)(want);                    \
        if (_g == _w) {                                                        \
            ctest_passed++;                                                    \
        } else {                                                               \
            ctest_note_fail();                                                 \
            CTEST__HDR();                                                      \
            fprintf(stderr,                                                    \
                    "  input:    %s\n  expected: %llu\n  actual:   %llu\n",     \
                    #got, _w, _g);                                             \
            CTX;                                                               \
        }                                                                      \
    } while (0)

#define CHECK_UINT_EQ(got, want)          CTEST__UINT_EQ(got, want, (void)0)
#define CHECK_UINT_EQ_MSG(got, want, ...)                                      \
    CTEST__UINT_EQ(got, want, CTEST__CTX(__VA_ARGS__))

/* ---- string equality --------------------------------------------------- */

#define CTEST__STR_EQ(got, want, CTX)                                          \
    do {                                                                       \
        ctest_section_idx++;                                                   \
        const char *_g = (got);                                                \
        const char *_w = (want);                                               \
        if (_g && _w && strcmp(_g, _w) == 0) {                                 \
            ctest_passed++;                                                    \
        } else {                                                               \
            ctest_note_fail();                                                 \
            CTEST__HDR();                                                      \
            fprintf(stderr,                                                    \
                    "  input:    %s\n  expected: \"%s\"\n  actual:   \"%s\"\n", \
                    #got, _w ? _w : "(null)", _g ? _g : "(null)");             \
            CTX;                                                               \
        }                                                                      \
    } while (0)

#define CHECK_STR_EQ(got, want)          CTEST__STR_EQ(got, want, (void)0)
#define CHECK_STR_EQ_MSG(got, want, ...)                                       \
    CTEST__STR_EQ(got, want, CTEST__CTX(__VA_ARGS__))

/* ---- pointer null-ness ------------------------------------------------- */

#define CTEST__PTR(p, wantnull, CTX)                                           \
    do {                                                                       \
        ctest_section_idx++;                                                   \
        void *_p = (void *)(p);                                                \
        if ((wantnull) ? (_p == NULL) : (_p != NULL)) {                        \
            ctest_passed++;                                                    \
        } else {                                                               \
            ctest_note_fail();                                                 \
            CTEST__HDR();                                                      \
            fprintf(stderr,                                                    \
                    "  input:    %s\n  expected: %s\n  actual:   %p\n",         \
                    #p, (wantnull) ? "NULL" : "non-NULL", _p);                 \
            CTX;                                                               \
        }                                                                      \
    } while (0)

#define CHECK_PTR_NULL(p)             CTEST__PTR(p, 1, (void)0)
#define CHECK_PTR_NONNULL(p)          CTEST__PTR(p, 0, (void)0)
#define CHECK_PTR_NULL_MSG(p, ...)    CTEST__PTR(p, 1, CTEST__CTX(__VA_ARGS__))
#define CHECK_PTR_NONNULL_MSG(p, ...) CTEST__PTR(p, 0, CTEST__CTX(__VA_ARGS__))

/* ---- end of run -------------------------------------------------------- */

#define CTEST_END()                                                            \
    do {                                                                       \
        if (ctest_failed_section_count > 0) {                                  \
            fprintf(stderr, "%s: %d passed, %d failed (in:", __FILE__,         \
                    ctest_passed, ctest_failed);                               \
            for (int _i = 0; _i < ctest_failed_section_count; _i++) {          \
                fprintf(stderr, " %s", ctest_failed_sections[_i]);             \
            }                                                                  \
            fprintf(stderr, ")\n");                                            \
        } else {                                                               \
            fprintf(stderr, "%s: %d passed, %d failed\n", __FILE__,            \
                    ctest_passed, ctest_failed);                               \
        }                                                                      \
        return ctest_failed ? 1 : 0;                                           \
    } while (0)

#endif /* PROPERC_CTEST_H */
