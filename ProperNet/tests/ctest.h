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
 *     input:    <the expression under test>  (var=value ...)
 *     expected: <what it should have been>
 *     actual:   <what it actually was>
 *
 * where `section` is the current SECTION label and `#N` is the index of the
 * check within that section.
 *
 * ABORT-ON-FIRST-FAILURE PER SECTION: within a section, checks run until the
 * first failure; that failure is printed, then the remaining checks in the
 * SAME section are skipped (their expressions are still evaluated so any side
 * effects happen, but they are not recorded or printed) until the next
 * SECTION() re-arms. So every function in the file still runs, but each one
 * stops at its first failure — a fuzz loop that diverges shows ONE block, not
 * thousands. CTEST_END() lists every section that failed and the program exits
 * nonzero, so `make test` halts on the first bad module.
 *
 * The *_MSG variants take a trailing printf-style format + args and append the
 * concrete variable values to the `input:` line in parentheses. Use them where
 * the input is a variable (e.g. a fuzz-loop index) rather than a literal:
 *
 *   CHECK_INT_EQ_MSG(vec_get(&v, i), shadow[i], "op=%d i=%zu", op, i);
 *
 * prints:  input:    vec_get(&v, i)  (op=322 i=7)
 *
 * Because every test is a separate translation unit / binary, it's fine for
 * these counters to live in the header as statics.
 */

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/*
 * Watchdog: if any single check (including the function call it makes) or the
 * code between checks runs longer than CTEST_TIMEOUT_SECS, a SIGALRM fires and
 * we print which SECTION was running, then exit 124. This turns an infinite
 * loop (in your implementation or a test loop) from a silent hang into a
 * message naming the culprit function. Override at compile time with
 * -DCTEST_TIMEOUT_SECS=30 for a legitimately slow module.
 */
#ifndef CTEST_TIMEOUT_SECS
#define CTEST_TIMEOUT_SECS 10
#endif

static int ctest_passed = 0;
static int ctest_failed = 0;

/* Function/topic currently under test, and the check index within it. */
static const char *ctest_section = "(top level)";
static int ctest_section_idx = 0;

/* Set once the current section has failed; suppresses the rest of its checks
 * until the next SECTION() clears it. */
static int ctest_section_aborted = 0;

/* Distinct sections that had a failure (for the summary line). */
static const char *ctest_failed_sections[128];
static int ctest_failed_section_count = 0;

/*
 * SECTION("fn") — label the function/topic the following checks exercise, and
 * re-arm checking (clears the abort flag so the next failure is shown).
 */
#define SECTION(name)                                                          \
    do {                                                                       \
        ctest_section = (name);                                                \
        ctest_section_idx = 0;                                                 \
        ctest_section_aborted = 0;                                             \
        ctest_arm();                                                           \
    } while (0)

/* Record a failure, remember its section (deduplicated) for the summary, and
 * arm the abort flag so the rest of this section is skipped.
 * Marked unused so a test that skips (calls CTEST_END with no CHECK, e.g. a
 * Linux-only module on macOS) still compiles clean under -Werror. */
__attribute__((unused)) static void ctest_note_fail(void) {
    ctest_failed++;
    ctest_section_aborted = 1;
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

/* async-signal-safe writes for the watchdog handler (no fprintf in a signal
 * handler). */
static void ctest__wstr(const char *s) {
    if (s) {
        ssize_t r = write(2, s, strlen(s));
        (void)r;
    }
}
static void ctest__wint(int v) {
    char buf[24];
    int i = (int)sizeof(buf);
    unsigned u;
    if (v < 0) {
        ctest__wstr("-");
        u = (unsigned)(-(long)v);
    } else {
        u = (unsigned)v;
    }
    if (u == 0) {
        ctest__wstr("0");
        return;
    }
    while (u && i > 0) {
        buf[--i] = (char)('0' + (u % 10));
        u /= 10;
    }
    ssize_t r = write(2, buf + i, (size_t)((int)sizeof(buf) - i));
    (void)r;
}

/* SIGALRM handler: name the section that was running, then bail out. */
__attribute__((unused)) static void ctest_on_timeout(int sig) {
    (void)sig;
    ctest__wstr("\n*** TIMEOUT after ");
    ctest__wint(CTEST_TIMEOUT_SECS);
    ctest__wstr("s in section: ");
    ctest__wstr(ctest_section);
    ctest__wstr(" (reached check #");
    ctest__wint(ctest_section_idx + 1);
    ctest__wstr(")\n    Likely an infinite loop or a stuck call in this "
                "function.\n");
    _exit(124);
}

/* (Re)arm the watchdog. Installs the handler once, then restarts the timer so
 * each check/section gets a fresh CTEST_TIMEOUT_SECS budget. */
__attribute__((unused)) static void ctest_arm(void) {
    static int installed = 0;
    if (!installed) {
        signal(SIGALRM, ctest_on_timeout);
        installed = 1;
    }
    alarm(CTEST_TIMEOUT_SECS);
}

/* Print the "FAIL file:line [section #N]" header for the current check. */
#define CTEST__HDR()                                                           \
    fprintf(stderr, "FAIL %s:%d [%s #%d]\n", __FILE__, __LINE__,               \
            ctest_section, ctest_section_idx)

/* Print the start of the input line (no trailing newline yet). */
#define CTEST__IN(instr) fprintf(stderr, "  input:    %s", (instr))

/* Optional concrete values appended to the input line, from the *_MSG variants.
 * The base checks pass CTEST__NOCTX, which appends nothing. */
#define CTEST__NOCTX ((void)0)
#define CTEST__CTX(...)                                                        \
    do {                                                                       \
        fprintf(stderr, "  (");                                                \
        fprintf(stderr, __VA_ARGS__);                                          \
        fprintf(stderr, ")");                                                  \
    } while (0)

/* ---- boolean checks ---------------------------------------------------- */

#define CTEST__BOOL(cond, instr, exp, act, CTX)                                \
    do {                                                                       \
        ctest_arm();                                                           \
        int _c = (cond) ? 1 : 0;                                               \
        if (ctest_section_aborted) break;                                      \
        ctest_section_idx++;                                                   \
        if (_c) {                                                              \
            ctest_passed++;                                                    \
        } else {                                                               \
            ctest_note_fail();                                                 \
            CTEST__HDR();                                                      \
            CTEST__IN(instr);                                                  \
            CTX;                                                               \
            fprintf(stderr, "\n  expected: %s\n  actual:   %s\n", exp, act);   \
        }                                                                      \
    } while (0)

#define CHECK(cond)       CTEST__BOOL((cond), #cond, "true", "false", CTEST__NOCTX)
#define CHECK_TRUE(cond)  CTEST__BOOL((cond), #cond, "true", "false", CTEST__NOCTX)
#define CHECK_FALSE(cond) CTEST__BOOL(!(cond), #cond, "false", "true", CTEST__NOCTX)

#define CHECK_MSG(cond, ...)                                                   \
    CTEST__BOOL((cond), #cond, "true", "false", CTEST__CTX(__VA_ARGS__))
#define CHECK_TRUE_MSG(cond, ...)                                              \
    CTEST__BOOL((cond), #cond, "true", "false", CTEST__CTX(__VA_ARGS__))
#define CHECK_FALSE_MSG(cond, ...)                                             \
    CTEST__BOOL(!(cond), #cond, "false", "true", CTEST__CTX(__VA_ARGS__))

/* ---- integer equality -------------------------------------------------- */

#define CTEST__INT_EQ(got, want, CTX)                                          \
    do {                                                                       \
        ctest_arm();                                                           \
        long long _g = (long long)(got);                                       \
        long long _w = (long long)(want);                                      \
        if (ctest_section_aborted) break;                                      \
        ctest_section_idx++;                                                   \
        if (_g == _w) {                                                        \
            ctest_passed++;                                                    \
        } else {                                                               \
            ctest_note_fail();                                                 \
            CTEST__HDR();                                                      \
            CTEST__IN(#got);                                                   \
            CTX;                                                               \
            fprintf(stderr, "\n  expected: %lld\n  actual:   %lld\n", _w, _g); \
        }                                                                      \
    } while (0)

#define CHECK_INT_EQ(got, want)          CTEST__INT_EQ(got, want, CTEST__NOCTX)
#define CHECK_INT_EQ_MSG(got, want, ...)                                       \
    CTEST__INT_EQ(got, want, CTEST__CTX(__VA_ARGS__))

/* ---- unsigned integer equality ----------------------------------------- */

#define CTEST__UINT_EQ(got, want, CTX)                                         \
    do {                                                                       \
        ctest_arm();                                                           \
        unsigned long long _g = (unsigned long long)(got);                     \
        unsigned long long _w = (unsigned long long)(want);                    \
        if (ctest_section_aborted) break;                                      \
        ctest_section_idx++;                                                   \
        if (_g == _w) {                                                        \
            ctest_passed++;                                                    \
        } else {                                                               \
            ctest_note_fail();                                                 \
            CTEST__HDR();                                                      \
            CTEST__IN(#got);                                                   \
            CTX;                                                               \
            fprintf(stderr, "\n  expected: %llu\n  actual:   %llu\n", _w, _g); \
        }                                                                      \
    } while (0)

#define CHECK_UINT_EQ(got, want)          CTEST__UINT_EQ(got, want, CTEST__NOCTX)
#define CHECK_UINT_EQ_MSG(got, want, ...)                                      \
    CTEST__UINT_EQ(got, want, CTEST__CTX(__VA_ARGS__))

/* ---- string equality --------------------------------------------------- */

#define CTEST__STR_EQ(got, want, CTX)                                          \
    do {                                                                       \
        ctest_arm();                                                           \
        const char *_g = (got);                                                \
        const char *_w = (want);                                               \
        if (ctest_section_aborted) break;                                      \
        ctest_section_idx++;                                                   \
        if (_g && _w && strcmp(_g, _w) == 0) {                                 \
            ctest_passed++;                                                    \
        } else {                                                               \
            ctest_note_fail();                                                 \
            CTEST__HDR();                                                      \
            CTEST__IN(#got);                                                   \
            CTX;                                                               \
            fprintf(stderr, "\n  expected: \"%s\"\n  actual:   \"%s\"\n",      \
                    _w ? _w : "(null)", _g ? _g : "(null)");                   \
        }                                                                      \
    } while (0)

#define CHECK_STR_EQ(got, want)          CTEST__STR_EQ(got, want, CTEST__NOCTX)
#define CHECK_STR_EQ_MSG(got, want, ...)                                       \
    CTEST__STR_EQ(got, want, CTEST__CTX(__VA_ARGS__))

/* ---- pointer null-ness ------------------------------------------------- */

#define CTEST__PTR(p, wantnull, CTX)                                           \
    do {                                                                       \
        ctest_arm();                                                           \
        void *_p = (void *)(p);                                                \
        if (ctest_section_aborted) break;                                      \
        ctest_section_idx++;                                                   \
        if ((wantnull) ? (_p == NULL) : (_p != NULL)) {                        \
            ctest_passed++;                                                    \
        } else {                                                               \
            ctest_note_fail();                                                 \
            CTEST__HDR();                                                      \
            CTEST__IN(#p);                                                     \
            CTX;                                                               \
            fprintf(stderr, "\n  expected: %s\n  actual:   %p\n",              \
                    (wantnull) ? "NULL" : "non-NULL", _p);                     \
        }                                                                      \
    } while (0)

#define CHECK_PTR_NULL(p)             CTEST__PTR(p, 1, CTEST__NOCTX)
#define CHECK_PTR_NONNULL(p)          CTEST__PTR(p, 0, CTEST__NOCTX)
#define CHECK_PTR_NULL_MSG(p, ...)    CTEST__PTR(p, 1, CTEST__CTX(__VA_ARGS__))
#define CHECK_PTR_NONNULL_MSG(p, ...) CTEST__PTR(p, 0, CTEST__CTX(__VA_ARGS__))

/* ---- end of run -------------------------------------------------------- */

#define CTEST_END()                                                            \
    do {                                                                       \
        alarm(0); /* cancel the watchdog before the summary print */           \
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
