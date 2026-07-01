#ifndef PROPEROS_PROCFS_H
#define PROPEROS_PROCFS_H

#include <stddef.h>

/*
 * Linux-only — the /proc filesystem (TLPI ch.12 flavor).
 *
 * There is no /proc on macOS, so this module is RUNTIME-SKIPPED there:
 * on non-Linux both stub and solution compile as functions returning -1,
 * and the test prints "SKIP: procfs is Linux-only" and passes. On Linux the
 * stub is the usual TODO compile error and the tests are real.
 */

/*
 * Count the memory regions of THIS process by opening /proc/self/maps and
 * counting lines (each line = one mapping). Returns the count (> 0 for any
 * live process), or -1 on error / on non-Linux.
 */
int count_maps_regions(void);

/*
 * Look up `field` in /proc/self/status. Lines have the form
 *     Name:\tvalue...
 * Match `field` against the part before the ':' (exact match); copy the
 * value — with leading whitespace stripped and the trailing newline
 * removed — into out (at most outsz-1 bytes, NUL-terminated).
 * Returns 0 if the field was found, -1 if not found / error / non-Linux.
 */
int read_status_field(const char *field, char *out, size_t outsz);

#endif /* PROPEROS_PROCFS_H */
