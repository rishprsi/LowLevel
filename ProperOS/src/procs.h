#ifndef PROPEROS_PROCS_H
#define PROPEROS_PROCS_H

#include <stddef.h>

/*
 * APUE ch.8 — process control: fork, exec, wait.
 */

/*
 * Run the program at `path` with argument vector `argv` (NULL-terminated,
 * argv[0] = program name) as a child process: fork(2), execv(2) in the
 * child, waitpid(2) in the parent.
 *
 * Returns the child's exit status (0..255) if it exited normally — decode
 * the waitpid status with WIFEXITED/WEXITSTATUS — or -1 on fork/wait failure
 * or if the child was killed by a signal. If execv fails in the child, the
 * child must _exit(127) (do NOT return into the caller's stack twice).
 */
int run_command(const char *path, char *const argv[]);

/*
 * Like run_command, but capture the child's stdout into `out` (at most
 * outsz-1 bytes; always NUL-terminated): create a pipe(2), redirect the
 * child's STDOUT_FILENO onto the write end with dup2(2), close unused ends,
 * read everything from the read end in the parent, then reap the child.
 *
 * Returns the number of bytes captured (excluding the NUL), or -1 on error
 * or if the child did not exit with status 0.
 */
int spawn_capture(const char *path, char *const argv[], char *out,
                  size_t outsz);

#endif /* PROPEROS_PROCS_H */
