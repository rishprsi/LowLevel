#ifndef PROPEROS_PIPES_H
#define PROPEROS_PIPES_H

#include <stddef.h>

/*
 * APUE ch.15 — IPC with pipes: build `cmd1 | cmd2` by hand.
 */

/*
 * Trim leading and trailing ASCII whitespace from s IN PLACE and return s.
 * (Move the remaining bytes to the front with memmove; keep NUL-termination.)
 * Tests use this to compare command output like "       2\n" against "2".
 */
char *trim_ws(char *s);

/*
 * Run `argv1 | argv2`: two children connected by a pipe(2). Each argv is
 * NULL-terminated and argv[0] is the ABSOLUTE PATH of the program (pass it
 * to execv directly; no PATH search). Child 1's stdout
 * is the pipe's write end; child 2's stdin is the read end. Capture child 2's
 * stdout into `out` (at most outsz-1 bytes, NUL-terminated) via a second
 * pipe. Close every unused pipe end in every process — a forgotten write end
 * means cmd2 never sees EOF. Reap BOTH children with waitpid.
 *
 * Returns 0 if both children exited normally with status 0, else -1.
 */
int pipeline2(char *const argv1[], char *const argv2[], char *out,
              size_t outsz);

/*
 * Stretch goal: `argv1 | argv2 | argv3`, same contract as pipeline2 (capture
 * the LAST command's stdout, reap all three children).
 */
int pipeline3(char *const argv1[], char *const argv2[], char *const argv3[],
              char *out, size_t outsz);

#endif /* PROPEROS_PIPES_H */
